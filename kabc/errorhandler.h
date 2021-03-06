/*
    This file is part of libkabc.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KABC_ERRORHANDLER_H
#define KABC_ERRORHANDLER_H

#include "kabc_export.h"
#include <QtCore/QString>

class QWidget;

namespace KABC {

/**
  Abstract class that provides displaying of error messages.
  We need this to make libkabc gui independent on the one side
  and provide user friendly error messages on the other side.
  Use @p ConsoleErrorHandler or @p GuiErrorHandler in your
  application or provide your own ErrorHandler.
*/
class KABC_EXPORT ErrorHandler
{
  public:
    /**
      Destroys the handler instance.
    */
    virtual ~ErrorHandler();

    /**
      Show error message.

      @param msg The error message to show
    */
    virtual void error( const QString &msg ) = 0;
};

/**
  This class prints the error messages to stderr via kError().
*/
class KABC_EXPORT ConsoleErrorHandler : public ErrorHandler
{
  public:
    /**
      Create an error handler for console output.

      Uses kError() to write the error messages.
    */
    ConsoleErrorHandler();

    /**
      Destroys the handler instance.
    */
    virtual ~ConsoleErrorHandler();

    virtual void error( const QString &msg );

  private:
    class Private;
    Private *const d;

    Q_DISABLE_COPY( ConsoleErrorHandler )
};

/**
  This class shows messages boxes for every
  error message.
*/
class KABC_EXPORT GuiErrorHandler : public ErrorHandler
{
  public:
    /**
      Create error handler.

      Uses KMessageBox::error() to display the error messages.

      @param parent Widget which is used as parent for the error dialogs.
    */
    GuiErrorHandler( QWidget *parent = 0 );

    /**
      Destroys the handler instance.
    */
    virtual ~GuiErrorHandler();

    virtual void error( const QString &msg );

  private:
    class Private;
    Private *const d;

    Q_DISABLE_COPY( GuiErrorHandler )
};

}

#endif
