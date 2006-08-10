/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef SYNDICATION_ATOM_CONSTANTS_H
#define SYNDICATION_ATOM_CONSTANTS_H

#include "ksyndication.h"

class QString;

namespace Syndication {
namespace Atom {

/**
 * namespace used by Atom 1.0 elements
 */
SYNDICATION_EXPORT
QString atom1Namespace();

/**
 * namespace used by Atom 0.3 elements
 */
SYNDICATION_EXPORT
QString atom0_3Namespace();

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_CONSTANTS_H
