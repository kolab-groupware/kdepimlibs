/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_MONITOR_H
#define AKONADI_MONITOR_H

#include <akonadi/collection.h>
#include <akonadi/collectionstatus.h>
#include <akonadi/item.h>
#include <akonadi/job.h>
#include <QtCore/QObject>

namespace Akonadi {

class MonitorPrivate;
class Session;

/**
  Monitors an Item or Collection for changes and emits signals if some
  of these objects are changed or removed or new ones are added to the storage
  backend.

  Optionally, the changed objects can be fetched automatically from the server.
  To enable this, see fetchCollection(), fetchItemMetaData(), fetchItemData().

  @todo: support un-monitoring
  @todo: distinguish between monitoring collection properties and collection content.
  @todo: special case for collection content counts changed
*/
class AKONADI_EXPORT Monitor : public QObject
{
  Q_OBJECT

  public:
    /**
      Creates a new monitor.
      @param parent The parent object.
    */
    explicit Monitor( QObject *parent = 0 );

    /**
      Destroys this monitor.
    */
    virtual ~Monitor();

    /**
      Monitors the specified collection for changes.
      Monitoring Collection::root() monitors all collections.
      @param collection The collection to monitor.
    */
    void monitorCollection( const Collection &collection );

    /**
      Monitors the specified item for changes.
      @param item The item to monitor.
    */
    void monitorItem( const Item &item );

    /**
      Monitors the specified resource for changes.
      @param resource The resource identifier.
    */
    void monitorResource( const QByteArray &resource );

    /**
      Monitor all items matching the specified mimetype.
      @param mimetype The mimetype.
    */
    void monitorMimeType( const QString &mimetype );

    /**
      Monitor all items.
    */
    void monitorAll();

    /**
      Ignore all notifications caused by the given session.
      @param session The session you want to ignore.
    */
    void ignoreSession( Session *session );

    /**
      Enable automatic fetching of changed collections from the server.
      @param enable @c true enables auto-fetching, @c false disables auto-fetching.
    */
    void fetchCollection( bool enable );

    /**
      Enable automatic fetching of changed collection status information.
      @param enable @c true to enable.
    */
    void fetchCollectionStatus( bool enable );

    /**
      Sets the part identifier of the parts that shall be fetched for
      items. As default no parts are fetched.
    */
    void addFetchPart( const QString &identifier );

    /**
      Fetch all item parts.
    */
    void fetchAllParts();

  Q_SIGNALS:
    /**
      Emitted if a monitored item has changed: item parts have been modified.
      @param item The changed item.
      @param partIdentifiers The identifiers of the item parts that has been changed.
    */
    void itemChanged( const Akonadi::Item &item, const QStringList &partIdentifiers );

    /**
      Emitted if a monitored item has been moved between two collections
      @param item The moved item
      @param collectionSource The collection the item has been moved from
      @param collectionDestination The collection the item has been moved to
    */
    void itemMoved( const Akonadi::Item &item, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination);

    /**
      Emitted if a item has been added to the storage, in a monitored collection.
      @param item The new item.
      @param collection The collection the item is added to.
    */
    void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection );

    /**
      Emitted if a monitored object has been removed from the storage and from a monitored collection.
      @param item The removed item.
    */
    void itemRemoved( const Akonadi::Item &item);

    /**
      Emitted if a new collection was added in the storage and if
      a monitored collection got this new collection as a child.
      @param collection The new collection.
      @param parent The parent collection.
    */
    void collectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection &parent );

    /**
      Emitted if a monitored collection changed (properties and
      content). Also emitted if the collection was reparented.
      @param collection The changed collection.
     */
    void collectionChanged( const Akonadi::Collection &collection );

    /**
      Emitted if a monitored collection has been removed from the storage.
      @param collection The removed collection.
    */
    void collectionRemoved( const Akonadi::Collection &collection );

    /**
      Emitted if the status information of a monitored collection
      has changed.
      @param id The collection identifier of the changed collection.
      @param status The updated collection status, invalid of automatic
      fetching of status changes is disabled.
    */
    void collectionStatusChanged( Akonadi::Collection::Id id, const Akonadi::CollectionStatus &status );

  protected:
    //@cond PRIVATE
    MonitorPrivate *d_ptr;
    explicit Monitor( MonitorPrivate *d, QObject *parent = 0 );
    //@endcond

  private:
    Q_DECLARE_PRIVATE( Monitor )

    Q_PRIVATE_SLOT( d_ptr, void slotStatusChangedFinished( KJob* ) )
    Q_PRIVATE_SLOT( d_ptr, void slotFlushRecentlyChangedCollections() )

    Q_PRIVATE_SLOT( d_ptr, void slotNotify( const Akonadi::NotificationMessage::List &msgs ) )
    Q_PRIVATE_SLOT( d_ptr, void slotItemJobFinished( KJob *job ) )
    Q_PRIVATE_SLOT( d_ptr, void slotCollectionJobFinished( KJob *job ) )
};

}

#endif
