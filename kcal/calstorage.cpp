/*
  This file is part of the kcal library.

  Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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
  defines the CalStorage abstract base class.

  @brief
  An abstract base class that provides a calendar storage interface.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "calstorage.h"
#include "calendar.h"

using namespace KCal;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCal::CalStorage::Private
{
  public:
    Private( Calendar *cal )
      : mCalendar( cal )
    {}
    Calendar *mCalendar;
};
//@endcond

CalStorage::CalStorage( Calendar *cal )
  : d( new KCal::CalStorage::Private ( cal ) )
{
}

CalStorage::~CalStorage()
{
  delete d;
}

Calendar *CalStorage::calendar() const
{
  return d->mCalendar;
}
