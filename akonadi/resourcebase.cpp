/*
    Copyright (c) 2006 Till Adam <adam@kde.org>
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

#include "resourcebase.h"
#include "agentbase_p.h"

#include "resourceadaptor.h"
#include "collectionsync.h"
#include "itemsync.h"
#include "resourcescheduler.h"
#include "tracerinterface.h"
#include "xdgbasedirs_p.h"

#include "akonadi/changerecorder.h"
#include "akonadi/collectionfetchjob.h"
#include "akonadi/collectionmodifyjob.h"
#include "akonadi/itemfetchjob.h"
#include "akonadi/itemfetchscope.h"
#include "akonadi/itemmodifyjob.h"
#include "akonadi/session.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtDBus/QtDBus>

using namespace Akonadi;

class Akonadi::ResourceBasePrivate : public AgentBasePrivate
{
  public:
    ResourceBasePrivate( ResourceBase *parent )
      : AgentBasePrivate( parent ),
        scheduler( 0 )
    {
      mStatusMessage = defaultReadyMessage();
    }

    Q_DECLARE_PUBLIC( ResourceBase )

    void delayedInit()
    {
      if ( !QDBusConnection::sessionBus().registerService( QLatin1String( "org.kde.Akonadi.Resource." ) + mId ) )
        kFatal() << "Unable to register service at D-Bus: " << QDBusConnection::sessionBus().lastError().message();
      AgentBasePrivate::delayedInit();
    }

    virtual void changeProcessed()
    {
      mMonitor->changeProcessed();
      if ( !mMonitor->isEmpty() )
        scheduler->scheduleChangeReplay();
      scheduler->taskDone();
    }

    void slotDeliveryDone( KJob* job );

    void slotCollectionSyncDone( KJob *job );
    void slotLocalListDone( KJob *job );
    void slotSynchronizeCollection( const Collection &col, const QStringList &parts );
    void slotCollectionListDone( KJob *job );

    void slotItemSyncDone( KJob *job );

    void slotPercent( KJob* job, unsigned long percent );

    QString mName;

    // synchronize states
    Collection currentCollection;

    ResourceScheduler *scheduler;
};

ResourceBase::ResourceBase( const QString & id )
  : AgentBase( new ResourceBasePrivate( this ), id )
{
  Q_D( ResourceBase );

  new ResourceAdaptor( this );

  const QString name = d->mSettings->value( QLatin1String( "Resource/Name" ) ).toString();
  if ( !name.isEmpty() )
    d->mName = name;

  d->scheduler = new ResourceScheduler( this );

  d->mMonitor->setChangeRecordingEnabled( true );
  connect( d->mMonitor, SIGNAL(changesAdded()),
           d->scheduler, SLOT(scheduleChangeReplay()) );

  d->mMonitor->setResourceMonitored( d->mId.toLatin1() );

  connect( d->scheduler, SIGNAL(executeFullSync()),
           SLOT(retrieveCollections()) );
  connect( d->scheduler, SIGNAL(executeCollectionTreeSync()),
           SLOT(retrieveCollections()) );
  connect( d->scheduler, SIGNAL(executeCollectionSync(Akonadi::Collection,QStringList)),
           SLOT(slotSynchronizeCollection(Akonadi::Collection,QStringList)) );
  connect( d->scheduler, SIGNAL(executeItemFetch(Akonadi::Item,QStringList)),
           SLOT(retrieveItem(Akonadi::Item,QStringList)) );
  connect( d->scheduler, SIGNAL(executeChangeReplay()),
           d->mMonitor, SLOT(replayNext()) );

  d->scheduler->setOnline( d->mOnline );
  if ( !d->mMonitor->isEmpty() )
    d->scheduler->scheduleChangeReplay();
}

ResourceBase::~ResourceBase()
{
}

void ResourceBase::synchronize()
{
  d_func()->scheduler->scheduleFullSync();
}

void ResourceBase::setName( const QString &name )
{
  Q_D( ResourceBase );
  if ( name == d->mName )
    return;

  // TODO: rename collection
  d->mName = name;

  if ( d->mName.isEmpty() || d->mName == d->mId )
    d->mSettings->remove( QLatin1String( "Resource/Name" ) );
  else
    d->mSettings->setValue( QLatin1String( "Resource/Name" ), d->mName );

  d->mSettings->sync();

  emit nameChanged( d->mName );
}

QString ResourceBase::name() const
{
  Q_D( const ResourceBase );
  if ( d->mName.isEmpty() )
    return d->mId;
  else
    return d->mName;
}

static char* sAppName = 0;

QString ResourceBase::parseArguments( int argc, char **argv )
{
  QString identifier;
  if ( argc < 3 ) {
    kDebug( 5250 ) << "Not enough arguments passed...";
    exit( 1 );
  }

  for ( int i = 1; i < argc - 1; ++i ) {
    if ( QLatin1String( argv[ i ] ) == QLatin1String( "--identifier" ) )
      identifier = QLatin1String( argv[ i + 1 ] );
  }

  if ( identifier.isEmpty() ) {
    kDebug( 5250 ) << "Identifier argument missing";
    exit( 1 );
  }

  sAppName = qstrdup( identifier.toLatin1().constData() );
  KCmdLineArgs::init( argc, argv, sAppName, 0,
                      ki18nc("@title, application name", "Akonadi Resource"), "0.1",
                      ki18nc("@title, application description", "Akonadi Resource") );

  KCmdLineOptions options;
  options.add("identifier <argument>",
              ki18nc("@label, commandline option", "Resource identifier"));
  KCmdLineArgs::addCmdLineOptions( options );

  return identifier;
}

int ResourceBase::init( ResourceBase *r )
{
  QApplication::setQuitOnLastWindowClosed( false );
  int rv = kapp->exec();
  delete r;
  delete[] sAppName;
  return rv;
}

void ResourceBase::itemRetrieved( const Item &item )
{
  Q_D( ResourceBase );
  Q_ASSERT( d->scheduler->currentTask().type == ResourceScheduler::FetchItem );
  if ( !item.isValid() ) {
    QDBusMessage reply( d->scheduler->currentTask().dbusMsg );
    reply << false;
    QDBusConnection::sessionBus().send( reply );
    d->scheduler->taskDone();
    return;
  }

  Item i( item );
  QStringList requestedParts = d->scheduler->currentTask().itemParts;
  foreach ( QString part, requestedParts ) {
    if ( !item.loadedPayloadParts().contains( part ) ) {
      kWarning( 5250 ) << "Item does not provide part" << part;
    }
  }

  ItemModifyJob *job = new ItemModifyJob( i, session() );
  job->storePayload();
  // FIXME: remove once the item with which we call retrieveItem() has a revision number
  job->disableRevisionCheck();
  connect( job, SIGNAL(result(KJob*)), SLOT(slotDeliveryDone(KJob*)) );
}

void ResourceBasePrivate::slotDeliveryDone(KJob * job)
{
  Q_Q( ResourceBase );
  Q_ASSERT( scheduler->currentTask().type == ResourceScheduler::FetchItem );
  QDBusMessage reply( scheduler->currentTask().dbusMsg );
  if ( job->error() ) {
    q->error( QLatin1String( "Error while creating item: " ) + job->errorString() );
    reply << false;
  } else {
    reply << true;
  }
  QDBusConnection::sessionBus().send( reply );
  scheduler->taskDone();
}

void ResourceBase::changesCommitted(const Item& item)
{
  Q_D( ResourceBase );
  ItemModifyJob *job = new ItemModifyJob( item, session() );
  job->setClean();
  job->disableRevisionCheck(); // TODO: remove, but where/how do we handle the error?
  d->changeProcessed();
}

void ResourceBase::changesCommitted( const Collection &collection )
{
  Q_D( ResourceBase );
  CollectionModifyJob *job = new CollectionModifyJob( collection, session() );
  Q_UNUSED( job );
  //TODO: error checking
  d->changeProcessed();
}

bool ResourceBase::requestItemDelivery(qint64 uid, const QString & remoteId, const QStringList &parts )
{
  Q_D( ResourceBase );
  if ( !isOnline() ) {
    error( i18nc( "@info", "Cannot fetch item in offline mode." ) );
    return false;
  }

  setDelayedReply( true );
  // FIXME: we need at least the revision number too
  Item item( uid );
  item.setRemoteId( remoteId );
  d->scheduler->scheduleItemFetch( item, parts, message().createReply() );

  return true;
}

void ResourceBase::collectionsRetrieved(const Collection::List & collections)
{
  Q_D( ResourceBase );
  CollectionSync *syncer = new CollectionSync( d->mId, session() );
  syncer->setRemoteCollections( collections );
  connect( syncer, SIGNAL(result(KJob*)), SLOT(slotCollectionSyncDone(KJob*)) );
}

void ResourceBase::collectionsRetrievedIncremental(const Collection::List & changedCollections, const Collection::List & removedCollections)
{
  Q_D( ResourceBase );
  CollectionSync *syncer = new CollectionSync( d->mId, session() );
  syncer->setRemoteCollections( changedCollections, removedCollections );
  connect( syncer, SIGNAL(result(KJob*)), SLOT(slotCollectionSyncDone(KJob*)) );
}

void ResourceBasePrivate::slotCollectionSyncDone(KJob * job)
{
  Q_Q( ResourceBase );
  if ( job->error() ) {
    q->error( job->errorString() );
  } else {
    if ( scheduler->currentTask().type == ResourceScheduler::SyncAll ) {
      CollectionFetchJob *list = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive, mSession );
      list->setResource( mId );
      q->connect( list, SIGNAL(result(KJob*)), q, SLOT(slotLocalListDone(KJob*)) );
      return;
    }
  }
  if ( scheduler->isEmpty() )
    emit q->status( AgentBase::Idle );
  scheduler->taskDone();
}

void ResourceBasePrivate::slotLocalListDone(KJob * job)
{
  Q_Q( ResourceBase );
  if ( job->error() ) {
    q->error( job->errorString() );
  } else {
    Collection::List cols = static_cast<CollectionFetchJob*>( job )->collections();
    foreach ( const Collection &col, cols ) {
      scheduler->scheduleSync( col );
    }
  }
  scheduler->taskDone();
}

void ResourceBasePrivate::slotSynchronizeCollection( const Collection &col, const QStringList &parts )
{
  Q_Q( ResourceBase );
  currentCollection = col;
  // check if this collection actually can contain anything
  QStringList contentTypes = currentCollection.contentMimeTypes();
  contentTypes.removeAll( Collection::mimeType() );
  if ( !contentTypes.isEmpty() ) {
    emit q->status( AgentBase::Running, i18nc( "@info:status", "Syncing collection '%1'", currentCollection.name() ) );
    q->retrieveItems( currentCollection, parts );
    return;
  }
  scheduler->taskDone();
}

void ResourceBase::itemsRetrieved()
{
  Q_D( ResourceBase );
  if ( d->scheduler->isEmpty() )
    emit status( Idle );
  d->scheduler->taskDone();
}

Collection ResourceBase::currentCollection() const
{
  Q_D( const ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection ,
              "ResourceBase::currentCollection()",
              "Trying to access current collection although no item retrieval is in progress" );
  return d->currentCollection;
}

Item ResourceBase::currentItem() const
{
  const Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::FetchItem ,
              "ResourceBase::currentItem()",
              "Trying to access current item although no item retrieval is in progress" );
  return d->scheduler->currentTask().item;
}

void ResourceBase::synchronizeCollectionTree()
{
  d_func()->scheduler->scheduleCollectionTreeSync();
}

void ResourceBase::doSetOnline( bool state )
{
  d_func()->scheduler->setOnline( state );
}

void ResourceBase::synchronizeCollection(qint64 collectionId )
{
  CollectionFetchJob* job = new CollectionFetchJob( Collection(collectionId), CollectionFetchJob::Base, session() );
  job->setResource( identifier() );
  connect( job, SIGNAL(result(KJob*)), SLOT(slotCollectionListDone(KJob*)) );
}

void ResourceBasePrivate::slotCollectionListDone( KJob *job )
{
  if ( !job->error() ) {
    Collection::List list = static_cast<CollectionFetchJob*>( job )->collections();
    if ( !list.isEmpty() ) {
      Collection col = list.first();
      scheduler->scheduleSync( col );
    }
  }
  // TODO: error handling
}

void ResourceBase::itemsRetrieved(const Item::List &items)
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection,
              "ResourceBase::itemsRetrieved()",
              "Calling itemsRetrieved() although no item retrieval is in progress" );
  ItemSync *syncer = new ItemSync( currentCollection(), session() );
  connect( syncer, SIGNAL(percent(KJob*,unsigned long)), SLOT(slotPercent(KJob*,unsigned long)) );
  connect( syncer, SIGNAL(result(KJob*)), SLOT(slotItemSyncDone(KJob*)) );
  syncer->setFullSyncItems( items );
}

void ResourceBase::itemsRetrievedIncremental(const Item::List &changedItems, const Item::List &removedItems)
{
  Q_D( ResourceBase );
  Q_ASSERT_X( d->scheduler->currentTask().type == ResourceScheduler::SyncCollection,
              "ResourceBase::itemsRetrievedIncremental()",
              "Calling itemsRetrievedIncremental() although no item retrieval is in progress" );
  ItemSync *syncer = new ItemSync( currentCollection(), session() );
  connect( syncer, SIGNAL(percent(KJob*,unsigned long)), SLOT(slotPercent(KJob*,unsigned long)) );
  connect( syncer, SIGNAL(result(KJob*)), SLOT(slotItemSyncDone(KJob*)) );
  syncer->setIncrementalSyncItems( changedItems, removedItems );
}

void ResourceBasePrivate::slotItemSyncDone( KJob *job )
{
  Q_Q( ResourceBase );
  if ( job->error() ) {
    q->error( job->errorString() );
  }
  if ( scheduler->isEmpty() )
    emit q->status( AgentBase::Idle );
  scheduler->taskDone();
}

void ResourceBasePrivate::slotPercent( KJob *job, unsigned long percent )
{
  Q_Q( ResourceBase );
  Q_UNUSED( job );
  emit q->percent( percent );
}

#include "resourcebase.moc"
