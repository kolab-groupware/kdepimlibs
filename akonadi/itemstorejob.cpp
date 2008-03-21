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

#include "itemstorejob.h"
#include "imapparser_p.h"

#include <kdebug.h>

using namespace Akonadi;

class ItemStoreJob::Private
{
  public:
    enum Operation {
      SetFlags,
      AddFlags,
      RemoveFlags,
      RemoveParts,
      Move,
      RemoteId,
      Dirty
    };

    Private( ItemStoreJob *parent, const Item &it )
    : mParent( parent ), item( it ), revCheck( true )
    {
    }

    ItemStoreJob *mParent;
    Item::Flags flags;
    Item::Flags addFlags;
    Item::Flags removeFlags;
    QList<QByteArray> removeParts;
    QSet<int> operations;
    QByteArray tag;
    Collection collection;
    Item item;
    bool revCheck;
    QStringList parts;
    QByteArray pendingData;

    QByteArray nextPartHeader()
    {
      QByteArray command;
      if ( !parts.isEmpty() ) {
        QString label = parts.takeFirst();
        pendingData = item.part( label );
        command += ' ' + label.toUtf8();
        command += ".SILENT {" + QByteArray::number( pendingData.size() ) + '}';
        if ( pendingData.size() > 0 )
          command += '\n';
        else
          command += nextPartHeader();
      } else {
        command += ")\n";
      }
      return command;
    }
};

ItemStoreJob::ItemStoreJob(const Item &item, QObject * parent) :
    Job( parent ),
    d( new Private( this, item ) )
{
  d->operations.insert( Private::RemoteId );
}

ItemStoreJob::~ ItemStoreJob()
{
  delete d;
}

void ItemStoreJob::setFlags(const Item::Flags & flags)
{
  d->flags = flags;
  d->operations.insert( Private::SetFlags );
}

void ItemStoreJob::addFlag(const Item::Flag & flag)
{
  d->addFlags.insert( flag );
  d->operations.insert( Private::AddFlags );
}

void ItemStoreJob::removeFlag(const Item::Flag & flag)
{
  d->removeFlags.insert( flag );
  d->operations.insert( Private::RemoveFlags );
}

void ItemStoreJob::removePart(const QByteArray & part)
{
  d->removeParts.append( part );
  d->operations.insert( Private::RemoveParts );
}

void ItemStoreJob::setCollection(const Collection &collection)
{
  d->collection = collection;
  d->operations.insert( Private::Move );
}

void ItemStoreJob::setClean()
{
  d->operations.insert( Private::Dirty );
}

void ItemStoreJob::doStart()
{
  // nothing to do
  if ( d->operations.isEmpty() && d->parts.isEmpty() ) {
    emitResult();
    return;
  }

  d->tag = newTag();
  QByteArray command = d->tag;
  command += " UID STORE " + QByteArray::number( d->item.reference().id() ) + ' ';
  if ( !d->revCheck || d->addFlags.contains( "\\Deleted" ) ) {
    command += "NOREV ";
  } else {
    command += "REV " + QByteArray::number( d->item.rev() );
  }
  QList<QByteArray> changes;
  foreach ( int op, d->operations ) {
    switch ( op ) {
      case Private::SetFlags:
        changes << "FLAGS.SILENT";
        changes << "(" + ImapParser::join( d->flags, " " ) + ')';
        break;
      case Private::AddFlags:
        changes << "+FLAGS.SILENT";
        changes << "(" + ImapParser::join( d->addFlags, " " ) + ')';
        break;
      case Private::RemoveFlags:
        changes << "-FLAGS.SILENT";
        changes << "(" + ImapParser::join( d->removeFlags, " " ) + ')';
        break;
      case Private::RemoveParts:
        changes << "-PARTS.SILENT";
        changes << "(" + ImapParser::join( d->removeParts, " " ) + ')';
        break;
      case Private::Move:
        changes << "COLLECTION.SILENT";
        changes << QByteArray::number( d->collection.id() );
        break;
      case Private::RemoteId:
        if ( !d->item.reference().remoteId().isNull() ) {
          changes << "REMOTEID.SILENT";
          changes << ImapParser::quote( d->item.reference().remoteId().toLatin1() );
        }
        break;
      case Private::Dirty:
        changes << "DIRTY.SILENT";
        changes << "false";
        break;
    }
  }
  command += " (" + ImapParser::join( changes, " " );
  command += d->nextPartHeader();
  writeData( command );
  newTag(); // hack to circumvent automatic response handling
}

void ItemStoreJob::doHandleResponse(const QByteArray &_tag, const QByteArray & data)
{
  if ( _tag == "+" ) { // ready for literal data
    writeData( d->pendingData );
    writeData( d->nextPartHeader() );
    return;
  }
  if ( _tag == d->tag ) {
    if ( data.startsWith( "OK" ) ) {
      // increase item revision of item, given by calling function
      if( d->revCheck )
        d->item.incRev();
      else
        // increase item revision of own copy of item
        d->item.incRev();
    } else {
      setError( Unknown );
      setErrorText( QString::fromUtf8( data ) );
    }
    emitResult();
    return;
  }
  kDebug( 5250 ) << "Unhandled response: " << _tag << data;
}

void ItemStoreJob::storePayload()
{
  Q_ASSERT( !d->item.mimeType().isEmpty() );
  d->parts = d->item.availableParts();
}

void ItemStoreJob::noRevCheck()
{
  d->revCheck = false;
}

Item ItemStoreJob::item() const
{
  return d->item;
}

#include "itemstorejob.moc"
