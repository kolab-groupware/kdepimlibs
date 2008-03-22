/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#include "itemfetchjob.h"

#include "collection.h"
#include "collectionselectjob.h"
#include "imapparser_p.h"
#include "itemserializerplugin.h"

#include <kdebug.h>

#include <QtCore/QTimer>

using namespace Akonadi;

class ItemFetchJob::Private
{
  public:
    Private( ItemFetchJob *parent )
      : mParent( parent ),
        fetchAllParts( false ),
        emitTimer( new QTimer( parent ) )
    {
      emitTimer->setSingleShot( true );
      emitTimer->setInterval( 100 );
      QObject::connect( emitTimer, SIGNAL(timeout()), parent, SLOT(timeout()) );
      QObject::connect( parent, SIGNAL(result(KJob*)), parent, SLOT(timeout()) );
    }

    void timeout()
    {
      emitTimer->stop(); // in case we are called by result()
      if ( !pendingItems.isEmpty() ) {
        emit mParent->itemsReceived( pendingItems );
        pendingItems.clear();
      }
    }

    void startFetchJob();
    void selectDone( KJob * job );

    ItemFetchJob *mParent;
    Collection collection;
    Item mItem;
    Item::List items;
    QStringList mFetchParts;
    bool fetchAllParts;
    Item::List pendingItems; // items pending for emitting itemsReceived()
    QTimer* emitTimer;
};

void ItemFetchJob::Private::startFetchJob()
{
  QByteArray command = mParent->newTag();
  if ( !mItem.isValid() )
    command += " FETCH 1:*";
  else
    command += " UID FETCH " + QByteArray::number( mItem.id() );

  if ( !fetchAllParts ) {
    command += " (UID REMOTEID FLAGS";
    foreach ( QString part, mFetchParts ) {
      command += ' ' + part.toUtf8();
    }
    command += ")\n";
  } else {
    command += " AKALL\n";
  }
  mParent->writeData( command );
}

void ItemFetchJob::Private::selectDone( KJob * job )
{
  if ( !job->error() )
    // the collection is now selected, fetch the message(s)
    startFetchJob();
}

ItemFetchJob::ItemFetchJob( QObject *parent ) :
    Job( parent ),
    d( new Private( this ) )
{
}

ItemFetchJob::ItemFetchJob( const Collection &collection, QObject * parent ) :
    Job( parent ),
    d( new Private( this ) )
{
  d->collection = collection;
}

ItemFetchJob::ItemFetchJob( const Item & item, QObject * parent) :
    Job( parent ),
    d( new Private( this ) )
{
  setItem( item );
}

ItemFetchJob::~ ItemFetchJob( )
{
  delete d;
}

void ItemFetchJob::doStart()
{
  if ( !d->mItem.isValid() ) { // collection content listing
    if ( d->collection == Collection::root() ) {
      setErrorText( QLatin1String("Cannot list root collection.") );
      setError( Unknown );
      emitResult();
    }
    CollectionSelectJob *job = new CollectionSelectJob( d->collection, this );
    connect( job, SIGNAL(result(KJob*)), SLOT(selectDone(KJob*)) );
    addSubjob( job );
  } else
    d->startFetchJob();
}

void ItemFetchJob::doHandleResponse( const QByteArray & tag, const QByteArray & data )
{
  if ( tag == "*" ) {
    int begin = data.indexOf( "FETCH" );
    if ( begin >= 0 ) {

      // split fetch response into key/value pairs
      QList<QByteArray> fetchResponse;
      ImapParser::parseParenthesizedList( data, fetchResponse, begin + 6 );

      // create a new item object
      Item::Id uid = -1;
      int rev = -1;
      QString rid;
      QString mimeType;

      for ( int i = 0; i < fetchResponse.count() - 1; i += 2 ) {
        const QByteArray key = fetchResponse.value( i );
        const QByteArray value = fetchResponse.value( i + 1 );

        if ( key == "UID" )
          uid = value.toLongLong();
        else if ( key == "REV" )
          rev = value.toInt();
        else if ( key == "REMOTEID" )
          rid = QString::fromUtf8( value );
        else if ( key == "MIMETYPE" )
          mimeType = QString::fromLatin1( value );
      }

      if ( uid < 0 || rev < 0 || mimeType.isEmpty() ) {
        kWarning( 5250 ) << "Broken fetch response: UID, RID, REV or MIMETYPE missing!";
        return;
      }

      Item item( uid );
      item.setRemoteId( rid );
      item.setRev( rev );
      item.setMimeType( mimeType );
      if ( !item.isValid() )
        return;

      // parse fetch response fields
      for ( int i = 0; i < fetchResponse.count() - 1; i += 2 ) {
        const QByteArray key = fetchResponse.value( i );
        // skip stuff we dealt with already
        if ( key == "UID" || key == "REV" || key == "REMOTEID" || key == "MIMETYPE" )
          continue;
        // flags
        if ( key == "FLAGS" ) {
          QList<QByteArray> flags;
          ImapParser::parseParenthesizedList( fetchResponse[i + 1], flags );
          foreach ( const QByteArray flag, flags ) {
            item.setFlag( flag );
          }
        } else {
          try {
            item.addPart( QString::fromLatin1(key), fetchResponse.value( i+1 ) );
          } catch ( ItemSerializerException &e ) {
            // FIXME how do we react to this? Should we still append?
            kWarning( 5250 ) << "Failed to construct the payload of type: " << item.mimeType();
          }
        }
      }

      d->items.append( item );
      d->pendingItems.append( item );
      if ( !d->emitTimer->isActive() )
        d->emitTimer->start();
      return;
    }
  }
  kDebug( 5250 ) << "Unhandled response: " << tag << data;
}

Item::List ItemFetchJob::items() const
{
  return d->items;
}

void ItemFetchJob::setCollection(const Collection &collection)
{
  d->collection = collection;
  d->mItem = Item();
}

void Akonadi::ItemFetchJob::setItem(const Item & item)
{
  d->collection = Collection::root();
  d->mItem = item;
}

void ItemFetchJob::addFetchPart( const QString &identifier )
{
  if ( !d->mFetchParts.contains( identifier ) )
    d->mFetchParts.append( identifier );
}

void ItemFetchJob::fetchAllParts()
{
  d->fetchAllParts = true;
}

#include "itemfetchjob.moc"
