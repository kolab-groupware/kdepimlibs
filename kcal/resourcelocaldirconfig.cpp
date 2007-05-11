/*
    This file is part of the kcal library.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "resourcelocaldirconfig.moc"

#include "resourcelocaldir.h"

#include <typeinfo>

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

using namespace KCal;

ResourceLocalDirConfig::ResourceLocalDirConfig( QWidget *parent )
    : KRES::ConfigWidget( parent ), d( 0 )
{
  resize( 245, 115 );
  QGridLayout *mainLayout = new QGridLayout( this );

  QLabel *label = new QLabel( i18n( "Location:" ), this );
  mURL = new KUrlRequester( this );
  mURL->setMode( KFile::Directory | KFile::LocalOnly );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mURL, 1, 1 );
}

void ResourceLocalDirConfig::loadSettings( KRES::Resource *resource )
{
  ResourceLocalDir* res = static_cast<ResourceLocalDir*>( resource );
  if ( res ) {
    mURL->setUrl( res->mURL.prettyUrl() );
  } else
    kDebug(5700) << "ERROR: ResourceLocalDirConfig::loadSettings(): no ResourceLocalDir, cast failed" << endl;
}

void ResourceLocalDirConfig::saveSettings( KRES::Resource *resource )
{
  ResourceLocalDir* res = static_cast<ResourceLocalDir*>( resource );
  if (res) {
    res->mURL = mURL->url();
  } else
    kDebug(5700) << "ERROR: ResourceLocalDirConfig::saveSettings(): no ResourceLocalDir, cast failed" << endl;
}
