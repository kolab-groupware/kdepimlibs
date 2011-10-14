/*
    Copyright (c) 2008 Thomas McGuire <thomas.mcguire@gmx.net>

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

#include "collectionstatisticsdelegate.h"
#include "collectionstatisticsmodel.h"

#include <kcolorscheme.h>
#include <kdebug.h>

#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QStyleOptionViewItemV4>
#include <QtGui/QAbstractItemView>
#include <QtGui/QTreeView>

#include "entitytreemodel.h"
#include "collectionstatistics.h"
#include "collection.h"
#include "progressspinnerdelegate_p.h"

using namespace Akonadi;

namespace Akonadi {

enum CountType
{
  UnreadCount,
  TotalCount
};

class CollectionStatisticsDelegatePrivate
{
  public:
    QAbstractItemView *parent;
    bool drawUnreadAfterFolder;
    DelegateAnimator *animator;
    QColor mSelectedUnreadColor;
    QColor mDeselectedUnreadColor;

    CollectionStatisticsDelegatePrivate( QAbstractItemView *treeView )
        : parent( treeView ),
          drawUnreadAfterFolder( false ),
          animator( 0 )
    {
      mSelectedUnreadColor = KColorScheme( QPalette::Active, KColorScheme::Selection )
                                         .foreground( KColorScheme::LinkText ).color();
      mDeselectedUnreadColor = KColorScheme( QPalette::Active, KColorScheme::View )
                                           .foreground( KColorScheme::LinkText ).color();
    }

    void getCountRecursive( const QModelIndex &index, qint64 &totalCount, qint64 &unreadCount ) const
    {
      Collection collection = qvariant_cast<Collection>( index.data( EntityTreeModel::CollectionRole ) );
      Q_ASSERT( collection.isValid() );
      CollectionStatistics statistics = collection.statistics();
      totalCount += qMax( 0LL, statistics.count() );
      unreadCount += qMax( 0LL, statistics.unreadCount() );

      if ( index.model()->hasChildren( index ) )
      {
        const int rowCount = index.model()->rowCount( index );
        for ( int row = 0; row < rowCount; row++ )
        {
          static const int column = 0;
          getCountRecursive( index.model()->index( row, column, index ), totalCount, unreadCount );
        }
      }
    }
};

}

CollectionStatisticsDelegate::CollectionStatisticsDelegate( QAbstractItemView *parent )
  : QStyledItemDelegate( parent ),
    d_ptr( new CollectionStatisticsDelegatePrivate( parent ) )
{

}

CollectionStatisticsDelegate::CollectionStatisticsDelegate( QTreeView *parent )
  : QStyledItemDelegate( parent ),
    d_ptr( new CollectionStatisticsDelegatePrivate( parent ) )
{

}

CollectionStatisticsDelegate::~CollectionStatisticsDelegate()
{
  delete d_ptr;
}

void CollectionStatisticsDelegate::setUnreadCountShown( bool enable )
{
  Q_D( CollectionStatisticsDelegate );
  d->drawUnreadAfterFolder = enable;
}

bool CollectionStatisticsDelegate::unreadCountShown() const
{
  Q_D( const CollectionStatisticsDelegate );
  return d->drawUnreadAfterFolder;
}

void CollectionStatisticsDelegate::setProgressAnimationEnabled( bool enable )
{
  Q_D( CollectionStatisticsDelegate );
  if ( enable == ( d->animator != 0 ) )
      return;
  if ( enable ) {
    Q_ASSERT( !d->animator );
    Akonadi::DelegateAnimator *animator = new Akonadi::DelegateAnimator( d->parent );
    d->animator = animator;
  } else {
    delete d->animator;
    d->animator = 0;
  }
}

bool CollectionStatisticsDelegate::progressAnimationEnabled() const
{
  Q_D( const CollectionStatisticsDelegate );
  return d->animator != 0;
}

void CollectionStatisticsDelegate::initStyleOption( QStyleOptionViewItem *option,
                                                    const QModelIndex &index ) const
{
  Q_D( const CollectionStatisticsDelegate );

  QStyleOptionViewItemV4 *noTextOption =
      qstyleoption_cast<QStyleOptionViewItemV4 *>( option );
  QStyledItemDelegate::initStyleOption( noTextOption, index );
  if ( option->decorationPosition != QStyleOptionViewItem::Top ) {
    noTextOption->text.clear();
  }

  if ( d->animator ) {

    const Akonadi::Collection collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

    if (!collection.isValid())
    {
      d->animator->pop(index);
      return;
    }

    if (index.data(Akonadi::EntityTreeModel::FetchStateRole).toInt() != Akonadi::EntityTreeModel::FetchingState)
    {
      d->animator->pop(index);
      return;
    }

    d->animator->push(index);

    if (QStyleOptionViewItemV4 *v4 = qstyleoption_cast<QStyleOptionViewItemV4 *>(option)) {
      v4->icon = d->animator->sequenceFrame(index);
    }
  }
}

class PainterStateSaver
{
  public:
    PainterStateSaver( QPainter *painter )
    {
      mPainter = painter;
      mPainter->save();
    }

    ~PainterStateSaver()
    {
      mPainter->restore();
    }

  private:
    QPainter *mPainter;
};

void CollectionStatisticsDelegate::paint( QPainter *painter,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index ) const
{
  Q_D( const CollectionStatisticsDelegate );
  PainterStateSaver stateSaver( painter );

  // First, paint the basic, but without the text. We remove the text
  // in initStyleOption(), which gets called by QStyledItemDelegate::paint().
  QStyledItemDelegate::paint( painter, option, index );

  // No, we retrieve the correct style option by calling intiStyleOption from
  // the superclass.
  QStyleOptionViewItemV4 option4 = option;
  QStyledItemDelegate::initStyleOption( &option4, index );
  QString text = option4.text;

  // Now calculate the rectangle for the text
  QStyle *s = d->parent->style();
  const QWidget *widget = option4.widget;
  const QRect textRect = s->subElementRect( QStyle::SE_ItemViewItemText, &option4, widget );
  const QRect iconRect = s->subElementRect( QStyle::SE_ItemViewItemDecoration, &option4, widget );

   // When checking if the item is expanded, we need to check that for the first
  // column, as Qt only recogises the index as expanded for the first column
  QModelIndex firstColumn = index.model()->index( index.row(), 0, index.parent() );
  QTreeView* treeView = qobject_cast<QTreeView*>( d->parent );
  bool expanded = treeView && treeView->isExpanded( firstColumn );

  if ( option.state & QStyle::State_Selected ) {
    painter->setPen( option.palette.highlightedText().color() );
  }

  Collection collection = index.sibling( index.row(), 0 ).data( EntityTreeModel::CollectionRole ).value<Collection>();

  Q_ASSERT(collection.isValid());

  CollectionStatistics statistics = collection.statistics();

  qint64 unreadCount = qMax( 0LL, statistics.unreadCount() );
  qint64 totalRecursiveCount = 0;
  qint64 unreadRecursiveCount = 0;
  d->getCountRecursive( index.sibling( index.row(), 0 ), totalRecursiveCount, unreadRecursiveCount );

  // Draw the unread count after the folder name (in parenthesis)
  if ( d->drawUnreadAfterFolder && index.column() == 0 ) {
    // Construct the string which will appear after the foldername (with the
    // unread count)
    QString unread;
//     qDebug() << expanded << unreadCount << unreadRecursiveCount;
    if ( expanded && unreadCount > 0 )
      unread = QString::fromLatin1( " (%1)" ).arg( unreadCount );
    else if ( !expanded ) {
      if ( unreadCount != unreadRecursiveCount )
        unread = QString::fromLatin1( " (%1 + %2)" ).arg( unreadCount ).arg( unreadRecursiveCount - unreadCount );
      else if ( unreadCount > 0 )
        unread = QString::fromLatin1( " (%1)" ).arg( unreadCount );
    }

    PainterStateSaver stateSaver( painter );

    if ( !unread.isEmpty() ) {
      QFont font = painter->font();
      font.setBold( true );
      painter->setFont( font );
    }

    const QColor unreadColor = (option.state & QStyle::State_Selected) ? d->mSelectedUnreadColor : d->mDeselectedUnreadColor;

    if ( option.decorationPosition == QStyleOptionViewItem::Left
         || option.decorationPosition == QStyleOptionViewItem::Right ) {
      // Squeeze the folder text if it is to big and calculate the rectangles
      // where the folder text and the unread count will be drawn to
      QString folderName = text;
      QFontMetrics fm( painter->fontMetrics() );
      int unreadWidth = fm.width( unread );
      int folderWidth( fm.width( folderName ) );
      if ( folderWidth + unreadWidth > textRect.width() ) {
        folderName = fm.elidedText( folderName, Qt::ElideRight,
                                   textRect.width() - unreadWidth );
        folderWidth = fm.width( folderName );
      }
      QRect folderRect = textRect;
      QRect unreadRect = textRect;
      folderRect.setRight( textRect.left() + folderWidth );
      unreadRect.setLeft( folderRect.right() );

      // Draw folder name and unread count
      painter->drawText( folderRect, Qt::AlignLeft | Qt::AlignVCenter, folderName );
      painter->setPen( unreadColor );
      painter->drawText( unreadRect, Qt::AlignLeft | Qt::AlignVCenter, unread );
    } else if ( option.decorationPosition == QStyleOptionViewItem::Top ) {
      // draw over the icon
      painter->setPen( unreadColor );
      if ( unreadCount > 0 ) {
        painter->drawText( iconRect, Qt::AlignCenter, QString::number( unreadCount ) );
      }
    }
    return;
  }

  // For the unread/total column, paint the summed up count if the item
  // is collapsed
  if ( ( index.column() == 1 || index.column() == 2 ) ) {

    QFont savedFont = painter->font();
    QString sumText;
    if ( index.column() == 1 && ( ( !expanded && unreadRecursiveCount > 0 ) || ( expanded && unreadCount > 0 ) ) ) {
      QFont font = painter->font();
      font.setBold( true );
      painter->setFont( font );
      sumText = QString::number( expanded ? unreadCount : unreadRecursiveCount );
    } else {

      qint64 totalCount = statistics.count();
      if (index.column() == 2 && ( ( !expanded && totalRecursiveCount > 0 ) || ( expanded && totalCount > 0 ) ) ) {
        sumText = QString::number( expanded ? totalCount : totalRecursiveCount );
      }
    }

    painter->drawText( textRect, Qt::AlignRight | Qt::AlignVCenter, sumText );
    painter->setFont( savedFont );
    return;
  }

  painter->drawText( textRect, option4.displayAlignment | Qt::AlignVCenter, text );
}

#include "collectionstatisticsdelegate.moc"
