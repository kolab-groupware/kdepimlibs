/*
    This file is part of the kcal library.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

//
// IMIPScheduler - iMIP implementation of iTIP methods
//

#include "imipscheduler.h"

#include "event.h"
#include "icalformat.h"

using namespace KCal;

IMIPScheduler::IMIPScheduler( Calendar *calendar )
  : Scheduler( calendar ), d( 0 )
{
}

IMIPScheduler::~IMIPScheduler()
{
}

bool IMIPScheduler::publish (IncidenceBase * /*incidence*/,const QString &/*recipients*/)
{
  return false;
}

bool IMIPScheduler::performTransaction(IncidenceBase *incidence,Method method)
{
  mFormat->createScheduleMessage(incidence,method);

  return false;
}

QList<ScheduleMessage*> IMIPScheduler::retrieveTransactions()
{
  QList<ScheduleMessage*> messageList;

  return messageList;
}
