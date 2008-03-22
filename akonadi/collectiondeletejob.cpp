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

#include "collectiondeletejob.h"

#include "job_p.h"

using namespace Akonadi;

class Akonadi::CollectionDeleteJobPrivate : public JobPrivate
{
  public:
    CollectionDeleteJobPrivate( CollectionDeleteJob *parent )
      : JobPrivate( parent )
    {
    }

    Collection mCollection;
};

CollectionDeleteJob::CollectionDeleteJob(const Collection &collection, QObject * parent)
  : Job( new CollectionDeleteJobPrivate( this ), parent )
{
  Q_D( CollectionDeleteJob );

  d->mCollection = collection;
}

CollectionDeleteJob::~ CollectionDeleteJob()
{
  Q_D( CollectionDeleteJob );

  delete d;
}

void CollectionDeleteJob::doStart()
{
  Q_D( CollectionDeleteJob );

  writeData( newTag() + " DELETE " + QByteArray::number( d->mCollection.id() ) + '\n' );
}

#include "collectiondeletejob.moc"
