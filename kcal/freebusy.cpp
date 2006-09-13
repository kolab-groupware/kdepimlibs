/*
    This file is part of the kcal library.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <kdebug.h>

#include "freebusy.h"
//Added by qt3to4:
#include <QList>

using namespace KCal;

FreeBusy::FreeBusy()
{
}

FreeBusy::FreeBusy(const KDateTime &start, const KDateTime &end)
{
  setDtStart(start);
  setDtEnd(end);
}

FreeBusy::FreeBusy( Calendar *calendar, const KDateTime &start, const KDateTime &end )
{
  kDebug(5800) << "FreeBusy::FreeBusy" << endl;
  mCalendar = calendar;

  setDtStart(start);
  setDtEnd(end);

  // Get all the events in the calendar
  Event::List eventList = mCalendar->rawEvents( start.date(), end.date() );

  int extraDays, i, x, duration;
  duration = start.daysTo(end);
  QDate day;
  KDateTime tmpStart;
  KDateTime tmpEnd;
  // Loops through every event in the calendar
  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    Event *event = *it;

    // The code below can not handle floating events. Fixing this resulted
    // in a lot of duplicated code. Instead, make a copy of the event and
    // set the period to the full day(s). This trick works for recurring,
    // multiday, and single day floating events.
    Event *floatingEvent = 0;
    if ( event->doesFloat() ) {
      // Floating event. Do the hack
      kDebug(5800) << "Floating event\n";
      floatingEvent = new Event( *event );

      // Set the start and end times to be on midnight
      KDateTime st = floatingEvent->dtStart();
      st.setTime( QTime( 0, 0 ) );
      KDateTime nd = floatingEvent->dtEnd();
      nd.setTime( QTime( 23, 59, 59, 999 ) );
      floatingEvent->setFloats( false );
      floatingEvent->setDtStart( st );
      floatingEvent->setDtEnd( nd );

      kDebug(5800) << "Use: " << st.toString() << " to " << nd.toString()
                    << endl;
      // Finally, use this event for the setting below
      event = floatingEvent;
    }

    // This whole for loop is for recurring events, it loops through
    // each of the days of the freebusy request

    // First check if this is transparent. If it is, it shouldn't be in the
    // freebusy list
    if ( event->transparency() == Event::Transparent )
      // Transparent
      continue;

    for( i = 0; i <= duration; ++i ) {
      day=start.addDays(i).date();
      tmpStart.setDate(day);
      tmpEnd.setDate(day);

      if( event->doesRecur() ) {
        if ( event->isMultiDay() ) {
// FIXME: This doesn't work for sub-daily recurrences or recurrences with
//        a different time than the original event.
          extraDays = event->dtStart().daysTo(event->dtEnd());
          for ( x = 0; x <= extraDays; ++x ) {
            if ( event->recursOn(day.addDays(-x), start.timeSpec()) ) {
              tmpStart.setDate(day.addDays(-x));
              tmpStart.setTime(event->dtStart().time());
              tmpEnd=tmpStart.addSecs( (event->duration()) );

              addLocalPeriod( tmpStart, tmpEnd );
              break;
            }
          }
        } else {
          if (event->recursOn(day, start.timeSpec())) {
            tmpStart.setTime(event->dtStart().time());
            tmpEnd.setTime(event->dtEnd().time());

            addLocalPeriod (tmpStart, tmpEnd);
          }
        }
      }

    }
    // Non-recurring events
    addLocalPeriod(event->dtStart(), event->dtEnd());

    // Clean up
    delete floatingEvent;
  }

  sortList();
}

FreeBusy::~FreeBusy()
{
}

void FreeBusy::setDtStart(const KDateTime &dtStart)
{
  IncidenceBase::setDtStart( dtStart.toUtc() );
  updated();
}

bool FreeBusy::setDtEnd( const KDateTime &end )
{
  mDtEnd = end;
  return true;
}

KDateTime FreeBusy::dtEnd() const
{
  return mDtEnd;
}

PeriodList FreeBusy::busyPeriods() const
{
  return mBusyPeriods;
}

bool FreeBusy::addLocalPeriod(const KDateTime &eventStart, const KDateTime &eventEnd ) {
  KDateTime tmpStart;
  KDateTime tmpEnd;

  //Check to see if the start *or* end of the event is
  //between the start and end of the freebusy dates.
  if ( !( ( ( dtStart().secsTo(eventStart) >= 0 ) &&
            ( eventStart.secsTo(dtEnd()) >= 0 ) )
       || ( ( dtStart().secsTo(eventEnd) >= 0 ) &&
            ( eventEnd.secsTo(dtEnd()) >= 0 ) ) ) )
    return false;

  if ( eventStart.secsTo( dtStart() ) >= 0 ) {
    tmpStart = dtStart();
  } else {
    tmpStart = eventStart;
  }

  if ( eventEnd.secsTo( dtEnd() ) <= 0 ) {
    tmpEnd = dtEnd();
  } else {
    tmpEnd = eventEnd;
  }

  Period p(tmpStart, tmpEnd);
  mBusyPeriods.append( p );

  return true;
}

FreeBusy::FreeBusy( const PeriodList& busyPeriods)
{
  mBusyPeriods = busyPeriods;
}

void FreeBusy::sortList()
{
  qSort( mBusyPeriods );
  return;
}

void FreeBusy::addPeriods(const PeriodList &list )
{
  mBusyPeriods += list;
  sortList();
}

void FreeBusy::addPeriod(const KDateTime &start, const KDateTime &end)
{
  mBusyPeriods.append( Period(start, end) );
  sortList();
}

void FreeBusy::addPeriod( const KDateTime &start, const Duration &dur )
{
  mBusyPeriods.append( Period(start, dur) );
  sortList();
}

void FreeBusy::merge( FreeBusy *freeBusy )
{
  if ( freeBusy->dtStart() < dtStart() )
    setDtStart( freeBusy->dtStart() );

  if ( freeBusy->dtEnd() > dtEnd() )
    setDtEnd( freeBusy->dtEnd() );

  QList<Period> periods = freeBusy->busyPeriods();
  QList<Period>::ConstIterator it;
  for ( it = periods.begin(); it != periods.end(); ++it )
    addPeriod( (*it).start(), (*it).end() );
}

// DEPRECATED methods
#include "icaltimezones.h"
FreeBusy::FreeBusy(const QDateTime &start, const QDateTime &end)
{
  setDtStart(KDateTime(start));  // use local time zone
  setDtEnd(KDateTime(end));
}

FreeBusy::FreeBusy( Calendar *calendar, const QDateTime &start, const QDateTime &end )
{
  kDebug(5800) << "FreeBusy::FreeBusy" << endl;
  mCalendar = calendar;

  setDtStart(KDateTime(start, mCalendar->timeSpec()));
  setDtEnd(KDateTime(end, mCalendar->timeSpec()));

  // Get all the events in the calendar
  Event::List eventList = mCalendar->rawEvents( start.date(), end.date() );

  int extraDays, i, x, duration;
  duration = start.daysTo(end);
  QDate day;
  KDateTime tmpStart;
  KDateTime tmpEnd;
  // Loops through every event in the calendar
  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    Event *event = *it;

    // The code below can not handle floating events. Fixing this resulted
    // in a lot of duplicated code. Instead, make a copy of the event and
    // set the period to the full day(s). This trick works for recurring,
    // multiday, and single day floating events.
    Event *floatingEvent = 0;
    if ( event->doesFloat() ) {
      // Floating event. Do the hack
      kDebug(5800) << "Floating event\n";
      floatingEvent = new Event( *event );

      // Set the start and end times to be on midnight
      KDateTime st = floatingEvent->dtStart();
      st.setTime( QTime( 0, 0 ) );
      KDateTime nd = floatingEvent->dtEnd();
      nd.setTime( QTime( 23, 59, 59, 999 ) );
      floatingEvent->setFloats( false );
      floatingEvent->setDtStart( st );
      floatingEvent->setDtEnd( nd );

      kDebug(5800) << "Use: " << st.toString() << " to " << nd.toString()
                    << endl;
      // Finally, use this event for the setting below
      event = floatingEvent;
    }

    // This whole for loop is for recurring events, it loops through
    // each of the days of the freebusy request

    // First check if this is transparent. If it is, it shouldn't be in the
    // freebusy list
    if ( event->transparency() == Event::Transparent )
      // Transparent
      continue;

    for( i = 0; i <= duration; ++i ) {
      day=start.addDays(i).date();
      tmpStart.setDate(day);
      tmpEnd.setDate(day);

      if( event->doesRecur() ) {
        if ( event->isMultiDay() ) {
// FIXME: This doesn't work for sub-daily recurrences or recurrences with
//        a different time than the original event.
          extraDays = event->dtStart().daysTo(event->dtEnd());
          for ( x = 0; x <= extraDays; ++x ) {
            if ( event->recursOn(day.addDays(-x), mCalendar->timeSpec())) {
              tmpStart.setDate(day.addDays(-x));
              tmpStart.setTime(event->dtStart().time());
              tmpEnd=tmpStart.addSecs( (event->duration()) );

              addLocalPeriod( tmpStart, tmpEnd );
              break;
            }
          }
        } else {
          if (event->recursOn(day, mCalendar->timeSpec())) {
            tmpStart.setTime(event->dtStart().time());
            tmpEnd.setTime(event->dtEnd().time());

            addLocalPeriod (tmpStart, tmpEnd);
          }
        }
      }

    }
    // Non-recurring events
    addLocalPeriod(event->dtStart(), event->dtEnd());

    // Clean up
    delete floatingEvent;
  }

  sortList();
}

void FreeBusy::setDtStart( const QDateTime &dtStart )
{
  if (mCalendar)
    setDtStart(KDateTime(dtStart, mCalendar->timeSpec()));
  else
    setDtStart(KDateTime(dtStart));  // use local time zone
}
bool FreeBusy::setDtEnd( const QDateTime &end )
{
  if (dtStart().isValid())
    return setDtEnd(KDateTime(end, dtStart().timeSpec()));
  else if (mCalendar)
    return setDtEnd(KDateTime(end, mCalendar->timeSpec()));
  else
    return setDtEnd(KDateTime(end));  // use local time zone
}

void FreeBusy::shiftTimes(const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec)
{
  IncidenceBase::shiftTimes( oldSpec, newSpec );
  mDtEnd = mDtEnd.toTimeSpec( oldSpec );
  mDtEnd.setTimeSpec( newSpec );
  for ( int i = 0, end = mBusyPeriods.count();  i < end;  ++end)
    mBusyPeriods[i].shiftTimes( oldSpec, newSpec );
}

