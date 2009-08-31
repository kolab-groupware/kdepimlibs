/*
    Copyright 2009 Constantin Berzan <exit3219@gmail.com>

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

#include "localfoldersrequestjobtest.h"

#include "../../collectionpathresolver_p.h"

#include <QSignalSpy>

#include <KDebug>
#include <KStandardDirs>

#include <akonadi/collectioncreatejob.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionmodifyjob.h>
#include <akonadi/control.h>
#include <akonadi/qtest_akonadi.h>
#include <akonadi/kmime/localfolderattribute.h>
#include <akonadi/kmime/localfolders.h>
#include <akonadi/kmime/localfoldersrequestjob.h>
#include "../localfolderstesting.h"
#include "../localfoldershelperjobs_p.h"

using namespace Akonadi;

void LocalFoldersRequestJobTest::initTestCase()
{
  QVERIFY( Control::start() );
  QTest::qWait( 1000 );

  LocalFolders *lf = LocalFolders::self();
  LocalFoldersTesting *lft = LocalFoldersTesting::_t_self();
  Q_ASSERT( lf );
  Q_ASSERT( lft );

  // No one has created the default resource.  LF has no folders.
  QVERIFY( lf->defaultResourceId().isEmpty() );
  QCOMPARE( lft->_t_knownResourceCount(), 0 );
  QCOMPARE( lft->_t_knownFolderCount(), 0 );
}

void LocalFoldersRequestJobTest::testRequestWithNoDefaultResourceExisting()
{
  LocalFolders *lf = LocalFolders::self();
  LocalFoldersTesting *lft = LocalFoldersTesting::_t_self();
  Q_ASSERT( lf );
  Q_ASSERT( lft );
  QSignalSpy spy( lf, SIGNAL(foldersChanged(QString)) );
  QSignalSpy defSpy( lf, SIGNAL(defaultFoldersChanged()) );

  // Initially the defaut maildir does not exist.
  QVERIFY( !QFile::exists( KGlobal::dirs()->localxdgdatadir() + nameForType( LocalFolders::Root ) ) );

  // Request some default folders.
  {
    LocalFoldersRequestJob *rjob = new LocalFoldersRequestJob( this );
    rjob->requestDefaultFolder( LocalFolders::Outbox );
    rjob->requestDefaultFolder( LocalFolders::Drafts );
    AKVERIFYEXEC( rjob );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( defSpy.count(), 1 );
    QCOMPARE( lft->_t_knownResourceCount(), 1 );
    QCOMPARE( lft->_t_knownFolderCount(), 3 ); // Outbox, Drafts, and Root.
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Outbox ) );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Drafts ) );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Root ) );
  }

  // The maildir should exist now.
  QVERIFY( QFile::exists( KGlobal::dirs()->localxdgdatadir() + nameForType( LocalFolders::Root ) ) );
}

void LocalFoldersRequestJobTest::testRequestWithDefaultResourceAlreadyExisting()
{
  LocalFolders *lf = LocalFolders::self();
  LocalFoldersTesting *lft = LocalFoldersTesting::_t_self();
  Q_ASSERT( lf );
  Q_ASSERT( lft );
  QSignalSpy spy( lf, SIGNAL(foldersChanged(QString)) );
  QSignalSpy defSpy( lf, SIGNAL(defaultFoldersChanged()) );

  // Prerequisites (from testRequestWithNoDefaultResourceExisting()).
  QVERIFY( QFile::exists( KGlobal::dirs()->localxdgdatadir() + nameForType( LocalFolders::Root ) ) );
  QVERIFY( !lf->hasDefaultFolder( LocalFolders::Inbox ) );
  QVERIFY( lf->hasDefaultFolder( LocalFolders::Outbox ) );
  const Collection oldOutbox = lf->defaultFolder( LocalFolders::Outbox );

  // Request some default folders.
  {
    LocalFoldersRequestJob *rjob = new LocalFoldersRequestJob( this );
    rjob->requestDefaultFolder( LocalFolders::Outbox ); // Exists previously.
    rjob->requestDefaultFolder( LocalFolders::Inbox ); // Must be created.
    AKVERIFYEXEC( rjob );
    QCOMPARE( spy.count(), 1 );
    QCOMPARE( defSpy.count(), 1 );
    QCOMPARE( lft->_t_knownResourceCount(), 1 );
    QCOMPARE( lft->_t_knownFolderCount(), 4 ); // Inbox, Outbox, Drafts, and Root.
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Inbox ) );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Outbox ) );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Drafts ) );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Root ) );
  }

  // This should be untouched.
  QCOMPARE( lf->defaultFolder( LocalFolders::Outbox ), oldOutbox );
}

void LocalFoldersRequestJobTest::testMixedRequest()
{
  LocalFolders *lf = LocalFolders::self();
  LocalFoldersTesting *lft = LocalFoldersTesting::_t_self();
  Q_ASSERT( lf );
  Q_ASSERT( lft );
  QSignalSpy spy( lf, SIGNAL(foldersChanged(QString)) );
  QSignalSpy defSpy( lf, SIGNAL(defaultFoldersChanged()) );

  // Get our knut collection.
  Collection res1;
  {
    CollectionPathResolver *resolver = new CollectionPathResolver( "res1", this );
    QVERIFY( resolver->exec() );
    res1 = Collection( resolver->collection() );
    CollectionFetchJob *fjob = new CollectionFetchJob( res1, CollectionFetchJob::Base, this );
    AKVERIFYEXEC( fjob );
    Q_ASSERT( fjob->collections().count() == 1 );
    res1 = fjob->collections().first();
    QVERIFY( res1.isValid() );
    QVERIFY( !res1.resource().isEmpty() );
  }

  // Create a LocalFolder in the knut resource.
  Collection knutOutbox;
  {
    knutOutbox.setName( QLatin1String( "my_outbox" ) );
    knutOutbox.setParentCollection( res1 );
    kDebug() << res1;
    knutOutbox.addAttribute( new LocalFolderAttribute( LocalFolders::Outbox ) );
    CollectionCreateJob *cjob = new CollectionCreateJob( knutOutbox, this );
    AKVERIFYEXEC( cjob );
    knutOutbox = cjob->collection();
  }

  // Prerequisites (from the above two functions).
  QVERIFY( QFile::exists( KGlobal::dirs()->localxdgdatadir() + nameForType( LocalFolders::Root ) ) );
  QVERIFY( !lf->hasDefaultFolder( LocalFolders::SentMail ) );
  QVERIFY( lf->hasDefaultFolder( LocalFolders::Outbox ) );
  const Collection oldOutbox = lf->defaultFolder( LocalFolders::Outbox );

  // Request some folders, both in our default resource and in the knut resource.
  {
    LocalFoldersRequestJob *rjob = new LocalFoldersRequestJob( this );
    rjob->requestDefaultFolder( LocalFolders::Outbox ); // Exists previously.
    rjob->requestDefaultFolder( LocalFolders::SentMail ); // Must be created.
    rjob->requestFolder( LocalFolders::Outbox, res1.resource() ); // Exists previously, but unregistered with LF.
    rjob->requestFolder( LocalFolders::SentMail, res1.resource() ); // Must be created.
    AKVERIFYEXEC( rjob );
    QCOMPARE( spy.count(), 2 ); // Default resource and knut resource.
    QCOMPARE( defSpy.count(), 1 );
    QCOMPARE( lft->_t_knownResourceCount(), 2 );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::Outbox ) );
    QVERIFY( lf->hasDefaultFolder( LocalFolders::SentMail ) );
    QVERIFY( lf->hasFolder( LocalFolders::Outbox, res1.resource() ) );
    QVERIFY( lf->hasFolder( LocalFolders::SentMail, res1.resource() ) );
  }

  // These should be untouched.
  QCOMPARE( lf->defaultFolder( LocalFolders::Outbox ), oldOutbox );
  QCOMPARE( lf->folder( LocalFolders::Outbox, res1.resource() ), knutOutbox );
}

QTEST_AKONADIMAIN( LocalFoldersRequestJobTest, NoGUI )

#include "localfoldersrequestjobtest.moc"
