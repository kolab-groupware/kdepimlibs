/*
 *  version.h  -  program version functions
 *  This file is part of kalarmcal library, which provides access to KAlarm
 *  calendar data.
 *  Copyright © 2005,2009-2011 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KALARM_VERSION_H
#define KALARM_VERSION_H

#include "kalarmcal_export.h"
#include <QString>

namespace KAlarmCal
{

/** Return a specified version as an integer. */
inline int Version(int major, int minor, int rev)     { return major*10000 + minor*100 + rev; }

/** Convert a version string to an integer. */
KALARMCAL_EXPORT int getVersionNumber(const QString& version, QString* subVersion = 0);

/** Convert a version integer to a string. */
KALARMCAL_EXPORT QString getVersionString(int version);

}

#endif // KALARM_VERSION_H

// vim: et sw=4:
