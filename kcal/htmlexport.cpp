/*
  This file is part of the kcal library.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "htmlexport.h"
#include "htmlexportsettings.h"
#include "calendar.h"
#include "event.h"
#include "todo.h"
#ifndef KORG_NOKABC
 #include "kabc/stdaddressbook.h"
#endif

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcalendarsystem.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QTextCodec>
#include <QtCore/QRegExp>
#include <QtCore/QMap>
#include <QtGui/QApplication>

using namespace KCal;

static QString cleanChars( const QString &txt );

class KCal::HtmlExport::Private
{
  public:
    Private( Calendar *calendar, HTMLExportSettings *settings )
      : mCalendar( calendar ),
        mSettings( settings )
    {}

    Calendar *mCalendar;
    HTMLExportSettings *mSettings;
    QMap<QDate,QString> mHolidayMap;
};

HtmlExport::HtmlExport( Calendar *calendar, HTMLExportSettings *settings )
  : d( new Private( calendar, settings ) )
{
}

HtmlExport::~HtmlExport()
{
  delete d;
}

bool HtmlExport::save( const QString &fileName )
{
  QString fn( fileName );
  if ( fn.isEmpty() && d->mSettings ) {
    fn = d->mSettings->outputFile();
  }
  if ( !d->mSettings || fn.isEmpty() ) {
    return false;
  }
  QFile f( fileName );
  if ( !f.open( QIODevice::WriteOnly ) ) {
    return false;
  }
  QTextStream ts( &f );
  bool success = save( &ts );
  f.close();
  return success;
}

bool HtmlExport::save( QTextStream *ts )
{
  if ( !d->mSettings ) {
    return false;
  }
  ts->setCodec( "UTF-8" );

  // Write HTML header
  *ts << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" ";
  *ts << "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n";

  *ts << "<html><head>" << endl;
  *ts << "  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=";
  *ts << "UTF-8\" />\n";
  if ( !d->mSettings->pageTitle().isEmpty() ) {
    *ts << "  <title>" << d->mSettings->pageTitle() << "</title>\n";
  }
  *ts << "  <style type=\"text/css\">\n";
  *ts << styleSheet();
  *ts << "  </style>\n";
  *ts << "</head><body>\n";

  // FIXME: Write header
  // (Heading, Calendar-Owner, Calendar-Date, ...)

  if ( d->mSettings->eventView() || d->mSettings->monthView() || d->mSettings->weekView() ) {
    if ( !d->mSettings->eventTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->eventTitle() << "</h1>\n";
    }

    // Write Week View
    if ( d->mSettings->weekView() ) {
      createWeekView( ts );
    }
    // Write Month View
    if ( d->mSettings->monthView() ) {
      createMonthView( ts );
    }
    // Write Event List
    if ( d->mSettings->eventView() ) {
      createEventList( ts );
    }
  }

  // Write Todo List
  if ( d->mSettings->todoView() ) {
    if ( !d->mSettings->todoListTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->todoListTitle() << "</h1>\n";
    }
    createTodoList( ts );
  }

  // Write Journals
  if ( d->mSettings->journalView() ) {
    if ( !d->mSettings->journalTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->journalTitle() << "</h1>\n";
    }
    createJournalView( ts );
  }

  // Write Free/Busy
  if ( d->mSettings->freeBusyView() ) {
    if ( !d->mSettings->freeBusyTitle().isEmpty() ) {
      *ts << "<h1>" << d->mSettings->freeBusyTitle() << "</h1>\n";
    }
    createFreeBusyView( ts );
  }

  createFooter( ts );

  // Write HTML trailer
  *ts << "</body></html>\n";

  return true;
}

void HtmlExport::createMonthView( QTextStream *ts )
{
  QDate start = fromDate();
  start.setYMD( start.year(), start.month(), 1 );  // go back to first day in month

  QDate end( start.year(), start.month(), start.daysInMonth() );

  int startmonth = start.month();
  int startyear = start.year();

  while ( start < toDate() ) {
    // Write header
    *ts << "<h2>"
        << i18nc( "@title month and year", "%1 %2",
                  KGlobal::locale()->calendar()->monthName( start ), start.year() )
        << "</h2>\n";
    if ( KGlobal::locale()->weekStartDay() == 1 ) {
      start = start.addDays( 1 - start.dayOfWeek() );
    } else {
      if ( start.dayOfWeek() != 7 ) {
        start = start.addDays( -start.dayOfWeek() );
      }
    }
    *ts << "<table border=\"1\">\n";

    // Write table header
    *ts << "  <tr>";
    for ( int i=0; i < 7; ++i ) {
      *ts << "<th>" << KGlobal::locale()->calendar()->weekDayName( start.addDays(i) ) << "</th>";
    }
    *ts << "</tr>\n";

    // Write days
    while ( start <= end ) {
      *ts << "  <tr>\n";
      for ( int i=0; i < 7; ++i ) {
        *ts << "    <td valign=\"top\"><table border=\"0\">";

        *ts << "<tr><td ";
        if ( d->mHolidayMap.contains( start ) || start.dayOfWeek() == 7 ) {
          *ts << "class=\"dateholiday\"";
        } else {
          *ts << "class=\"date\"";
        }
        *ts << ">" << QString::number( start.day() );

        if ( d->mHolidayMap.contains( start ) ) {
          *ts << " <em>" << d->mHolidayMap[start] << "</em>";
        }

        *ts << "</td></tr><tr><td valign=\"top\">";

        Event::List events = d->mCalendar->events( start, d->mCalendar->timeSpec(),
                                                   EventSortStartDate,
                                                   SortDirectionAscending );
        if ( events.count() ) {
          *ts << "<table>";
          Event::List::ConstIterator it;
          for ( it = events.begin(); it != events.end(); ++it ) {
            if ( checkSecrecy( *it ) ) {
              createEvent( ts, *it, start, false );
            }
          }
          *ts << "</table>";
        } else {
          *ts << "&nbsp;";
        }

        *ts << "</td></tr></table></td>\n";
        start = start.addDays( 1 );
      }
      *ts << "  </tr>\n";
    }
    *ts << "</table>\n";
    startmonth += 1;
    if ( startmonth > 12 ) {
      startyear += 1;
      startmonth = 1;
    }
    start.setYMD( startyear, startmonth, 1 );
    end.setYMD( start.year(), start.month(), start.daysInMonth() );
  }
}

void HtmlExport::createEventList( QTextStream *ts )
{
  int columns = 3;
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18nc( "@title:column", "Start Time" ) << "</th>\n";
  *ts << "    <th>" << i18nc( "@title:column", "End Time" ) << "</th>\n";
  *ts << "    <th>" << i18nc( "@title:column", "Event" ) << "</th>\n";
  if ( d->mSettings->eventLocation() ) {
    *ts << "    <th>" << i18nc( "@title:column", "Location" ) << "</th>\n";
    ++columns;
  }
  if ( d->mSettings->eventCategories() ) {
    *ts << "    <th>" << i18nc( "@title:column", "Categories" ) << "</th>\n";
    ++columns;
  }
  if ( d->mSettings->eventAttendees() ) {
    *ts << "    <th>" << i18nc( "@title:column", "Attendees" ) << "</th>\n";
    ++columns;
  }

  *ts << "  </tr>\n";

  for ( QDate dt = fromDate(); dt <= toDate(); dt = dt.addDays(1) ) {
    kDebug(5800) << "Getting events for" << dt.toString();
    Event::List events = d->mCalendar->events( dt, d->mCalendar->timeSpec(),
                                               EventSortStartDate,
                                               SortDirectionAscending );
    if ( events.count() ) {
      *ts << "  <tr><td colspan=\"" << QString::number( columns )
          << "\" class=\"datehead\"><i>"
          << KGlobal::locale()->formatDate( dt )
          << "</i></td></tr>\n";

      Event::List::ConstIterator it;
      for ( it = events.begin(); it != events.end(); ++it ) {
        if ( checkSecrecy( *it ) ) {
          createEvent( ts, *it, dt );
        }
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createEvent ( QTextStream *ts, Event *event,
                               QDate date, bool withDescription )
{
  kDebug(5800) << "HtmlExport::createEvent():" << event->summary();
  *ts << "  <tr>\n";

  if ( !event->floats() ) {
    if ( event->isMultiDay( d->mCalendar->timeSpec() ) && ( event->dtStart().date() != date ) ) {
      *ts << "    <td>&nbsp;</td>\n";
    } else {
      *ts << "    <td valign=\"top\">"
          << event->dtStartTimeStr( true, d->mCalendar->timeSpec() ) << "</td>\n";
    }
    if ( event->isMultiDay( d->mCalendar->timeSpec() ) && ( event->dtEnd().date() != date ) ) {
      *ts << "    <td>&nbsp;</td>\n";
    } else {
      *ts << "    <td valign=\"top\">"
          << event->dtEndTimeStr( true, d->mCalendar->timeSpec() ) << "</td>\n";
    }
  } else {
    *ts << "    <td>&nbsp;</td><td>&nbsp;</td>\n";
  }

  *ts << "    <td class=\"sum\">\n";
  *ts << "      <b>" << cleanChars( event->summary() ) << "</b>\n";
  if ( withDescription && !event->description().isEmpty() ) {
    *ts << "      <p>" << breakString( cleanChars( event->description() ) ) << "</p>\n";
  }
  *ts << "    </td>\n";

  if ( d->mSettings->eventLocation() ) {
    *ts << "  <td>\n";
    formatLocation( ts, event );
    *ts << "  </td>\n";
  }

  if ( d->mSettings->eventCategories() ) {
    *ts << "  <td>\n";
    formatCategories( ts, event );
    *ts << "  </td>\n";
  }

  if ( d->mSettings->eventAttendees() ) {
    *ts << "  <td>\n";
    formatAttendees( ts, event );
    *ts << "  </td>\n";
  }

  *ts << "  </tr>\n";
}

void HtmlExport::createTodoList ( QTextStream *ts )
{
  Todo::List rawTodoList = d->mCalendar->todos();

  int index = 0;
  while ( index < rawTodoList.count() ) {
    Todo *ev = rawTodoList[ index ];
    Todo *subev = ev;
    if ( ev->relatedTo() ) {
      if ( ev->relatedTo()->type() == "Todo" ) {
        if ( !rawTodoList.contains( static_cast<Todo *>( ev->relatedTo() ) ) ) {
          rawTodoList.append( static_cast<Todo *>( ev->relatedTo() ) );
        }
      }
    }
    index = rawTodoList.indexOf( subev );
    ++index;
  }

  // FIXME: Sort list by priorities. This is brute force and should be
  // replaced by a real sorting algorithm.
  Todo::List todoList;
  Todo::List::ConstIterator it;
  for ( int i = 1; i <= 9; ++i ) {
    for ( it = rawTodoList.begin(); it != rawTodoList.end(); ++it ) {
      if ( (*it)->priority() == i && checkSecrecy( *it ) ) {
        todoList.append( *it );
      }
    }
  }
  for ( it = rawTodoList.begin(); it != rawTodoList.end(); ++it ) {
    if ( (*it)->priority() == 0 && checkSecrecy( *it ) ) {
      todoList.append( *it );
    }
  }

  int columns = 3;
  *ts << "<table border=\"0\" cellpadding=\"3\" cellspacing=\"3\">\n";
  *ts << "  <tr>\n";
  *ts << "    <th class=\"sum\">" << i18nc( "@title:column", "Task" ) << "</th>\n";
  *ts << "    <th>" << i18nc( "@title:column to-do priority", "Priority" ) << "</th>\n";
  *ts << "    <th>" << i18nc( "@title:column to-do percent completed", "Completed" ) << "</th>\n";
  if ( d->mSettings->taskDueDate() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do due date", "Due Date" ) << "</th>\n";
    ++columns;
  }
  if ( d->mSettings->taskLocation() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do location", "Location" ) << "</th>\n";
    ++columns;
  }
  if ( d->mSettings->taskCategories() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do categories", "Categories" ) << "</th>\n";
    ++columns;
  }
  if ( d->mSettings->taskAttendees() ) {
    *ts << "    <th>" << i18nc( "@title:column to-do attendees", "Attendees" ) << "</th>\n";
    ++columns;
  }
  *ts << "  </tr>\n";

  // Create top-level list.
  for ( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !(*it)->relatedTo() ) {
      createTodo( ts, *it );
    }
  }

  // Create sub-level lists
  for ( it = todoList.begin(); it != todoList.end(); ++it ) {
    Incidence::List relations = (*it)->relations();
    if ( relations.count() ) {
      // Generate sub-task list
      *ts << "  <tr>\n";
      *ts << "    <td class=\"subhead\" colspan=";
      *ts << "\"" << QString::number(columns) << "\"";
      *ts << "><a name=\"sub" << (*it)->uid() << "\"></a>"
          << i18nc( "@title:column", "Sub-Tasks of: " ) << "<a href=\"#"
          << (*it)->uid() << "\"><b>" << cleanChars( (*it)->summary() )
          << "</b></a></td>\n";
      *ts << "  </tr>\n";

      Todo::List sortedList;
      // FIXME: Sort list by priorities. This is brute force and should be
      // replaced by a real sorting algorithm.
      for ( int i = 1; i <= 9; ++i ) {
        Incidence::List::ConstIterator it2;
        for ( it2 = relations.begin(); it2 != relations.end(); ++it2 ) {
          Todo *ev3 = dynamic_cast<Todo *>( *it2 );
          if ( ev3 && ev3->priority() == i ) {
            sortedList.append( ev3 );
          }
        }
      }
      Incidence::List::ConstIterator it2;
      for ( it2 = relations.begin(); it2 != relations.end(); ++it2 ) {
        Todo *ev3 = dynamic_cast<Todo *>( *it2 );
        if ( ev3 && ev3->priority() == 0 ) {
          sortedList.append( ev3 );
        }
      }

      Todo::List::ConstIterator it3;
      for ( it3 = sortedList.begin(); it3 != sortedList.end(); ++it3 ) {
        createTodo( ts, *it3 );
      }
    }
  }

  *ts << "</table>\n";
}

void HtmlExport::createTodo( QTextStream *ts, Todo *todo )
{
  kDebug(5800) << "HtmlExport::createTodo()";

  bool completed = todo->isCompleted();
  Incidence::List relations = todo->relations();

  *ts << "<tr>\n";

  *ts << "  <td class=\"sum";
  if (completed) *ts << "done";
  *ts << "\">\n";
  *ts << "    <a name=\"" << todo->uid() << "\"></a>\n";
  *ts << "    <b>" << cleanChars( todo->summary() ) << "</b>\n";
  if ( !todo->description().isEmpty() ) {
    *ts << "    <p>" << breakString( cleanChars( todo->description() ) ) << "</p>\n";
  }
  if ( relations.count() ) {
    *ts << "    <div align=\"right\"><a href=\"#sub" << todo->uid()
        << "\">" << i18nc( "@title:column", "Sub-Tasks" ) << "</a></div>\n";
  }
  *ts << "  </td>\n";

  *ts << "  <td";
  if ( completed ) {
    *ts << " class=\"done\"";
  }
  *ts << ">\n";
  *ts << "    " << todo->priority() << "\n";
  *ts << "  </td>\n";

  *ts << "  <td";
  if ( completed ) {
    *ts << " class=\"done\"";
  }
  *ts << ">\n";
  *ts << "    " << i18nc( "@info to-do percent complete",
                          "%1 %", todo->percentComplete() ) << "\n";
  *ts << "  </td>\n";

  if ( d->mSettings->taskDueDate() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">\n";
    if ( todo->hasDueDate() ) {
      *ts << "    " << todo->dtDueDateStr() << "\n";
    } else {
      *ts << "    &nbsp;\n";
    }
    *ts << "  </td>\n";
  }

  if ( d->mSettings->taskLocation() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">\n";
    formatLocation( ts, todo );
    *ts << "  </td>\n";
  }

  if ( d->mSettings->taskCategories() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">\n";
    formatCategories( ts, todo );
    *ts << "  </td>\n";
  }

  if ( d->mSettings->taskAttendees() ) {
    *ts << "  <td";
    if ( completed ) {
      *ts << " class=\"done\"";
    }
    *ts << ">\n";
    formatAttendees( ts, todo );
    *ts << "  </td>\n";
  }

  *ts << "</tr>\n";
}

void HtmlExport::createWeekView( QTextStream *ts )
{
  Q_UNUSED( ts );
  // FIXME: Implement this!
}

void HtmlExport::createJournalView( QTextStream *ts )
{
  Q_UNUSED( ts );
//   Journal::List rawJournalList = d->mCalendar->journals();
  // FIXME: Implement this!
}

void HtmlExport::createFreeBusyView( QTextStream *ts )
{
  Q_UNUSED( ts );
  // FIXME: Implement this!
}

bool HtmlExport::checkSecrecy( Incidence *incidence )
{
  int secrecy = incidence->secrecy();
  if ( secrecy == Incidence::SecrecyPublic ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyPrivate && !d->mSettings->excludePrivate() ) {
    return true;
  }
  if ( secrecy == Incidence::SecrecyConfidential &&
       !d->mSettings->excludeConfidential() ) {
    return true;
  }
  return false;
}

void HtmlExport::formatLocation( QTextStream *ts, Incidence *incidence )
{
  if ( !incidence->location().isEmpty() ) {
    *ts << "    " << cleanChars( incidence->location() ) << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void HtmlExport::formatCategories( QTextStream *ts, Incidence *incidence )
{
  if ( !incidence->categoriesStr().isEmpty() ) {
    *ts << "    " << cleanChars( incidence->categoriesStr() ) << "\n";
  } else {
    *ts << "    &nbsp;\n";
  }
}

void HtmlExport::formatAttendees( QTextStream *ts, Incidence *incidence )
{
  Attendee::List attendees = incidence->attendees();
  if ( attendees.count() ) {
    *ts << "<em>";
#ifndef KORG_NOKABC
    KABC::AddressBook *add_book = KABC::StdAddressBook::self( true );
    KABC::Addressee::List addressList;
    addressList = add_book->findByEmail( incidence->organizer().email() );
    KABC::Addressee o = addressList.first();
    if ( !o.isEmpty() && addressList.size() < 2 ) {
      *ts << "<a href=\"mailto:" << incidence->organizer().email() << "\">";
      *ts << cleanChars( o.formattedName() ) << "</a>\n";
    } else {
      *ts << incidence->organizer().fullName();
    }
#else
    *ts << incidence->organizer().fullName();
#endif
    *ts << "</em><br />";
    Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      Attendee *a = *it;
      if ( !a->email().isEmpty() ) {
        *ts << "<a href=\"mailto:" << a->email();
        *ts << "\">" << cleanChars( a->name() ) << "</a>";
      } else {
        *ts << "    " << cleanChars( a->name() );
      }
      *ts << "<br />" << "\n";
    }
  } else {
    *ts << "    &nbsp;\n";
  }
}

QString HtmlExport::breakString( const QString &text )
{
  int number = text.count( "\n" );
  if ( number <= 0 ) {
    return text;
  } else {
    QString out;
    QString tmpText = text;
    int pos = 0;
    QString tmp;
    for ( int i=0; i<=number; i++ ) {
      pos = tmpText.indexOf( "\n" );
      tmp = tmpText.left( pos );
      tmpText = tmpText.right( tmpText.length() - pos - 1 );
      out += tmp + "<br />";
    }
    return out;
  }
}

void HtmlExport::createFooter( QTextStream *ts )
{
  // FIXME: Implement this in a translatable way!
  QString trailer = i18nc( "@info", "This page was created " );

/*  bool hasPerson = false;
  bool hasCredit = false;
  bool hasCreditURL = false;
  QString mail, name, credit, creditURL;*/
  if ( !d->mSettings->eMail().isEmpty() ) {
    if ( !d->mSettings->name().isEmpty() ) {
      trailer += i18nc( "@info page creator email link with name",
                        "by <link url='mailto:%1'>%2</link>",
                        d->mSettings->eMail(), d->mSettings->name() );
    } else {
      trailer += i18nc( "@info page creator email link",
                        "by <link url='mailto:%1'>%2</link>",
                        d->mSettings->eMail(), d->mSettings->eMail() );
    }
  } else {
    if ( !d->mSettings->name().isEmpty() ) {
      trailer += i18nc( "@info page creator name only",
                        "by %1 ", d->mSettings->name() );
    }
  }
  if ( !d->mSettings->creditName().isEmpty() ) {
    if ( !d->mSettings->creditURL().isEmpty() ) {
      trailer += i18nc( "@info page credit with name and link",
                        "with <link url='%1'>%2</link>",
                        d->mSettings->creditURL(), d->mSettings->creditName() );
    } else {
      trailer += i18nc( "@info page credit name only",
                        "with %1", d->mSettings->creditName() );
    }
  }
  *ts << "<p>" << trailer << "</p>\n";
}

QString cleanChars( const QString &text )
{
  QString txt = text;
  txt = txt.replace( "&", "&amp;" );
  txt = txt.replace( "<", "&lt;" );
  txt = txt.replace( ">", "&gt;" );
  txt = txt.replace( "\"", "&quot;" );
  txt = txt.replace( QString::fromUtf8( "ä" ), "&auml;" );
  txt = txt.replace( QString::fromUtf8( "Ä" ), "&Auml;" );
  txt = txt.replace( QString::fromUtf8( "ö" ), "&ouml;" );
  txt = txt.replace( QString::fromUtf8( "Ö" ), "&Ouml;" );
  txt = txt.replace( QString::fromUtf8( "ü" ), "&uuml;" );
  txt = txt.replace( QString::fromUtf8( "Ü" ), "&Uuml;" );
  txt = txt.replace( QString::fromUtf8( "ß" ), "&szlig;" );
  txt = txt.replace( QString::fromUtf8( "€" ), "&euro;" );
  txt = txt.replace( QString::fromUtf8( "é" ), "&eacute;" );

  return txt;
}

QString HtmlExport::styleSheet() const
{
  if ( !d->mSettings->styleSheet().isEmpty() ) {
    return d->mSettings->styleSheet();
  }

  QString css;

  if ( QApplication::isRightToLeft() ) {
    css += "    body { background-color:white; color:black; direction: rtl }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sumdone { background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.dateholiday { color:red }\n";
  } else {
    css += "    body { background-color:white; color:black }\n";
    css += "    td { text-align:center; background-color:#eee }\n";
    css += "    th { text-align:center; background-color:#228; color:white }\n";
    css += "    td.sum { text-align:left }\n";
    css += "    td.sumdone { text-align:left; background-color:#ccc }\n";
    css += "    td.done { background-color:#ccc }\n";
    css += "    td.subhead { text-align:center; background-color:#ccf }\n";
    css += "    td.datehead { text-align:center; background-color:#ccf }\n";
    css += "    td.space { background-color:white }\n";
    css += "    td.date { text-align:left }\n";
    css += "    td.dateholiday { text-align:left; color:red }\n";
  }

  return css;
}

void HtmlExport::addHoliday( const QDate &date, const QString &name )
{
  if ( d->mHolidayMap[date].isEmpty() ) {
    d->mHolidayMap[date] = name;
  } else {
    d->mHolidayMap[date] = i18nc( "@info holiday by date and name",
                                  "%1, %2", d->mHolidayMap[date], name );
  }
}

QDate HtmlExport::fromDate() const
{
  return d->mSettings->dateStart().date();
}

QDate HtmlExport::toDate() const
{
  return d->mSettings->dateEnd().date();
}
