/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "collectionsync_p.h"
#include "collection.h"

#include "collectioncreatejob.h"
#include "collectiondeletejob.h"
#include "collectionfetchjob.h"
#include "collectionmodifyjob.h"


#include <kdebug.h>

using namespace Akonadi;

/**
 * @internal
 */
class CollectionSync::Private
{
  public:
    Private() :
      pendingJobs( 0 ),
      incremental( false )
    {
    }

    QString resourceId;

    // local: mapped remote id -> collection, id -> collection
    QHash<QString,Collection> localCollections;
    QSet<Collection> unprocessedLocalCollections;

    // remote: mapped id -> collection
    QHash<Collection::Id, Collection> remoteCollections;

    // remote collections waiting for a parent
    QList<Collection> orphanRemoteCollections;

    // removed remote collections
    Collection::List removedRemoteCollections;

    // create counter
    int pendingJobs;

    bool incremental;
};

CollectionSync::CollectionSync( const QString &resourceId, QObject *parent ) :
    TransactionSequence( parent ),
    d( new Private )
{
  d->resourceId = resourceId;
}

CollectionSync::~CollectionSync()
{
  delete d;
}

void CollectionSync::setRemoteCollections(const Collection::List & remoteCollections)
{
  foreach ( const Collection &c, remoteCollections ) {
    d->remoteCollections.insert( c.id(), c );
  }
}

void CollectionSync::setRemoteCollections(const Collection::List & changedCollections, const Collection::List & removedCollections)
{
  d->incremental = true;
  foreach ( const Collection &c, changedCollections ) {
    d->remoteCollections.insert( c.id(), c );
  }
  d->removedRemoteCollections = removedCollections;
}

void CollectionSync::doStart()
{
  CollectionFetchJob *job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, this );
  job->setResource( d->resourceId );
  connect( job, SIGNAL(result(KJob*)), SLOT(slotLocalListDone(KJob*)) );
}

void CollectionSync::slotLocalListDone(KJob * job)
{
  if ( job->error() )
    return;

  Collection::List list = static_cast<CollectionFetchJob*>( job )->collections();
  foreach ( const Collection &c, list ) {
    d->localCollections.insert( c.remoteId(), c );
    d->unprocessedLocalCollections.insert( c );
  }


  // added / updated
  foreach ( const Collection &c, d->remoteCollections ) {
    if ( c.remoteId().isEmpty() ) {
      kWarning( 5250 ) << "Collection '" << c.name() <<"' does not have a remote identifier - skipping";
      continue;
    }

    Collection local = d->localCollections.value( c.remoteId() );
    d->unprocessedLocalCollections.remove( local );
    // missing locally
    if ( !local.isValid() ) {
      // determine local parent
      Collection localParent;
      if ( c.parent() >= 0 )
        localParent = Collection( c.parent() );
      if ( c.parentRemoteId().isEmpty() )
        localParent = Collection::root();
      else
        localParent = d->localCollections.value( c.parentRemoteId() );

      // no local parent found, create later
      if ( !localParent.isValid() ) {
        d->orphanRemoteCollections << c;
        continue;
      }

      createLocalCollection( c, localParent );
      continue;
    }

    // update local collection
    d->pendingJobs++;
    Collection upd( c );
    upd.setId( local.id() );
    CollectionModifyJob *mod = new CollectionModifyJob( upd, this );
    connect( mod, SIGNAL(result(KJob*)), SLOT(slotLocalChangeDone(KJob*)) );
  }

  // removed
  if ( !d->incremental )
    d->removedRemoteCollections = d->unprocessedLocalCollections.toList();
  foreach ( const Collection &c, d->removedRemoteCollections ) {
    d->pendingJobs++;
    CollectionDeleteJob *job = new CollectionDeleteJob( c, this );
    connect( job, SIGNAL(result(KJob*)), SLOT(slotLocalChangeDone(KJob*)) );
  }
  d->localCollections.clear();

  checkDone();
}

void CollectionSync::slotLocalCreateDone(KJob * job)
{
  d->pendingJobs--;
  if ( job->error() )
    return;

  Collection newLocal = static_cast<CollectionCreateJob*>( job )->collection();
//   d->localCollections.insert( newLocal.remoteId(), newLocal );

  // search for children we can create now
  Collection::List stillOrphans;
  foreach ( const Collection &orphan, d->orphanRemoteCollections ) {
    if ( orphan.parentRemoteId() == newLocal.remoteId() ) {
      createLocalCollection( orphan, newLocal );
    } else {
      stillOrphans << orphan;
    }
  }
  d->orphanRemoteCollections = stillOrphans;

  checkDone();
}

void CollectionSync::createLocalCollection(const Collection & c, const Collection & parent)
{
  d->pendingJobs++;
  Collection col( c );
  col.setParent( parent );
  CollectionCreateJob *create = new CollectionCreateJob( col, this );
  connect( create, SIGNAL(result(KJob*)), SLOT(slotLocalCreateDone(KJob*)) );
}

void CollectionSync::checkDone()
{
  // still running jobs
  if ( d->pendingJobs > 0 )
    return;

  // still orphan collections
  if ( !d->orphanRemoteCollections.isEmpty() ) {
    setError( Unknown );
    setErrorText( QLatin1String( "Found unresolved orphan collections" ) );
    foreach ( const Collection &col, d->orphanRemoteCollections )
      kDebug() << "found orphan collection:" << col.remoteId() << "parent:" << col.parentRemoteId();
  }

  commit();
}

void CollectionSync::slotLocalChangeDone(KJob * job)
{
  if ( job->error() )
    return;
  d->pendingJobs--;
  checkDone();
}

#include "collectionsync_p.moc"
