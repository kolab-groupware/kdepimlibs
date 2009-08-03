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
#include "protocolhelper.cpp"

using namespace Akonadi;

class ProtocolHelperTest : public QObject
{
  Q_OBJECT
  private slots:
    void testItemSetToByteArray_data()
    {
      QTest::addColumn<Item::List>( "items" );
      QTest::addColumn<QByteArray>( "result" );
      QTest::addColumn<bool>( "shouldThrow" );

      Item u1; u1.setId( 1 );
      Item u2; u2.setId( 2 );
      Item u3; u3.setId( 3 );
      Item r1; r1.setRemoteId( "A" );
      Item r2; r2.setRemoteId( "B" );

      QTest::newRow( "empty" ) << Item::List() << QByteArray() << true;
      QTest::newRow( "single uid" ) << (Item::List() << u1) << QByteArray( " UID CMD 1" ) << false;
      QTest::newRow( "multi uid" ) << (Item::List() << u1 << u3) << QByteArray( " UID CMD 1,3" ) << false;
      QTest::newRow( "block uid" ) << (Item::List() << u1 << u2 << u3) << QByteArray( " UID CMD 1:3" ) << false;
      QTest::newRow( "single rid" ) << (Item::List() << r1) << QByteArray( " RID CMD (\"A\")" ) << false;
      QTest::newRow( "multi rid" ) << (Item::List() << r1 << r2) << QByteArray( " RID CMD (\"A\" \"B\")" ) << false;
      QTest::newRow( "invalid" ) << (Item::List() << Item()) << QByteArray() << true;
      QTest::newRow( "mixed" ) << (Item::List() << u1 << r1) << QByteArray() << true;
    }

    void testItemSetToByteArray()
    {
      QFETCH( Item::List, items );
      QFETCH( QByteArray, result );
      QFETCH( bool, shouldThrow );

      bool didThrow = false;
      try {
        const QByteArray r = ProtocolHelper::itemSetToByteArray( items, "CMD" );
        QCOMPARE( r, result );
      } catch ( const std::exception &e ) {
        qDebug() << e.what();
        didThrow = true;
      }
      QCOMPARE( didThrow, shouldThrow );
    }

    void testCollectionParsing_data()
    {
      QTest::addColumn<QByteArray>( "input" );
      QTest::addColumn<Collection>( "collection" );

      const QByteArray b1 = "2 1 (REMOTEID \"r2\" NAME \"n2\")";
      Collection c1;
      c1.setId( 2 );
      c1.setRemoteId( "r2" );
      c1.parentCollection().setId( 1 );
      c1.setName( "n2" );
      QTest::newRow( "no ancestors" ) << b1 << c1;

      const QByteArray b2 = "3 2 (REMOTEID \"r3\" ANCESTORS ((2 \"r2\") (1 \"r1\") (0 \"\")))";
      Collection c2;
      c2.setId( 3 );
      c2.setRemoteId( "r3" );
      c2.parentCollection().setId( 2 );
      c2.parentCollection().setRemoteId( "r2" );
      c2.parentCollection().parentCollection().setId( 1 );
      c2.parentCollection().parentCollection().setRemoteId( "r1" );
      c2.parentCollection().parentCollection().setParentCollection( Collection::root() );
      QTest::newRow( "ancestors" ) << b2 << c2;
    }

    void testCollectionParsing()
    {
      QFETCH( QByteArray, input );
      QFETCH( Collection, collection );

      Collection parsedCollection;
      ProtocolHelper::parseCollection( input, parsedCollection );

      QCOMPARE( parsedCollection.name(), collection.name() );

      while ( collection.isValid() || parsedCollection.isValid() ) {
        QCOMPARE( parsedCollection.id(), collection.id() );
        QCOMPARE( parsedCollection.remoteId(), collection.remoteId() );
        const Collection p1( parsedCollection.parentCollection() );
        const Collection p2( collection.parentCollection() );
        parsedCollection = p1;
        collection = p2;
      }
    }
};

QTEST_KDEMAIN( ProtocolHelperTest, NoGUI )

#include "protocolhelpertest.moc"
