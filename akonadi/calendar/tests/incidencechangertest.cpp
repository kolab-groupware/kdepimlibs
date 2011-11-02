/*
    Copyright (c) 2010-2011 Sérgio Martins <iamsergio@gmail.com>

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

#include "../incidencechanger.h"


#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/qtest_akonadi.h>

#include <Akonadi/Item>
#include <Akonadi/Collection>

#include <KCalCore/Event>
#include <KCalCore/Journal>
#include <KCalCore/Todo>

#include <QtCore/QObject>
#include <QPushButton>

using namespace Akonadi;
using namespace KCalCore;

class IncidenceChangerTest : public QObject
{
  Q_OBJECT
  Collection mCollection;

  bool mWaitingForIncidenceChangerSignals;
  IncidenceChanger::ResultCode mExpectedResult;
  IncidenceChanger *mChanger;

  QSet<int> mKnownChangeIds;

  private slots:
    void initTestCase()
    {
      return;
      mWaitingForIncidenceChangerSignals = false;
      mExpectedResult = IncidenceChanger::ResultCodeSuccess;
      //Control::start(); //TODO: uncomment when using testrunner
      qRegisterMetaType<Akonadi::IncidenceChanger::ResultCode>("Akonadi::IncidenceChanger::ResultCode");
      qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
      CollectionFetchJob *job = new CollectionFetchJob( Collection::root(),
                                                        CollectionFetchJob::Recursive,
                                                        this );
      // Get list of collections
      job->fetchScope().setContentMimeTypes( QStringList() << QLatin1String( "application/x-vnd.akonadi.calendar.event" ) );
      AKVERIFYEXEC( job );

      // Find our collection
      Collection::List collections = job->collections();
      QVERIFY( !collections.isEmpty() );
      mCollection = collections.first();

      QVERIFY( mCollection.isValid() );

      mChanger = new IncidenceChanger();
      mChanger->setShowDialogsOnError( false );

      connect( mChanger, SIGNAL(createFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
               SLOT(createFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );

      connect( mChanger, SIGNAL(deleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)),
               SLOT(deleteFinished(int,QVector<Akonadi::Item::Id>,Akonadi::IncidenceChanger::ResultCode,QString)) );

      connect( mChanger,SIGNAL(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)),
               SLOT(modifyFinished(int,Akonadi::Item,Akonadi::IncidenceChanger::ResultCode,QString)) );
    }

    void testCreating()
    {
      return;
      int changeId;

      { // Create 5 incidences, wait for the signal.
        mWaitingForIncidenceChangerSignals = true;
        for ( int i = 0; i < 5; ++i ) {
          const QString uid( QLatin1String( "uid" ) + QString::number( i ) );
          const QString summary( QLatin1String( "summary" ) + QString::number( i ) );
          Incidence::Ptr incidence( new Event() );
          incidence->setUid( uid );
          incidence->setSummary( summary );
          changeId = mChanger->createIncidence( incidence, mCollection );
          QVERIFY( changeId != -1 );
          mKnownChangeIds.insert( changeId );
        }
        waitForSignals();
      }

      { // Invalid parameters
        changeId = mChanger->createIncidence( Incidence::Ptr(), // Invalid payload
                                              mCollection );
        QVERIFY( changeId == -1 );
      }

      { // Invalid collections


      }
    }

    void testDeleting()
    {
      /*
      int changeId;
      const Item::List incidences = mCalendar->rawIncidences();

      { // Delete 5 incidences, previously created
        foreach( const Item &item, incidences ) {
          mPendingDeletesInETM.append( item.payload<Incidence::Ptr>()->uid() );
        }
        changeId = mChanger->deleteIncidences( incidences );
        mKnownChangeIds.insert( changeId );
        QVERIFY( changeId != -1 );
        waitForSignals();
      }

      { // Delete something already deleted
        mWaitingForIncidenceChangerSignals = true;
        changeId = mChanger->deleteIncidences( incidences );
        mKnownChangeIds.insert( changeId );
        QVERIFY( changeId != -1 );
        mExpectedResult = IncidenceChanger::ResultCodeAlreadyDeleted;
        waitForSignals();
      }

      { // If we provide an empty list, a job won't be created
        changeId = mChanger->deleteIncidences( Item::List() );
        mKnownChangeIds.insert( changeId );
        QVERIFY( changeId == -1 );
      }

      { // If we provide a list with at least one invalid item, a job won't be created
        Item::List list;
        list << Item();
        changeId = mChanger->deleteIncidences( list );
        QVERIFY( changeId == -1 );
      }
*/
    }

    void testModifying()
    {
      /*
      int changeId;

      // First create an incidence
      const QString uid( "uid");
      const QString summary( "summary");
      Incidence::Ptr incidence( new Event() );
      incidence->setUid( uid );
      incidence->setSummary( summary );
      mWaitingForIncidenceChangerSignals = true;
      changeId = mChanger->createIncidence( incidence, mCollection );
      QVERIFY( changeId != -1 );
      mKnownChangeIds.insert( changeId );
      waitForSignals();

      { // Just a summary change
        Item item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        item.payload<Incidence::Ptr>()->setSummary( "summary2" );
        mWaitingForIncidenceChangerSignals = true;
        changeId = mChanger->modifyIncidence( item );
        QVERIFY( changeId != -1 );
        mKnownChangeIds.insert( changeId );
        waitForSignals();
        item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        QVERIFY( item.payload<Incidence::Ptr>()->summary() == "summary2" );
      }

      { // Invalid item
        changeId = mChanger->modifyIncidence( Item() );
        QVERIFY( changeId == -1 );
      }

      { // Delete it and try do modify it, should result in error
        Item item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        mPendingDeletesInETM.append( uid );
        changeId = mChanger->deleteIncidence( item );
        QVERIFY( changeId != -1 );
        mKnownChangeIds.insert( changeId );
        waitForSignals();

        mWaitingForIncidenceChangerSignals = true;
        changeId = mChanger->modifyIncidence( item );
        mKnownChangeIds.insert( changeId );
        mExpectedResult = IncidenceChanger::ResultCodeAlreadyDeleted;
        QVERIFY( changeId != -1 );
        waitForSignals();
      }
      */
    }

    void testMassModifyForConflicts()
    {
      /*
      int changeId;

      // First create an incidence
      const QString uid( "uid");
      const QString summary( "summary");
      Incidence::Ptr incidence( new Event() );
      incidence->setUid( uid );
      incidence->setSummary( summary );
      mPendingInsertsInETM.append( uid );
      changeId = mChanger->createIncidence( incidence,
                                            mCollection );
      QVERIFY( changeId != -1 );
      mKnownChangeIds.insert( changeId );
      waitForSignals();

      kDebug() << "Doing 30 modifications, but waiting for jobs to end before starting a new one.";
      const int LIMIT = 30;
      for ( int i = 0; i < LIMIT; ++i ) {
        mWaitingForIncidenceChangerSignals = true;
        mPendingUpdatesInETM.append( uid );
        Item item = mCalendar->itemForIncidenceUid( uid );
        QVERIFY( item.isValid() );
        item.payload<Incidence::Ptr>()->setSummary( QString::number( i ) );
        int changeId = mChanger->modifyIncidence( item );
        QVERIFY( changeId > -1 );
        mKnownChangeIds.insert( changeId );
        waitForSignals();
      }

      Item item = mCalendar->itemForIncidenceUid( uid );
      QVERIFY( item.isValid() );

      kDebug() << "Doing 30 modifications, and not for jobs to end before starting a new one.";

      for ( int i = 0; i < LIMIT; ++i ) {
        item.payload<Incidence::Ptr>()->setSummary( QString::number( i ) );
        const int changeId = mChanger->modifyIncidence( item );
        QVERIFY( changeId > -1 );
        mKnownChangeIds.insert( changeId );

        if ( i == LIMIT-1 ) {
          // Let's catch the last signal, so we don't exit our test with jobs still running
          mWaitingForIncidenceChangerSignals = true;
        }
        QTest::qWait( 100 );
      }
      waitForSignals();

      // Cleanup, delete the incidence
      item = mCalendar->itemForIncidenceUid( uid );
      QVERIFY( item.isValid() );
      mPendingDeletesInETM.append( uid );
      changeId = mChanger->deleteIncidence( item );
      QVERIFY( changeId != -1 );
      mKnownChangeIds.insert( changeId );
      waitForSignals();
      */
    }

  public Q_SLOTS:

    void waitForSignals()
    {
      while ( mWaitingForIncidenceChangerSignals ) {
        QTest::qWait( 1000 );
      }
    }

  void deleteFinished( int changeId,
                       const QVector<Akonadi::Item::Id> &deletedIds,
                       Akonadi::IncidenceChanger::ResultCode resultCode,
                       const QString &errorMessage )
  {
    Q_UNUSED( deletedIds );
    QVERIFY( mKnownChangeIds.contains( changeId ) );
    QVERIFY( changeId != -1 );

    if ( resultCode != IncidenceChanger::ResultCodeSuccess ) {
      kDebug() << "Error string is " << errorMessage;
    } else {
      QVERIFY( !deletedIds.isEmpty() );
      foreach( Akonadi::Item::Id id , deletedIds ) {
        QVERIFY( id != -1 );
      }
    }

    QVERIFY( resultCode == mExpectedResult );
    mExpectedResult = IncidenceChanger::ResultCodeSuccess;
    mWaitingForIncidenceChangerSignals = false;
  }

  void createFinished( int changeId,
                       const Akonadi::Item &item,
                       Akonadi::IncidenceChanger::ResultCode resultCode,
                       const QString &errorString )
  {
    QVERIFY( mKnownChangeIds.contains( changeId ) );
    QVERIFY( changeId != -1 );

    if ( resultCode == IncidenceChanger::ResultCodeSuccess ) {
      QVERIFY( item.isValid() );
      QVERIFY( item.parentCollection().isValid() );
    } else {
      kDebug() << "Error string is " << errorString;
    }

    QVERIFY( resultCode == mExpectedResult );
    mExpectedResult = IncidenceChanger::ResultCodeSuccess;
    mWaitingForIncidenceChangerSignals = false;
  }

  void modifyFinished( int changeId,
                       const Akonadi::Item &item,
                       Akonadi::IncidenceChanger::ResultCode resultCode,
                       const QString &errorString )
  {
    Q_UNUSED( item );
    QVERIFY( mKnownChangeIds.contains( changeId ) );
    QVERIFY( changeId != -1 );

    if ( resultCode == IncidenceChanger::ResultCodeSuccess )
      QVERIFY( item.isValid() );
    else
      kDebug() << "Error string is " << errorString;

    QVERIFY( resultCode == mExpectedResult );

    mExpectedResult = IncidenceChanger::ResultCodeSuccess;
    mWaitingForIncidenceChangerSignals = false;
  }
};

QTEST_AKONADIMAIN( IncidenceChangerTest, NoGUI )

#include "incidencechangertest.moc"
