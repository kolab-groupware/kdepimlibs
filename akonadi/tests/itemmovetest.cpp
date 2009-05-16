/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>

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

#include "test_utils.h"

#include <akonadi/collection.h>
#include <akonadi/collectionselectjob_p.h>
#include <akonadi/control.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemmovejob.h>
#include <akonadi/itemfetchscope.h>

#include <QtCore/QObject>

#include <qtest_akonadi.h>

using namespace Akonadi;

class ItemMoveTest: public QObject
{
  Q_OBJECT
  private slots:
    void initTestCase()
    {
      Control::start();
    }

    // TODO: test inter and intra resource moves
    void testMove_data()
    {
      QTest::addColumn<Item::List>( "items" );

      QTest::newRow( "single uid" ) << (Item::List() << Item( 1 ));
//       QTest::newRow( "two uid" ) << (Item::List() << Item( 2 ) << Item( 3 ));
//       Item r1; r1.setRemoteId( "D" );
//       QTest::newRow( "single rid" ) << (Item::List() << r1);
    }

    void testMove()
    {
      QFETCH( Item::List, items );

      Collection destination( collectionIdFromPath( "res3" ) );
      QVERIFY( destination.isValid() );
      Collection source( collectionIdFromPath( "res1/foo" ) );
      QVERIFY( source.isValid() );

      ItemFetchJob *prefetchjob = new ItemFetchJob( destination, this );
      AKVERIFYEXEC( prefetchjob );
      int baseline = prefetchjob->items().size();

      CollectionSelectJob *select = new CollectionSelectJob( source );
      AKVERIFYEXEC( select ); // for rid based moves
      ItemMoveJob *move = new ItemMoveJob( items, destination, this );
      AKVERIFYEXEC( move );

      ItemFetchJob *fetch = new ItemFetchJob( destination, this );
      fetch->fetchScope().fetchFullPayload();
      AKVERIFYEXEC( fetch );
      QCOMPARE( fetch->items().count(), items.count() + baseline );
      foreach ( const Item& movedItem, fetch->items() ) {
        QVERIFY( movedItem.hasPayload() );
        QVERIFY( !movedItem.payload<QByteArray>().isEmpty() );
      }
    }

    void testIllegalMove()
    {
      Collection col( collectionIdFromPath( "res2" ) );
      QVERIFY( col.isValid() );

      ItemFetchJob *prefetchjob = new ItemFetchJob( Item( 1 ) );
      QVERIFY( prefetchjob->exec() );
      QCOMPARE( prefetchjob->items().count(), 1 );
      Item item = prefetchjob->items()[0];

      // move into invalid collection
      ItemMoveJob *store = new ItemMoveJob( item, Collection( INT_MAX ), this );
      QVERIFY( !store->exec() );

      // move item into folder that doesn't support its mimetype
      store = new ItemMoveJob( item, col, this );
      QEXPECT_FAIL( "", "Check not yet implemented by the server.", Continue );
      QVERIFY( !store->exec() );
    }
};

QTEST_AKONADIMAIN( ItemMoveTest, NoGUI )

#include "itemmovetest.moc"
