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

#ifndef AKONADI_ITEMMODEL_H
#define AKONADI_ITEMMODEL_H

#include "akonadi_export.h"
#include <akonadi/item.h>
#include <akonadi/job.h>

#include <QtCore/QAbstractTableModel>

namespace Akonadi {

class Collection;
class ItemFetchScope;
class Job;
class Session;

/**
  A flat self-updating message model.
*/
class AKONADI_EXPORT ItemModel : public QAbstractTableModel
{
  Q_OBJECT

  public:
    /**
      Columns types.
    */
    enum Column {
      Id = 0, /**< The unique id. */
      RemoteId, /**< The remote identifier. */
      MimeType /**< Item mimetype. */
    };

    enum Roles {
      IdRole = Qt::UserRole + 1,
      ItemRole,
      MimeTypeRole,
      UserRole = Qt::UserRole + 42
    };

    /**
      Creates a new message model.

      @param parent The parent object.
    */
    explicit ItemModel( QObject* parent = 0 );

    /**
      Deletes the message model.
    */
    virtual ~ItemModel();

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

    /**
      Reimplemented from QAbstractItemModel.
     */
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    /**
      Reimplemented from QAbstractItemModel
     */
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    /**
      Reimplemented from QAbstractItemModel
     */
    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

    /**
      Reimplemented from QAbstractItemModel
    */
    virtual QStringList mimeTypes() const;

    void setFetchScope( const ItemFetchScope &fetchScope );

    ItemFetchScope &fetchScope();

    /**
      Returns the item at given index.
    */
    Item itemForIndex( const QModelIndex &index ) const;

    /**
      Returns the model index for the given item, with the given column
      @param item The item to find
      @param column The column for the returned index
    */
    QModelIndex indexForItem( const Akonadi::Item& item, const int column ) const;

    /* reimpl */
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

  public Q_SLOTS:
    /**
      Sets the collection the model should display. If the collection has
      changed, the model is reset and a new message listing is requested
      from the storage backend.
      @param collection The collection.
    */
    void setCollection( const Akonadi::Collection &collection );

  Q_SIGNALS:
    /**
      SetCollection emits this signal when the collection has changed.
     */
     void collectionChanged( const Collection &collection );

  protected:

    /**
      Returns the Session object used for all operations by this model.
    */
    Session* session() const;

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void listingDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void itemChanged( const Akonadi::Item&, const QStringList& ) )
    Q_PRIVATE_SLOT( d, void itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& ) )
    Q_PRIVATE_SLOT( d, void itemAdded( const Akonadi::Item& ) )
    Q_PRIVATE_SLOT( d, void itemsAdded( const Akonadi::Item::List& ) )
    Q_PRIVATE_SLOT( d, void itemRemoved( const Akonadi::Item& ) )
};

}

#endif
