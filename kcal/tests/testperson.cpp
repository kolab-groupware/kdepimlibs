/*
  This file is part of the kcal library.
  Copyright (C) 2006 Allen Winter <winter@kde.org>

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
#include <qtest_kde.h>

#include "testperson.h"
#include "testperson.moc"

QTEST_KDEMAIN( PersonTest, NoGUI )

#include "kcal/person.h"
using namespace KCal;

void PersonTest::testValidity() {
  Person person( "fred", "fred@flintstone.com" );
  QVERIFY( person.name() == "fred" );
}

void PersonTest::testCompare() {
  Person person1( "fred", "fred@flintstone.com" );
  Person person2( "wilma", "wilma@flintstone.com" );
  Person person3( "fred <fred@flintstone.com>" );
  QVERIFY( !( person1 == person2 ) );
  QVERIFY( person1 == person3 );
  QVERIFY( person1.name() == "fred" );
  QVERIFY( person2.email() == "wilma@flintstone.com" );
  QVERIFY( person3.name() == "fred" );
  QVERIFY( person3.email() == "fred@flintstone.com" );
}
