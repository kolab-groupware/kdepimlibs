/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <kdebug.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

#include <qfile.h>

#include "resourcefile.h"
#include "resourcefileconfig.h"

#include "resourcefactory.h"

using namespace KABC;

ResourceFactory *ResourceFactory::mSelf = 0;

ResourceFactory *ResourceFactory::self()
{
    kdDebug(5700) << "ResourceFactory::self()" << endl;

    if ( !mSelf ) {
	mSelf = new ResourceFactory;
    }

    return mSelf;
}

ResourceFactory::ResourceFactory()
{
    mResourceList.setAutoDelete( true );

    // default resource
    ResourceInfo *info = new ResourceInfo;
    info->library = "<none>";
    info->name = i18n( "File" );
    info->desc = "<none>";

    mResourceList.insert( "file", info );

    QStringList list = KGlobal::dirs()->findAllResources( "data" ,"kabc/*.plugin", true, true );
    for ( QStringList::iterator it = list.begin(); it != list.end(); ++it )
    {
	KSimpleConfig config( *it, true );

	if ( !config.hasGroup( "Misc" ) || !config.hasGroup( "Plugin" ) )
	    continue;

	info = new ResourceInfo;

	config.setGroup( "Plugin" );
	QString descr = config.readEntry( "Type" );
	info->library = config.readEntry( "X-KDE-Library" );
	
	config.setGroup( "Misc" );
	info->name = config.readEntry( "Name" );
	info->desc = config.readEntry( "Comment", i18n( "No description available." ) );

	mResourceList.insert( descr, info );
    }
}

ResourceFactory::~ResourceFactory()
{
    mResourceList.clear();
}

QStringList ResourceFactory::resources()
{
    QStringList retval;
	
    // make sure file is the first entry
    retval << "file";

    QDictIterator<ResourceInfo> it( mResourceList );
    for ( ; it.current(); ++it )
	if ( it.currentKey() != "file" )
	    retval << it.currentKey();

    return retval;
}

ResourceInfo *ResourceFactory::info( const QString& resName )
{
    if ( resName.isEmpty() )
	return 0;
    else
	return mResourceList[ resName ];
}

ResourceConfigWidget *ResourceFactory::configWidget( const QString& resName, QWidget *parent )
{
    ResourceConfigWidget *widget = 0;

    if ( resName.isEmpty() )
	return 0;

    if ( resName == "file" ) {
	return new ResourceFileConfig( parent, "ResourceFileConfig" );
    }

    QString libName = mResourceList[ resName ]->library;

    KLibrary *library = openLibrary( libName );
    if ( !library )
	return 0;

    void *widget_func = library->symbol( "config_widget" );

    if ( widget_func ) {
        widget = ((ResourceConfigWidget* (*)(QWidget *wdg))widget_func)( parent );
    } else {
	kdDebug( 5700 ) << "'" << resName << "' is not a kabc plugin." << endl;
	return 0;
    }

    return widget;
}

Resource *ResourceFactory::resource( const QString& resName, AddressBook *ab, const KConfig *config )
{
    Resource *resource = 0;

    if ( resName.isEmpty() )
	return 0;

    if ( resName == "file" ) {
	return new ResourceFile( ab, config );
    }

    QString libName = mResourceList[ resName ]->library;

    KLibrary *library = openLibrary( libName );
    if ( !library )
	return 0;

    void *resource_func = library->symbol( "resource" );

    if( resource_func ) {
        resource = ((Resource* (*)(AddressBook *, const KConfig *))resource_func)( ab, config );
    } else {
	kdDebug( 5700 ) << "'" << resName << "' is not a kabc plugin." << endl;
	return 0;
    }

    return resource;
}


KLibrary *ResourceFactory::openLibrary( const QString& libName )
{
    KLibrary *library = 0;

    QString path = KLibLoader::findLibrary( QFile::encodeName( libName ) );

    if ( path.isEmpty() ) {
        kdDebug( 5700 ) << "No resource plugin library was found!" << endl;
	return 0;
    }

    library = KLibLoader::self()->library( QFile::encodeName( path ) );

    if ( !library ) {
	kdDebug( 5700 ) << "Could not load library '" << libName << "'" << endl;
	return 0;
    }

    return library;
}
