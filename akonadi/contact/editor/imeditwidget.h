/*
    This file is part of KContactManager.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef IMEDITWIDGET_H
#define IMEDITWIDGET_H

#include <QtGui/QWidget>

namespace KABC
{
class Addressee;
}

class KLineEdit;

/**
 * This widget displays an input field for changing
 * the instant messaging id of a contact.
 */
class IMEditWidget : public QWidget
{
  Q_OBJECT

  public:
    IMEditWidget( QWidget *parent = 0 );
    ~IMEditWidget();

    void loadContact( const KABC::Addressee &contact );
    void storeContact( KABC::Addressee &contact ) const;

    void setReadOnly( bool readOnly );

  private:
    KLineEdit *mIMEdit;
};

#endif
