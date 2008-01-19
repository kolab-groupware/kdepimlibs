/*
    This file is part of libkabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef RESOURCEFILECONFIG_H
#define RESOURCEFILECONFIG_H

#include "kabc/kabc_export.h"
#include "kresources/configwidget.h"
#include <kcombobox.h>
#include <kurlrequester.h>

namespace KABC {

class KABC_FILE_CORE_EXPORT ResourceFileConfig : public KRES::ConfigWidget
{
  Q_OBJECT

  public:
    ResourceFileConfig( QWidget *parent = 0 );
    void setEditMode( bool value );

  public Q_SLOTS:
    void loadSettings( KRES::Resource *resource );
    void saveSettings( KRES::Resource *resource );

  protected Q_SLOTS:
    void checkFilePermissions( const QString &fileName );

  private:
    KComboBox *mFormatBox;
    KUrlRequester *mFileNameEdit;
    bool mInEditMode;
    QStringList mFormatTypes;
};

}

#endif
