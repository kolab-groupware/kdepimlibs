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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    $Id$
*/

#ifndef RESOURCECONFIGDLG_H
#define RESOURCECONFIGDLG_H

#include <qcheckbox.h>
#include <qdialog.h>

#include <kbuttonbox.h>
#include <kconfig.h>
#include <klineedit.h>

#include "resourceconfigwidget.h"

class ResourceConfigDlg : QDialog
{
    Q_OBJECT
public:
    ResourceConfigDlg( QWidget *parent, const QString& type,
	    KConfig *config, const char *name = 0);

    KButtonBox *buttonBox;
    KLineEdit *resourceName;
    QCheckBox *resourceIsReadOnly;
    QCheckBox *resourceIsFast;

public slots:
    int exec();

protected slots:
    void accept();

private:
    ResourceConfigWidget *mConfigWidget;
    KConfig *mConfig;
};

#endif
