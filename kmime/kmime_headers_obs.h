/*
    kmime_headers.h

    KMime, the KDE internet mail/usenet news message library.
    Copyright (c) 2001 the KMime authors.
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
#ifndef __KMIME_HEADERS_OBS_H__
#define __KMIME_HEADERS_OBS_H__

#if defined(KMIME_NEW_STYLE_CLASSTREE)
#error You cannot use this file with the new header classes!
#endif

#include "kmime.h"
#include <QByteArray>


/** Represents a "Content-Type" header */
class KMIME_EXPORT ContentType : public Base {

  public:
    ContentType() : Base(),m_imeType("invalid/invalid"),c_ategory(CCsingle)  {}
    ContentType(Content *p) : Base(p),m_imeType("invalid/invalid"),c_ategory(CCsingle)  {}
    ContentType(Content *p, const QByteArray &s) : Base(p)  { from7BitString(s); }
    ContentType(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~ContentType()  {}

    virtual void from7BitString(const QByteArray &s);
    virtual QByteArray as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QByteArray&);
    virtual QString asUnicodeString();
    virtual void clear()            { m_imeType.resize(0); p_arams.resize(0); }
    virtual bool isEmpty() const { return (m_imeType.isEmpty()); }
    virtual const char* type() const { return "Content-Type"; }


    //mime-type handling
    QByteArray mimeType()                     { return m_imeType; }
    QByteArray mediaType();
    QByteArray subType();
    void setMimeType(const QByteArray &s);
    bool isMediatype(const char *s);
    bool isSubtype(const char *s);
    bool isText();
    bool isPlainText();
    bool isHTMLText();
    bool isImage();
    bool isMultipart();
    bool isPartial();

    //parameter handling
    QByteArray charset();
    void setCharset(const QByteArray &s);
    QByteArray boundary();
    void setBoundary(const QByteArray &s);
    QString name();
    void setName(const QString &s, const QByteArray &cs);
    QByteArray id();
    void setId(const QByteArray &s);
    int partialNumber();
    int partialCount();
    void setPartialParams(int total, int number);

    //category
    contentCategory category()            { return c_ategory; }
    void setCategory(contentCategory c)   { c_ategory=c; }

  protected:
    QByteArray getParameter(const char *name);
    void setParameter(const QByteArray &name, const QByteArray &value, bool doubleQuotes=false);
    QByteArray m_imeType, p_arams;
    contentCategory c_ategory;

};


/** Represents a "Content-Transfer-Encoding" header */
class KMIME_EXPORT CTEncoding : public Base {

  public:
    CTEncoding() : Base(),c_te(CE7Bit),d_ecoded(true)  {}
    CTEncoding(Content *p) : Base(p),c_te(CE7Bit),d_ecoded(true)  {}
    CTEncoding(Content *p, const QByteArray &s) : Base(p)  { from7BitString(s); }
    CTEncoding(Content *p, const QString &s) : Base(p)  { fromUnicodeString(s, Latin1); }
    ~CTEncoding()  {}

    virtual void from7BitString(const QByteArray &s);
    virtual QByteArray as7BitString(bool incType=true);
    virtual void fromUnicodeString(const QString &s, const QByteArray&);
    virtual QString asUnicodeString();
    virtual void clear()            { d_ecoded=true; c_te=CE7Bit; }
    virtual const char* type() const { return "Content-Transfer-Encoding"; }

    contentEncoding cte()                   { return c_te; }
    void setCte(contentEncoding e)          { c_te=e; }
    bool decoded()                          { return d_ecoded; }
    void setDecoded(bool d=true)            { d_ecoded=d; }
    bool needToEncode()                     { return (d_ecoded && (c_te==CEquPr || c_te==CEbase64)); }

  protected:
    contentEncoding c_te;
    bool d_ecoded;

};

#endif  // __KMIME_HEADERS_OBS_H__
