/*
    This file is part of the kcal library.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
  defines the IncidenceBase class.

  @authors
  Cornelius Schumacher \<schumacher@kde.org\>,
  Reinhold Kainhofer \<reinhold@kainhofer.com\>,
  Rafal Rzepecki \<divide@users.sourceforge.net\>

  @port4 doesFloat() method renamed to floats().

  @glossary @anchor incidence @b incidence:
  General term for a calendar component.
  Examples are events, to-dos, and journals.

  @glossary @anchor event @b event:
  An @ref incidence that has a start and end time, typically representing some
  occurrence of social or personal importance. May be recurring.
  Examples are appointments, meetings, or holidays.

  @glossary @anchor to-do @b to-do:
  An @ref incidence that has an optional start time and an optional due time
  typically representing some undertaking to be performed. May be recurring.
  Examples are "fix the bug" or "pay the bills".

  @glossary @anchor todo @b todo:
  See @ref to-do.

  @glossary @anchor journal @b journal:
  An @ref incidence with a start date that represents a diary or daily record
  of one's activities. May @b not be recurring.
*/

#ifndef KCAL_INCIDENCEBASE_H
#define KCAL_INCIDENCEBASE_H

#include <QStringList>
#include <QByteArray>
#include <QList>

#include <kdatetime.h>

#include "attendee.h"
#include "customproperties.h"
#include "sortablelist.h"

class KUrl;

namespace KCal {

/** List of dates */
typedef SortableList<QDate> DateList;
/** List of times */
typedef SortableList<KDateTime> DateTimeList;
class Event;
class Todo;
class Journal;
class FreeBusy;

/**
  @brief
  An abstract class that provides a common base for all calendar @ref incidence
  classes.

  define: floats
  define: organizer (person)
  define: uid (same as the attendee uid?)

  Several properties are not allowed for VFREEBUSY objects (see rfc:2445),
  so they are not in IncidenceBase. The hierarchy is:

  IncidenceBase
  - FreeBusy
  - Incidence
  - Event
  - Todo
  - Journal

  So IncidenceBase contains all properties that are common to all classes,
  and Incidence contains all additional properties that are common to
  Events, Todos and Journals, but are not allowed for FreeBusy entries.
*/
class KCAL_EXPORT IncidenceBase : public CustomProperties
{
  public:
    /**
      This class provides the interface for a visitor of calendar components.
      It serves as base class for concrete visitors, which implement certain
      actions on calendar components. It allows to add functions, which operate
      on the concrete types of calendar components, without changing the
      calendar component classes.
    */
    class Visitor
    {
      public:
        /** Destruct Incidence::Visitor */
        virtual ~Visitor() {}

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on an Event object.
        */
        virtual bool visit( Event *event )
        { Q_UNUSED( event ); return false; }

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on a Todo object.
        */
        virtual bool visit( Todo *todo )
        { Q_UNUSED ( todo ); return false; }

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on an Journal object.
        */
        virtual bool visit( Journal *journal )
        { Q_UNUSED( journal ); return false; }

        /**
          Reimplement this function in your concrete subclass of
          IncidenceBase::Visitor to perform actions on a FreeBusy object.
        */
        virtual bool visit( FreeBusy *freebusy )
        { Q_UNUSED( freebusy ); return false; }

      protected:
        /**
          Constructor is protected to prevent direct creation of visitor
          base class.
        */
        Visitor() {}
    };

    /**
      The Observer class.
    */
    class Observer {
      public:

        /**
          Destroys the Observer.
        */
        virtual ~Observer() {}

        /**
          The Observer interface.

          @param incidenceBase is a pointer to an IncidenceBase object.
        */
        virtual void incidenceUpdated( IncidenceBase *incidenceBase ) = 0;
    };

    /**
      Constructs an empty IncidenceBase.
    */
    IncidenceBase();

    /**
      Constructs an IncidenceBase as a copy of another IncidenceBase object.

      @param ib is the IncidenceBase to copy.
    */

    IncidenceBase( const IncidenceBase &ib );

    /**
      Destroys the IncidenceBase.
    */
    virtual ~IncidenceBase();

    /**
      Compares this with IncidenceBase @p ib for equality.

      @p ib is the IncidenceBase to compare.
    */
    bool operator==( const IncidenceBase &ib ) const;

    /**
      Accept IncidenceVisitor. A class taking part in the visitor mechanism
      has to provide this implementation:
      <pre>
        bool accept(Visitor &v) { return v.visit(this); }
      </pre>
    */
    virtual bool accept( Visitor &v )
    { Q_UNUSED( v ); return false; }

    virtual QByteArray type() const = 0;

    /**
      Sets the unique id for the incidence to @p uid.

      @param uid is the string containing the incidence @ref uid.

      @see uid()
    */
    void setUid( const QString &uid );

    /**
      Returns the unique id (@ref uid) for the incidence.

      @see setUid()
    */
    QString uid() const;

    /**
      >Returns the uri for the incidence, of form urn:x-ical:\<uid\>
    */
    KUrl uri() const;

    /**
      Sets the time the incidence was last modified to @p lm.
      It is stored as a UTC date/time.

      @param lm is the #KDateTime when the incidence was last modified.

      @see lastModified()
    */
    void setLastModified( const KDateTime &lm );

    /**
      Returns the time the incidence was last modified.

      @see setLastModified()
    */
    KDateTime lastModified() const;

    /**
      Sets the organizer for the incidence.

      @param organizer is a #Person to use as the incidence @ref organizer.

      @see organizer(), setOrganizer(const QString &)
    */
    void setOrganizer( const Person &organizer );

    /**
      Sets the incidence organizer to any string @p organizer.

      @param organizer is a string to use as the incidence @ref organizer.

      @see organizer(), setOrganizer(const Person &)
    */
    void setOrganizer( const QString &organizer );

    /**
      Returns the #Person associated with this incidence.

      @see setOrganizer(const QString &), setOrganizer(const Person &)
    */
    Person organizer() const;

    /**
      Sets readonly status.
    */
    virtual void setReadOnly( bool readOnly );

    /**
      Returns if the object is read-only.
    */
    bool isReadOnly() const { return mReadOnly; }

    /**
      Sets the incidence's starting date/time with a KDateTime.
      The incidence's floating status is set according to whether @p dtStart
      is a date/time (not floating) or date-only (floating).
    */
    virtual void setDtStart( const KDateTime &dtStart );
    virtual KDE_DEPRECATED void setDtStart( const QDateTime &dtStart )
      { setDtStart( KDateTime( dtStart ) ); }  // use local time zone

    /**
      Returns an incidence's starting date/time as a KDateTime.
    */
    virtual KDateTime dtStart() const;

    /**
      Returns an incidence's starting time as a string formatted according
      to the user's locale settings.
    */
    virtual QString dtStartTimeStr() const;

    /**
      Returns an incidence's starting date as a string formatted according
      to the user's locale settings.
    */
    virtual QString dtStartDateStr( bool shortfmt = true ) const;

    /**
      Returns an incidence's starting date and time as a string formatted
      according to the user's locale settings.
    */
    virtual QString dtStartStr() const;

    /**
      Sets the incidence duration to @p seconds seconds.

      @param seconds is the length of the incidence duration in seconds.

      @see duration()
    */
    virtual void setDuration( int seconds );

    /**
      Returns the length of the incidence duration in seconds.

      @see setDuration()
    */
    int duration() const;

    /**
      Sets if the incidence has a duration.

      @param hasDuration true if the incidence has a duration; false otherwise.

      @see hasDuration()
    */
    void setHasDuration( bool hasDuration );

    /**
      Returns true if the incidence has a duration; false otherwise.

      @see setHasDuration()
    */
    bool hasDuration() const;

    /**
      Returns true or false depending on whether the incidence "floats,"
      i.e. has a date but no time attached to it.

      @see setFloats()
    */
    bool floats() const;

    /**
      Sets whether the incidence floats, i.e. has a date but no time attached
      to it.

      @param floats sets (true) or unsets (false) whether the incidence floats.

      @see floats()
    */
    void setFloats( bool floats );

    /**
      Shift the times of the incidence so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00
      America/New York, using an old viewing time zone (@p oldSpec)
      of Europe/London, to a new time zone (@p newSpec) of Europe/Paris,
      will result in the time being shifted from 14:00 (which is the London
      time of the incidence start) to 14:00 Paris time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    /**
      Add a comment to this incidence.

      Does not add a linefeed character.  Just appends the text as passed in.

      @param comment  The comment to add.
    */
    void addComment( const QString &comment );

    /**
      Remove a comment from the incidence.

      Removes first comment whose string is an exact match for the string
      passed in.

      @return true if match found, false otherwise.
     */
    bool removeComment( const QString &comment );

    /**
      Deletes all comments associated with this incidence.
    */
    void clearComments();

    /**
      Returns all comments associated with this incidence.
    */
    QStringList comments() const;

    /**
      Add Attendee to this incidence. IncidenceBase takes ownership of the
      Attendee object.

      @param attendee a pointer to the attendee to add
      @param doUpdate If true the Observers are notified, if false they are not.
    */
    void addAttendee( Attendee *attendee, bool doUpdate = true );

    /**
      Remove all Attendees.
    */
    void clearAttendees();

    /**
      Returns list of attendees.
    */
    const Attendee::List &attendees() const;

    /**
      Returns number of attendees.
    */
    int attendeeCount() const;

    /**
      Returns the Attendee with this email address.
    */
    Attendee *attendeeByMail( const QString &email ) const;

    /**
      Returns first Attendee with one of the given email addresses.
    */
    Attendee *attendeeByMails( const QStringList &emails,
                               const QString &email = QString() ) const;

    /**
      Returns attendee with given uid.
    */
    Attendee *attendeeByUid( const QString &uid ) const;

    /**
      Pilot synchronization states
    */
    enum {
      SYNCNONE = 0,
      SYNCMOD = 1,
      SYNCDEL = 3
    };

    /**
      Sets synchronization satus.
    */
    void setSyncStatus( int status );

    /**
      Returns synchronization status.
    */
    int syncStatus() const;

    /**
      Sets Pilot Id.
    */
    void setPilotId( unsigned long id );

    /**
      Returns Pilot Id.
    */
    unsigned long pilotId() const;

    /**
      Register observer. The observer is notified when the observed object
      changes.
    */
    void registerObserver( Observer *observer );

    /**
      Unregister observer. It isn't notified anymore about changes.
    */
    void unRegisterObserver( Observer *observer );

    /**
      Call this to notify the observers after the IncidenceBase object has
      changed.
    */
    void updated();

  protected:
    bool mReadOnly;

  private:
    //@cond PRIVATE
    class Private;
    Private *d;
    //@endcond
};

}

#endif
