/*
  This file is part of the kcalcore library.

  Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  Contact: Alvaro Manera <alvaro.manera@nokia.com>

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
#ifndef KCALCORE_SORTING_P_H
#define KCALCORE_SORTING_P_H

#include "event.h"
#include "journal.h"
#include "todo.h"
#include "freebusy.h"
#include "person.h"

#include "kcalcore_export.h"

namespace KCalCore {

namespace Events {

  KCALCORE_EXPORT bool startDateLessThan( const Event::ConstPtr &e1, const Event::ConstPtr &e2 );

  KCALCORE_EXPORT bool summaryLessThan( const Event::ConstPtr &e1, const Event::ConstPtr &e2 );

  KCALCORE_EXPORT bool summaryMoreThan( const Event::ConstPtr &e1, const Event::ConstPtr &e2 );

  KCALCORE_EXPORT bool startDateMoreThan( const Event::ConstPtr &e1, const Event::ConstPtr &e2 );

  KCALCORE_EXPORT bool endDateLessThan( const Event::ConstPtr &e1, const Event::ConstPtr &e2 );

  KCALCORE_EXPORT bool endDateMoreThan( const Event::ConstPtr &e1, const Event::ConstPtr &e2 );

}

namespace Todos {

  KCALCORE_EXPORT bool startDateLessThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool startDateMoreThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool dueDateLessThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool dueDateMoreThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool priorityLessThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool priorityMoreThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool percentLessThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool percentMoreThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool summaryLessThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool summaryMoreThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool createdLessThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

  KCALCORE_EXPORT bool createdMoreThan( const Todo::ConstPtr &t1, const Todo::ConstPtr &t2 );

}

namespace Journals {

  KCALCORE_EXPORT bool dateLessThan( const Journal::ConstPtr &j1, const Journal::ConstPtr &j2 );

  KCALCORE_EXPORT bool dateMoreThan( const Journal::ConstPtr &j1, const Journal::ConstPtr &j2 );

  KCALCORE_EXPORT bool summaryLessThan( const Journal::ConstPtr &j1, const Journal::ConstPtr &j2 );

  KCALCORE_EXPORT bool summaryMoreThan( const Journal::ConstPtr &j1, const Journal::ConstPtr &j2 );

}

namespace Incidences {

  KCALCORE_EXPORT bool dateLessThan( const Incidence::ConstPtr &i1,
                                     const Incidence::ConstPtr &i2 );

  KCALCORE_EXPORT bool dateMoreThan( const Incidence::ConstPtr &i1,
                                     const Incidence::ConstPtr &i2 );

  KCALCORE_EXPORT bool createdLessThan( const Incidence::ConstPtr &i1,
                                        const Incidence::ConstPtr &i2 );

  KCALCORE_EXPORT bool createdMoreThan( const Incidence::ConstPtr &i1,
                                        const Incidence::ConstPtr &i2 );

  KCALCORE_EXPORT bool summaryLessThan( const Incidence::ConstPtr &i1,
                                        const Incidence::ConstPtr &i2 );

  KCALCORE_EXPORT bool summaryMoreThan( const Incidence::ConstPtr &i1,
                                        const Incidence::ConstPtr &i2 );

}

namespace Persons {
  KCALCORE_EXPORT bool countMoreThan( const Person::ConstPtr &p1, const Person::ConstPtr &p2 );
}

}

#endif
