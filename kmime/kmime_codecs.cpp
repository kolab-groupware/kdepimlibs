/*
    kmime_codecs.cpp

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001-2002 the KMime authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, US
*/

#include "kmime_codecs.h"
#include "kmime_util.h"

#include "kmime_codec_base64.h"
#include "kmime_codec_qp.h"

#include <kdebug.h>

#include <cassert>
#include <cstring>

using namespace KMime;

namespace KMime {

// global list of KMime::Codec's
QAsciiDict<Codec> Codec::all( 11, false /* case-insensitive */);
#if defined(QT_THREAD_SUPPORT)
QMutex Codec::dictLock;
#endif

void Codec::fillDictionary() {

  all.setAutoDelete(true);

  all.insert( "base64", new Base64Codec() );
  all.insert( "quoted-printable", new QuotedPrintableCodec() );
  all.insert( "b", new Rfc2047BEncodingCodec() );
  all.insert( "q", new Rfc2047QEncodingCodec() );
  all.insert( "x-kmime-rfc2231", new Rfc2231EncodingCodec() );

}

Codec * Codec::codecForName( const char * name ) {
#if defined(QT_THREAD_SUPPORT)
  dictLock.lock(); // protect "all"
#endif
  if ( all.isEmpty() )
    fillDictionary();
  Codec * codec = all[ name ];
#if defined(QT_THREAD_SUPPORT)
  dictLock.unlock();
#endif

  if ( !codec )
    kdWarning() << "Unknown codec \"" << name << "\" requested!" << endl;

  return codec;
}

Codec * Codec::codecForName( const QCString & name ) {
  return codecForName( name.data() );
}

bool Codec::encode( const char* & scursor, const char * const send,
		    char* & dcursor, const char * const dend,
		    bool withCRLF ) const
{
  // get an encoder:
  Encoder * enc = makeEncoder( withCRLF );
  assert( enc );

  // encode and check for output buffer overflow:
  while ( !enc->encode( scursor, send, dcursor, dend ) )
    if ( dcursor == dend ) {
      delete enc;
      return false; // not enough space in output buffer
    }

  // finish and check for output buffer overflow:
  while ( !enc->finish( dcursor, dend ) )
    if ( dcursor == dend ) {
      delete enc;
      return false; // not enough space in output buffer
    }

  // cleanup and return:
  delete enc;
  return true; // successfully encoded.
}

bool Codec::decode( const char* & scursor, const char * const send,
		    char* & dcursor, const char * const dend,
		    bool withCRLF ) const
{
  // get a decoder:
  Decoder * dec = makeDecoder( withCRLF );
  assert( dec );

  // decode and check for output buffer overflow:
  while ( !dec->decode( scursor, send, dcursor, dend ) )
    if ( dcursor == dend ) {
      delete dec;
      return false; // not enough space in output buffer
    }

  // finish and check for output buffer overflow:
  while ( !dec->finish( dcursor, dend ) )
    if ( dcursor == dend ) {
      delete dec;
      return false; // not enough space in output buffer
    }

  // cleanup and return:
  delete dec;
  return true; // successfully encoded.
}


}; // namespace KMime
