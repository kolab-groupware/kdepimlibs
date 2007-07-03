/*
 * spellingfilter.cpp
 *
 * Copyright (c) 2002 Dave Corrie <kde@davecorrie.com>
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
 */

#include "spellingfilter.h"

#include <kdebug.h>

using namespace KPIMUtils;

//-----------------------------------------------------------------------------
// SpellingFilter implementation
//

SpellingFilter::SpellingFilter( const QString &text,
                                const QString &quotePrefix,
                                UrlFiltering filterUrls,
                                EmailAddressFiltering filterEmailAddresses,
                                const QStringList &filterStrings )
  : mOriginal( text )
{
  TextCensor c( text );

  if ( !quotePrefix.isEmpty() ) {
    c.censorQuotations( quotePrefix );
  }

  if ( filterUrls ) {
    c.censorUrls();
  }

  if ( filterEmailAddresses ) {
    c.censorEmailAddresses();
  }

  QStringList::const_iterator iter = filterStrings.begin();
  while ( iter != filterStrings.end() ) {
    c.censorString( *iter );
    ++iter;
  }

  mFiltered = c.censoredText();
}

QString SpellingFilter::originalText() const
{
  return mOriginal;
}

QString SpellingFilter::filteredText() const
{
  return mFiltered;
}

//-----------------------------------------------------------------------------
// SpellingFilter::TextCensor implementation
//

SpellingFilter::TextCensor::TextCensor( const QString &s )
  : LinkLocator( s )
{
}

void SpellingFilter::TextCensor::censorQuotations( const QString &quotePrefix )
{
  mPos = 0;
  while ( mPos < mText.length() ) {
    // Find start of quotation
    findQuotation( quotePrefix );
    if ( mPos < mText.length() ) {
      int start = mPos;
      skipQuotation( quotePrefix );

      // Replace quotation with spaces
      int len = mPos - start;
      QString spaces;
      spaces.fill( ' ', len );
      mText.replace( start, len, spaces );

      //kDebug(5321) << "censored quotation ["
      //  << start << ", " << mPos << ")" << endl;
    }
  }
}

void SpellingFilter::TextCensor::censorUrls()
{
  mPos = 0;
  while ( mPos < mText.length() ) {
    // Find start of url
    QString url;
    while ( mPos < mText.length() && url.isEmpty() ) {
      url = getUrl();
      ++mPos;
    }

    if ( mPos < mText.length() && !url.isEmpty() ) {
      int start = mPos - url.length();

      // Replace url with spaces
      url.fill( ' ' );
      mText.replace( start, url.length(), url );

      //kDebug(5321) << "censored url ["
      //  << start << ", " << mPos << ")" << endl;
    }
  }
}

void SpellingFilter::TextCensor::censorEmailAddresses()
{
  mPos = 0;
  while ( mPos < mText.length() ) {
    // Find start of email address
    findEmailAddress();
    if ( mPos < mText.length() ) {
      QString address = getEmailAddress();
      ++mPos;
      if ( !address.isEmpty() ) {
        int start = mPos - address.length();

        // Replace address with spaces
        address.fill( ' ' );
        mText.replace( start, address.length(), address );

        //kDebug(5321) << "censored addr ["
        //  << start << ", "<< mPos << ")" << endl;
      }
    }
  }
}

void SpellingFilter::TextCensor::censorString( const QString &s )
{
  mPos = 0;
  while ( mPos != -1 ) {
    // Find start of string
    mPos = mText.indexOf( s, mPos );
    if ( mPos != -1 ) {
      // Replace string with spaces
      QString spaces;
      spaces.fill( ' ', s.length() );
      mText.replace( mPos, s.length(), spaces );
      mPos += s.length();

      //kDebug(5321) << "censored string ["
      //  << mPos << ", "<< mPos+s.length() << ")" << endl;
    }
  }
}

QString SpellingFilter::TextCensor::censoredText() const
{
  return mText;
}

//-----------------------------------------------------------------------------
// text censorship helper functions
//

bool SpellingFilter::TextCensor::atLineStart() const
{
  return
    ( mPos == 0 && mText.length() > 0 ) ||
    ( mText[mPos - 1] == '\n' );
}

void SpellingFilter::TextCensor::skipLine()
{
  mPos = mText.indexOf( '\n', mPos );
  if ( mPos == -1 ) {
    mPos = mText.length();
  } else {
    ++mPos;
  }
}

bool SpellingFilter::TextCensor::atQuotation( const QString &quotePrefix ) const
{
  return atLineStart() &&
    mText.mid(mPos, quotePrefix.length()) == quotePrefix;
}

void SpellingFilter::TextCensor::skipQuotation( const QString &quotePrefix )
{
  while ( atQuotation( quotePrefix ) ) {
    skipLine();
  }
}

void SpellingFilter::TextCensor::findQuotation( const QString &quotePrefix )
{
  while ( mPos < mText.length() &&
          !atQuotation( quotePrefix ) ) {
    skipLine();
  }
}

void SpellingFilter::TextCensor::findEmailAddress()
{
  while ( mPos < mText.length() && mText[mPos] != '@' ) {
    ++mPos;
  }
}

