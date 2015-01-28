/*
    This file is part of libkabc.
    Copyright (c) 2015 Laurent Montel <montel@kde.org>

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


#ifndef GENDER_H
#define GENDER_H

#include "kabc_export.h"
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>
#include <QtCore/QMap>

/** @short Class that holds a Gender for a contact.
 *  @since 4.14.5
 */

namespace KABC {

class KABC_EXPORT Gender
{
    friend KABC_EXPORT QDataStream &operator<<( QDataStream &, const Gender & );
    friend KABC_EXPORT QDataStream &operator>>( QDataStream &, Gender & );
public:
    /**
     * Creates an empty Gender object.
     */
    Gender();
    Gender(const Gender &other);
    Gender(const QString &gender);

    ~Gender();

    void setGender(const QString &gender);
    QString gender() const;

    bool isValid() const;

    void setParameters(const QMap<QString, QStringList> &params);
    QMap<QString, QStringList> parameters() const;

    bool operator==( const Gender &other ) const;
    bool operator!=( const Gender &other ) const;

    Gender &operator=( const Gender &other );

    QString toString() const;
private:
    class Private;
    QSharedDataPointer<Private> d;
};

KABC_EXPORT QDataStream &operator<<( QDataStream &stream, const Gender &object );

KABC_EXPORT QDataStream &operator>>( QDataStream &stream, Gender &object );

}
#endif // GENDER_H
