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

#include "lang.h"
#include <QMap>
#include <qstringlist.h>

using namespace KABC;

class Lang::Private : public QSharedData
{
public:
    Private()
    {
    }

    Private( const Private &other )
        : QSharedData( other )
    {
        parameters = other.parameters;
        language = other.language;
    }
    QMap<QString, QStringList> parameters;
    QString language;
};


Lang::Lang()
    : d(new Private)
{

}

Lang::Lang(const Lang &other)
    : d( other.d )
{

}

Lang::Lang(const QString &lang)
    : d(new Private)
{
    d->language = lang;
}

Lang::~Lang()
{

}

void Lang::setLanguage(const QString &lang)
{
    d->language = lang;
}

QString Lang::language() const
{
    return d->language;
}

bool Lang::isValid() const
{
    return !d->language.isEmpty();
}

void Lang::setParameters(const QMap<QString, QStringList> &params)
{
    d->parameters = params;
}

QMap<QString, QStringList> Lang::parameters() const
{
    return d->parameters;
}

bool Lang::operator==(const Lang &other) const
{
    return (d->parameters == other.parameters()) && (d->language == other.language());
}

bool Lang::operator!=(const Lang &other) const
{
    return !( other == *this );
}

Lang &Lang::operator=(const Lang &other)
{
    if ( this != &other ) {
      d = other.d;
    }

    return *this;
}

QString Lang::toString() const
{
    QString str;
    str += QString::fromLatin1( "Lang {\n" );
    str += QString::fromLatin1( "    language: %1\n" ).arg( d->language );
    if (!d->parameters.isEmpty()) {
        QMapIterator<QString, QStringList> i(d->parameters);
        QString param;
        while (i.hasNext()) {
            i.next();
            param += QString::fromLatin1("%1 %2").arg(i.key()).arg(i.value().join(QLatin1String(",")));
        }
        str += QString::fromLatin1( "    parameters: %1\n" ).arg( param );
    }
    str += QString::fromLatin1( "}\n" );
    return str;
}


QDataStream &KABC::operator<<(QDataStream &s, const Lang &lang)
{
    return s << lang.d->parameters << lang.d->language;
}

QDataStream &KABC::operator>>(QDataStream &s, Lang &lang)
{
    s >> lang.d->parameters >> lang.d->language;
    return s;
}
