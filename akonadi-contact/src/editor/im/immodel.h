/*
    This file is part of Akonadi Contact.

    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_CONTACT_IMMODEL_H
#define AKONADI_CONTACT_IMMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QVector>

class IMAddress
{
public:
    typedef QVector<IMAddress> List;

    IMAddress();
    IMAddress(const QString &protocol, const QString &name, bool preferred);

    void setProtocol(const QString &protocol);
    QString protocol() const;

    void setName(const QString &name);
    QString name() const;

    void setPreferred(bool preferred);
    bool preferred() const;

private:
    QString mProtocol;
    QString mName;
    bool mPreferred;
};

class IMModel : public QAbstractItemModel
{
public:
    enum Role {
        ProtocolRole = Qt::UserRole,
        IsPreferredRole
    };

    explicit IMModel(QObject *parent = Q_NULLPTR);
    ~IMModel();

    void setAddresses(const IMAddress::List &addresses);
    IMAddress::List addresses() const;

    QModelIndex index(int row, int col, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

private:
    IMAddress::List mAddresses;
};

#endif
