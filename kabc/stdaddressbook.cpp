/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include "stdaddressbook.h"
#include "resourcefactory.h"

using namespace KABC;

AddressBook *StdAddressBook::mSelf = 0;

QString StdAddressBook::fileName()
{
    return locateLocal( "data", "kabc/std.vcf" );
}

AddressBook *StdAddressBook::self()
{
    kdDebug(5700) << "StdAddressBook::self()" << endl;

    if ( !mSelf ) {
	mSelf = new StdAddressBook;
    }
    return mSelf;
}

bool StdAddressBook::save()
{
    kdDebug(5700) << "StdAddressBook::save()" << endl;

    return self()->saveAll();
}


StdAddressBook::StdAddressBook()
{
    KSimpleConfig config( "kabcrc", true );
    ResourceFactory *factory = ResourceFactory::self();
    config.setGroup( "General" );

    QStringList keys = config.readListEntry( "ResourceKeys" );
    for ( QStringList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
	config.setGroup( "Resource_" + (*it) );
	QString type = config.readEntry( "ResourceType" );

	Resource *resource = factory->resource( type, this, &config );

	if ( !resource )
	    continue;

	resource->setReadOnly( config.readBoolEntry( "ResourceIsReadOnly" ) );
	resource->setFastResource( config.readBoolEntry( "ResourceIsFast" ) );
	resource->setName( config.readEntry( "ResourceName" ) );
    }

    for ( uint i = 0; i < mResources.count(); ++i ) {
	Resource *resource = mResources.at( i );
	mIdentifier += ( i == 0 ? "" : ":" ) + resource->identifier();
    }

    load();
}

StdAddressBook::~StdAddressBook()
{
    save();
}

QString StdAddressBook::identifier()
{
    return mIdentifier;
}
