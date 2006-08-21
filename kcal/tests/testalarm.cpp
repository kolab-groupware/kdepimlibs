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
#include <QtTest/QtTest>

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "kcal/event.h"
#include "kcal/alarm.h"
#include "testalarm.moc"

using namespace KCal;

int main( int argc, char *argv[] )
{
  KAboutData aboutData( "qttest", "qttest", "version" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  AlarmTest t;
  return QTest::qExec( &t, argc, argv );
}


void AlarmTest::testValidity() {
  Event inc;
  Alarm alarm( &inc );
  alarm.setType( Alarm::Email );
  QVERIFY( alarm.type() == Alarm::Email );
}

void AlarmTest::testCompare() {
  Event inc1, inc2;
  Alarm alarm1( &inc1 ), alarm2( &inc2 );
  alarm1.setType( Alarm::Email ); alarm2.setType( Alarm::Email );
  QVERIFY( alarm1 == alarm2 );
  alarm2.setType( Alarm::Display );
  QVERIFY( alarm1 != alarm2 );
}
