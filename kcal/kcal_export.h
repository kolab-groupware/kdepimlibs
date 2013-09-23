/*
    This file is part of the kcal library.

     Copyright (C) 2004 Dirk Mueller <mueller@kde.org>

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
#ifndef KCAL_EXPORT_H
#define KCAL_EXPORT_H

#include <kdemacros.h>

#ifndef KCAL_EXPORT
# if defined(KDEPIM_STATIC_LIBS)
   /* No export/import for static libraries */
#  define KCAL_EXPORT
# elif defined(MAKE_KCAL_LIB)
   /* We are building this library */
#  define KCAL_EXPORT KDE_EXPORT
# else
   /* We are using this library */
#  define KCAL_EXPORT KDE_IMPORT
# endif
#endif

# ifndef KCAL_EXPORT_DEPRECATED
#  if !defined( WANT_DEPRECATED_KCAL_API )
#    define KCAL_EXPORT_DEPRECATED KDE_DEPRECATED KCAL_EXPORT
#    define KCAL_DEPRECATED_EXPORT KDE_DEPRECATED KCAL_EXPORT
#    define KCAL_DEPRECATED KDE_DEPRECATED
#  else
#    define KCAL_EXPORT_DEPRECATED KCAL_EXPORT
#    define KCAL_DEPRECATED_EXPORT KCAL_EXPORT
#    define KCAL_DEPRECATED
#  endif
# endif

/**
  @namespace KCal

  @brief
  Contains all the KCal library global classes, objects, and functions.
*/

#endif
