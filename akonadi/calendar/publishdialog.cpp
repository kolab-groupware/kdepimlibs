/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Allen Winter <winter@kde.org>

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

// TODO: validate hand-entered email addresses
// TODO: don't allow duplicates; at least remove dupes when passing back
// TODO: the list in PublishDialog::addresses()

#include "publishdialog.h"

#include <Akonadi/Contact/EmailAddressSelectionDialog>

#include <KCalCore/Attendee>
#include <KCalCore/Person>

#include <KPIMUtils/Email>

#include <KLineEdit>
#include <KLocale>

#include <QTreeView>

PublishDialog::PublishDialog( QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Select Addresses" ) );
  setButtons( Ok|Cancel|Help );
  setHelp( "group-scheduling", "korganizer" );
  QWidget *widget = new QWidget( this );
  widget->setObjectName( "PublishFreeBusy" );
  mUI.setupUi( widget );
  setMainWidget( widget );
  mUI.mListWidget->setSelectionMode( QAbstractItemView::SingleSelection );
  mUI.mNameLineEdit->setEnabled( false );
  mUI.mEmailLineEdit->setEnabled( false );

  setButtonToolTip( Ok, i18n( "Send email to these recipients" ) );
  setButtonWhatsThis( Ok, i18n( "Clicking the <b>Ok</b> button will cause "
                                "an email to be sent to the recipients you "
                                "have entered." ) );
  setButtonToolTip( Cancel, i18n( "Cancel recipient selection and the email" ) );
  setButtonWhatsThis( Cancel, i18n( "Clicking the <b>Cancel</b> button will "
                                    "cause the email operation to be terminated." ) );

  setButtonWhatsThis( Help, i18n( "Click the <b>Help</b> button to read "
                                  "more information about Group Scheduling." ) );

  mUI.mNew->setIcon( KIcon( "list-add" ) );
  mUI.mRemove->setIcon( KIcon( "list-remove" ) );
  mUI.mRemove->setEnabled( false );
  mUI.mSelectAddressee->setIcon( KIcon( "view-pim-contacts" ) );

  connect( mUI.mListWidget, SIGNAL(itemSelectionChanged()),
           SLOT(updateInput()) );
  connect( mUI.mNew, SIGNAL(clicked()),
           SLOT(addItem()) );
  connect( mUI.mRemove, SIGNAL(clicked()),
           SLOT(removeItem()) );
  connect( mUI.mSelectAddressee, SIGNAL(clicked()),
           SLOT(openAddressbook()) );
  connect( mUI.mNameLineEdit, SIGNAL(textChanged(QString)),
           SLOT(updateItem()) );
  connect( mUI.mEmailLineEdit, SIGNAL(textChanged(QString)),
           SLOT(updateItem()) );
}

PublishDialog::~PublishDialog()
{
}

void PublishDialog::addAttendee( const Attendee::Ptr &attendee )
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
  Person person( attendee->name(), attendee->email() );
  item->setText( person.fullName() );
  mUI.mListWidget->addItem( item );
  mUI.mRemove->setEnabled( !mUI.mListWidget->selectedItems().isEmpty() );
}

QString PublishDialog::addresses() const
{
  QString to;
  QListWidgetItem *item;
  const int count = mUI.mListWidget->count();
  for ( int i=0; i<count; ++i ) {
    item = mUI.mListWidget->item( i );
    if( !item->text().isEmpty() ) {
      to += item->text();
      if ( i < count-1 ) {
        to += ", ";
      }
    }
  }
  return to;
}

void PublishDialog::addItem()
{
  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );
  QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
  mUI.mListWidget->addItem( item );
  mUI.mListWidget->setItemSelected( item, true );
  mUI.mNameLineEdit->setClickMessage( i18n( "(EmptyName)" ) );
  mUI.mEmailLineEdit->setClickMessage( i18n( "(EmptyEmail)" ) );

  mUI.mRemove->setEnabled( true );
}

void PublishDialog::removeItem()
{
  if ( mUI.mListWidget->selectedItems().isEmpty() ) {
    return;
  }
  QListWidgetItem *item;
  item = mUI.mListWidget->selectedItems().first();

  int row = mUI.mListWidget->row( item );
  mUI.mListWidget->takeItem( row );

  if ( !mUI.mListWidget->count() ) {
    mUI.mNameLineEdit->setText( QString() );
    mUI.mNameLineEdit->setEnabled( false );
    mUI.mEmailLineEdit->setText( QString() );
    mUI.mEmailLineEdit->setEnabled( false );
    mUI.mRemove->setEnabled( false );
    return;
  }
  if ( row > 0 ) {
    row--;
  }

  mUI.mListWidget->setCurrentRow( row );
}

void PublishDialog::openAddressbook()
{
  QWeakPointer<Akonadi::EmailAddressSelectionDialog> dialog(
    new Akonadi::EmailAddressSelectionDialog( this ) );
  dialog.data()->view()->view()->setSelectionMode( QAbstractItemView::MultiSelection );

  if ( dialog.data()->exec() == QDialog::Accepted ) {

    Akonadi::EmailAddressSelectionDialog *dialogPtr = dialog.data();
    if ( dialogPtr ) {
      const Akonadi::EmailAddressSelection::List selections = dialogPtr->selectedAddresses();
      if ( !selections.isEmpty() ) {
        foreach ( const Akonadi::EmailAddressSelection &selection, selections ) {
          mUI.mNameLineEdit->setEnabled( true );
          mUI.mEmailLineEdit->setEnabled( true );
          QListWidgetItem *item = new QListWidgetItem( mUI.mListWidget );
          mUI.mListWidget->setItemSelected( item, true );
          mUI.mNameLineEdit->setText( selection.name() );
          mUI.mEmailLineEdit->setText( selection.email() );
          mUI.mListWidget->addItem( item );
        }

        mUI.mRemove->setEnabled( true );
      }
    }
  }
  delete dialog.data();
}

void PublishDialog::updateItem()
{
  if ( !mUI.mListWidget->selectedItems().count() ) {
    return;
  }

  Person person( mUI.mNameLineEdit->text(), mUI.mEmailLineEdit->text() );
  QListWidgetItem *item = mUI.mListWidget->selectedItems().first();
  item->setText( person.fullName() );
}

void PublishDialog::updateInput()
{
  if ( !mUI.mListWidget->selectedItems().count() ) {
    return;
  }

  mUI.mNameLineEdit->setEnabled( true );
  mUI.mEmailLineEdit->setEnabled( true );

  QListWidgetItem *item = mUI.mListWidget->selectedItems().first();
  QString mail, name;
  KPIMUtils::extractEmailAddressAndName( item->text(), mail, name );
  mUI.mNameLineEdit->setText( name );
  mUI.mEmailLineEdit->setText( mail );
}

#include "publishdialog.moc"
