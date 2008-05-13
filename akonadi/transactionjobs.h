/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_TRANSACTIONJOBS_H
#define AKONADI_TRANSACTIONJOBS_H

#include "akonadi_export.h"

#include <akonadi/job.h>

namespace Akonadi {

class TransactionBeginJobPrivate;
class TransactionRollbackJobPrivate;
class TransactionCommitJobPrivate;

/**
  Begins a session-global transaction.
  @note This will only have an effect when used as a subjob or with a Session.
*/
class AKONADI_EXPORT TransactionBeginJob : public Job
{
  Q_OBJECT

  public:
    /**
      Creates a new TransactionBeginJob.
      @param parent The parent job or Session, must not be 0.
    */
    explicit TransactionBeginJob( QObject *parent );

    /**
      Destroys this job.
    */
    ~TransactionBeginJob();

  protected:
    virtual void doStart();

  private:
    Q_DECLARE_PRIVATE( TransactionBeginJob )
};


/**
  Aborts a session-global transaction.
  @note This will only have an effect when used as a subjob or with a Session.
*/
class AKONADI_EXPORT TransactionRollbackJob : public Job
{
  Q_OBJECT

  public:
    /**
      Creates a new TransactionRollbackJob.
      The parent must be the same parent as for the TransactionBeginJob.
      @param parent The parent job or Session, must not be 0.
    */
    explicit TransactionRollbackJob( QObject *parent );

    /**
      Destroys this TransactionRollbackJob.
    */
    ~TransactionRollbackJob();

  protected:
    virtual void doStart();

  private:
    Q_DECLARE_PRIVATE( TransactionRollbackJob )
};


/**
  Commits a session-global transaction.
*/
class AKONADI_EXPORT TransactionCommitJob : public Job
{
  Q_OBJECT

  public:
    /**
      Creates a new TransactionCommitJob.
      The parent must be the same parent as for the TransactionBeginJob.
      @param parent The parent job or Session, must not be 0.
     */
    explicit TransactionCommitJob( QObject *parent );

    /**
      Destroys this TransactionCommitJob.
    */
    ~TransactionCommitJob();

  protected:
    virtual void doStart();

  private:
    Q_DECLARE_PRIVATE( TransactionCommitJob )
};

}

#endif
