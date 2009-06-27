/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MAILTRANSPORT_TRANSPORTTYPE_P_H
#define MAILTRANSPORT_TRANSPORTTYPE_P_H

#include <QtCore/QSharedData>
#include <QtCore/QString>

#include <akonadi/agenttype.h>

namespace MailTransport {

/**
  @internal
*/
class TransportType::Private : public QSharedData
{
  public:
    Private()
    {
      mType = -1;
    }

    Private( const Private &other )
      : QSharedData( other )
    {
      mType = other.mType;
      mName = other.mName;
      mDescription = other.mDescription;
      mAgentType = other.mAgentType;
    }

    int mType;
    QString mName;
    QString mDescription;
    Akonadi::AgentType mAgentType;
};

} // namespace MailTransport

#endif //MAILTRANSPORT_TRANSPORTTYPE_P_H
