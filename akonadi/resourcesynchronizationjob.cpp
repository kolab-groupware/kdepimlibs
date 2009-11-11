/*
 * Copyright (c) 2009 Volker Krause <vkrause@kde.org>
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

#include "resourcesynchronizationjob.h"

#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>

#include <KDebug>
#include <KLocale>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QTimer>

namespace Akonadi
{

class ResourceSynchronizationJobPrivate
{
  public:
    ResourceSynchronizationJobPrivate( ResourceSynchronizationJob* parent ) :
      q( parent ),
      interface( 0 ),
      safetyTimer( 0 ),
      timeoutCount( 0 )
    {}

    ResourceSynchronizationJob *q;
    AgentInstance instance;
    QDBusInterface* interface;
    QTimer* safetyTimer;
    int timeoutCount;
    static int timeoutCountLimit;

    void slotSynchronized();
    void slotTimeout();
};

int ResourceSynchronizationJobPrivate::timeoutCountLimit = 60;

ResourceSynchronizationJob::ResourceSynchronizationJob(const AgentInstance& instance, QObject* parent) :
  KJob( parent ),
  d( new ResourceSynchronizationJobPrivate( this ) )
{
  d->instance = instance;
  d->safetyTimer = new QTimer( this );
  connect( d->safetyTimer, SIGNAL(timeout()), SLOT(slotTimeout()) );
  d->safetyTimer->setInterval( 10 * 1000 );
  d->safetyTimer->setSingleShot( false );
}

ResourceSynchronizationJob::~ResourceSynchronizationJob()
{
  delete d;
}

void ResourceSynchronizationJob::start()
{
  if ( !d->instance.isValid() ) {
    setError( UserDefinedError );
    setErrorText( i18n( "Invalid resource instance." ) );
    emitResult();
    return;
  }

  d->interface = new QDBusInterface( QString::fromLatin1( "org.freedesktop.Akonadi.Resource.%1" ).arg( d->instance.identifier() ),
                                      QString::fromLatin1( "/" ),
                                      QString::fromLatin1( "org.freedesktop.Akonadi.Resource" ), QDBusConnection::sessionBus(), this );
  connect( d->interface, SIGNAL(synchronized()), SLOT(slotSynchronized()) );

  if ( d->interface->isValid() ) {
    d->instance.synchronize();
    d->safetyTimer->start();
  } else {
    setError( UserDefinedError );
    setErrorText( i18n( "Unable to obtain D-Bus interface for resource '%1'", d->instance.identifier() ) );
    emitResult();
    return;
  }
}

void ResourceSynchronizationJobPrivate::slotSynchronized()
{
  q->disconnect( interface, SIGNAL(synchronized()), q, SLOT(slotSynchronized()) );
  safetyTimer->stop();
  q->emitResult();
}

void ResourceSynchronizationJobPrivate::slotTimeout()
{
  instance = AgentManager::self()->instance( instance.identifier() );
  timeoutCount++;

  if ( timeoutCount > timeoutCountLimit ) {
    safetyTimer->stop();
    q->setError( KJob::UserDefinedError );
    q->setErrorText( i18n( "Resource synchronization timed out." ) );
    q->emitResult();
    return;
  }

  if ( instance.status() == AgentInstance::Idle ) {
    // try again, we might have lost the synchronized() signal
    kDebug() << "trying again to sync resource" << instance.identifier();
    instance.synchronize();
  }
}

AgentInstance ResourceSynchronizationJob::resource() const
{
  return d->instance;
}

}

#include "resourcesynchronizationjob.moc"
