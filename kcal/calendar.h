/*
    This file is part of the kcal library.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
  @file calendar.h
  Provides the main "calendar" object class.

  @author Preston Brown
  @author Cornelius Schumacher
  @author Reinhold Kainhofer

  @port4 getOwner() method renamed to owner().
  @port4 appendAlarms() and appendRecurringAlarms() methods have
  been moved into this class from CalendarLocal.
 */
#ifndef KCAL_CALENDAR_H
#define KCAL_CALENDAR_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMultiHash>

#include <kdatetime.h>

#include "customproperties.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "kcalversion.h"
#include "person.h"

namespace KCal {

class ICalTimeZone;
class ICalTimeZones;
class CalFilter;

/**
  Calendar Incidence sort directions.
*/
enum SortDirection {
  SortDirectionAscending,  /**< Sort in ascending order (first to last) */
  SortDirectionDescending  /**< Sort in descending order (last to first) */
};

/**
  Calendar Event sort keys.
*/
enum EventSortField {
  EventSortUnsorted,       /**< Events are to be unsorted */
  EventSortStartDate,      /**< Sort Events chronologically, by start date */
  EventSortEndDate,        /**< Sort Events chronologically, by end date */
  EventSortSummary         /**< Sort Events alphabetically, by summary */
};

/**
  Calendar Todo sort keys.
*/
enum TodoSortField {
  TodoSortUnsorted,        /**< Todos are to be unsorted */
  TodoSortStartDate,       /**< Sort Todos chronologically, by start date */
  TodoSortDueDate,         /**< Sort Todos chronologically, by due date */
  TodoSortPriority,        /**< Sort Todos by priority */
  TodoSortPercentComplete, /**< Sort Todos by percentage completed */
  TodoSortSummary          /**< Sort Todos alphabetically, by summary */
};

/**
  Calendar Journal sort keys.
*/
enum JournalSortField {
  JournalSortUnsorted,     /**< Journals are to be unsorted */
  JournalSortDate,         /**< Sort Journals chronologically by date */
  JournalSortSummary       /**< Sort Journals alphabetically, by summary */
};

/**
  @brief
  >This is the main "calendar" object class.  It holds information like
  Incidences(Events, To-dos, Journals), time zones, user information, etc. etc.

  This is an abstract base class defining the interface to a calendar. It is
  implemented by subclasses like CalendarLocal, which use different
  methods to store and access the data.

  <b>Ownership of Incidences</b>:

  Incidence ownership is handled by the following policy: As soon as an
  Incidence (or any other subclass of IncidenceBase) object is added to the
  Calendar by an add...() method it is owned by the Calendar object.
  The Calendar takes care of deleting it.  All Incidences returned by the
  query functions are returned as pointers so that changes to the returned
  Incidences are immediately visible in the calendar.  Do <em>Not</em>
  delete any Incidence object you get from Calendar.

  <b>Time Zone Handling</b>:

  - Incidence Storing:
     - By default, (when LocalTime is unset) Incidence dates will have the
       "UTC" time zone when stored into a calendar file.
     - To store Incidence dates without a time zone (i.e, "floating time
       zone") LocalTime must be set using the setLocalTime() method.

  - Incidence Viewing:
     - By default, Incidence dates will have the "UTC" time zone when
       read from a calendar.
     - To view Incidence dates using another time zone TimeZoneId must
       be set using the setTimeZoneId() method, or the TimeZoneId can
       be passed to the Calendar constructor.
     - It is permitted to switch viewing time zones using setTimeZoneId()
       as desired after the Calendar object has been constructed.

  - Note that:
     - The Calendar class doesn't do anything with TimeZoneId: it simply
       saves it for later use by the ICalFormat class.
     - The ICalFormat class takes TimeZoneId and applies it to loaded
       Incidences before returning them in ICalFormat::load().
     - Each Incidence can have its own time zone (or have a floating
       time zone).
     - Once an Incidence is loaded it is adjusted to use the viewing
       time zone, TimeZoneId.
     - Depending on the LocalTime setting, all loaded Incidences are stored
       either in UTC or without a time zone (floating time zone).
*/
class KCAL_EXPORT Calendar : public QObject, public CustomProperties,
                             public IncidenceBase::Observer
{
  Q_OBJECT

  public:

    /**
      Construct Calendar object using a time specification (time zone, etc.).
      The time specification is used for creating or modifying incidences
      in the Calendar. It is also used for viewing incidences (see
      setViewTimeSpec()). The time specification does not alter existing
      incidences.

      @param timeSpec time specification
    */
    Calendar( const KDateTime::Spec &timeSpec );

    /**
      Construct Calendar object using a time zone ID.
      The time zone ID is used for creating or modifying incidences in the
      Calendar. It is also used for viewing incidences. The time zone does
      not alter existing incidences.

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid.  If no time zone is found, the viewing time
      specification is set to local clock time.
      @e Example: "Europe/Berlin"
    */
    explicit Calendar( const QString &timeZoneId );

    /**
      >Destructor
    */
    virtual ~Calendar();

    /**
      Sets the calendar Product ID to @p productId.

      @param productId is a string containing the Product ID.

      @see productId() const
    */
    void setProductId( const QString &productId );

    /**
      Returns the calendar's Product ID.

      @return the string containing the Product ID.

      @see setProductId()
    */
    QString productId() const;

    /**
      Sets the owner of the calendar to @p owner.

      @param owner is a #Person object.

      @see owner()
    */
    void setOwner( const Person &owner );

    /**
      Returns the owner of the calendar.

      @return the owner Person object.

      @see setOwner()
    */
    const Person &owner() const;

    /**
      Sets the default time specification (time zone, etc.) used for creating
      or modifying incidences in the Calendar. It also sets the time
      specification for viewing incidences (see setViewTimeSpec()).

      @param timeSpec time specification
    */
    void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
       Get the time specification (time zone etc.) used for creating or
       modifying incidences in the Calendar.

       @return time specification
    */
    KDateTime::Spec timeSpec() const;

    /**
      Sets the time zone ID used for creating or modifying incidences in the
      Calendar. Also sets the time zone for viewing incidences. This method
      has no effect on existing incidences.

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid. The time zone ID is used to set the time zone
      for viewing Incidence date/times. If no time zone is found, the
      viewing time specification is set to local clock time.
      @e Example: "Europe/Berlin"
      @see setTimeSpec()
    */
    void setTimeZoneId( const QString &timeZoneId );

    /**
      Returns the time zone ID used for creating or modifying incidences in
      the calendar.

      @return the string containing the time zone ID, or empty string if the
              creation/modification time specification is not a time zone.
    */
    QString timeZoneId() const;

    /**
      Sets the time zone used for viewing the incidences in this calendar. This
      is simply a convenience method which makes a note of the new time zone so
      that it can be read back by viewTimeSpec(). It has no effect on the
      calendar data or on the the creation or modification of incidences.
      @ref viewTimeSpec()
    */
    void setViewTimeSpec( const KDateTime::Spec &spec ) const;
    void setViewTimeZoneId( const QString &timeZoneId ) const;
    virtual KDE_DEPRECATED void setTimeZoneIdViewOnly( const QString &timeZoneId ) { setViewTimeZoneId(timeZoneId); };

    /**
      Returns the time specification (time zone, etc.) used for viewing the
      incidences in this calendar. This simply returns the time specification
      last set by setViewTimeSpec().
    */
    KDateTime::Spec viewTimeSpec() const;
    QString viewTimeZoneId() const;

    /**
      Shifts the times of all incidences so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldSpec) of Europe/London, to a new time
      zone (@p newSpec) of Europe/Paris, will result in the time being shifted
      from 14:00 (which is the London time of the incidence start) to 14:00 Paris
      time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification
    */
    void shiftTimes(const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec);

    /**
      Returns the time zone collection used by the calendar.

      @return the time zones collection.
    */
    ICalTimeZones *timeZones() const;

    /**
       Set the time zone collection used by the calendar.

       @param zones time zones collection. Important: all time zones references
                    in the calendar must be included in the collection.
     */
    void setTimeZones( const ICalTimeZones &zones );

    /**
      Sets if the calendar has been modified.

      @param modified is true if the calendar has been modified since open
      or last save.
    */
    void setModified( bool modified );

    /**
      Determine the calendar's modification status.

      @return true if the calendar has been modified since open or last save.
    */
    bool isModified() const;

    /**
      Clears out the current calendar, freeing all used memory etc.
    */
    virtual void close() = 0;

    /**
      Sync changes in memory to persistent storage.
    */
    virtual void save() = 0;

    /**
      Load the calendar contents from storage. This requires the calendar
      to have been loaded once before, in other words initialized.
    */
    virtual bool reload() = 0;
    /** Use reload() plus setTimeZoneId() instead. */
    KDE_DEPRECATED virtual bool reload( const QString &tz ) = 0;

    /**
      Determine if the calendar is currently being saved.

      @return true if the calendar is currently being saved; false otherwise.
    */
    virtual bool isSaving() { return false; }

    /**
      Return a list of all categories used by Incidences in this Calendar.

      @return a QStringList containing all the categories.
    */
    QStringList categories();

// Incidence Specific Methods //

    /**
      Insert an Incidence into the calendar.

      @param incidence is a pointer to the Incidence to insert.

      @return true if the Incidence was successfully inserted; false otherwise.
    */
    virtual bool addIncidence( Incidence *incidence );

    /**
      Remove an Incidence from the calendar.

      @param incidence is a pointer to the Incidence to remove.

      @return true if the Incidence was successfully removed; false otherwise.
    */
    virtual bool deleteIncidence( Incidence *incidence );

    /**
      Return a filtered list of all Incidences for this Calendar.

      @return the list of all filtered Incidences.
    */
    virtual Incidence::List incidences();

    /**
      Return a filtered list of all Incidences which occur on the given date.

      @param date request filtered Incidence list for this QDate only.

      @return the list of filtered Incidences occurring on the specified date.
    */
    virtual Incidence::List incidences( const QDate &date );

    /**
      Return an unfiltered list of all Incidences for this Calendar.

      @return the list of all unfiltered Incidences.
    */
    virtual Incidence::List rawIncidences();

    /**
      Returns the Incidence associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the Incidence.
      A null pointer is returned if no such Incidence exists.
    */
    Incidence *incidence( const QString &uid );

    /**
      Returns the Incidence associated with the given scheduling identifier.

      @param sid is a unique scheduling identifier string.

      @return a pointer to the Incidence.
      A null pointer is returned if no such Incidence exists.
    */
    Incidence *incidenceFromSchedulingID( const QString &sid );

    /**
      Create a merged list of Events, Todos, and Journals.

      @param events is an Event list to merge.
      @param todos is a Todo list to merge.
      @param journals is a Journal list to merge.

      @return a list of merged Incidences.
    */
    static Incidence::List mergeIncidenceList( const Event::List &events,
                                               const Todo::List &todos,
                                               const Journal::List &journals );

    /**
      Flag that a change to a Calendar Incidence is starting.

      @param incidence is a pointer to the Incidence that will be changing.
    */
    virtual bool beginChange( Incidence *incidence );

    /**
      Flag that a change to a Calendar Incidence has completed.

      @param incidence is a pointer to the Incidence that was changed.
    */
    virtual bool endChange( Incidence *incidence );

// Dissociate a single occurrence or all future occurrences from a recurring
// sequence. The new incidence is returned, but not automatically inserted
// into the calendar, which is left to the calling application.
    /**
      Dissociate an Incidence from a recurring Incidence.
      By default, only one single Incidence for the specified @a date
      will be dissociated and returned.  If @a single is false, then
      the recurrence will be split at @a date, the old Incidence will
      have its recurrence ending at @a date and the new Incidence
      will have all recurrences past the @a date.

      @param incidence is a pointer to a recurring Incidence.
      @param date is the QDate within the recurring Incidence on which
      the dissociation will be performed.
      @param single is a flag meaning that a new Incidence should be created
      from the recurring Incidences after @a date.

      @return a pointer to a new recurring Incidence if @a single is false.
    */
    Incidence *dissociateOccurrence( Incidence *incidence, const QDate &date,
                                     bool single = true );

// Event Specific Methods //

    /**
      Insert an Event into the calendar.

      @param event is a pointer to the Event to insert.

      @return true if the Event was successfully inserted; false otherwise.
    */
    virtual bool addEvent( Event *event ) = 0;

    /**
      Remove an Event from the calendar.

      @param event is a pointer to the Event to remove.

      @return true if the Event was successfully remove; false otherwise.
    */
    virtual bool deleteEvent( Event *event ) = 0;

    /**
      Sort a list of Events.

      @param eventList is a pointer to a list of Events.
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return a list of Events sorted as specified.
    */
    static Event::List sortEvents( Event::List *eventList,
                                   EventSortField sortField,
                                   SortDirection sortDirection );
    /**
      Return a sorted, filtered list of all Events for this Calendar.

      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all filtered Events sorted as specified.
    */
    virtual Event::List events(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Return a filtered list of all Events which occur on the given timestamp.

      @param dt request filtered Event list for this KDateTime only.

      @return the list of filtered Events occurring on the specified timestamp.
    */
    Event::List events( const KDateTime &dt );
    KDE_DEPRECATED Event::List events( const QDateTime &qdt );

    /**
      Return a filtered list of all Events occurring within a date range.

      @param start is the starting date.
      @param end is the ending date.
      @param inclusive if true only Events which are completely included
      within the date range are returned.

      @return the list of filtered Events occurring within the specified
      date range.
    */
    Event::List events( const QDate &start, const QDate &end,
                        bool inclusive = false );

    /**
      Return a sorted, filtered list of all Events which occur on the given
      date.  The Events are sorted according to @a sortField and
      @a sortDirection.

      @param date request filtered Event list for this QDate only.
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of sorted, filtered Events occurring on @a date.
    */
    Event::List events(
      const QDate &date,
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Return a sorted, unfiltered list of all Events for this Calendar.

      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all unfiltered Events sorted as specified.
    */
    virtual Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Return an unfiltered list of all Events which occur on the given
      timestamp.

      @param dt request unfiltered Event list for this KDateTime only.

      @return the list of unfiltered Events occurring on the specified
      timestamp.
    */
    virtual Event::List rawEventsForDate( const KDateTime &dt ) = 0;
    KDE_DEPRECATED Event::List rawEventsForDate( const QDateTime &qdt );

    /**
      Return an unfiltered list of all Events occurring within a date range.

      @param start is the starting date.
      @param end is the ending date.
      @param inclusive if true only Events which are completely included
      within the date range are returned.

      @return the list of unfiltered Events occurring within the specified
      date range.
    */
    virtual Event::List rawEvents( const QDate &start, const QDate &end,
                                   bool inclusive = false ) = 0;

    /**
      Return a sorted, unfiltered list of all Events which occur on the given
      date.  The Events are sorted according to @a sortField and
      @a sortDirection.

      @param date request unfiltered Event list for this QDate only.
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of sorted, unfiltered Events occurring on @a date.
    */
    virtual Event::List rawEventsForDate(
      const QDate &date,
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns the Event associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the Event.
      A null pointer is returned if no such Event exists.
    */
    virtual Event *event( const QString &uid ) = 0;

// Todo Specific Methods //

    /**
      Insert a Todo into the calendar.

      @param todo is a pointer to the Todo to insert.

      @return true if the Todo was successfully inserted; false otherwise.
    */
    virtual bool addTodo( Todo *todo ) = 0;

    /**
      Remove a Todo from the calendar.

      @param todo is a pointer to the Todo to remove.

      @return true if the Todo was successfully removed; false otherwise.
    */
    virtual bool deleteTodo( Todo *todo ) = 0;

    /**
      Sort a list of Todos.

      @param todoList is a pointer to a list of Todos.
      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.

      @return a list of Todos sorted as specified.
    */
    static Todo::List sortTodos( Todo::List *todoList,
                                 TodoSortField sortField,
                                 SortDirection sortDirection );

    /**
      Return a sorted, filtered list of all Todos for this Calendar.

      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all filtered Todos sorted as specified.
    */
    virtual Todo::List todos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Return a filtered list of all Todos which are due on the specified date.

      @param date request filtered Todos due on this QDate.

      @return the list of filtered Todos due on the specified date.
    */
    virtual Todo::List todos( const QDate &date );

    /**
      Return a sorted, unfiltered list of all Todos for this Calendar.

      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all unfiltered Todos sorted as specified.
    */
    virtual Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Return an unfiltered list of all Todos which due on the specified date.

      @param date request unfiltered Todos due on this QDate.

      @return the list of unfiltered Todos due on the specified date.
    */
    virtual Todo::List rawTodosForDate( const QDate &date ) = 0;

    /**
      Returns the Todo associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the Todo.
      A null pointer is returned if no such Todo exists.
    */
    virtual Todo *todo( const QString &uid ) = 0;

// Journal Specific Methods //

    /**
      Insert a Journal into the calendar.

      @param journal is a pointer to the Journal to insert.

      @return true if the Journal was successfully inserted; false otherwise.
    */
    virtual bool addJournal( Journal *journal ) = 0;

    /**
      Remove a Journal from the calendar.

      @param journal is a pointer to the Journal to remove.

      @return true if the Journal was successfully removed; false otherwise.
    */
    virtual bool deleteJournal( Journal *journal ) = 0;

    /**
      Sort a list of Journals.

      @param journalList is a pointer to a list of Journals.
      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.

      @return a list of Journals sorted as specified.
    */
    static Journal::List sortJournals( Journal::List *journalList,
                                       JournalSortField sortField,
                                       SortDirection sortDirection );
    /**
      Return a sorted, filtered list of all Journals for this Calendar.

      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all filtered Journals sorted as specified.
    */
    virtual Journal::List journals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Return a filtered list of all Journals for on the specified date.

      @param date request filtered Journals for this QDate only.

      @return the list of filtered Journals for the specified date.
    */
    virtual Journal::List journals( const QDate &date );

    /**
      Return a sorted, unfiltered list of all Journals for this Calendar.

      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all unfiltered Journals sorted as specified.
    */
    virtual Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Return an unfiltered list of all Journals for on the specified date.

      @param date request unfiltered Journals for this QDate only.

      @return the list of unfiltered Journals for the specified date.
    */
    virtual Journal::List rawJournalsForDate( const QDate &date ) = 0;

    /**
      Returns the Journal associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the Journal.
      A null pointer is returned if no such Journal exists.
    */
    virtual Journal *journal( const QString &uid ) = 0;

// Relations Specific Methods //

    /**
      Setup Relations for an Incidence.

      @param incidence is a pointer to the Incidence to have a
      Relation setup.
    */
    virtual void setupRelations( Incidence *incidence );

    /**
      Remove all Relations from an Incidence.

      @param incidence is a pointer to the Incidence to have a
      Relation removed.
    */
    virtual void removeRelations( Incidence *incidence );

// Filter Specific Methods //

    /**
      Sets the calendar filter.

      @param filter a pointer to a CalFilter object which will be
      used to filter Calendar Incidences.
    */
    void setFilter( CalFilter *filter );

    /**
      Return the calendar filter.

      @return a pointer to the calendar CalFilter.
      A null pointer is returned if no such CalFilter exists.
    */
    CalFilter *filter();

// Alarm Specific Methods //

    /**
      Return a list of Alarms within a time range for this Calendar.

      @param from is the starting timestamp.
      @param to is the ending timestamp.

      @return the list of Alarms for the for the specified time range.
    */
    virtual Alarm::List alarms( const KDateTime &from,
                                const KDateTime &to ) = 0;
    KDE_DEPRECATED Alarm::List alarms( const QDateTime &from,
                                const QDateTime &to );

// Observer Specific Methods //

    /**
      @class Observer

      The Observer class.
    */
    class Observer
    {
      public:
        /**
          Destructor.
        */
        virtual ~Observer() {}

        /**
          Notify the Observer that a Calendar has been modified.

          @param modified set if the calendar has been modified.
          @param calendar is a pointer to the Calendar object that
          is being observed.
        */
        virtual void calendarModified( bool modified,
                                       Calendar *calendar )
        { Q_UNUSED( modified ); Q_UNUSED( calendar ); }

        /**
          Notify the Observer that an Incidence has been inserted.

          @param incidence is a pointer to the Incidence that was inserted.
        */
        virtual void calendarIncidenceAdded( Incidence *incidence )
        { Q_UNUSED( incidence );}

        /**
          Notify the Observer that an Incidence has been modified.

          @param incidence is a pointer to the Incidence that was modified.
        */
        virtual void calendarIncidenceChanged( Incidence *incidence )
        { Q_UNUSED( incidence ); }

        /**
          Notify the Observer that an Incidence has been removed.

          @param incidence is a pointer to the Incidence that was removed.
        */
        virtual void calendarIncidenceDeleted( Incidence *incidence )
        { Q_UNUSED( incidence ); }
    };

    /**
      Register an Observer for this Calendar.

      @param observer is a pointer to an Observer object that will be
      watching this Calendar.
     */
    void registerObserver( Observer *observer );

    /**
      Unregister an Observer for this Calendar.

      @param observer is a pointer to an Observer object that has been
      watching this Calendar.
     */
    void unregisterObserver( Observer *observer );

  signals:
    /**
      Signal that the calendar has been modified.
     */
    void calendarChanged();

    /**
      Signal that the calendar has been saved.
     */
    void calendarSaved();

    /**
      Signal that the calendar has been loaded into memory.
     */
    void calendarLoaded();

  protected:
    /**
      The Observer interface. So far not implemented.

      @param incidenceBase is a pointer an IncidenceBase object.
    */
    void incidenceUpdated( IncidenceBase *incidenceBase );

    /**
      Let Calendar subclasses set the time specification.

      @param timeSpec is the time specification (time zone, etc.) for
                      viewing Incidence dates.\n
    */
    virtual void doSetTimeSpec( const KDateTime::Spec &/*timeSpec*/ ) {}

    /**
      Let Calendar subclasses notify that they inserted an Incidence.

      @param incidence is a pointer to the Incidence object that was inserted.
    */
    void notifyIncidenceAdded( Incidence *incidence );

    /**
      Let Calendar subclasses notify that they modified an Incidence.

      @param incidence is a pointer to the Incidence object that was modified.
    */
    void notifyIncidenceChanged( Incidence *incidence );

    /**
      Let Calendar subclasses notify that they removed an Incidence.

      @param incidence is a pointer to the Incidence object that was removed.
    */
    void notifyIncidenceDeleted( Incidence *incidence );

    /**
      Let Calendar subclasses notify that they enabled an Observer.

      @param enabled if true tells the calendar that a subclass has
      enabled an Observer.
    */
    void setObserversEnabled( bool enabled );

    /** Append alarms of incidence in interval to list of alarms. */
    void appendAlarms( Alarm::List &alarms, Incidence *incidence,
                       const KDateTime &from, const KDateTime &to );
    KDE_DEPRECATED void appendAlarms( Alarm::List &alarms, Incidence *incidence,
                       const QDateTime &from, const QDateTime &to );

    /** Append alarms of recurring events in interval to list of alarms. */
    void appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
                       const KDateTime &from, const KDateTime &to );
    KDE_DEPRECATED void appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
                       const QDateTime &from, const QDateTime &to );

  private:
    //@cond PRIVATE
    class Private;
    Private *d;
    //@endcond
  };

}

#endif
