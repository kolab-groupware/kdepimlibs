/*
  This file is part of the kcal library.

  Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef DUMMYSCHEDULER_H
#define DUMMYSCHEDULER_H

#include "scheduler.h"
#include <QtCore/QList>

namespace KCal {

/**
  \internal

  @brief
  This class implements the iTIP interface as a primitive local version for
  testing. It uses a file dummyscheduler.store as inbox/outbox.
*/
class DummyScheduler : public Scheduler
{
  public:
    explicit DummyScheduler( Calendar * );
    virtual ~DummyScheduler();

    bool publish( IncidenceBase *incidence, const QString &recipients );
    bool performTransaction( IncidenceBase *incidence, iTIPMethod method );
    bool performTransaction( IncidenceBase *incidence, iTIPMethod method,
                             const QString &recipients );
    QList<ScheduleMessage*> retrieveTransactions();

    virtual QString freeBusyDir();

  protected:
    bool saveMessage( const QString &message );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( DummyScheduler )
    class Private;
    Private *d;
    //@endcond
};

}

#endif

