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
/**
  @file
  This file is part of the API for handling calendar data and
  defines the ICalFormat class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCAL_ICALFORMAT_H
#define KCAL_ICALFORMAT_H

#include "calformat.h"
#include "scheduler.h"

#include <kdatetime.h>

#include <QtCore/QString>
#include <QtCore/QByteArray>

namespace KCal {

class ICalFormatImpl;
class FreeBusy;

/**
  @brief
  iCalendar format implementation.

  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal
  representation as Calendar and Incidences.
*/
class KCAL_EXPORT ICalFormat : public CalFormat
{
  public:
    /**
      Constructor a new iCalendar Format object.
    */
    ICalFormat();

    /**
      Destructor.
    */
    virtual ~ICalFormat();

    /**
      @copydoc
      CalFormat::load()
    */
    bool load( Calendar *calendar, const QString &fileName );

    /**
      @copydoc
      CalFormat::save()
    */
    bool save( Calendar *calendar, const QString &fileName );

    /**
      @copydoc
      CalFormat::fromString()
    */
    bool fromString( Calendar *calendar, const QString &string );

    /**
      Parses a string, returning the first iCal component as an Incidence.

      @param string is a QString containing the data to be parsed.

      @return non-zero pointer if the parsing was successful; 0 otherwise.
      @see fromString(Calendar *, const QString &), fromRawString()
    */
    Incidence *fromString( const QString &string );

    /**
      Parses a string and fills a RecurrenceRule object with the information.

      @param rule is a pointer to a RecurrenceRule object.
      @param string is a QString containing the data to be parsed.
      @return true if successful; false otherwise.
    */
    bool fromString ( RecurrenceRule *rule, const QString &string );

    /**
      @copydoc
      CalFormat::fromRawString()
    */
    bool fromRawString( Calendar *calendar, const QByteArray &string );

    /**
      @copydoc
      CalFormat::toString()
    */
    QString toString( Calendar *calendar );

    /**
      Converts an Incidence to a QString.
      @param incidence is a pointer to an Incidence object to be converted
      into a QString.

      @return the QString will be Null if the conversion was unsuccessful.
    */
    QString toString( Incidence *incidence );

    /**
      Converts a RecurrenceRule to a QString.
      @param rule is a pointer to a RecurrenceRule object to be converted
      into a QString.

      @return the QString will be Null if the conversion was unsuccessful.
    */
    QString toString( RecurrenceRule *rule );

    /**
      Converts an Incidence to iCalendar formatted text.

      @param incidence is a pointer to an Incidence object to be converted
      into iCal formatted text.
      @return the QString will be Null if the conversion was unsuccessful.
    */
    QString toICalString( Incidence *incidence );

    /**
      Creates a scheduling message string for an Incidence.

      @param incidence is a pointer to an IncidenceBase object to be scheduled.
      @param method is a Scheduler::Method

      @return a QString containing the message if successful; 0 otherwise.
    */
    QString createScheduleMessage( IncidenceBase *incidence,
                                   iTIPMethod method );

    /**
      Parses a Calendar scheduling message string into ScheduleMessage object.

      @param calendar is a pointer to a Calendar object associated with the
      scheduling message.
      @param string is a QString containing the data to be parsed.

      @return a pointer to a ScheduleMessage object if successful; 0 otherwise.
      The calling routine may later free the return memory.
    */
    ScheduleMessage *parseScheduleMessage( Calendar *calendar, const QString &string );

    /**
      Converts a QString into a FreeBusy object.

      @param string is a QString containing the data to be parsed.
      @return a pointer to a FreeBusy object if successful; 0 otherwise.

      @note Do not attempt to free the FreeBusy memory from the calling routine.
    */
    FreeBusy *parseFreeBusy( const QString &string );

    /**
      Sets the iCalendar time specification (time zone, etc.).
      @param timeSpec is the time specification to set.
      @see timeSpec().
    */
    void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
      Returns the iCalendar time specification.
      @see setTimeSpec().
    */
    KDateTime::Spec timeSpec() const;

    /**
      Returns the timezone id string used by the iCalendar; an empty string
      if the iCalendar does not have a timezone.
    */
    QString timeZoneId() const;

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
