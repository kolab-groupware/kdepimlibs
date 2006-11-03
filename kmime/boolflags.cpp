/*
  Copyright (c) 1999-2001 the KMime authors.
  See file AUTHORS for details

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

#include "boolflags.h"

using namespace KMime;

void BoolFlags::set( unsigned int i, bool b )
{
  if ( i > 15 ) return;

  unsigned char p; //bitmask
  int n;

  if ( i < 8 ) { //first byte
    p = (1 << i);
    n = 0;
  } else { //second byte
    p = (1 << i-8);
    n = 1;
  }

  if ( b ) {
    bits[n] = bits[n] | p;
  } else {
    bits[n] = bits[n] & (255 - p);
  }
}

bool BoolFlags::get( unsigned int i )
{
  if ( i > 15 ) return false;

  unsigned char p; //bitmask
  int n;

  if ( i < 8 ) { //first byte
    p = (1 << i);
    n = 0;
  } else { //second byte
    p = (1 << i-8);
    n = 1;
  }

  return ( (bits[n] & p)>0 );
}
