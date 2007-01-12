/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_CONTROL_H
#define AKONADI_CONTROL_H

#include <QtCore/QObject>

#include <kdepim_export.h>

class QEventLoop;

namespace Akonadi {

/**
  This class provides methods to control the Akonadi server.
*/
class AKONADI_EXPORT Control : public QObject
{
  Q_OBJECT
  public:
    /**
      Starts the Akonadi server synchronously if necessary.
    */
    static void start();

  private:
    Control();
    static Control* self();
    void startInternal();

  private slots:
    void serviceOwnerChanged( const QString &name, const QString &oldOwner, const QString &newOwner );

  private:
    static Control* mInstance;
    QEventLoop *mEventLoop;

};

}

#endif
