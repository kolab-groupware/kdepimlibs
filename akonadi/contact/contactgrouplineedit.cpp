/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "contactgrouplineedit_p.h"

#include "contactcompletionmodel_p.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <klocalizedstring.h>

#include <QtCore/QAbstractItemModel>
#include <QAction>
#include <QCompleter>
#include <QMenu>

ContactGroupLineEdit::ContactGroupLineEdit( QWidget *parent )
  : KLineEdit( parent ),
    mCompleter( 0 ),
    mContainsReference( false )
{
  setClearButtonShown( true );
}

void ContactGroupLineEdit::setCompletionModel( QAbstractItemModel *model )
{
  mCompleter = new QCompleter( model, this );
  mCompleter->setCompletionColumn( Akonadi::ContactCompletionModel::NameAndEmailColumn );
  connect( mCompleter, SIGNAL(activated(QModelIndex)),
           this, SLOT(autoCompleted(QModelIndex)) );

  setCompleter( mCompleter );
}

bool ContactGroupLineEdit::containsReference() const
{
  return mContainsReference;
}

void ContactGroupLineEdit::setContactData( const KABC::ContactGroup::Data &groupData )
{
  mContactData = groupData;
  mContainsReference = false;

  setText( QString::fromLatin1( "%1 <%2>" ).arg( groupData.name() ).arg( groupData.email() ) );
}

KABC::ContactGroup::Data ContactGroupLineEdit::contactData() const
{
  QString fullName, email;
  KABC::Addressee::parseEmailAddress( text(), fullName, email );

  if ( fullName.isEmpty() || email.isEmpty() ) {
    return KABC::ContactGroup::Data();
  }

  KABC::ContactGroup::Data groupData( mContactData );
  groupData.setName( fullName );
  groupData.setEmail( email );

  return groupData;
}

void ContactGroupLineEdit::setContactReference( const KABC::ContactGroup::ContactReference &reference )
{
  mContactReference = reference;
  mContainsReference = true;

  disconnect( this, SIGNAL(textChanged(QString)), this, SLOT(invalidateReference()) );

  updateView( reference );
}

KABC::ContactGroup::ContactReference ContactGroupLineEdit::contactReference() const
{
  return mContactReference;
}

void ContactGroupLineEdit::autoCompleted( const QModelIndex &index )
{
  if ( !index.isValid() ) {
    return;
  }

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !item.isValid() ) {
    return;
  }

  disconnect( this, SIGNAL(textChanged(QString)), this, SLOT(invalidateReference()) );
  mContainsReference = true;

  updateView( item );

  connect( this, SIGNAL(textChanged(QString)), SLOT(invalidateReference()) );
}

void ContactGroupLineEdit::invalidateReference()
{
  disconnect( this, SIGNAL(textChanged(QString)), this, SLOT(invalidateReference()) );
  mContainsReference = false;
}

void ContactGroupLineEdit::updateView( const KABC::ContactGroup::ContactReference &reference )
{
  Akonadi::Item item;
  if ( !reference.gid().isEmpty() ) {
    item.setGid( reference.gid() );
  } else {
    item.setId( reference.uid().toLongLong() );
  }
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item );
  job->fetchScope().fetchFullPayload();
  job->setProperty( "preferredEmail", reference.preferredEmail() );
  connect( job, SIGNAL(result(KJob*)), SLOT(fetchDone(KJob*)) );
}

void ContactGroupLineEdit::fetchDone( KJob *job )
{
  Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );

  if ( !fetchJob->items().isEmpty() ) {
    const Akonadi::Item item = fetchJob->items().first();
    updateView( item, fetchJob->property( "preferredEmail" ).toString() );
  }

  connect( this, SIGNAL(textChanged(QString)), SLOT(invalidateReference()) );
}

void ContactGroupLineEdit::updateView( const Akonadi::Item &item, const QString &preferredEmail )
{
  if ( !item.hasPayload<KABC::Addressee>() ) {
    return;
  }

  const KABC::Addressee contact = item.payload<KABC::Addressee>();

  QString email( preferredEmail );
  if ( email.isEmpty() ) {
    email = requestPreferredEmail( contact );
  }

  QString name = contact.formattedName();
  if ( name.isEmpty() ) {
    name = contact.assembledName();
  }

  if ( email.isEmpty() ) {
    setText( QString::fromLatin1( "%1" ).arg( name ) );
  } else {
    setText( QString::fromLatin1( "%1 <%2>" ).arg( name ).arg( email ) );
  }

  mContactReference.setGid( contact.uid() );
  mContactReference.setUid( QString::number( item.id() ) );

  if ( contact.preferredEmail() != email ) {
    mContactReference.setPreferredEmail( email );
  }
}

QString ContactGroupLineEdit::requestPreferredEmail( const KABC::Addressee &contact ) const
{
  const QStringList emails = contact.emails();

  if ( emails.isEmpty() ) {
    qDebug( "ContactGroupLineEdit::requestPreferredEmail(): Warning!!! no email addresses available" );
    return QString();
  }

  if ( emails.count() == 1 ) {
    return emails.first();
  }

  QAction *action = 0;

  QMenu menu;
  menu.setTitle( i18n( "Select preferred email address" ) );
  const int numberOfEmails( emails.count() );
  for ( int i = 0; i < numberOfEmails; ++i ) {
    action = menu.addAction( emails.at( i ) );
    action->setData( i );
  }

  action = menu.exec( mapToGlobal( QPoint( x() + width()/2, y() + height()/2 ) ) );
  if ( !action ) {
    return emails.first(); // use preferred email
  }

  return emails.at( action->data().toInt() );
}

#include "moc_contactgrouplineedit_p.cpp"
