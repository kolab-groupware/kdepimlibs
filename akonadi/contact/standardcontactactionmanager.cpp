/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 - 2010 Tobias Koenig <tokoe@kde.org>

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

#include "standardcontactactionmanager.h"

#include "contacteditordialog.h"
#include "contactgroupeditordialog.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/mimetypechecker.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <QtCore/QPointer>
#include <QtGui/QItemSelectionModel>

using namespace Akonadi;

class StandardContactActionManager::Private
{
  public:
    Private( KActionCollection *actionCollection, QWidget *parentWidget, StandardContactActionManager *parent )
      : mActionCollection( actionCollection ), mParentWidget( parentWidget ),
        mCollectionSelectionModel( 0 ), mItemSelectionModel( 0 ), mParent( parent )
    {
      mGenericManager = new StandardActionManager( actionCollection, parentWidget );
      mParent->connect( mGenericManager, SIGNAL( actionStateUpdated() ),
                        mParent, SIGNAL( actionStateUpdated() ) );
      mGenericManager->createAllActions();

      mGenericManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Address Book Folder..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CreateCollection )->setWhatsThis( i18n( "Add a new address book folder to the currently selected address book folder." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Address Book Folder", "Copy %1 Address Book Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CopyCollections )->setWhatsThis( i18n( "Copy the selected address book folders to the clipboard." ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Address Book Folder" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteCollections )->setWhatsThis( i18n( "Delete the selected address book folders from the address book." ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Update Address Book Folder" ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setWhatsThis( i18n( "Update the content of the address book folder" ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CutCollections, ki18np( "Cut Address Book Folder", "Cut %1 Address Book Folders" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CutCollections )->setWhatsThis( i18n( "Cut the selected address book folders from the address book." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Folder Properties..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CollectionProperties)->setWhatsThis( i18n( "Open a dialog to edit the properties of the selected address book folder." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems, ki18np( "Copy Contact", "Copy %1 Contacts" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CopyItems )->setWhatsThis( i18n( "Copy the selected contacts to the clipboard." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete Contact", "Delete %1 Contacts" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteItems )->setWhatsThis( i18n( "Delete the selected contacts from the address book." ) );
      mGenericManager->setActionText( Akonadi::StandardActionManager::CutItems, ki18np( "Cut Contact", "Cut %1 Contacts" ) );
      mGenericManager->action( Akonadi::StandardActionManager::CutItems )->setWhatsThis( i18n( "Cut the selected contacts from the address book." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CreateResource )->setText( i18n( "Add &Address Book..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::CreateResource )->setWhatsThis( i18n( "Add a new address book<p>You will be presented with a dialog where you can select the type of the address book that shall be added.</p>" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteResource )->setText( i18n( "&Delete Address Book" ) );
      mGenericManager->action( Akonadi::StandardActionManager::DeleteResource )->setWhatsThis( i18n( "Delete the selected address book<p>The currently selected address book will be deleted, along with all the contacts and contact groups it contains.</p>" ) );
      mGenericManager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Address Book Properties..." ) );
      mGenericManager->action( Akonadi::StandardActionManager::ResourceProperties )->setWhatsThis( i18n( "Open a dialog to edit properties of the selected address book." ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeResource )->setText( i18n( "Update Address Book" ) );
      mGenericManager->action( Akonadi::StandardActionManager::SynchronizeResource )->setWhatsThis( i18n( "Updates the content of all folders of the address book." ) );

      mGenericManager->setContextText( StandardActionManager::CreateCollection, StandardActionManager::DialogTitle,
                                       i18nc( "@title:window", "New Address Book Folder" ) );
      mGenericManager->setContextText( StandardActionManager::CreateCollection, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not create address book folder: %1" ) );
      mGenericManager->setContextText( StandardActionManager::CreateCollection, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Address book folder creation failed" ) );

      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxText,
                                       i18n( "Do you really want to delete address book folder '%1' and all its sub-folders?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxAlternativeText,
                                       i18n( "Do you really want to delete the address book search folder '%1'?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::MessageBoxTitle,
                                       i18nc( "@title:window", "Delete address book folder?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not delete address book folder: %1" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteCollections, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Address book folder deletion failed" ) );

      mGenericManager->setContextText( StandardActionManager::CollectionProperties, StandardActionManager::DialogTitle,
                                       i18nc( "@title:window", "Properties of Address Book Folder %1" ) );

      mGenericManager->setContextText( StandardActionManager::DeleteItems, StandardActionManager::MessageBoxText,
                                       i18n( "Do you really want to delete all selected contacts?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteItems, StandardActionManager::MessageBoxTitle,
                                       i18nc( "@title:window", "Delete Contacts?" ) );

      mGenericManager->setContextText( StandardActionManager::CreateResource, StandardActionManager::DialogTitle,
                                       i18nc( "@title:window", "Add Address Book" ) );
      mGenericManager->setContextText( StandardActionManager::CreateResource, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not create address book: %1" ) );
      mGenericManager->setContextText( StandardActionManager::CreateResource, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Address book creation failed" ) );

      mGenericManager->setContextText( StandardActionManager::DeleteResource, StandardActionManager::MessageBoxText,
                                       i18n( "Do you really want to delete address book '%1'?" ) );
      mGenericManager->setContextText( StandardActionManager::DeleteResource, StandardActionManager::MessageBoxTitle,
                                       i18nc( "@title:window", "Delete Address Book?" ) );

      mGenericManager->setContextText( StandardActionManager::Paste, StandardActionManager::ErrorMessageText,
                                       i18n( "Could not paste contact: %1" ) );
      mGenericManager->setContextText( StandardActionManager::Paste, StandardActionManager::ErrorMessageTitle,
                                       i18n( "Paste failed" ) );

      mGenericManager->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() << KABC::ContactGroup::mimeType() );
      mGenericManager->setCapabilityFilter( QStringList() << QLatin1String( "Resource" ) );
    }

    ~Private()
    {
      delete mGenericManager;
    }

    static bool hasWritableCollection( const QModelIndex &index, const QString &mimeType )
    {
      const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() ) {
        if ( collection.contentMimeTypes().contains( mimeType ) && (collection.rights() & Akonadi::Collection::CanCreateItem) )
          return true;
      }

      const QAbstractItemModel *model = index.model();
      if ( !model )
        return false;

      for ( int row = 0; row < model->rowCount( index ); ++row ) {
        if ( hasWritableCollection( model->index( row, 0, index ), mimeType ) )
          return true;
      }

      return false;
    }

    bool hasWritableCollection( const QString &mimeType ) const
    {
      if ( !mCollectionSelectionModel )
        return false;

      const QAbstractItemModel *collectionModel = mCollectionSelectionModel->model();
      for ( int row = 0; row < collectionModel->rowCount(); ++row ) {
        if ( hasWritableCollection( collectionModel->index( row, 0, QModelIndex() ), mimeType ) )
          return true;
      }

      return false;
    }

    void updateActions()
    {
      int itemCount = 0;
      if ( mItemSelectionModel ) {
        itemCount = mItemSelectionModel->selectedRows().count();
        if ( itemCount == 1 ) {
          const QModelIndex index = mItemSelectionModel->selectedRows().first();
          if ( index.isValid() ) {
            const QString mimeType = index.data( EntityTreeModel::MimeTypeRole ).toString();
            if ( mimeType == KABC::Addressee::mimeType() ) {
              mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems,
                                              ki18np( "Copy Contact", "Copy %1 Contacts" ) );
              mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems,
                                              ki18np( "Delete Contact", "Delete %1 Contacts" ) );
              mGenericManager->setActionText( Akonadi::StandardActionManager::CutItems,
                                              ki18np( "Cut Contact", "Cut %1 Contacts" ) );
              if ( mActions.contains( StandardContactActionManager::EditItem ) )
                mActions.value( StandardContactActionManager::EditItem )->setText( i18n( "Edit Contact..." ) );
            } else if ( mimeType == KABC::ContactGroup::mimeType() ) {
              mGenericManager->setActionText( Akonadi::StandardActionManager::CopyItems,
                                              ki18np( "Copy Group", "Copy %1 Groups" ) );
              mGenericManager->setActionText( Akonadi::StandardActionManager::DeleteItems,
                                              ki18np( "Delete Group", "Delete %1 Groups" ) );
              mGenericManager->setActionText( Akonadi::StandardActionManager::CutItems,
                                              ki18np( "Cut Group", "Cut %1 Groups" ) );
              if ( mActions.contains( StandardContactActionManager::EditItem ) )
                mActions.value( StandardContactActionManager::EditItem )->setText( i18n( "Edit Group..." ) );
            }
          }
        }
      }

      if ( mActions.contains( StandardContactActionManager::CreateContact ) )
        mActions[ StandardContactActionManager::CreateContact ]->setEnabled( hasWritableCollection( KABC::Addressee::mimeType() ) );
      if ( mActions.contains( StandardContactActionManager::CreateContactGroup ) )
        mActions[ StandardContactActionManager::CreateContactGroup ]->setEnabled( hasWritableCollection( KABC::ContactGroup::mimeType() ) );

      if ( mActions.contains( StandardContactActionManager::EditItem ) ) {
        bool canEditItem = true;

        // only one selected item can be edited
        canEditItem = canEditItem && (itemCount == 1);

        // check whether parent collection allows changing the item
        const QModelIndexList rows = mItemSelectionModel->selectedRows();
        if ( rows.count() == 1 ) {
          const QModelIndex index = rows.first();
          const Collection parentCollection = index.data( EntityTreeModel::ParentCollectionRole ).value<Collection>();
          if ( parentCollection.isValid() )
            canEditItem = canEditItem && (parentCollection.rights() & Collection::CanChangeItem);
        }

        mActions.value( StandardContactActionManager::EditItem )->setEnabled( canEditItem );
      }

      emit mParent->actionStateUpdated();
    }

    Collection selectedCollection() const
    {
      if ( !mCollectionSelectionModel )
        return Collection();

      if ( mCollectionSelectionModel->selectedIndexes().isEmpty() )
        return Collection();

      const QModelIndex index = mCollectionSelectionModel->selectedIndexes().first();
      if ( !index.isValid() )
        return Collection();

      return index.data( EntityTreeModel::CollectionRole).value<Collection>();
    }

    void slotCreateContact()
    {
      if ( mInterceptedActions.contains( StandardContactActionManager::CreateContact ) )
        return;

      Akonadi::ContactEditorDialog dlg( Akonadi::ContactEditorDialog::CreateMode, mParentWidget );
      dlg.setDefaultAddressBook( selectedCollection() );

      dlg.exec();
    }

    void slotCreateContactGroup()
    {
      if ( mInterceptedActions.contains( StandardContactActionManager::CreateContactGroup ) )
        return;

      Akonadi::ContactGroupEditorDialog dlg( Akonadi::ContactGroupEditorDialog::CreateMode, mParentWidget );
      dlg.setDefaultAddressBook( selectedCollection() );

      dlg.exec();
    }

    void slotEditItem()
    {
      if ( mInterceptedActions.contains( StandardContactActionManager::EditItem ) )
        return;

      if ( !mItemSelectionModel )
        return;

      if ( mItemSelectionModel->selectedIndexes().isEmpty() )
        return;

      const QModelIndex index = mItemSelectionModel->selectedIndexes().first();
      if ( !index.isValid() )
        return;

      const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();
      if ( !item.isValid() )
        return;

      if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::Addressee::mimeType() ) ) {
        Akonadi::ContactEditorDialog dlg( Akonadi::ContactEditorDialog::EditMode, mParentWidget );
        dlg.setContact( item );
        dlg.exec();
      }
      else if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::ContactGroup::mimeType() ) ) {
        Akonadi::ContactGroupEditorDialog dlg( Akonadi::ContactGroupEditorDialog::EditMode, mParentWidget );
        dlg.setContactGroup( item );
        dlg.exec();
      }
    }

    KActionCollection *mActionCollection;
    QWidget *mParentWidget;
    StandardActionManager *mGenericManager;
    QItemSelectionModel *mCollectionSelectionModel;
    QItemSelectionModel *mItemSelectionModel;
    QHash<StandardContactActionManager::Type, KAction*> mActions;
    QSet<StandardContactActionManager::Type> mInterceptedActions;
    StandardContactActionManager *mParent;
};

StandardContactActionManager::StandardContactActionManager( KActionCollection *actionCollection, QWidget *parent )
  : QObject( parent ), d( new Private( actionCollection, parent, this ) )
{
}

StandardContactActionManager::~StandardContactActionManager()
{
  delete d;
}

void StandardContactActionManager::setCollectionSelectionModel( QItemSelectionModel *selectionModel )
{
  d->mCollectionSelectionModel = selectionModel;
  d->mGenericManager->setCollectionSelectionModel( selectionModel );

  connect( selectionModel->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
           SLOT( updateActions() ) );
  connect( selectionModel->model(), SIGNAL( rowsRemoved( const QModelIndex&, int, int ) ),
           SLOT( updateActions() ) );
  connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( updateActions() ) );

  d->updateActions();
}

void StandardContactActionManager::setItemSelectionModel( QItemSelectionModel* selectionModel )
{
  d->mItemSelectionModel = selectionModel;
  d->mGenericManager->setItemSelectionModel( selectionModel );

  connect( selectionModel, SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( updateActions() ) );

  d->updateActions();
}

KAction* StandardContactActionManager::createAction( Type type )
{
  if ( d->mActions.contains( type ) )
    return d->mActions.value( type );

  KAction *action = 0;

  switch ( type ) {
    case CreateContact:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "contact-new" ) ) );
      action->setText( i18n( "New &Contact..." ) );
      action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
      action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add data about a person, including addresses and phone numbers.</p>" ) );
      d->mActions.insert( CreateContact, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_create" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotCreateContact() ) );
      break;
    case CreateContactGroup:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "user-group-new" ) ) );
      action->setText( i18n( "New &Group..." ) );
      action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
      action->setWhatsThis( i18n( "Create a new group<p>You will be presented with a dialog where you can add a new group of contacts.</p>" ) );
      d->mActions.insert( CreateContactGroup, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_group_create" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotCreateContactGroup() ) );
      break;
    case EditItem:
      action = new KAction( d->mParentWidget );
      action->setIcon( KIcon( QLatin1String( "document-edit" ) ) );
      action->setText( i18n( "Edit Contact..." ) );
      action->setWhatsThis( i18n( "Edit the selected contact<p>You will be presented with a dialog where you can edit the data stored about a person, including addresses and phone numbers.</p>" ) );
      action->setEnabled( false );
      d->mActions.insert( EditItem, action );
      d->mActionCollection->addAction( QString::fromLatin1( "akonadi_contact_item_edit" ), action );
      connect( action, SIGNAL( triggered( bool ) ), this, SLOT( slotEditItem() ) );
      break;
    default:
      Q_ASSERT( false ); // should never happen
      break;
  }

  return action;
}

KAction* StandardContactActionManager::createAction( StandardActionManager::Type type )
{
  return d->mGenericManager->createAction( type );
}

void StandardContactActionManager::createAllActions()
{
  createAction( CreateContact );
  createAction( CreateContactGroup );
  createAction( EditItem );

  d->mGenericManager->createAllActions();

  d->updateActions();
}

KAction* StandardContactActionManager::action( Type type ) const
{
  if ( d->mActions.contains( type ) )
    return d->mActions.value( type );

  return 0;
}

KAction* StandardContactActionManager::action( StandardActionManager::Type type ) const
{
  return d->mGenericManager->action( type );
}

void StandardContactActionManager::interceptAction( Type type, bool intercept )
{
  if ( intercept )
    d->mInterceptedActions.insert( type );
  else
    d->mInterceptedActions.remove( type );
}

void StandardContactActionManager::interceptAction( StandardActionManager::Type type, bool intercept )
{
  d->mGenericManager->interceptAction( type, intercept );
}

Akonadi::Collection::List StandardContactActionManager::selectedCollections() const
{
  return d->mGenericManager->selectedCollections();
}

Akonadi::Item::List StandardContactActionManager::selectedItems() const
{
  return d->mGenericManager->selectedItems();
}

#include "standardcontactactionmanager.moc"
