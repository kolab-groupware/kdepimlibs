/*
    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>
                  2006 Marc Mutz <mutz@kde.org>
                  2006 - 2007 Volker Krause <vkrause@kde.org>

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

#ifndef AKONADI_JOB_H
#define AKONADI_JOB_H

#include "akonadi_export.h"

#include <kcompositejob.h>

class QString;

namespace Akonadi {

class JobPrivate;
class Session;
class SessionPrivate;

/**
  This class encapsulates a request to the pim storage service,
  the code looks like

  \code
    Akonadi::Job *job = new Akonadi::SomeJob( some parameter );
    connect( job, SIGNAL( result( KJob* ) ),
             this, SLOT( slotResult( KJob* ) ) );
  \endcode

  The job is queued for execution as soon as the event loop is entered
  again.

  And the slotResult is usually at least:

  \code
    if ( job->error() )
      // handle error...
  \endcode

  With the synchronous interface the code looks like

  \code
    Akonadi::SomeJob *job = new Akonadi::SomeJob( some parameter );
    if ( !job->exec() ) {
      qDebug( "Error: %s", qPrintable( job->errorString() ) );
    } else {
      // do something
    }
  \endcode

  Warning: Using the synchronous method is error prone, use this only
  if the asynchronous access is not possible and none of the following
  known issues apply:
  - exec() must not be called directly from a result slot of another
    job in the same Session. This will trigger a dead-lock since the
    program won't return from the result slot and thus will never complete
    the finishing of the current job, keeping it in the internal queue
    and blocking the execution of any following job.
  - exec() must not be called from within another event-loop that might
    finish before the newly started inner sub-eventloop. This will crash.

  Subclasses must reimplement @see doStart().

  @note KJob-derived objects delete itself, it is thus not possible
  to create job objects on the stack!
 */
class AKONADI_EXPORT Job : public KCompositeJob
{
  Q_OBJECT

  friend class Session;
  friend class SessionPrivate;

  public:

    typedef QList<Job*> List;

    /**
      Error codes that can be emitted by this class,
      subclasses can provide additional codes.
     */
    enum Error
    {
      ConnectionFailed = UserDefinedError,
      UserCanceled,
      Unknown,
      UserError = UserDefinedError + 42
    };
    //FIXME_API: extend error code + renaming

    /**
      Creates a new job.
      If the parent object is a Job object, the new job will be a subjob of @p parent.
      If the parent object is a Session object, it will be used for server communication
      instead of the default session.
      @param parent The parent object, job or session.
     */
    explicit Job( QObject *parent = 0 );

    /**
      Destroys the job.
     */
    virtual ~Job();

    /**
      Jobs are started automatically once entering the event loop again, no need
      to explicitly call this.
     */
    void start();

    /**
      Returns the error string, if there has been an error, an empty
      string otherwise.
     */
    virtual QString errorString() const;

  Q_SIGNALS:
    /**
      Emitted directly before the job will be started.
      @param job The started job.
    */
    void aboutToStart( Akonadi::Job *job );

    /**
      Emitted if the job has finished all write operations, ie.
      if this signal is emitted, the job guarantees to not call writeData() again.
      Do not emit this signal directly, call emitWriteFinished() instead.
      @param job This job.
      @see emitWriteFinished()
    */
    void writeFinished( Akonadi::Job *job );

  protected:
    /**
      This method must be reimplemented in the concrete jobs. It will be called
      after the job has been started and a connection to the Akonadi backend has
      been established.
    */
    virtual void doStart() = 0;

    /**
      This method should be reimplemented in the concrete jobs in case you want
      to handle incoming data. It will be called on received data from the backend.
      The default implementation does nothing.
      @param tag The tag of the corresponding command, empty if this is an untagges response.
      @param data The received data.
    */
    virtual void doHandleResponse( const QByteArray &tag, const QByteArray &data );

    /**
      Adds the given job as a subjob to this job. This method is automatically called
      if you construct a job using another job as parent object.
      The base implementation does the necessary setup to share the network connection
      with the backend.
      @param job The new subjob.
    */
    virtual bool addSubjob( KJob* job );

    virtual bool removeSubjob( KJob* job );
    virtual bool doKill();

    /**
      Call this method to indicate that this job will not call writeData() again.
      @see writeFinished()
    */
    void emitWriteFinished();

  protected Q_SLOTS:
    virtual void slotResult( KJob* job );

  protected:
    //@cond PRIVATE
    Job( JobPrivate *dd, QObject *parent );
    JobPrivate* const d_ptr;
    //@endcond

  private:
    Q_DECLARE_PRIVATE( Job )

    Q_PRIVATE_SLOT( d_func(), void slotSubJobAboutToStart( Akonadi::Job* ) )
    Q_PRIVATE_SLOT( d_func(), void startNext() )
};

}

#endif
