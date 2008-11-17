/*
 * Copyright (c) 2008  Igor Trindade Oliveira <igor_trindade@yahoo.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "vcarditem.h"
#include "calitem.h"
#include "itemfactory.h"
#include <QFile>
#include "feeditem.h"

Item *ItemFactory::createItem( QFile *file ){
  if( file->fileName().endsWith(".vcf") ){
    return new VCardItem(file, QString("text/vcard") );
  } else {
    if( file->fileName().endsWith(".ics") ){
      return new CalItem( file, QString("text/calendar") );
    } else {
      if( file->fileName().endsWith(".xml") ){
        return new FeedItem( file, QString("application/rss+xml") );
      }
    }
  }
  
  return 0;
}

