/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_COLLECTIONMODIFYJOB_H
#define AKONADI_COLLECTIONMODIFYJOB_H

#include "akonadi_export.h"
#include <akonadi/cachepolicy.h>
#include <akonadi/collection.h>
#include <akonadi/job.h>

namespace Akonadi {

class CollectionModifyJobPrivate;

/**
  Job to modify properties of existing collections.
*/
class AKONADI_EXPORT CollectionModifyJob : public Job
{
  Q_OBJECT

  public:
    /**
      Create a new modify job for the given collection.
      @param collection The Collection to modify
      @param parent The parent object
    */
    explicit CollectionModifyJob( const Collection &collection, QObject *parent = 0 );

    /**
      Destroys this job.
    */
    ~CollectionModifyJob();

    /**
      Sets the supported content mimetypes of the collection to the given value.
      @param mimeTypes The new supported content mimetypes.
    */
    void setContentTypes( const QStringList &mimeTypes );

    /**
      Sets the cache policy of the collection.
      @param policy The cache policy.
    */
    void setCachePolicy( const CachePolicy &policy );

    /**
      Changes name of this collection.
      @param name The new name.
    */
    void setName( const QString &name );

    /**
      Changes the parent of this collection.
      @param parent The new parent collection.
    */
    void setParent( const Collection &parent );

    /**
      Sets the given collection attribute.
      @param attr A collection attribute, ownership stays with the caller.
    */
    void setAttribute( Attribute* attr );

    /**
      Removes the given collection attribute.
      @param attrName The attribute name.
    */
    void removeAttribute( const QByteArray &attrName );

  protected:
    virtual void doStart();

  private:
    CollectionModifyJobPrivate* const d;
};

}

#endif
