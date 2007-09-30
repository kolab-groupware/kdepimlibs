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
  defines the Todo class.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#ifndef KCAL_TODO_H
#define KCAL_TODO_H

#include "incidence.h"
#include <QtCore/QByteArray>

namespace KCal {

/**
  @brief
  Provides a To-do in the sense of RFC2445.
*/
class KCAL_EXPORT Todo : public Incidence
{
  public:
    /**
      List of to-dos.
    */
    typedef ListBase<Todo> List;

    /**
      Constructs an empty to-do.
    */
    Todo();

    /**
      Copy constructor.
      @param other is the to-do to copy.
    */
    Todo( const Todo &other );

    /**
      Destroys a to-do.
    */
    ~Todo();
    Todo& operator=( const Todo& );

    /**
      @copydoc
      IncidenceBase::type()
    */
    QByteArray type() const;

    /**
      Returns an exact copy of this todo. The returned object is owned by the
      caller.
    */
    Todo *clone();

    /**
      Sets due date and time.

      @param dtDue The due date/time.
      @param first If true and the todo recurs, the due date of the first
      occurrence will be returned. If false and recurrent, the date of the
      current occurrence will be returned. If non-recurrent, the normal due
      date will be returned.
    */
    void setDtDue( const KDateTime &dtDue, bool first = false );

    /**
      Returns due date and time.

      @param first If true and the todo recurs, the due date of the first
      occurrence will be returned. If false and recurrent, the date of the
      current occurrence will be returned. If non-recurrent, the normal due
      date will be returned.
    */
    KDateTime dtDue( bool first = false ) const;

    /**
      Returns due time as string formatted according to the user's locale
      settings.

      @param shortfmt If set, use short date format; else use long format.
      @param spec If set, return the time in the given spec, else use the
      todo's current spec.
    */
    QString dtDueTimeStr( bool shortfmt = true,
                          const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns due date as string formatted according to the user's locale
      settings.

      @param shortfmt If set, use short date format; else use long format.
      @param spec If set, return the date in the given spec, else use the
      todo's current spec.
    */
    QString dtDueDateStr( bool shortfmt = true,
                          const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns due date and time as string formatted according to the user's
      locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param spec If set, return the date/time in the given spec, else use
      the todo's current spec.
    */
    QString dtDueStr( bool shortfmt = true,
                      const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns true if the todo has a due date, otherwise return false.
    */
    bool hasDueDate() const;

    /**
      Sets if the todo has a due date.

      @param hasDueDate true if todo has a due date, otherwise false
    */
    void setHasDueDate( bool hasDueDate );

    /**
      Returns true if the todo has a start date, otherwise return false.
    */
    bool hasStartDate() const;

    /**
      Sets if the todo has a start date.

      @param hasStartDate true if todo has a start date, otherwise false
    */
    void setHasStartDate( bool hasStartDate );

    /**
      Returns the start date of the todo.
      @param first If true, the start date of the todo will be returned. If the
      todo recurs, the start date of the first occurrence will be returned.
      If false and the todo recurs, the relative start date will be returned,
      based on the date returned by dtRecurrence().
    */
    KDateTime dtStart( bool first = false ) const;

    /**
      Sets the start date of the todo.

      @param dtStart is the to-do start date.
    */
    void setDtStart( const KDateTime &dtStart );

    /**
      Returns a todo's starting time as a string formatted according to the
      user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param first If true, the start date of the todo will be returned. If the
      todo recurs, the start date of the first occurrence will be returned.
      If false and the todo recurs, the relative start date will be returned,
      based on the date returned by dtRecurrence().
      @param spec If set, returns the time in the given spec, else use the
      todo's current spec.
    */
    QString dtStartTimeStr( bool shortfmt = true, bool first = false,
                            const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns a todo's starting date as a string formatted according to the
      user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param first If true, the start date of the todo will be returned. If the
      todo recurs, the start date of the first occurrence will be returned.
      If false and the todo recurs, the relative start date will be returned,
      based on the date returned by dtRecurrence().
      @param spec If set, returns the date in the given spec, else use the
      todo's current spec.
    */
    QString dtStartDateStr( bool shortfmt = true, bool first = false,
                            const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns a todo's starting date and time as a string formatted according
      to the user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
      @param first If true, the start date of the todo will be returned. If the
      todo recurs, the start date of the first occurrence will be returned.
      If false and the todo recurs, the relative start date will be returned,
      based on the date returned by dtRecurrence().
      @param spec If set, returns the date and time in the given spec, else
      use the todo's current spec.
    */
    QString dtStartStr( bool shortfmt = true, bool first = false,
                        const KDateTime::Spec &spec = KDateTime::Spec() ) const;

    /**
      Returns true if the todo is 100% completed, otherwise return false.

      @see setCompleted(), percentComplete()
    */
    bool isCompleted() const;

    /**
      Sets completed state.

      @param completed If true set completed state to 100%, if false set
      completed state to 0%.

      @see isCompleted(), percentComplete()
    */
    void setCompleted( bool completed );

    /**
      Returns what percentage of the to-do is completed. Returns a value
      between 0 and 100.
    */
    int percentComplete() const;

    /**
      Sets what percentage of the to-do is completed. Valid values are in the
      range from 0 to 100.

      @param percent is the completion percentage, which as integer value
      between 0 and 100, inclusive.

      @see isCompleted(), setCompleted()
    */
    void setPercentComplete( int percent );

    /**
      Returns date and time when todo was completed.
    */
    KDateTime completed() const;

    /**
      Returns string contaiting date and time when the todo was completed
      formatted according to the user's locale settings.

      @param shortfmt If set, use short date format; else use long format.
    */
    QString completedStr( bool shortfmt = false ) const;

    /**
      Sets date and time of completion.

      @param completeDate is the to-do completion date.
    */
    void setCompleted( const KDateTime &completeDate );

    /**
      Returns true, if todo has a date associated with completion, otherwise
      return false.
    */
    bool hasCompletedDate() const;

    /**
      @copydoc
      IncidenceBase::shiftTimes()
    */
    virtual void shiftTimes( const KDateTime::Spec &oldSpec,
                             const KDateTime::Spec &newSpec );

    /**
      Sets the due date/time of the current occurrence if recurrent.

      @param dt is the
    */
    void setDtRecurrence( const KDateTime &dt );

    /**
      Returns the due date/time of the current occurrence if recurrent.
    */
    KDateTime dtRecurrence() const;

    /**
      Returns true if the @p date specified is one on which the to-do will
      recur. Todos are a special case, hence the overload. It adds an extra
      check, which make it return false if there's an occurrence between
      the recur start and today.

      @param date is the date to check.
      @param timeSpec is the
    */
    virtual bool recursOn( const QDate &date,
                           const KDateTime::Spec &timeSpec ) const;

    /**
      Returns true if this todo is overdue (e.g. due date is lower than today
      and not completed), else false.
     */
    bool isOverdue() const;

    /**
      Compare this with @p todo for equality.

      @param todo is the to-do to compare.
    */
    bool operator==( const Todo &todo ) const;

  protected:
    /**
      Returns the end date/time of the base incidence.
    */
    virtual KDateTime endDateRecurrenceBase() const;

  private:
    /**
      @copydoc
      IncidenceBase::accept()
    */
    bool accept( Visitor &v ) { return v.visit( this ); }

    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};

}

#endif
