/*
    Copyright (c) 2008 Thomas McGuire <mcguire@kde.org>

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
#include "qtest_kde.h"
#include "identitytest.h"
#include "../identitymanager.h"
#include "../identity.h"

#include <KConfig>
#include <KConfigGroup>

using namespace KPIMIdentities;

QTEST_KDEMAIN_CORE( IdentityTester )

void IdentityTester::test_NullIdentity()
{
  IdentityManager manager;
  QVERIFY( manager.identityForAddress( QLatin1String("thisaddressforsuredoesnotexist@kde.org") ).isNull() );
}

void IdentityTester::test_Aliases()
{
  IdentityManager manager;

  // It is picking up identities from older tests somethimes, so cleanup
  while ( manager.identities().size() > 1 ) {
    manager.removeIdentity( manager.identities().first() );
    manager.commit();
  }

  Identity &i1 = manager.newFromScratch( QLatin1String("Test1") );
  i1.setPrimaryEmailAddress( QLatin1String("firstname.lastname@example.com") );
  i1.setEmailAliases( QStringList() << QLatin1String("firstname@example.com") << QLatin1String("lastname@example.com") );
  QVERIFY( i1.matchesEmailAddress( QLatin1String("\"Lastname, Firstname\" <firstname@example.com>") ) );
  QVERIFY( i1.matchesEmailAddress( QLatin1String("\"Lastname, Firstname\" <firstname.lastname@example.com>") ) );
  QCOMPARE( i1.emailAliases().size(), 2 );

  KConfig testConfig( QLatin1String("test") );
  KConfigGroup testGroup( &testConfig, "testGroup" );
  i1.writeConfig( testGroup );
  i1.readConfig( testGroup );
  QCOMPARE( i1.emailAliases().size(), 2 );

  manager.commit();

  Identity &i2 = manager.newFromScratch( QLatin1String("Test2") );
  i2.setPrimaryEmailAddress( QLatin1String("test@test.de") );
  QVERIFY( i2.emailAliases().isEmpty() );
  manager.commit();

  // Remove the first identity, which we couldn't remove above
  manager.removeIdentity( manager.identities().first() );
  manager.commit();

  QCOMPARE( manager.allEmails().size(), 4 );
  QCOMPARE( manager.identityForAddress( QLatin1String("firstname@example.com") ).identityName().toLatin1().data(), "Test1" );
}
