/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TABLEFORMATDIALOG_H
#define TABLEFORMATDIALOG_H
#include <KDialog>
#include <QTextLength>

namespace KPIMTextEdit {

class TableFormatDialog : public KDialog
{
public:
  explicit TableFormatDialog(QWidget *parent);
  ~TableFormatDialog();
  int columns() const;
  int rows() const;
  int border() const;

  void setColumns(int);
  void setRows(int);
  void setBorder(int);

  int padding() const;
  void setPadding(int);

  int spacing() const;
  void setSpacing(int);


  void setAlignment(Qt::Alignment alignment);
  Qt::Alignment alignment() const;

  QTextLength::Type typeOfLength() const;
  int length() const;


private:
  class TableFormatDialogPrivate;
  TableFormatDialogPrivate *d;
};
}
#endif // TABLEFORMATDIALOG_H
