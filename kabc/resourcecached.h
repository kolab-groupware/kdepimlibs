/*
    This file is part of kabc.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_RESOURCECACHED_H
#define KABC_RESOURCECACHED_H

#include <kabc/kabc_export.h>
#include <kabc/resource.h>

#include <kresources/idmapper.h>

namespace KABC {

class KABC_EXPORT ResourceCached : public Resource
{
  Q_OBJECT

  public:
    ResourceCached();
    ResourceCached( const KConfigGroup &group );
    ~ResourceCached();

    /**
      Writes the resource specific config to file.
     */
    virtual void writeConfig( KConfigGroup &group );

    /**
      Insert an addressee into the resource.
     */
    virtual void insertAddressee( const Addressee &addr );

    /**
      Removes an addressee from resource.
     */
    virtual void removeAddressee( const Addressee &addr );

    bool loadFromCache();
    void saveToCache();
    void cleanUpCache( const KABC::Addressee::List &list );

    /**
      Returns a reference to the id mapper.
     */
    KRES::IdMapper &idMapper();

    bool hasChanges() const;
    void clearChanges();
    void clearChange( const KABC::Addressee &addr );
    void clearChange( const QString &uid );

    KABC::Addressee::List addedAddressees() const;
    KABC::Addressee::List changedAddressees() const;
    KABC::Addressee::List deletedAddressees() const;

  protected:
    virtual QString cacheFile() const;

    /**
      Functions for keeping the changes persistent.
     */
    virtual QString changesCacheFile( const QString &type ) const;
    void loadChangesCache();
    void saveChangesCache();

    void setIdMapperIdentifier();

  private:
    class Private;
    Private *const d;
};

}

#endif
