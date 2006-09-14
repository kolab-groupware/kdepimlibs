/*
    This file is part of the kcal library.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ResourceLocal class.

  @author Preston Brown <pbrown@kde.org>
  @author Cornelius Schumacher <schumacher@kde.org>
*/

#include <typeinfo>
#include <stdlib.h>

#include <QString>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"

#include "kresources/configwidget.h"

#include "resourcelocalconfig.h"

#include "resourcelocal.h"

using namespace KCal;

ResourceLocal::ResourceLocal( const KConfig *config )
  : ResourceCached( config ), d( new ResourceLocal::Private() )
{
  d->mLock = 0;
  if ( config ) {
    QString url = config->readPathEntry( "CalendarURL" );
    d->mURL = KUrl( url );

    QString format = config->readEntry( "Format" );
    if ( format == "ical" ) {
      d->mFormat = new ICalFormat();
    } else if ( format == "vcal" ) {
      d->mFormat = new VCalFormat();
    } else {
      d->mFormat = new ICalFormat();
    }
  } else {
    d->mURL = KUrl();
    d->mFormat = new ICalFormat();
  }
  init();
}

ResourceLocal::ResourceLocal( const QString &fileName )
  : ResourceCached( 0 ), d( new ResourceLocal::Private )
{
  d->mURL = KUrl::fromPath( fileName );
  d->mFormat = new ICalFormat();
  init();
}

void ResourceLocal::writeConfig( KConfig *config )
{
  kDebug(5800) << "ResourceLocal::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );
  config->writePathEntry( "CalendarURL", d->mURL.prettyUrl() );

  if ( typeid( *d->mFormat ) == typeid( ICalFormat ) ) {
    config->writeEntry( "Format", "ical" );
  } else if ( typeid( *d->mFormat ) == typeid( VCalFormat ) ) {
    config->writeEntry( "Format", "vcal" );
  } else {
    kDebug(5800) << "ERROR: Unknown format type" << endl;
  }
}

void ResourceLocal::init()
{

  setType( "file" );

  connect( &d->mDirWatch, SIGNAL( dirty( const QString & ) ),
           SLOT( reload() ) );
  connect( &d->mDirWatch, SIGNAL( created( const QString & ) ),
           SLOT( reload() ) );
  connect( &d->mDirWatch, SIGNAL( deleted( const QString & ) ),
           SLOT( reload() ) );

  d->mLock = new KABC::Lock( d->mURL.path() );

  d->mDirWatch.addFile( d->mURL.path() );
  d->mDirWatch.startScan();
}

ResourceLocal::~ResourceLocal()
{
  d->mDirWatch.stopScan();

  close();

  delete d->mLock;

  delete d;
}

KDateTime ResourceLocal::readLastModified()
{
  QFileInfo fi( d->mURL.path() );
  return KDateTime( fi.lastModified() );  // use local time zone
}

bool ResourceLocal::doLoad( bool )
{
  bool success;

  if ( !KStandardDirs::exists( d->mURL.path() ) ) {
    kDebug(5800) << "ResourceLocal::load(): File doesn't exist yet." << endl;
    // Save the empty calendar, so the calendar file will be created.
    success = doSave( true );
  } else {
    success = mCalendar.load( d->mURL.path() );
    if ( success ) d->mLastModified = readLastModified();
  }

  return success;
}

bool ResourceLocal::doSave( bool )
{
  bool success = mCalendar.save( d->mURL.path() );
  d->mLastModified = readLastModified();

  return success;
}

KABC::Lock *ResourceLocal::lock()
{
  return d->mLock;
}

bool ResourceLocal::doReload()
{
  kDebug(5800) << "ResourceLocal::doReload()" << endl;

  if ( !isOpen() ) return false;

  if ( d->mLastModified == readLastModified() ) {
    kDebug(5800) << "ResourceLocal::reload(): file not modified since last read."
              << endl;
    return false;
  }

  mCalendar.close();
  mCalendar.load( d->mURL.path() );
  return true;
}

void ResourceLocal::reload()
{
  if ( doReload() )
    emit resourceChanged( this );
}

void ResourceLocal::dump() const
{
  ResourceCalendar::dump();
  kDebug(5800) << "  Url: " << d->mURL.url() << endl;
}

QString ResourceLocal::fileName() const
{
  return d->mURL.path();
}

bool ResourceLocal::setFileName( const QString &fileName )
{
  bool open = isOpen();
  if ( open ) close();
  delete d->mLock;
  d->mDirWatch.stopScan();
  d->mDirWatch.removeFile( d->mURL.path() );
  d->mURL = KUrl::fromPath( fileName );
  d->mLock = new KABC::Lock( d->mURL.path() );
  d->mDirWatch.addFile( d->mURL.path() );
  d->mDirWatch.startScan();
  return true;
}

bool ResourceLocal::setValue( const QString &key, const QString &value )
{
  if ( key == "File" ) {
    return setFileName( value );
  } else {
    return false;
  }
}

bool ResourceLocal::operator==( const ResourceLocal &other )
{
  return ( d->mURL == other.d->mURL &&
           d->mLastModified == other.d->mLastModified );
}

ResourceLocal &ResourceLocal::operator=( const ResourceLocal &other )
{
  d->mURL = other.d->mURL;
  d->mLastModified = other.d->mLastModified;
  return *this;
}

#include "resourcelocal.moc"
