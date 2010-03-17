/*
    Copyright (c) 2006-2008 Volker Krause <vkrause@kde.org>

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

#include "transactionsequence.h"
#include "transactionjobs.h"

#include "job_p.h"

#include <QtCore/QSet>

using namespace Akonadi;

class Akonadi::TransactionSequencePrivate : public JobPrivate
{
  public:
    TransactionSequencePrivate( TransactionSequence *parent )
      : JobPrivate( parent ),
        mState( Idle )
    {
    }

    enum TransactionState
    {
      Idle,
      Running,
      WaitingForSubjobs,
      RollingBack,
      Committing
    };

    Q_DECLARE_PUBLIC( TransactionSequence )

    TransactionState mState;
    QSet<KJob*> mIgnoredErrorJobs;

    void commitResult( KJob *job )
    {
      Q_Q( TransactionSequence );

      if ( job->error() ) {
        q->setError( job->error() );
        q->setErrorText( job->errorText() );
      }
      q->emitResult();
    }

    void rollbackResult( KJob *job )
    {
      Q_Q( TransactionSequence );

      Q_UNUSED( job );
      q->emitResult();
    }
};

TransactionSequence::TransactionSequence( QObject * parent )
  : Job( new TransactionSequencePrivate( this ), parent )
{
}

TransactionSequence::~TransactionSequence()
{
}

bool TransactionSequence::addSubjob(KJob * job)
{
  Q_D( TransactionSequence );

  if ( d->mState == TransactionSequencePrivate::Idle ) {
    d->mState = TransactionSequencePrivate::Running;
    new TransactionBeginJob( this );
  }
  return Job::addSubjob( job );
}

void TransactionSequence::slotResult(KJob * job)
{
  Q_D( TransactionSequence );

  if ( !job->error() || d->mIgnoredErrorJobs.contains( job ) ) {
    // If we have an error but want to ignore it, we can't call Job::slotResult
    // because it would confuse the subjob queue processing logic. Just removing
    // the subjob instead is fine.
    if ( !job->error() )
      Job::slotResult( job );
    else
      Job::removeSubjob( job );

    if ( subjobs().isEmpty() && d->mState == TransactionSequencePrivate::WaitingForSubjobs ) {
      d->mState = TransactionSequencePrivate::Committing;
      TransactionCommitJob *job = new TransactionCommitJob( this );
      connect( job, SIGNAL(result(KJob*)), SLOT(commitResult(KJob*)) );
    }
  } else {
    setError( job->error() );
    setErrorText( job->errorText() );
    removeSubjob( job );
    clearSubjobs();
    if ( d->mState == TransactionSequencePrivate::Running || d->mState == TransactionSequencePrivate::WaitingForSubjobs ) {
      d->mState = TransactionSequencePrivate::RollingBack;
      TransactionRollbackJob *job = new TransactionRollbackJob( this );
      connect( job, SIGNAL(result(KJob*)), SLOT(rollbackResult(KJob*)) );
    }
  }
}

void TransactionSequence::commit()
{
  Q_D( TransactionSequence );

  if ( d->mState == TransactionSequencePrivate::Running )
    d->mState = TransactionSequencePrivate::WaitingForSubjobs;
  else
    return;

  if ( subjobs().isEmpty() ) {
    if ( !error() ) {
      d->mState = TransactionSequencePrivate::Committing;
      TransactionCommitJob *job = new TransactionCommitJob( this );
      connect( job, SIGNAL(result(KJob*)), SLOT(commitResult(KJob*)) );
    } else {
      d->mState = TransactionSequencePrivate::RollingBack;
      TransactionRollbackJob *job = new TransactionRollbackJob( this );
      connect( job, SIGNAL(result(KJob*)), SLOT(rollbackResult(KJob*)) );
    }
  }
}

void TransactionSequence::continueOnJobFailure( KJob *job )
{
  Q_D( TransactionSequence );

  // make sure this is one of our sub jobs
  Q_ASSERT( subjobs().contains( job ) );

  d->mIgnoredErrorJobs.insert( job );
}

void TransactionSequence::doStart()
{
  Q_D( TransactionSequence );

  if ( d->mState == TransactionSequencePrivate::Idle )
    emitResult();
  else
    commit();
}

#include "transactionsequence.moc"
