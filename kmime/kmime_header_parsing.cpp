/*  -*- c++ -*-
    kmime_header_parsing.cpp

    KMime, the KDE Internet mail/usenet news message library.
    Copyright (c) 2001-2002 Marc Mutz <mutz@kde.org>

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

#include "kmime_header_parsing.h"

#include "kmime_codecs.h"
#include "kmime_headerfactory_p.h"
#include "kmime_headers.h"
#include "kmime_util.h"
#include "kmime_util_p.h"
#include "kmime_dateformatter.h"
#include "kmime_warning.h"

#include <kglobal.h>
#include <kcharsets.h>

#include <QtCore/QTextCodec>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

#include <ctype.h> // for isdigit
#include <cassert>

using namespace KMime;
using namespace KMime::Types;

namespace KMime {

namespace Types {

// QUrl::fromAce is extremely expensive, so only use it when necessary.
// Fortunately, the presence of IDNA is readily detected with a substring match...
static inline QString QUrl_fromAce_wrapper( const QString & domain )
{
    if ( domain.contains( QLatin1String( "xn--" ) ) )
        return QUrl::fromAce( domain.toLatin1() );
    else
        return domain;
}

static QString addr_spec_as_string( const AddrSpec & as, bool pretty )
{
  if ( as.isEmpty() ) {
    return QString();
  }

  bool needsQuotes = false;
  QString result;
  result.reserve( as.localPart.length() + as.domain.length() + 1 );
  for ( int i = 0 ; i < as.localPart.length() ; ++i ) {
    const char ch = as.localPart[i].toLatin1();
    if ( ch == '.' || isAText( ch ) ) {
      result += ch;
    } else {
      needsQuotes = true;
      if ( ch == '\\' || ch == '"' ) {
        result += '\\';
      }
      result += ch;
    }
  }
  const QString dom = pretty ? QUrl_fromAce_wrapper( as.domain ) : as.domain ;
  if ( needsQuotes ) {
    result = '"' + result + "\"";
  }
  if( dom.isEmpty() ) {
    return result;
  } else {
    return result + '@' + dom;
  }
}

QString AddrSpec::asString() const
{
    return addr_spec_as_string( *this, false );
}

QString AddrSpec::asPrettyString() const
{
    return addr_spec_as_string( *this, true );
}

bool AddrSpec::isEmpty() const
{
  return localPart.isEmpty() && domain.isEmpty();
}

QByteArray Mailbox::address() const
{
  return mAddrSpec.asString().toLatin1();
}

AddrSpec Mailbox::addrSpec() const
{
  return mAddrSpec;
}

QString Mailbox::name() const
{
  return mDisplayName;
}

void Mailbox::setAddress( const AddrSpec &addr )
{
  mAddrSpec = addr;
}

void Mailbox::setAddress( const QByteArray &addr )
{
  const char *cursor = addr.constData();
  if ( !HeaderParsing::parseAngleAddr( cursor,
                                       cursor + addr.length(), mAddrSpec ) ) {
    if ( !HeaderParsing::parseAddrSpec( cursor, cursor + addr.length(),
                                        mAddrSpec ) ) {
      kWarning() << "Invalid address";
      return;
    }
  }
}

void Mailbox::setName( const QString &name )
{
  mDisplayName = removeBidiControlChars( name );
}

void Mailbox::setNameFrom7Bit( const QByteArray &name,
                               const QByteArray &defaultCharset )
{
  QByteArray cs;
  setName( decodeRFC2047String( name, cs, defaultCharset, false ) );
}

bool Mailbox::hasAddress() const
{
  return !mAddrSpec.isEmpty();
}

bool Mailbox::hasName() const
{
  return !mDisplayName.isEmpty();
}

QString Mailbox::prettyAddress() const
{
  return prettyAddress( QuoteNever );
}

QString Mailbox::prettyAddress( Quoting quoting ) const
{
  if ( !hasName() ) {
    return address();
  }
  QString s = name();
  if ( quoting != QuoteNever ) {
    addQuotes( s, quoting == QuoteAlways /*bool force*/ );
  }

  if ( hasAddress() ) {
    s += QLatin1String(" <") + address() + QLatin1Char('>');
  }
  return s;
}

void Mailbox::fromUnicodeString( const QString &s )
{
  from7BitString( encodeRFC2047String( s, "utf-8", false ) );
}

void Mailbox::from7BitString( const QByteArray &s )
{
  const char *cursor = s.constData();
  HeaderParsing::parseMailbox( cursor, cursor + s.length(), *this );
}

QByteArray KMime::Types::Mailbox::as7BitString( const QByteArray &encCharset ) const
{
  if ( !hasName() ) {
    return address();
  }
  QByteArray rv;
  if ( isUsAscii( name() ) ) {
    QByteArray tmp = name().toLatin1();
    addQuotes( tmp, false );
    rv += tmp;
  } else {
    rv += encodeRFC2047String( name(), encCharset, true );
  }
  if ( hasAddress() ) {
    rv += " <" + address() + '>';
  }
  return rv;
}

} // namespace Types

namespace HeaderParsing {

// parse the encoded-word (scursor points to after the initial '=')
bool parseEncodedWord( const char* &scursor, const char * const send,
                       QString &result, QByteArray &language,
                       QByteArray &usedCS, const QByteArray &defaultCS,
                       bool forceCS )
{
  // make sure the caller already did a bit of the work.
  assert( *(scursor-1) == '=' );

  //
  // STEP 1:
  // scan for the charset/language portion of the encoded-word
  //

  char ch = *scursor++;

  if ( ch != '?' ) {
    // kDebug() << "first";
    //KMIME_WARN_PREMATURE_END_OF( EncodedWord );
    return false;
  }

  // remember start of charset (ie. just after the initial "=?") and
  // language (just after the first '*') fields:
  const char * charsetStart = scursor;
  const char * languageStart = 0;

  // find delimiting '?' (and the '*' separating charset and language
  // tags, if any):
  for ( ; scursor != send ; scursor++ ) {
    if ( *scursor == '?') {
      break;
    } else if ( *scursor == '*' && languageStart == 0 ) {
      languageStart = scursor + 1;
    }
  }

  // not found? can't be an encoded-word!
  if ( scursor == send || *scursor != '?' ) {
    // kDebug() << "second";
    KMIME_WARN_PREMATURE_END_OF( EncodedWord );
    return false;
  }

  // extract the language information, if any (if languageStart is 0,
  // language will be null, too):
  QByteArray maybeLanguage( languageStart, scursor - languageStart );
  // extract charset information (keep in mind: the size given to the
  // ctor is one off due to the \0 terminator):
  QByteArray maybeCharset( charsetStart,
                           ( languageStart ? languageStart - 1 : scursor ) - charsetStart );

  //
  // STEP 2:
  // scan for the encoding portion of the encoded-word
  //

  // remember start of encoding (just _after_ the second '?'):
  scursor++;
  const char * encodingStart = scursor;

  // find next '?' (ending the encoding tag):
  for ( ; scursor != send ; scursor++ ) {
    if ( *scursor == '?' ) {
      break;
    }
  }

  // not found? Can't be an encoded-word!
  if ( scursor == send || *scursor != '?' ) {
    // kDebug() << "third";
    KMIME_WARN_PREMATURE_END_OF( EncodedWord );
    return false;
  }

  // extract the encoding information:
  QByteArray maybeEncoding( encodingStart, scursor - encodingStart );

  // kDebug() << "parseEncodedWord: found charset == \"" << maybeCharset
  //         << "\"; language == \"" << maybeLanguage
  //         << "\"; encoding == \"" << maybeEncoding << "\"";

  //
  // STEP 3:
  // scan for encoded-text portion of encoded-word
  //

  // remember start of encoded-text (just after the third '?'):
  scursor++;
  const char * encodedTextStart = scursor;

  // find the '?=' sequence (ending the encoded-text):
  for ( ; scursor != send ; scursor++ ) {
    if ( *scursor == '?' ) {
      if ( scursor + 1 != send ) {
        if ( *( scursor + 1 ) != '=' ) { // We expect a '=' after the '?', but we got something else; ignore
          KMIME_WARN << "Stray '?' in q-encoded word, ignoring this.";
          continue;
        }
        else { // yep, found a '?=' sequence
          scursor += 2;
          break;
        }
      }
      else { // The '?' is the last char, but we need a '=' after it!
        KMIME_WARN_PREMATURE_END_OF( EncodedWord );
        return false;
      }
    }
  }

  if ( *( scursor - 2 ) != '?' || *( scursor - 1 ) != '=' ||
       scursor < encodedTextStart + 2 ) {
    KMIME_WARN_PREMATURE_END_OF( EncodedWord );
    return false;
  }

  // set end sentinel for encoded-text:
  const char * const encodedTextEnd = scursor - 2;

  //
  // STEP 4:
  // setup decoders for the transfer encoding and the charset
  //

  // try if there's a codec for the encoding found:
  Codec * codec = Codec::codecForName( maybeEncoding );
  if ( !codec ) {
    KMIME_WARN_UNKNOWN( Encoding, maybeEncoding );
    return false;
  }

  // get an instance of a corresponding decoder:
  Decoder * dec = codec->makeDecoder();
  assert( dec );

  // try if there's a (text)codec for the charset found:
  bool matchOK = false;
  QTextCodec *textCodec = 0;
  if ( forceCS || maybeCharset.isEmpty() ) {
    textCodec = KGlobal::charsets()->codecForName( defaultCS, matchOK );
    usedCS = cachedCharset( defaultCS );
  } else {
    textCodec = KGlobal::charsets()->codecForName( maybeCharset, matchOK );
    if ( !matchOK ) {  //no suitable codec found => use default charset
      textCodec = KGlobal::charsets()->codecForName( defaultCS, matchOK );
      usedCS = cachedCharset( defaultCS );
    } else {
      usedCS = cachedCharset( maybeCharset );
    }
  }

  if ( !matchOK || !textCodec ) {
    KMIME_WARN_UNKNOWN( Charset, maybeCharset );
    delete dec;
    return false;
  };

  // kDebug() << "mimeName(): \"" << textCodec->name() << "\"";

  // allocate a temporary buffer to store the 8bit text:
  int encodedTextLength = encodedTextEnd - encodedTextStart;
  QByteArray buffer;
  buffer.resize( codec->maxDecodedSizeFor( encodedTextLength ) );
  char *bbegin = buffer.data();
  char *bend = bbegin + buffer.length();

  //
  // STEP 5:
  // do the actual decoding
  //

  if ( !dec->decode( encodedTextStart, encodedTextEnd, bbegin, bend ) ) {
    KMIME_WARN << codec->name() << "codec lies about its maxDecodedSizeFor("
               << encodedTextLength << ")\nresult may be truncated";
  }

  result = textCodec->toUnicode( buffer.data(), bbegin - buffer.data() );

  // kDebug() << "result now: \"" << result << "\"";
  // cleanup:
  delete dec;
  language = maybeLanguage;

  return true;
}

static inline void eatWhiteSpace( const char* &scursor, const char * const send )
{
  while ( scursor != send &&
          ( *scursor == ' ' || *scursor == '\n' ||
            *scursor == '\t' || *scursor == '\r' ) )
    scursor++;
}

bool parseAtom( const char * &scursor, const char * const send,
                QString &result, bool allow8Bit )
{
  QPair<const char*,int> maybeResult;

  if ( parseAtom( scursor, send, maybeResult, allow8Bit ) ) {
    result += QString::fromLatin1( maybeResult.first, maybeResult.second );
    return true;
  }

  return false;
}

bool parseAtom( const char * &scursor, const char * const send,
                QPair<const char*,int> &result, bool allow8Bit )
{
  bool success = false;
  const char *start = scursor;

  while ( scursor != send ) {
    signed char ch = *scursor++;
    if ( ch > 0 && isAText( ch ) ) {
      // AText: OK
      success = true;
    } else if ( allow8Bit && ch < 0 ) {
      // 8bit char: not OK, but be tolerant.
      KMIME_WARN_8BIT( ch );
      success = true;
    } else {
      // CTL or special - marking the end of the atom:
      // re-set sursor to point to the offending
      // char and return:
      scursor--;
      break;
    }
  }
  result.first = start;
  result.second = scursor - start;
  return success;
}

bool parseToken( const char * &scursor, const char * const send,
                 QString &result, bool allow8Bit )
{
  QPair<const char*,int> maybeResult;

  if ( parseToken( scursor, send, maybeResult, allow8Bit ) ) {
    result += QString::fromLatin1( maybeResult.first, maybeResult.second );
    return true;
  }

  return false;
}

bool parseToken( const char * &scursor, const char * const send,
                 QPair<const char*,int> &result, bool allow8Bit )
{
  bool success = false;
  const char * start = scursor;

  while ( scursor != send ) {
    signed char ch = *scursor++;
    if ( ch > 0 && isTText( ch ) ) {
      // TText: OK
      success = true;
    } else if ( allow8Bit && ch < 0 ) {
      // 8bit char: not OK, but be tolerant.
      KMIME_WARN_8BIT( ch );
      success = true;
    } else {
      // CTL or tspecial - marking the end of the atom:
      // re-set sursor to point to the offending
      // char and return:
      scursor--;
      break;
    }
  }
  result.first = start;
  result.second = scursor - start;
  return success;
}

#define READ_ch_OR_FAIL if ( scursor == send ) {        \
    KMIME_WARN_PREMATURE_END_OF( GenericQuotedString ); \
    return false;                                       \
  } else {                                              \
    ch = *scursor++;                                    \
  }

// known issues:
//
// - doesn't handle quoted CRLF

bool parseGenericQuotedString( const char* &scursor, const char * const send,
                               QString &result, bool isCRLF,
                               const char openChar, const char closeChar )
{
  char ch;
  // We are in a quoted-string or domain-literal or comment and the
  // cursor points to the first char after the openChar.
  // We will apply unfolding and quoted-pair removal.
  // We return when we either encounter the end or unescaped openChar
  // or closeChar.

  assert( *(scursor-1) == openChar || *(scursor-1) == closeChar );

  while ( scursor != send ) {
    ch = *scursor++;

    if ( ch == closeChar || ch == openChar ) {
      // end of quoted-string or another opening char:
      // let caller decide what to do.
      return true;
    }

    switch( ch ) {
    case '\\':      // quoted-pair
      // misses "\" CRLF LWSP-char handling, see rfc822, 3.4.5
      READ_ch_OR_FAIL;
      KMIME_WARN_IF_8BIT( ch );
      result += QChar( ch );
      break;
    case '\r':
      // ###
      // The case of lonely '\r' is easy to solve, as they're
      // not part of Unix Line-ending conventions.
      // But I see a problem if we are given Unix-native
      // line-ending-mails, where we cannot determine anymore
      // whether a given '\n' was part of a CRLF or was occurring
      // on it's own.
      READ_ch_OR_FAIL;
      if ( ch != '\n' ) {
        // CR on it's own...
        KMIME_WARN_LONE( CR );
        result += QChar('\r');
        scursor--; // points to after the '\r' again
      } else {
        // CRLF encountered.
        // lookahead: check for folding
        READ_ch_OR_FAIL;
        if ( ch == ' ' || ch == '\t' ) {
          // correct folding;
          // position cursor behind the CRLF WSP (unfolding)
          // and add the WSP to the result
          result += QChar( ch );
        } else {
          // this is the "shouldn't happen"-case. There is a CRLF
          // inside a quoted-string without it being part of FWS.
          // We take it verbatim.
          KMIME_WARN_NON_FOLDING( CRLF );
          result += "\r\n";
          // the cursor is decremented again, so's we need not
          // duplicate the whole switch here. "ch" could've been
          // everything (incl. openChar or closeChar).
          scursor--;
        }
      }
      break;
    case '\n':
      // Note: CRLF has been handled above already!
      // ### LF needs special treatment, depending on whether isCRLF
      // is true (we can be sure a lonely '\n' was meant this way) or
      // false ('\n' alone could have meant LF or CRLF in the original
      // message. This parser assumes CRLF iff the LF is followed by
      // either WSP (folding) or NULL (premature end of quoted-string;
      // Should be fixed, since NULL is allowed as per rfc822).
      READ_ch_OR_FAIL;
      if ( !isCRLF && ( ch == ' ' || ch == '\t' ) ) {
        // folding
        // correct folding
        result += QChar( ch );
      } else {
        // non-folding
        KMIME_WARN_LONE( LF );
        result += QChar('\n');
        // pos is decremented, so's we need not duplicate the whole
        // switch here. ch could've been everything (incl. <">, "\").
        scursor--;
      }
      break;
    default:
      KMIME_WARN_IF_8BIT( ch );
      result += QChar( ch );
    }
  }

  return false;
}

// known issues:
//
// - doesn't handle encoded-word inside comments.

bool parseComment( const char* &scursor, const char * const send,
                   QString &result, bool isCRLF, bool reallySave )
{
  int commentNestingDepth = 1;
  const char *afterLastClosingParenPos = 0;
  QString maybeCmnt;
  const char *oldscursor = scursor;

  assert( *(scursor-1) == '(' );

  while ( commentNestingDepth ) {
    QString cmntPart;
    if ( parseGenericQuotedString( scursor, send, cmntPart, isCRLF, '(', ')' ) ) {
      assert( *(scursor-1) == ')' || *(scursor-1) == '(' );
      // see the kdoc for above function for the possible conditions
      // we have to check:
      switch ( *(scursor-1) ) {
      case ')':
        if ( reallySave ) {
          // add the chunk that's now surely inside the comment.
          result += maybeCmnt;
          result += cmntPart;
          if ( commentNestingDepth > 1 ) {
            // don't add the outermost ')'...
            result += QChar(')');
          }
          maybeCmnt.clear();
        }
        afterLastClosingParenPos = scursor;
        --commentNestingDepth;
        break;
      case '(':
        if ( reallySave ) {
          // don't add to "result" yet, because we might find that we
          // are already outside the (broken) comment...
          maybeCmnt += cmntPart;
          maybeCmnt += QChar('(');
        }
        ++commentNestingDepth;
        break;
      default: assert( 0 );
      } // switch
    } else {
      // !parseGenericQuotedString, ie. premature end
      if ( afterLastClosingParenPos ) {
        scursor = afterLastClosingParenPos;
      } else {
        scursor = oldscursor;
      }
      return false;
    }
  } // while

  return true;
}

// known issues: none.

bool parsePhrase( const char* &scursor, const char * const send,
                  QString &result, bool isCRLF )
{
  enum {
    None, Phrase, Atom, EncodedWord, QuotedString
  } found = None;

  QString tmp;
  QByteArray lang, charset;
  const char *successfullyParsed = 0;
  // only used by the encoded-word branch
  const char *oldscursor;
  // used to suppress whitespace between adjacent encoded-words
  // (rfc2047, 6.2):
  bool lastWasEncodedWord = false;

  while ( scursor != send ) {
    char ch = *scursor++;
    switch ( ch ) {
    case '.': // broken, but allow for intorop's sake
      if ( found == None ) {
        --scursor;
        return false;
      } else {
        if ( scursor != send && ( *scursor == ' ' || *scursor == '\t' ) ) {
          result += ". ";
        } else {
          result += '.';
        }
        successfullyParsed = scursor;
      }
      break;
    case '"': // quoted-string
      tmp.clear();
      if ( parseGenericQuotedString( scursor, send, tmp, isCRLF, '"', '"' ) ) {
        successfullyParsed = scursor;
        assert( *(scursor-1) == '"' );
        switch ( found ) {
        case None:
          found = QuotedString;
          break;
        case Phrase:
        case Atom:
        case EncodedWord:
        case QuotedString:
          found = Phrase;
          result += QChar(' '); // rfc822, 3.4.4
          break;
        default:
          assert( 0 );
        }
        lastWasEncodedWord = false;
        result += tmp;
      } else {
        // premature end of quoted string.
        // What to do? Return leading '"' as special? Return as quoted-string?
        // We do the latter if we already found something, else signal failure.
        if ( found == None ) {
          return false;
        } else {
          result += QChar(' '); // rfc822, 3.4.4
          result += tmp;
          return true;
        }
      }
      break;
    case '(': // comment
      // parse it, but ignore content:
      tmp.clear();
      if ( parseComment( scursor, send, tmp, isCRLF,
                         false /*don't bother with the content*/ ) ) {
        successfullyParsed = scursor;
        lastWasEncodedWord = false; // strictly interpreting rfc2047, 6.2
      } else {
        if ( found == None ) {
          return false;
        } else {
          scursor = successfullyParsed;
          return true;
        }
      }
      break;
    case '=': // encoded-word
      tmp.clear();
      oldscursor = scursor;
      lang.clear();
      charset.clear();
      if ( parseEncodedWord( scursor, send, tmp, lang, charset ) ) {
        successfullyParsed = scursor;
        switch ( found ) {
        case None:
          found = EncodedWord;
          break;
        case Phrase:
        case EncodedWord:
        case Atom:
        case QuotedString:
          if ( !lastWasEncodedWord ) {
            result += QChar(' '); // rfc822, 3.4.4
          }
          found = Phrase;
          break;
        default: assert( 0 );
        }
        lastWasEncodedWord = true;
        result += tmp;
        break;
      } else {
        // parse as atom:
        scursor = oldscursor;
      }
      // fall though...

    default: //atom
      tmp.clear();
      scursor--;
      if ( parseAtom( scursor, send, tmp, true /* allow 8bit */ ) ) {
        successfullyParsed = scursor;
        switch ( found ) {
        case None:
          found = Atom;
          break;
        case Phrase:
        case Atom:
        case EncodedWord:
        case QuotedString:
          found = Phrase;
          result += QChar(' '); // rfc822, 3.4.4
          break;
        default:
          assert( 0 );
        }
        lastWasEncodedWord = false;
        result += tmp;
      } else {
        if ( found == None ) {
          return false;
        } else {
          scursor = successfullyParsed;
          return true;
        }
      }
    }
    eatWhiteSpace( scursor, send );
  }

  return found != None;
}

bool parseDotAtom( const char* &scursor, const char * const send,
                   QString &result, bool isCRLF )
{
  eatCFWS( scursor, send, isCRLF );

  // always points to just after the last atom parsed:
  const char *successfullyParsed;

  QString tmp;
  if ( !parseAtom( scursor, send, tmp, false /* no 8bit */ ) ) {
    return false;
  }
  result += tmp;
  successfullyParsed = scursor;

  while ( scursor != send ) {

    // end of header or no '.' -> return
    if ( scursor == send || *scursor != '.' ) {
      return true;
    }
    scursor++; // eat '.'

    if ( scursor == send || !isAText( *scursor ) ) {
      // end of header or no AText, but this time following a '.'!:
      // reset cursor to just after last successfully parsed char and
      // return:
      scursor = successfullyParsed;
      return true;
    }

    // try to parse the next atom:
    QString maybeAtom;
    if ( !parseAtom( scursor, send, maybeAtom, false /*no 8bit*/ ) ) {
      scursor = successfullyParsed;
      return true;
    }

    result += QChar('.');
    result += maybeAtom;
    successfullyParsed = scursor;
  }

  scursor = successfullyParsed;
  return true;
}

void eatCFWS( const char* &scursor, const char * const send, bool isCRLF )
{
  QString dummy;

  while ( scursor != send ) {
    const char *oldscursor = scursor;

    char ch = *scursor++;

    switch( ch ) {
    case ' ':
    case '\t': // whitespace
    case '\r':
    case '\n': // folding
      continue;

    case '(': // comment
      if ( parseComment( scursor, send, dummy, isCRLF, false /*don't save*/ ) ) {
        continue;
      }
      scursor = oldscursor;
      return;

    default:
      scursor = oldscursor;
      return;
    }
  }
}

bool parseDomain( const char* &scursor, const char * const send,
                  QString &result, bool isCRLF )
{
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  // domain := dot-atom / domain-literal / atom *("." atom)
  //
  // equivalent to:
  // domain = dot-atom / domain-literal,
  // since parseDotAtom does allow CFWS between atoms and dots

  if ( *scursor == '[' ) {
    // domain-literal:
    QString maybeDomainLiteral;
    // eat '[':
    scursor++;
    while ( parseGenericQuotedString( scursor, send, maybeDomainLiteral,
                                      isCRLF, '[', ']' ) ) {
      if ( scursor == send ) {
        // end of header: check for closing ']':
        if ( *(scursor-1) == ']' ) {
          // OK, last char was ']':
          result = maybeDomainLiteral;
          return true;
        } else {
          // not OK, domain-literal wasn't closed:
          return false;
        }
      }
      // we hit openChar in parseGenericQuotedString.
      // include it in maybeDomainLiteral and keep on parsing:
      if ( *(scursor-1) == '[' ) {
        maybeDomainLiteral += QChar('[');
        continue;
      }
      // OK, real end of domain-literal:
      result = maybeDomainLiteral;
      return true;
    }
  } else {
    // dot-atom:
    QString maybeDotAtom;
    if ( parseDotAtom( scursor, send, maybeDotAtom, isCRLF ) ) {
      result = maybeDotAtom;
      // Domain may end with '.', if so preserve it'
      if ( scursor != send && *scursor == '.' ) {
        result += QChar('.');
        scursor++;
      }
      return true;
    }
  }
  return false;
}

bool parseObsRoute( const char* &scursor, const char* const send,
                    QStringList &result, bool isCRLF, bool save )
{
  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return false;
    }

    // empty entry:
    if ( *scursor == ',' ) {
      scursor++;
      if ( save ) {
        result.append( QString() );
      }
      continue;
    }

    // empty entry ending the list:
    if ( *scursor == ':' ) {
      scursor++;
      if ( save ) {
        result.append( QString() );
      }
      return true;
    }

    // each non-empty entry must begin with '@':
    if ( *scursor != '@' ) {
      return false;
    } else {
      scursor++;
    }

    QString maybeDomain;
    if ( !parseDomain( scursor, send, maybeDomain, isCRLF ) ) {
      return false;
    }
    if ( save ) {
      result.append( maybeDomain );
    }

    // eat the following (optional) comma:
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return false;
    }
    if ( *scursor == ':' ) {
      scursor++;
      return true;
    }
    if ( *scursor == ',' ) {
      scursor++;
    }
  }

  return false;
}

bool parseAddrSpec( const char* &scursor, const char * const send,
                    AddrSpec &result, bool isCRLF )
{
  //
  // STEP 1:
  // local-part := dot-atom / quoted-string / word *("." word)
  //
  // this is equivalent to:
  // local-part := word *("." word)

  QString maybeLocalPart;
  QString tmp;

  while ( scursor != send ) {
    // first, eat any whitespace
    eatCFWS( scursor, send, isCRLF );

    char ch = *scursor++;
    switch ( ch ) {
    case '.': // dot
      maybeLocalPart += QChar('.');
      break;

    case '@':
      goto SAW_AT_SIGN;
      break;

    case '"': // quoted-string
      tmp.clear();
      if ( parseGenericQuotedString( scursor, send, tmp, isCRLF, '"', '"' ) ) {
        maybeLocalPart += tmp;
      } else {
        return false;
      }
      break;

    default: // atom
      scursor--; // re-set scursor to point to ch again
      tmp.clear();
      if ( parseAtom( scursor, send, tmp, false /* no 8bit */ ) ) {
        maybeLocalPart += tmp;
      } else {
        return false; // parseAtom can only fail if the first char is non-atext.
      }
      break;
    }
  }

  return false;

  //
  // STEP 2:
  // domain
  //

SAW_AT_SIGN:

  assert( *(scursor-1) == '@' );

  QString maybeDomain;
  if ( !parseDomain( scursor, send, maybeDomain, isCRLF ) ) {
    return false;
  }

  result.localPart = maybeLocalPart;
  result.domain = maybeDomain;

  return true;
}

bool parseAngleAddr( const char* &scursor, const char * const send,
                     AddrSpec &result, bool isCRLF )
{
  // first, we need an opening angle bracket:
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '<' ) {
    return false;
  }
  scursor++; // eat '<'

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  if ( *scursor == '@' || *scursor == ',' ) {
    // obs-route: parse, but ignore:
    KMIME_WARN << "obsolete source route found! ignoring.";
    QStringList dummy;
    if ( !parseObsRoute( scursor, send, dummy,
                         isCRLF, false /* don't save */ ) ) {
      return false;
    }
    // angle-addr isn't complete until after the '>':
    if ( scursor == send ) {
      return false;
    }
  }

  // parse addr-spec:
  AddrSpec maybeAddrSpec;
  if ( !parseAddrSpec( scursor, send, maybeAddrSpec, isCRLF ) ) {
    return false;
  }

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != '>' ) {
    return false;
  }
  scursor++;

  result = maybeAddrSpec;
  return true;

}

bool parseMailbox( const char* &scursor, const char * const send,
                   Mailbox &result, bool isCRLF )
{
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  AddrSpec maybeAddrSpec;
  QString maybeDisplayName;

  // first, try if it's a vanilla addr-spec:
  const char * oldscursor = scursor;
  if ( parseAddrSpec( scursor, send, maybeAddrSpec, isCRLF ) ) {
    result.setAddress( maybeAddrSpec );
    // check for the obsolete form of display-name (as comment):
    eatWhiteSpace( scursor, send );
    if ( scursor != send && *scursor == '(' ) {
      scursor++;
      if ( !parseComment( scursor, send, maybeDisplayName, isCRLF, true /*keep*/ ) ) {
        return false;
      }
    }
    result.setNameFrom7Bit( maybeDisplayName.toLatin1() );
    return true;
  }
  scursor = oldscursor;

  // second, see if there's a display-name:
  if ( !parsePhrase( scursor, send, maybeDisplayName, isCRLF ) ) {
    // failed: reset cursor, note absent display-name
    maybeDisplayName.clear();
    scursor = oldscursor;
  } else {
    // succeeded: eat CFWS
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return false;
    }
  }

  // third, parse the angle-addr:
  if ( !parseAngleAddr( scursor, send, maybeAddrSpec, isCRLF ) ) {
    return false;
  }

  if ( maybeDisplayName.isNull() ) {
    // check for the obsolete form of display-name (as comment):
    eatWhiteSpace( scursor, send );
    if ( scursor != send && *scursor == '(' ) {
      scursor++;
      if ( !parseComment( scursor, send, maybeDisplayName, isCRLF, true /*keep*/ ) ) {
        return false;
      }
    }
  }

  result.setName( maybeDisplayName );
  result.setAddress( maybeAddrSpec );
  return true;
}

bool parseGroup( const char* &scursor, const char * const send,
                 Address &result, bool isCRLF )
{
  // group         := display-name ":" [ mailbox-list / CFWS ] ";" [CFWS]
  //
  // equivalent to:
  // group   := display-name ":" [ obs-mbox-list ] ";"

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  // get display-name:
  QString maybeDisplayName;
  if ( !parsePhrase( scursor, send, maybeDisplayName, isCRLF ) ) {
    return false;
  }

  // get ":":
  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != ':' ) {
    return false;
  }

  // KDE5 TODO: Don't expose displayName as public, but rather add setter for it that
  //            automatically calls removeBidiControlChars
  result.displayName = removeBidiControlChars( maybeDisplayName );

  // get obs-mbox-list (may contain empty entries):
  scursor++;
  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return false;
    }

    // empty entry:
    if ( *scursor == ',' ) {
      scursor++;
      continue;
    }

    // empty entry ending the list:
    if ( *scursor == ';' ) {
      scursor++;
      return true;
    }

    Mailbox maybeMailbox;
    if ( !parseMailbox( scursor, send, maybeMailbox, isCRLF ) ) {
      return false;
    }
    result.mailboxList.append( maybeMailbox );

    eatCFWS( scursor, send, isCRLF );
    // premature end:
    if ( scursor == send ) {
      return false;
    }
    // regular end of the list:
    if ( *scursor == ';' ) {
      scursor++;
      return true;
    }
    // eat regular list entry separator:
    if ( *scursor == ',' ) {
      scursor++;
    }
  }
  return false;
}

bool parseAddress( const char* &scursor, const char * const send,
                   Address &result, bool isCRLF )
{
  // address       := mailbox / group

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  // first try if it's a single mailbox:
  Mailbox maybeMailbox;
  const char * oldscursor = scursor;
  if ( parseMailbox( scursor, send, maybeMailbox, isCRLF ) ) {
    // yes, it is:
    result.displayName.clear();
    result.mailboxList.append( maybeMailbox );
    return true;
  }
  scursor = oldscursor;

  Address maybeAddress;

  // no, it's not a single mailbox. Try if it's a group:
  if ( !parseGroup( scursor, send, maybeAddress, isCRLF ) ) {
    return false;
  }

  result = maybeAddress;
  return true;
}

bool parseAddressList( const char* &scursor, const char * const send,
                       AddressList &result, bool isCRLF )
{
  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // end of header: this is OK.
    if ( scursor == send ) {
      return true;
    }
    // empty entry: ignore:
    if ( *scursor == ',' ) {
      scursor++;
      continue;
    }
    // broken clients might use ';' as list delimiter, accept that as well
    if ( *scursor == ';' ) {
      scursor++;
      continue;
    }

    // parse one entry
    Address maybeAddress;
    if ( !parseAddress( scursor, send, maybeAddress, isCRLF ) ) {
      return false;
    }
    result.append( maybeAddress );

    eatCFWS( scursor, send, isCRLF );
    // end of header: this is OK.
    if ( scursor == send ) {
      return true;
    }
    // comma separating entries: eat it.
    if ( *scursor == ',' ) {
      scursor++;
    }
  }
  return true;
}

static QString asterisk = QString::fromLatin1( "*0*", 1 );
static QString asteriskZero = QString::fromLatin1( "*0*", 2 );
//static QString asteriskZeroAsterisk = QString::fromLatin1( "*0*", 3 );

bool parseParameter( const char* &scursor, const char * const send,
                     QPair<QString,QStringOrQPair> &result, bool isCRLF )
{
  // parameter = regular-parameter / extended-parameter
  // regular-parameter = regular-parameter-name "=" value
  // extended-parameter =
  // value = token / quoted-string
  //
  // note that rfc2231 handling is out of the scope of this function.
  // Therefore we return the attribute as QString and the value as
  // (start,length) tupel if we see that the value is encoded
  // (trailing asterisk), for parseParameterList to decode...

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  //
  // parse the parameter name:
  //
  QString maybeAttribute;
  if ( !parseToken( scursor, send, maybeAttribute, false /* no 8bit */ ) ) {
    return false;
  }

  eatCFWS( scursor, send, isCRLF );
  // premature end: not OK (haven't seen '=' yet).
  if ( scursor == send || *scursor != '=' ) {
    return false;
  }
  scursor++; // eat '='

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    // don't choke on attribute=, meaning the value was omitted:
    if ( maybeAttribute.endsWith( asterisk ) ) {
      KMIME_WARN << "attribute ends with \"*\", but value is empty!"
        "Chopping away \"*\".";
      maybeAttribute.truncate( maybeAttribute.length() - 1 );
    }
    result = qMakePair( maybeAttribute.toLower(), QStringOrQPair() );
    return true;
  }

  const char * oldscursor = scursor;

  //
  // parse the parameter value:
  //
  QStringOrQPair maybeValue;
  if ( *scursor == '"' ) {
    // value is a quoted-string:
    scursor++;
    if ( maybeAttribute.endsWith( asterisk ) ) {
      // attributes ending with "*" designate extended-parameters,
      // which cannot have quoted-strings as values. So we remove the
      // trailing "*" to not confuse upper layers.
      KMIME_WARN << "attribute ends with \"*\", but value is a quoted-string!"
        "Chopping away \"*\".";
      maybeAttribute.truncate( maybeAttribute.length() - 1 );
    }

    if ( !parseGenericQuotedString( scursor, send, maybeValue.qstring, isCRLF ) ) {
      scursor = oldscursor;
      result = qMakePair( maybeAttribute.toLower(), QStringOrQPair() );
      return false; // this case needs further processing by upper layers!!
    }
  } else {
    // value is a token:
    if ( !parseToken( scursor, send, maybeValue.qpair, false /* no 8bit */ ) ) {
      scursor = oldscursor;
      result = qMakePair( maybeAttribute.toLower(), QStringOrQPair() );
      return false; // this case needs further processing by upper layers!!
    }
  }

  result = qMakePair( maybeAttribute.toLower(), maybeValue );
  return true;
}

bool parseRawParameterList( const char* &scursor, const char * const send,
                            QMap<QString,QStringOrQPair> &result,
                            bool isCRLF )
{
  // we use parseParameter() consecutively to obtain a map of raw
  // attributes to raw values. "Raw" here means that we don't do
  // rfc2231 decoding and concatenation. This is left to
  // parseParameterList(), which will call this function.
  //
  // The main reason for making this chunk of code a separate
  // (private) method is that we can deal with broken parameters
  // _here_ and leave the rfc2231 handling solely to
  // parseParameterList(), which will still be enough work.

  while ( scursor != send ) {
    eatCFWS( scursor, send, isCRLF );
    // empty entry ending the list: OK.
    if ( scursor == send ) {
      return true;
    }
    // empty list entry: ignore.
    if ( *scursor == ';' ) {
      scursor++;
      continue;
    }

    QPair<QString,QStringOrQPair> maybeParameter;
    if ( !parseParameter( scursor, send, maybeParameter, isCRLF ) ) {
      // we need to do a bit of work if the attribute is not
      // NULL. These are the cases marked with "needs further
      // processing" in parseParameter(). Specifically, parsing of the
      // token or the quoted-string, which should represent the value,
      // failed. We take the easy way out and simply search for the
      // next ';' to start parsing again. (Another option would be to
      // take the text between '=' and ';' as value)
      if ( maybeParameter.first.isNull() ) {
        return false;
      }
      while ( scursor != send ) {
        if ( *scursor++ == ';' ) {
          goto IS_SEMICOLON;
        }
      }
      // scursor == send case: end of list.
      return true;
    IS_SEMICOLON:
      // *scursor == ';' case: parse next entry.
      continue;
    }
    // successful parsing brings us here:
    result.insert( maybeParameter.first, maybeParameter.second );

    eatCFWS( scursor, send, isCRLF );
    // end of header: ends list.
    if ( scursor == send ) {
      return true;
    }
    // regular separator: eat it.
    if ( *scursor == ';' ) {
      scursor++;
    }
  }
  return true;
}

static void decodeRFC2231Value( Codec* &rfc2231Codec,
                                QTextCodec* &textcodec,
                                bool isContinuation, QString &value,
                                QPair<const char*,int> &source )
{
  //
  // parse the raw value into (charset,language,text):
  //

  const char * decBegin = source.first;
  const char * decCursor = decBegin;
  const char * decEnd = decCursor + source.second;

  if ( !isContinuation ) {
    // find the first single quote
    while ( decCursor != decEnd ) {
      if ( *decCursor == '\'' ) {
        break;
      } else {
        decCursor++;
      }
    }

    if ( decCursor == decEnd ) {
      // there wasn't a single single quote at all!
      // take the whole value to be in latin-1:
      KMIME_WARN << "No charset in extended-initial-value."
        "Assuming \"iso-8859-1\".";
      value += QString::fromLatin1( decBegin, source.second );
      return;
    }

    QByteArray charset( decBegin, decCursor - decBegin );

    const char * oldDecCursor = ++decCursor;
    // find the second single quote (we ignore the language tag):
    while ( decCursor != decEnd ) {
      if ( *decCursor == '\'' ) {
        break;
      } else {
        decCursor++;
      }
    }
    if ( decCursor == decEnd ) {
      KMIME_WARN << "No language in extended-initial-value."
        "Trying to recover.";
      decCursor = oldDecCursor;
    } else {
      decCursor++;
    }

    // decCursor now points to the start of the
    // "extended-other-values":

    //
    // get the decoders:
    //

    bool matchOK = false;
    textcodec = KGlobal::charsets()->codecForName( charset, matchOK );
    if ( !matchOK ) {
      textcodec = 0;
      KMIME_WARN_UNKNOWN( Charset, charset );
    }
  }

  if ( !rfc2231Codec ) {
    rfc2231Codec = Codec::codecForName("x-kmime-rfc2231");
    assert( rfc2231Codec );
  }

  if ( !textcodec ) {
    value += QString::fromLatin1( decCursor, decEnd - decCursor );
    return;
  }

  Decoder * dec = rfc2231Codec->makeDecoder();
  assert( dec );

  //
  // do the decoding:
  //

  QByteArray buffer;
  buffer.resize( rfc2231Codec->maxDecodedSizeFor( decEnd - decCursor ) );
  QByteArray::Iterator bit = buffer.begin();
  QByteArray::ConstIterator bend = buffer.end();

  if ( !dec->decode( decCursor, decEnd, bit, bend ) ) {
    KMIME_WARN << rfc2231Codec->name()
               << "codec lies about its maxDecodedSizeFor()" << endl
               << "result may be truncated";
  }

  value += textcodec->toUnicode( buffer.begin(), bit - buffer.begin() );

  // kDebug() << "value now: \"" << value << "\"";
  // cleanup:
  delete dec;
}

// known issues:
//  - permutes rfc2231 continuations when the total number of parts
//    exceeds 10 (other-sections then becomes *xy, ie. two digits)

bool parseParameterList( const char* &scursor, const char * const send,
                         QMap<QString,QString> &result, bool isCRLF )
{
  // parse the list into raw attribute-value pairs:
  QMap<QString,QStringOrQPair> rawParameterList;
  if (!parseRawParameterList( scursor, send, rawParameterList, isCRLF ) ) {
    return false;
  }

  if ( rawParameterList.isEmpty() ) {
    return true;
  }

  // decode rfc 2231 continuations and alternate charset encoding:

  // NOTE: this code assumes that what QMapIterator delivers is sorted
  // by the key!

  Codec * rfc2231Codec = 0;
  QTextCodec * textcodec = 0;
  QString attribute;
  QString value;
  enum Modes {
    NoMode = 0x0, Continued = 0x1, Encoded = 0x2
  } mode;

  QMap<QString,QStringOrQPair>::Iterator it, end = rawParameterList.end();

  for ( it = rawParameterList.begin() ; it != end ; ++it ) {
    if ( attribute.isNull() || !it.key().startsWith( attribute ) ) {
      //
      // new attribute:
      //

      // store the last attribute/value pair in the result map now:
      if ( !attribute.isNull() ) {
        result.insert( attribute, value );
      }
      // and extract the information from the new raw attribute:
      value.clear();
      attribute = it.key();
      mode = NoMode;
      // is the value encoded?
      if ( attribute.endsWith( asterisk ) ) {
        attribute.truncate( attribute.length() - 1 );
        mode = (Modes) ((int) mode | Encoded);
      }
      // is the value continued?
      if ( attribute.endsWith( asteriskZero ) ) {
        attribute.truncate( attribute.length() - 2 );
        mode = (Modes) ((int) mode | Continued);
      }
      //
      // decode if necessary:
      //
      if ( mode & Encoded ) {
        decodeRFC2231Value( rfc2231Codec, textcodec,
                            false, /* isn't continuation */
                            value, (*it).qpair );
      } else {
        // not encoded.
        if ( (*it).qpair.first ) {
          value += QString::fromLatin1( (*it).qpair.first, (*it).qpair.second );
        } else {
          value += (*it).qstring;
        }
      }

      //
      // shortcut-processing when the value isn't encoded:
      //

      if ( !(mode & Continued) ) {
        // save result already:
        result.insert( attribute, value );
        // force begin of a new attribute:
        attribute.clear();
      }
    } else { // it.key().startsWith( attribute )
      //
      // continuation
      //

      // ignore the section and trust QMap to have sorted the keys:
      if ( it.key().endsWith( asterisk ) ) {
        // encoded
        decodeRFC2231Value( rfc2231Codec, textcodec,
                            true, /* is continuation */
                            value, (*it).qpair );
      } else {
        // not encoded
        if ( (*it).qpair.first ) {
          value += QString::fromLatin1( (*it).qpair.first, (*it).qpair.second );
        } else {
          value += (*it).qstring;
        }
      }
    }
  }

  // write last attr/value pair:
  if ( !attribute.isNull() ) {
    result.insert( attribute, value );
  }

  return true;
}

static const char * const stdDayNames[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const int stdDayNamesLen = sizeof stdDayNames / sizeof *stdDayNames;

static bool parseDayName( const char* &scursor, const char * const send )
{
  // check bounds:
  if ( send - scursor < 3 ) {
    return false;
  }

  for ( int i = 0 ; i < stdDayNamesLen ; ++i ) {
    if ( qstrnicmp( scursor, stdDayNames[i], 3 ) == 0 ) {
      scursor += 3;
      // kDebug() << "found" << stdDayNames[i];
      return true;
    }
  }

  return false;
}

static const char * const stdMonthNames[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const int stdMonthNamesLen =
                              sizeof stdMonthNames / sizeof *stdMonthNames;

static bool parseMonthName( const char* &scursor, const char * const send,
                            int &result )
{
  // check bounds:
  if ( send - scursor < 3 ) {
    return false;
  }

  for ( result = 0 ; result < stdMonthNamesLen ; ++result ) {
    if ( qstrnicmp( scursor, stdMonthNames[result], 3 ) == 0 ) {
      scursor += 3;
      return true;
    }
  }

  // not found:
  return false;
}

static const struct {
  const char * tzName;
  long int secsEastOfGMT;
} timeZones[] = {
  // rfc 822 timezones:
  { "GMT", 0 },
  { "UT", 0 },
  { "EDT", -4*3600 },
  { "EST", -5*3600 },
  { "MST", -5*3600 },
  { "CST", -6*3600 },
  { "MDT", -6*3600 },
  { "MST", -7*3600 },
  { "PDT", -7*3600 },
  { "PST", -8*3600 },
  // common, non-rfc-822 zones:
  { "CET", 1*3600 },
  { "MET", 1*3600 },
  { "UTC", 0 },
  { "CEST", 2*3600 },
  { "BST", 1*3600 },
  // rfc 822 military timezones:
  { "Z", 0 },
  { "A", -1*3600 },
  { "B", -2*3600 },
  { "C", -3*3600 },
  { "D", -4*3600 },
  { "E", -5*3600 },
  { "F", -6*3600 },
  { "G", -7*3600 },
  { "H", -8*3600 },
  { "I", -9*3600 },
  // J is not used!
  { "K", -10*3600 },
  { "L", -11*3600 },
  { "M", -12*3600 },
  { "N", 1*3600 },
  { "O", 2*3600 },
  { "P", 3*3600 },
  { "Q", 4*3600 },
  { "R", 5*3600 },
  { "S", 6*3600 },
  { "T", 7*3600 },
  { "U", 8*3600 },
  { "V", 9*3600 },
  { "W", 10*3600 },
  { "X", 11*3600 },
  { "Y", 12*3600 },
};
static const int timeZonesLen = sizeof timeZones / sizeof *timeZones;

static bool parseAlphaNumericTimeZone( const char* &scursor,
                                       const char * const send,
                                       long int &secsEastOfGMT,
                                       bool &timeZoneKnown )
{
  QPair<const char*,int> maybeTimeZone( 0, 0 );
  if ( !parseToken( scursor, send, maybeTimeZone, false /*no 8bit*/ ) ) {
    return false;
  }
  for ( int i = 0 ; i < timeZonesLen ; ++i ) {
    if ( qstrnicmp( timeZones[i].tzName,
                    maybeTimeZone.first, maybeTimeZone.second ) == 0 ) {
      scursor += maybeTimeZone.second;
      secsEastOfGMT = timeZones[i].secsEastOfGMT;
      timeZoneKnown = true;
      return true;
    }
  }

  // don't choke just because we don't happen to know the time zone
  KMIME_WARN_UNKNOWN( time zone,
                      QByteArray( maybeTimeZone.first, maybeTimeZone.second ) );
  secsEastOfGMT = 0;
  timeZoneKnown = false;
  return true;
}

// parse a number and return the number of digits parsed:
int parseDigits( const char* &scursor, const char * const send, int &result )
{
  result = 0;
  int digits = 0;
  for ( ; scursor != send && isdigit( *scursor ) ; scursor++, digits++ ) {
    result *= 10;
    result += int( *scursor - '0' );
  }
  return digits;
}

static bool parseTimeOfDay( const char* &scursor, const char * const send,
                            int &hour, int &min, int &sec, bool isCRLF=false )
{
  // time-of-day := 2DIGIT [CFWS] ":" [CFWS] 2DIGIT [ [CFWS] ":" 2DIGIT ]

  //
  // 2DIGIT representing "hour":
  //
  if ( !parseDigits( scursor, send, hour ) ) {
    return false;
  }

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send || *scursor != ':' ) {
    return false;
  }
  scursor++; // eat ':'

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  //
  // 2DIGIT representing "minute":
  //
  if ( !parseDigits( scursor, send, min ) ) {
    return false;
  }

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return true; // seconds are optional
  }

  //
  // let's see if we have a 2DIGIT representing "second":
  //
  if ( *scursor == ':' ) {
    // yepp, there are seconds:
    scursor++; // eat ':'
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return false;
    }

    if ( !parseDigits( scursor, send, sec ) ) {
      return false;
    }
  } else {
    sec = 0;
  }

  return true;
}

bool parseTime( const char* &scursor, const char * send,
                int &hour, int &min, int &sec, long int &secsEastOfGMT,
                bool &timeZoneKnown, bool isCRLF )
{
  // time := time-of-day CFWS ( zone / obs-zone )
  //
  // obs-zone    := "UT" / "GMT" /
  //                "EST" / "EDT" / ; -0500 / -0400
  //                "CST" / "CDT" / ; -0600 / -0500
  //                "MST" / "MDT" / ; -0700 / -0600
  //                "PST" / "PDT" / ; -0800 / -0700
  //                "A"-"I" / "a"-"i" /
  //                "K"-"Z" / "k"-"z"

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  if ( !parseTimeOfDay( scursor, send, hour, min, sec, isCRLF ) ) {
    return false;
  }

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    timeZoneKnown = false;
    secsEastOfGMT = 0;
    return true; // allow missing timezone
  }

  timeZoneKnown = true;
  if ( *scursor == '+' || *scursor == '-' ) {
    // remember and eat '-'/'+':
    const char sign = *scursor++;
    // numerical timezone:
    int maybeTimeZone;
    if ( parseDigits( scursor, send, maybeTimeZone ) != 4 ) {
      return false;
    }
    secsEastOfGMT = 60 * ( maybeTimeZone / 100 * 60 + maybeTimeZone % 100 );
    if ( sign == '-' ) {
      secsEastOfGMT *= -1;
      if ( secsEastOfGMT == 0 ) {
        timeZoneKnown = false; // -0000 means indetermined tz
      }
    }
  } else {
    // maybe alphanumeric timezone:
    if ( !parseAlphaNumericTimeZone( scursor, send, secsEastOfGMT, timeZoneKnown ) ) {
      return false;
    }
  }
  return true;
}

bool parseDateTime( const char* &scursor, const char * const send,
                    KDateTime &result, bool isCRLF )
{
  // Parsing date-time; strict mode:
  //
  // date-time   := [ [CFWS] day-name [CFWS] "," ]                      ; wday
  // (expanded)     [CFWS] 1*2DIGIT CFWS month-name CFWS 2*DIGIT [CFWS] ; date
  //                time
  //
  // day-name    := "Mon" / "Tue" / "Wed" / "Thu" / "Fri" / "Sat" / "Sun"
  // month-name  := "Jan" / "Feb" / "Mar" / "Apr" / "May" / "Jun" /
  //                "Jul" / "Aug" / "Sep" / "Oct" / "Nov" / "Dec"

  result = KDateTime();
  QDateTime maybeDateTime;

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  //
  // let's see if there's a day-of-week:
  //
  if ( parseDayName( scursor, send ) ) {
    eatCFWS( scursor, send, isCRLF );
    if ( scursor == send ) {
      return false;
    }
    // day-name should be followed by ',' but we treat it as optional:
    if ( *scursor == ',' ) {
      scursor++; // eat ','
      eatCFWS( scursor, send, isCRLF );
    }
  }

  //
  // 1*2DIGIT representing "day" (of month):
  //
  int maybeDay;
  if ( !parseDigits( scursor, send, maybeDay ) ) {
    return false;
  }

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  //
  // month-name:
  //
  int maybeMonth = 0;
  if ( !parseMonthName( scursor, send, maybeMonth ) ) {
    return false;
  }
  if ( scursor == send ) {
    return false;
  }
  assert( maybeMonth >= 0 ); assert( maybeMonth <= 11 );
  ++maybeMonth; // 0-11 -> 1-12

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  //
  // 2*DIGIT representing "year":
  //
  int maybeYear;
  if ( !parseDigits( scursor, send, maybeYear ) ) {
    return false;
  }
  // RFC 2822 4.3 processing:
  if ( maybeYear < 50 ) {
    maybeYear += 2000;
  } else if ( maybeYear < 1000 ) {
    maybeYear += 1900;
  }
  // else keep as is
  if ( maybeYear < 1900 ) {
    return false; // rfc2822, 3.3
  }

  eatCFWS( scursor, send, isCRLF );
  if ( scursor == send ) {
    return false;
  }

  maybeDateTime.setDate( QDate( maybeYear, maybeMonth, maybeDay ) );

  //
  // time
  //
  int maybeHour, maybeMinute, maybeSecond;
  long int secsEastOfGMT;
  bool timeZoneKnown = true;

  if ( !parseTime( scursor, send,
                   maybeHour, maybeMinute, maybeSecond,
                   secsEastOfGMT, timeZoneKnown, isCRLF ) ) {
    return false;
  }

  maybeDateTime.setTime( QTime( maybeHour, maybeMinute, maybeSecond ) );
  if ( !maybeDateTime.isValid() )
    return false;

  result = KDateTime( maybeDateTime, KDateTime::Spec( KDateTime::OffsetFromUTC, secsEastOfGMT ) );
  if ( !result.isValid() )
    return false;
  return true;
}

Headers::Base *extractFirstHeader( QByteArray &head )
{
  int endOfFieldBody = 0;
  bool folded = false;
  Headers::Base *header = 0;

  int startOfFieldBody = head.indexOf( ":" );
  const int endOfFieldHeader = startOfFieldBody;

  if ( startOfFieldBody > -1 ) {    //there is another header
    startOfFieldBody++; //skip the ':'
    if ( head[startOfFieldBody] == ' ' ) { // skip the space after the ':', if there
      startOfFieldBody++;
    }
    endOfFieldBody = findHeaderLineEnd( head, startOfFieldBody, &folded );

    QByteArray rawType = head.left( endOfFieldHeader );
    QByteArray rawFieldBody = head.mid( startOfFieldBody, endOfFieldBody - startOfFieldBody );
    if ( folded ) {
      rawFieldBody = unfoldHeader( rawFieldBody );
    }
    header = HeaderFactory::self()->createHeader( rawType );
    if( !header ) {
      //kWarning() << "Returning Generic header of type" << rawType;
      header = new Headers::Generic( rawType );
    }
    header->from7BitString( rawFieldBody );

    head.remove( 0, endOfFieldBody + 1 );
  } else {
    head.clear();
  }

  return header;
}

Headers::Base::List parseHeaders( const QByteArray &head )
{
  Headers::Base::List ret;
  Headers::Base *h;

  QByteArray copy = head;
  while( ( h = extractFirstHeader( copy ) ) ) {
    ret << h;
  }

  return ret;
}

} // namespace HeaderParsing

} // namespace KMime
