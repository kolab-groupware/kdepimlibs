/*
    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_AGENTTYPEMODEL_H
#define AKONADI_AGENTTYPEMODEL_H

#include <akonadi_export.h>
#include <QtCore/QAbstractItemModel>

namespace Akonadi {

/**
 * This class provides a model for available agent types.
 */
class AKONADI_EXPORT AgentTypeModel : public QAbstractItemModel
{
  Q_OBJECT

  public:
    enum Role
    {
      TypeIdentifierRole = Qt::UserRole + 1,
      CommentRole,
      MimeTypesRole,
      CapabilitiesRole
    };

    /**
     * Creates a new agent type model.
     */
    explicit AgentTypeModel( QObject *parent );

    /**
     * Destroys the agent type model.
     */
    virtual ~AgentTypeModel();

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    virtual QModelIndex parent( const QModelIndex &index ) const;

  private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void agentTypeAdded( const QString& ) )
    Q_PRIVATE_SLOT( d, void agentTypeRemoved( const QString& ) )
};

}

#endif
