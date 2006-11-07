/*  -*- c++ -*-
    kmime_codec_base64.h

    KMime, the KDE internet mail/usenet news message library.
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

#ifndef __KMIME_CODEC_BASE64__
#define __KMIME_CODEC_BASE64__

#include "kmime_codecs.h"

namespace KMime {

class KMIME_EXPORT Base64Codec : public Codec
{
  protected:
    friend class Codec;
    Base64Codec() : Codec() {}

  public:
    virtual ~Base64Codec() {}

    const char * name() const
      { return "base64"; }

    int maxEncodedSizeFor( int insize, bool withCRLF=false ) const
      {
        // first, the total number of 4-char packets will be:
        int totalNumPackets = ( insize + 2 ) / 3;
        // now, after every 76/4'th packet there needs to be a linebreak:
        int numLineBreaks = totalNumPackets / (76/4);
        // and at the very end, too:
        ++numLineBreaks;
        // putting it all together, we have:
        return 4 * totalNumPackets + ( withCRLF ? 2 : 1 ) * numLineBreaks;
      }

    int maxDecodedSizeFor( int insize, bool withCRLF=false ) const
      {
        // assuming all characters are part of the base64 stream (which
        // does almost never hold due to required linebreaking; but
        // additional non-base64 chars don't affect the output size), each
        // 4-tupel of them becomes a 3-tupel in the decoded octet
        // stream. So:
        int result = ( ( insize + 3 ) / 4 ) * 3;
        // but all of them may be \n, so
        if ( withCRLF ) {
          result *= 2; // :-o
        }

        return result;
      }

    Encoder *makeEncoder( bool withCRLF=false ) const;
    Decoder *makeDecoder( bool withCRLF=false ) const;
};

class KMIME_EXPORT Rfc2047BEncodingCodec : public Base64Codec
{
  protected:
    friend class Codec;
    Rfc2047BEncodingCodec()
      : Base64Codec() {}

  public:
    virtual ~Rfc2047BEncodingCodec() {}

    const char *name() const
      { return "b"; }

    int maxEncodedSizeFor( int insize, bool withCRLF=false ) const
      {
        (void)withCRLF; // keep compiler happy
        // Each (begun) 3-octet triple becomes a 4 char quartet, so:
        return ( ( insize + 2 ) / 3 ) * 4;
      }

    int maxDecodedSizeFor( int insize, bool withCRLF=false ) const
      {
        (void)withCRLF; // keep compiler happy
        // Each 4-char quartet becomes a 3-octet triple, the last one
        // possibly even less. So:
        return ( ( insize + 3 ) / 4 ) * 3;
      }

    Encoder *makeEncoder( bool withCRLF=false ) const;
};

} // namespace KMime

#endif // __KMIME_CODEC_BASE64__
