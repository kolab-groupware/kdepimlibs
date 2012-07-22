/*
  Copyright (c) 2001,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2010,2012 Sérgio Martins <iamsergio@gmail.com>

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

#include "mailscheduler_p.h"

#include "mailclient_p.h"
#include "fetchjobcalendar.h"

#include <Akonadi/Item>

#include <KCalCore/ICalFormat>
#include <KCalCore/Incidence>
#include <KCalCore/ScheduleMessage>
#include <KCalUtils/Scheduler>
#include <KPIMIdentities/IdentityManager>

#include <KStandardDirs>
#include <KSystemTimeZone>
#include <KLocale>

#include <QDir>

using namespace Akonadi;
using namespace KPIMIdentities;

MailScheduler::MailScheduler( const Akonadi::FetchJobCalendar::Ptr &calendar,
                              const QString &email,
                              bool  bcc,
                              const QString &mailTransport,
                              QObject *parent ) : Scheduler( calendar, parent )
                                                , m_email( email )
                                                , m_bccMe( bcc )
                                                , m_transport( mailTransport )
                                                , m_identityManager( new IdentityManager( /*ro=*/ true, this ) )
{
}

MailScheduler::~MailScheduler()
{
}

Scheduler::TransactionId MailScheduler::publish( const KCalCore::IncidenceBase::Ptr &incidence,
                                                 const QString &recipients )
{
  if ( !incidence ) {
    return -1;
  }

  const QString messageText = mFormat->createScheduleMessage( incidence, KCalCore::iTIPPublish );

  MailClient mailer;

  // TODO: Catch the signal.
  mailer.mailTo( incidence,
                 m_identityManager->identityForAddress( m_email ),
                 m_email, m_bccMe, recipients, messageText,
                 m_transport );

  Scheduler::Result resultCode = Scheduler::ResultSuccess;
  QString errorMessage;
  /*if ( !result ) { // TODO
    errorMessage = QLatin1String( "Error sending e-mail" );
    resultCode = ResultGenericError;
  }*/

  const TransactionId tid = nextTransactionId();
  emit performTransactionFinished( tid, resultCode, errorMessage );
  return tid;
}

Scheduler::TransactionId MailScheduler::performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                                            KCalCore::iTIPMethod method,
                                                            const QString &recipients )
{
  if ( !incidence ) {
    return -1;
  }

  const QString messageText = mFormat->createScheduleMessage( incidence, method );

  MailClient mailer;
  // TODO: Catch signal
  mailer.mailTo( incidence,
                 m_identityManager->identityForAddress( m_email ),
                 m_email, m_bccMe, recipients, messageText,
                 m_transport );

  Scheduler::Result resultCode = ResultSuccess;
  QString errorMessage;
  /*if ( !result ) { // TODO
    errorMessage = QLatin1String( "Error sending e-mail" );
    resultCode = ResultGenericError;
  }*/
  const TransactionId tid = nextTransactionId();
  emit performTransactionFinished( tid, resultCode, errorMessage );
  return tid;
}

Scheduler::TransactionId MailScheduler::performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                                            KCalCore::iTIPMethod method )
{
  if ( !incidence ) {
    return -1;
  }

  const QString messageText = mFormat->createScheduleMessage( incidence, method );

  MailClient mailer;
  if ( method == KCalCore::iTIPRequest ||
       method == KCalCore::iTIPCancel ||
       method == KCalCore::iTIPAdd ||
       method == KCalCore::iTIPDeclineCounter ) {
    mailer.mailAttendees( // TODO handle error
      incidence,
      m_identityManager->identityForAddress( m_email ),
      m_bccMe, messageText, m_transport );
  } else {
    QString subject;
    KCalCore::Incidence::Ptr inc = incidence.dynamicCast<KCalCore::Incidence>() ;
    if ( inc && method == KCalCore::iTIPCounter ) {
      subject = i18n( "Counter proposal: %1", inc->summary() );
    }
    mailer.mailOrganizer( // TODO: handle error
      incidence,
      m_identityManager->identityForAddress( m_email ),
      m_email, m_bccMe, messageText, subject, m_transport );
  }

  Scheduler::Result resultCode = ResultSuccess;
  QString errorMessage;
  /*if ( !status ) {
    errorMessage = QLatin1String( "Error sending e-mail" );
    resultCode = ResultGenericError;
  }*/
  const TransactionId tid = nextTransactionId();
  emit performTransactionFinished( tid, resultCode, errorMessage );
  return tid;
}

QString MailScheduler::freeBusyDir() const
{
  return KStandardDirs::locateLocal( "data", QLatin1String( "korganizer/freebusy" ) );
}

//TODO: AKONADI_PORT review following code
Scheduler::TransactionId MailScheduler::acceptCounterProposal( const KCalCore::Incidence::Ptr &incidence )
{
  if ( !incidence ) {
    return -1;
  }

  Akonadi::Item exInc = mCalendar->item( incidence->uid() );
  if ( !exInc.isValid() ) {
    KCalCore::Incidence::Ptr exIncidence = mCalendar->incidenceFromSchedulingID( incidence->uid() );
    if ( exIncidence ) {
      exInc = mCalendar->item( exIncidence->uid() );
    }
    //exInc = exIncItem.isValid() && exIncItem.hasPayload<KCalCore::Incidence::Ptr>() ?
    //        exIncItem.payload<KCalCore::Incidence::Ptr>() : KCalCore::Incidence::Ptr();
  }

  incidence->setRevision( incidence->revision() + 1 );
  int changeId;
  Result result = ResultSuccess;

  if ( exInc.isValid() && exInc.hasPayload<KCalCore::Incidence::Ptr>() ) {
    KCalCore::Incidence::Ptr exIncPtr = exInc.payload<KCalCore::Incidence::Ptr>();
    incidence->setRevision( qMax( incidence->revision(), exIncPtr->revision() + 1 ) );
    // some stuff we don't want to change, just to be safe
    incidence->setSchedulingID( exIncPtr->schedulingID() );
    incidence->setUid( exIncPtr->uid() );

    Q_ASSERT( exIncPtr && incidence );

    KCalCore::IncidenceBase::Ptr i1 = exIncPtr;
    KCalCore::IncidenceBase::Ptr i2 = incidence;

    if ( i1->type() == i2->type() ) {
      *i1 = *i2;
    }

    exIncPtr->updated();

    changeId = mCalendar->modifyIncidence( exIncPtr );
    result = ResultModifyingError;
  } else {
    changeId = mCalendar->addIncidence( KCalCore::Incidence::Ptr( incidence->clone() ) );
    result = ResultCreatingError;
  }

  const Scheduler::TransactionId tid = nextTransactionId();

  if ( changeId > 0 ) {
  } else {
      emit performTransactionFinished( tid, result, QLatin1String( "Error creating job" ) );
  }

  return tid;
}