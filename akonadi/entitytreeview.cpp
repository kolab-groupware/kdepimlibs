/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "entitytreeview.h"

#include "dragdropmanager_p.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>

#include <KAction>
#include <KLocale>
#include <KMessageBox>
#include <KUrl>
#include <KXMLGUIFactory>

#include <akonadi/collection.h>
#include <akonadi/control.h>
#include <akonadi/item.h>
#include <akonadi/entitytreemodel.h>

#include <kdebug.h>
#include <kxmlguiclient.h>

using namespace Akonadi;

/**
 * @internal
 */
class EntityTreeView::Private
{
public:
  Private( EntityTreeView *parent )
      : mParent( parent ), mDragDropManager( new DragDropManager( mParent ) ), mXmlGuiClient( 0 )
  {
  }

  void init();
  void itemClicked( const QModelIndex& );
  void itemDoubleClicked( const QModelIndex& );
  void itemCurrentChanged( const QModelIndex& );

  void slotSelectionChanged( const QItemSelection & selected, const QItemSelection & deselected );

  EntityTreeView *mParent;
  QBasicTimer mDragExpandTimer;
  DragDropManager *mDragDropManager;
  KXMLGUIClient *mXmlGuiClient;
};

void EntityTreeView::Private::init()
{
  mParent->header()->setClickable( true );
  mParent->header()->setStretchLastSection( false );
//   mParent->setRootIsDecorated( false );

  // QTreeView::autoExpandDelay has very strange behaviour. It toggles the collapse/expand state
  // of the item the cursor is currently over when a timer event fires.
  // The behaviour we want is to expand a collapsed row on drag-over, but not collapse it.
  // mDragExpandTimer is used to achieve this.
//   mParent->setAutoExpandDelay ( QApplication::startDragTime() );

  mParent->setSortingEnabled( true );
  mParent->sortByColumn( 0, Qt::AscendingOrder );
  mParent->setEditTriggers( QAbstractItemView::EditKeyPressed );
  mParent->setAcceptDrops( true );
  mParent->setDropIndicatorShown( true );
  mParent->setDragDropMode( DragDrop );
  mParent->setDragEnabled( true );

  mParent->connect( mParent, SIGNAL( clicked( const QModelIndex& ) ),
                    mParent, SLOT( itemClicked( const QModelIndex& ) ) );
  mParent->connect( mParent, SIGNAL( doubleClicked( const QModelIndex& ) ),
                    mParent, SLOT( itemDoubleClicked( const QModelIndex& ) ) );

  Control::widgetNeedsAkonadi( mParent );
}

void EntityTreeView::Private::slotSelectionChanged( const QItemSelection & selected, const QItemSelection& )
{
  const int column = 0;
  foreach ( const QItemSelectionRange &range, selected ) {
    const QModelIndex index = range.topLeft();

    if ( index.column() > 0 )
      continue;

    for ( int row = index.row(); row <= range.bottomRight().row(); ++row ) {
      // Don't use canFetchMore here. We need to bypass the check in
      // the EntityFilterModel when it shows only collections.
      mParent->model()->fetchMore( index.sibling( row, column ) );
    }
  }
}

void EntityTreeView::Private::itemClicked( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const Collection collection = index.model()->data( index, EntityTreeModel::CollectionRole ).value<Collection>();
  if ( collection.isValid() ) {
    emit mParent->clicked( collection );
  } else {
    const Item item = index.model()->data( index, EntityTreeModel::ItemRole ).value<Item>();
    if ( item.isValid() )
      emit mParent->clicked( item );
  }
}

void EntityTreeView::Private::itemDoubleClicked( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const Collection collection = index.model()->data( index, EntityTreeModel::CollectionRole ).value<Collection>();
  if ( collection.isValid() ) {
    emit mParent->doubleClicked( collection );
  } else {
    const Item item = index.model()->data( index, EntityTreeModel::ItemRole ).value<Item>();
    if ( item.isValid() )
      emit mParent->doubleClicked( item );
  }
}

void EntityTreeView::Private::itemCurrentChanged( const QModelIndex &index )
{
  if ( !index.isValid() )
    return;

  const Collection collection = index.model()->data( index, EntityTreeModel::CollectionRole ).value<Collection>();
  if ( collection.isValid() ) {
    emit mParent->currentChanged( collection );
  } else {
    const Item item = index.model()->data( index, EntityTreeModel::ItemRole ).value<Item>();
    if ( item.isValid() )
      emit mParent->currentChanged( item );
  }
}

EntityTreeView::EntityTreeView( QWidget * parent )
  : QTreeView( parent ),
    d( new Private( this ) )
{
  setSelectionMode( QAbstractItemView::SingleSelection );
  d->init();
}

EntityTreeView::EntityTreeView( KXMLGUIClient *xmlGuiClient, QWidget * parent )
  : QTreeView( parent ),
    d( new Private( this ) )
{
  d->mXmlGuiClient = xmlGuiClient;
  d->init();
}

EntityTreeView::~EntityTreeView()
{
  delete d->mDragDropManager;
  delete d;
}

void EntityTreeView::setModel( QAbstractItemModel * model )
{
  if ( selectionModel() ) {
    disconnect( selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( itemCurrentChanged( const QModelIndex& ) ) );

    disconnect( selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( slotSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
  }

  QTreeView::setModel( model );
  header()->setStretchLastSection( true );

  connect( selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           SLOT( itemCurrentChanged( const QModelIndex& ) ) );

  connect( selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           SLOT( slotSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
}


void EntityTreeView::timerEvent( QTimerEvent *event )
{
  if ( event->timerId() == d->mDragExpandTimer.timerId() ) {
    const QPoint pos = viewport()->mapFromGlobal( QCursor::pos() );
    if ( state() == QAbstractItemView::DraggingState && viewport()->rect().contains( pos ) )
      setExpanded( indexAt( pos ), true );
  }

  QTreeView::timerEvent( event );
}

void EntityTreeView::dragMoveEvent( QDragMoveEvent * event )
{
  d->mDragExpandTimer.start( QApplication::startDragTime() , this );

  if ( d->mDragDropManager->dropAllowed( event ) ) {
    // All urls are supported. process the event.
    QTreeView::dragMoveEvent( event );
    return;
  }

  event->setDropAction( Qt::IgnoreAction );
  return;
}

void EntityTreeView::dropEvent( QDropEvent * event )
{
  if ( d->mDragDropManager->processDropEvent( event ) )
    QTreeView::dropEvent( event );
}

void EntityTreeView::contextMenuEvent( QContextMenuEvent * event )
{
  if ( !d->mXmlGuiClient || !model())
    return;

  const QModelIndex index = indexAt( event->pos() );

  QMenu *popup = 0;

  // check if the index under the cursor is a collection or item
  const Item item = model()->data( index, EntityTreeModel::ItemRole ).value<Item>();
  if ( item.isValid() )
    popup = static_cast<QMenu*>( d->mXmlGuiClient->factory()->container(
                                 QLatin1String( "akonadi_itemview_contextmenu" ), d->mXmlGuiClient ) );
  else
    popup = static_cast<QMenu*>( d->mXmlGuiClient->factory()->container(
                                 QLatin1String( "akonadi_collectionview_contextmenu" ), d->mXmlGuiClient ) );
  if ( popup )
    popup->exec( event->globalPos() );
}

void EntityTreeView::setXmlGuiClient( KXMLGUIClient * xmlGuiClient )
{
  d->mXmlGuiClient = xmlGuiClient;
}

void EntityTreeView::startDrag( Qt::DropActions supportedActions )
{
  d->mDragDropManager->startDrag( supportedActions );
}


void EntityTreeView::setShowDropActionMenu( bool show )
{
  d->mDragDropManager->setShowDropActionMenu( show );
}

bool EntityTreeView::showDropActionMenu() const
{
  return d->mDragDropManager->showDropActionMenu();
}

#include "entitytreeview.moc"
