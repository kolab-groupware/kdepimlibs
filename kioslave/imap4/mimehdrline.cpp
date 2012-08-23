/***************************************************************************
                          mimehdrline.cc  -  description
                             -------------------
    begin                : Wed Oct 11 2000
    copyright            : (C) 2000 by Sven Carstens
    email                : s.carstens@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mimehdrline.h"
#include <iostream>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

mimeHdrLine::mimeHdrLine ():
mimeValue (), mimeLabel ()
{
}

mimeHdrLine::mimeHdrLine (const QByteArray & aLabel, const QByteArray & aValue):
mimeValue (aValue),
mimeLabel (aLabel)
{
}

mimeHdrLine::mimeHdrLine (mimeHdrLine * aHdrLine):
mimeValue (aHdrLine->mimeValue), mimeLabel (aHdrLine->mimeLabel)
{
}

mimeHdrLine::~mimeHdrLine ()
{
}

int mimeHdrLine::appendStr (const char *aCStr)
{
  int retVal = 0;
  int skip;

  if ( aCStr ) {
    skip = skipWS( aCStr );
    if ( skip && !mimeLabel.isEmpty() ) {
      if ( skip > 0 ) {
        mimeValue += QByteArray( aCStr, skip );
        aCStr += skip;
        retVal += skip;
        skip = parseFullLine( aCStr );
        mimeValue += QByteArray( aCStr, skip );
        retVal += skip;
        aCStr += skip;
      }
    } else {
      if ( mimeLabel.isEmpty() ) {
        return setStr( aCStr );
      }
    }
  }
  return retVal;
}

/** parse a Line into the class
move input ptr accordingly
and report characters slurped */
int
mimeHdrLine::setStr (const char *aCStr)
{
  int retVal = 0;
//  char *begin = aCStr;
  mimeLabel = QByteArray();
  mimeValue = QByteArray();

  if ( aCStr ) {
    // can't have spaces on normal lines
    if ( !skipWS( aCStr ) ) {
      int label = 0, advance;
      while ( ( advance = parseWord( &aCStr[label] ) ) ) {
        label += advance;
      }
      if ( label && aCStr[label - 1] != ':' ) {
        retVal = 0;
      } else {
        mimeLabel = QByteArray( aCStr, label-1 );
        retVal += label;
        aCStr += label;
      }
    }
    if ( retVal ) {
      int skip;
      skip = skipWS( aCStr );
      if ( skip < 0 ) {
        skip *= -1;
      }
      aCStr += skip;
      retVal += skip;
      skip = parseFullLine( aCStr );
      mimeValue = QByteArray( aCStr, skip );
      retVal += skip;
      aCStr += skip;
    } else {
      //Skip malformed line
      while ( *aCStr && *aCStr != '\r' && *aCStr != '\n' ) {
        retVal--;
        aCStr++;
      }
      if ( *aCStr == '\r' ) {
        retVal--;
        aCStr++;
      }
      if ( *aCStr == '\n' ) {
        retVal--;
        aCStr++;
      }
    }
  } else {
    //debug
  }
  return retVal;
}

/** slurp one word*/
int mimeHdrLine::parseWord (const char *aCStr)
{
  int retVal = 0;

  if ( aCStr && *aCStr ) {
    if ( *aCStr == '"' ) {
      return mimeHdrLine::parseQuoted( '"', '"', aCStr );
    } else {
      return mimeHdrLine::parseHalfWord( aCStr );
    }
  } else {
    //debug();
  }
  return retVal;
}

/** slurp one word*/
int mimeHdrLine::parseQuoted (char startQuote, char endQuote, const char *aCStr)
{
  int retVal = 0;

  if ( aCStr && *aCStr ) {
    if ( *aCStr == startQuote ) {
      aCStr++;
      retVal++;
    } else {
      return 0;
    }
    while ( *aCStr && *aCStr != endQuote ) {
      //skip over backticks
      if ( *aCStr == '\\' ) {
        aCStr++;
        retVal++;
      }
      //eat this
      aCStr++;
      retVal++;
    }
    if ( *aCStr == endQuote ) {
      aCStr++;
      retVal++;
    }
  } else {
    //debug();
  }
  return retVal;
}

/** slurp one alphanumerical word without continuation*/
int mimeHdrLine::parseAlphaNum (const char *aCStr)
{
  int retVal = 0;

  if ( aCStr ) {
    while ( *aCStr && isalnum( *aCStr ) ) {
      //skip over backticks
      if ( *aCStr == '\\' ) {
        aCStr++;
        retVal++;
      }
      //eat this
      aCStr++;
      retVal++;
    }
  } else {
    //debug();
  }
  return retVal;
}

int mimeHdrLine::parseHalfWord (const char *aCStr)
{
  int retVal = 0;

  if ( aCStr && *aCStr ) {
    if ( isalnum( *aCStr ) ) {
      return mimeHdrLine::parseAlphaNum( aCStr );
    }
    //skip over backticks
    if ( *aCStr == '\\' ) {
      aCStr++;
      retVal++;
    } else if ( !isspace( *aCStr ) ) {
      //eat this
      aCStr++;
      retVal++;
    }
  } else {
    //debug();
  }
  return retVal;
}

/** slurp one line without continuation*/
int mimeHdrLine::parseHalfLine (const char *aCStr)
{
  int retVal = 0;

  if ( aCStr ) {
    while ( *aCStr && *aCStr != '\n' ) {
      //skip over backticks
      if ( *aCStr == '\\' ) {
        aCStr++;
        retVal++;
      }
      //eat this
      aCStr++;
      retVal++;
    }
    if ( *aCStr == '\n' ) {
      aCStr++;
      retVal++;
    }
  } else {
    //debug();
  }
  return retVal;
}

/** skip all white space characters including continuation*/
int mimeHdrLine::skipWS (const char *aCStr)
{
  int retVal = 0;

  if ( aCStr && *aCStr ) {
    while ( *aCStr == ' ' || *aCStr == '\t' ) {
      aCStr++;
      retVal++;
    }
    //check out for continuation lines
    if ( *aCStr == '\r' ) {
      aCStr++;
      retVal++;
    }
    if ( *aCStr++ == '\n' ) {
      if ( *aCStr == '\t' || *aCStr == ' ' ) {
        int skip = mimeHdrLine::skipWS( aCStr );
        if ( skip < 0 ) {
          skip *= -1;
        }
        retVal += 1 + skip;
      } else {
        retVal = -retVal - 1;
      }
    }
  } else {
    //debug();
  }
  return retVal;
}

/** parses continuated lines */
int mimeHdrLine::parseFullLine (const char *aCStr)
{
  int retVal = 0;
  int skip;

  if ( aCStr ) {
    //skip leading white space
    skip = skipWS( aCStr );
    if ( skip > 0 ) {
      aCStr += skip;
      retVal += skip;
    }
    while ( *aCStr ) {
      int advance;

      if ( ( advance = parseHalfLine( aCStr ) ) ) {
        retVal += advance;
        aCStr += advance;
      } else if ( ( advance = skipWS( aCStr ) ) ) {
        if ( advance > 0 ) {
          retVal += advance;
          aCStr += advance;
        } else {
          retVal -= advance;
          break;
        }
      } else {
        break;
      }
    }
  } else {
    //debug();
  }
  return retVal;
}

/** parses continuated lines */
int mimeHdrLine::parseSeparator (char separator, const char *aCStr)
{
  int retVal = 0;
  int skip;

  if ( aCStr ) {
    //skip leading white space
    skip = skipWS( aCStr );
    if ( skip > 0 ) {
      aCStr += skip;
      retVal += skip;
    }
    while ( *aCStr ) {
      int advance;

      if ( *aCStr != separator ) {
        if ( ( advance = mimeHdrLine::parseWord( aCStr ) ) ) {
          retVal += advance;
          aCStr += advance;
        } else if ( ( advance = mimeHdrLine::skipWS( aCStr ) ) ) {
          if ( advance > 0 ) {
            retVal += advance;
            aCStr += advance;
          } else {
            retVal -= advance;
            break;
          }
        } else {
          break;
        }
      } else {
        //include separator in result
        retVal++;
        aCStr++;
        break;
      }
    }
  } else {
    //debug();
  }
  return retVal;
}

/** return the label */

const QByteArray& mimeHdrLine::getLabel ()
{
  return mimeLabel;
}

/** return the value */
const QByteArray& mimeHdrLine::getValue ()
{
  return mimeValue;
}


// FIXME: very inefficient still
QByteArray mimeHdrLine::truncateLine(QByteArray aLine, unsigned int truncate)
{
  int cutHere;
  QByteArray retVal;
  uint len = aLine.length();

  // see if we have a line of the form "key: value" (like "Subject: bla")
  // then we do not want to truncate between key and value
  int validStart = aLine.indexOf(": ");
  if ( validStart > -1 ) {
    validStart += 2;
  }
  while ( len > truncate ) {
    cutHere = aLine.lastIndexOf( ' ', truncate );
    if ( cutHere < 1 || cutHere < validStart ) {
      cutHere = aLine.lastIndexOf( '\t', truncate );
      if ( cutHere < 1 ) {
        cutHere = aLine.indexOf( ' ', 1 );
        if ( cutHere < 1 ) {
          cutHere = aLine.indexOf( '\t', 1 );
          if ( cutHere < 1 ) {
            // simply truncate
            return aLine.left( truncate );
          }
        }
      }
    }

    retVal += aLine.left( cutHere ) + '\n';
    int chop = len - cutHere;
    aLine = aLine.right( chop );
    len -= chop;
  }
  retVal += aLine;

  return retVal;
}
