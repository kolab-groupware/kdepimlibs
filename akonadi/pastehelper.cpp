/*
    Copyright (c) 2008 Volker Krause <vkrause@kde.org>

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

#include "pastehelper_p.h"

#include "collectioncopyjob.h"
#include "collectionmodifyjob.h"
#include "item.h"
#include "itemcreatejob.h"
#include "itemcopyjob.h"
#include "itemmodifyjob.h"
#include "itemmovejob.h"
#include "transactionsequence.h"

#include <KDebug>
#include <KUrl>

#include <QtCore/QByteArray>
#include <QtCore/QMimeData>
#include <QtCore/QStringList>

using namespace Akonadi;

bool PasteHelper::canPaste(const QMimeData * mimeData, const Collection & collection)
{
  if ( !mimeData || !collection.isValid() )
    return false;

  // TODO check acls depending on mimetype
  if ( (collection.rights() & (Collection::CanCreateItem | Collection::CanCreateCollection) ) == 0 )
    return false;

  if ( KUrl::List::canDecode( mimeData ) )
    return true;

  foreach ( const QString &format, mimeData->formats() )
    if ( collection.contentMimeTypes().contains( format ) )
      return true;

  return false;
}

KJob* PasteHelper::paste(const QMimeData * mimeData, const Collection & collection, bool copy)
{
  if ( !canPaste( mimeData, collection ) )
    return 0;

  // we try to drop data not coming with the akonadi:// url
  // find a type the target collection supports
  foreach ( const QString &type, mimeData->formats() ) {
    if ( !collection.contentMimeTypes().contains( type ) )
      continue;

    QByteArray item = mimeData->data( type );
    // HACK for some unknown reason the data is sometimes 0-terminated...
    if ( !item.isEmpty() && item.at( item.size() - 1 ) == 0 )
      item.resize( item.size() - 1 );

    Item it;
    it.setMimeType( type );
    it.setPayloadFromData( item );

    ItemCreateJob *job = new ItemCreateJob( it, collection );
    return job;
  }

  if ( !KUrl::List::canDecode( mimeData ) )
    return 0;

  // data contains an url list
  TransactionSequence *transaction = new TransactionSequence();
  KUrl::List urls = KUrl::List::fromMimeData( mimeData );
  foreach ( const KUrl &url, urls ) {
    if ( Collection::fromUrl( url ).isValid() ) {
      Collection col = Collection::fromUrl( url );
      if ( !copy ) {
        col.setParent( collection );
        CollectionModifyJob *job = new CollectionModifyJob( col, transaction );
      } else {
        new CollectionCopyJob( col, collection, transaction );
      }
    } else if ( Item::fromUrl( url ).isValid() ) {
      // TODO Extract mimetype from url and check if collection accepts it
      const Item item = Item::fromUrl( url );
      if ( !copy ) {
        new ItemMoveJob( item, collection, transaction );
      } else  {
        new ItemCopyJob( item, collection, transaction );
      }
    } else {
      // non-akonadi URL
      // TODO
      kDebug() << "Implement me!";
    }
  }
  return transaction;
}

