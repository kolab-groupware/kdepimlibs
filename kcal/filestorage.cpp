/*
    This file is part of the kcal library.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "filestorage.h"

#include "calendar.h"
#include "vcalformat.h"
#include "icalformat.h"

#include <stdlib.h>

#include <QtCore/QString>

#include <kdebug.h>

using namespace KCal;

FileStorage::FileStorage( Calendar *cal, const QString &fileName,
                          CalFormat *format )
  : CalStorage( cal ),
    mFileName( fileName ),
    mSaveFormat( format ),
    d( 0 )
{
}

FileStorage::~FileStorage()
{
  delete mSaveFormat;
}

void FileStorage::setFileName( const QString &fileName )
{
  mFileName = fileName;
}

QString FileStorage::fileName()const
{
  return mFileName;
}


void FileStorage::setSaveFormat( CalFormat *format )
{
  delete mSaveFormat;
  mSaveFormat = format;
}

CalFormat *FileStorage::saveFormat()const
{
  return mSaveFormat;
}


bool FileStorage::open()
{
  return true;
}

bool FileStorage::load()
{
//  kDebug(5800) << "FileStorage::load(): '" << mFileName << "'" << endl;

  // do we want to silently accept this, or make some noise?  Dunno...
  // it is a semantical thing vs. a practical thing.
  if (mFileName.isEmpty()) return false;

  // Always try to load with iCalendar. It will detect, if it is actually a
  // vCalendar file.
  bool success;
  // First try the supplied format. Otherwise fall through to iCalendar, then
  // to vCalendar
  success = saveFormat() && saveFormat()->load( calendar(), mFileName );
  if ( !success ) {
    ICalFormat iCal;

    success = iCal.load( calendar(), mFileName);

    if ( !success ) {
      if ( iCal.exception() ) {
//        kDebug(5800) << "---Error: " << mFormat->exception()->errorCode() << endl;
        if ( iCal.exception()->errorCode() == ErrorFormat::CalVersion1 ) {
          // Expected non vCalendar file, but detected vCalendar
          kDebug(5800) << "FileStorage::load() Fallback to VCalFormat" << endl;
          VCalFormat vCal;
          success = vCal.load( calendar(), mFileName );
          calendar()->setProductId( vCal.productId() );
        } else {
          return false;
        }
      } else {
        kDebug(5800) << "Warning! There should be an exception set." << endl;
        return false;
      }
    } else {
//     kDebug(5800) << "---Success" << endl;
      calendar()->setProductId( iCal.loadedProductId() );
    }
  }

  calendar()->setModified( false );

  return true;
}

bool FileStorage::save()
{
  if ( mFileName.isEmpty() ) return false;

  CalFormat *format = 0;
  if ( mSaveFormat ) format = mSaveFormat;
  else format = new ICalFormat;

  bool success = format->save( calendar(), mFileName );

  if ( success ) {
    calendar()->setModified( false );
  } else {
    if ( !format->exception() ) {
      kDebug(5800) << "FileStorage::save(): Error. There should be an expection set."
                << endl;
    } else {
      kDebug(5800) << "FileStorage::save(): " << format->exception()->message()
                << endl;
    }
  }

  if ( !mSaveFormat ) delete format;

  return success;
}

bool FileStorage::close()
{
  return true;
}
