/*
 * This file is part of libsyndication
 *
 * Copyright (C) 2005 Frank Osterfeld <frank.osterfeld@kdemail.net>
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

#include "category.h"
#include "enclosure.h"
#include "item.h"
#include "source.h"
#include "tools.h"

#include <constants.h>
#include <specificitem.h>
#include <specificitemvisitor.h>
#include <tools.h>

#include <QDomElement>
#include <QString>
#include <QList>

namespace LibSyndication {
namespace RSS2 {

Item::Item() : ElementWrapper()
{
}

Item::Item(const QDomElement& element) : ElementWrapper(element)
{
}

QString Item::title() const
{
    QString t = extractElementTextNS(QString(), QString::fromUtf8("title"));
    
    if (t.isNull())
    {
        t = extractElementTextNS(dublinCoreNamespace(),
                                 QString::fromUtf8("title"));
    }
    return htmlize(t);
}

QString Item::link() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("link") );
}

QString Item::description() const
{
    QString d = extractElementTextNS(QString(), QString::fromUtf8("description"));
    
    if (d.isNull())
    {
        d = extractElementTextNS(dublinCoreNamespace(),
                                 QString::fromUtf8("description"));
    }
    
    return htmlize(d);
}

QString Item::content() const
{
    // parse encoded stuff from content:encoded, xhtml:body and friends into content
    return extractContent(*this);
}

QList<Category> Item::categories() const
{
    QList<QDomElement> cats = elementsByTagNameNS(QString(),
            QString::fromUtf8("category"));

    QList<Category> categories;

    QList<QDomElement>::ConstIterator it = cats.begin();
    for ( ; it != cats.end(); ++it)
    {
        categories.append(Category(*it));
    }
    return categories;
}

QString Item::comments() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("comments") );
}

QString Item::author() const
{
    QString a = extractElementTextNS(QString(), QString::fromUtf8("author") );
    if (!a.isNull()) 
    {
        return a;
    }
    else
    {
        // if author is not available, fall back to dc:creator
        return extractElementTextNS(dublinCoreNamespace(),
                                    QString::fromUtf8("creator") );
    }
    
}

QList<Enclosure> Item::enclosures() const
{
    QList<QDomElement> encs = elementsByTagNameNS(QString(),
            QString::fromUtf8("enclosure"));

    QList<Enclosure> enclosures;

    QList<QDomElement>::ConstIterator it = encs.begin();
    for ( ; it != encs.end(); ++it)
    {
        enclosures.append(Enclosure(*it));
    }
    return enclosures;
}

QString Item::guid() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("guid") );
}

bool Item::guidIsPermaLink() const
{
    bool guidIsPermaLink = true;  // true is default

    QDomElement guidNode = firstElementByTagNameNS(QString(), 
            QString::fromUtf8("guid"));
    if (!guidNode.isNull())
    {
        if (guidNode.attribute(QString::fromUtf8("isPermaLink")) 
            == QString::fromUtf8("false"))
        {
            guidIsPermaLink = false;
        }
    }

    return guidIsPermaLink;
}

time_t Item::pubDate() const
{
    QString str = extractElementTextNS(QString(), QString::fromUtf8("pubDate"));
    
    if (!str.isNull())
    {
        return parseDate(str, RFCDate);
    }
    
    // if there is no pubDate, check for dc:date
    str = extractElementTextNS(dublinCoreNamespace(), QString::fromUtf8("date"));
    return parseDate(str, ISODate);
}

time_t Item::expirationDate() const
{
    QString str = extractElementTextNS(QString(), QString::fromUtf8("expirationDate"));
    return parseDate(str, RFCDate);
}

Source Item::source() const
{
    return Source(firstElementByTagNameNS(QString(), QString::fromUtf8("source")));
}

QString Item::rating() const
{
    return extractElementTextNS(QString(), QString::fromUtf8("rating") );
}

QString Item::debugInfo() const
{
    QString info;
    info += "### Item: ###################\n";
    if (!title().isNull())
        info += "title: #" + title() + "#\n";
    if (!link().isNull())
        info += "link: #" + link() + "#\n";
    if (!description().isNull())
        info += "description: #" + description() + "#\n";
    if (!content().isNull())
        info += "content: #" + content() + "#\n";
    if (!author().isNull())
        info += "author: #" + author() + "#\n";
    if (!comments().isNull())
        info += "comments: #" + comments() + "#\n";
    QString dpubdate = dateTimeToString(pubDate());
    if (!dpubdate.isNull())
        info += "pubDate: #" + dpubdate + "#\n";
    if (!guid().isNull())
        info += "guid: #" + guid() + "#\n";
    if (guidIsPermaLink())
        info += "guid is PL: #true#\n";
    if (!source().isNull())
         info += source().debugInfo();
    
    QList<Category> cats = categories();
    for (QList<Category>::ConstIterator it = cats.begin(); it != cats.end(); ++it)
        info += (*it).debugInfo();
    QList<Enclosure> encs = enclosures();
    for (QList<Enclosure>::ConstIterator it = encs.begin(); it != encs.end(); ++it)
        info += (*it).debugInfo();

    info += "### Item end ################\n";
    return info;
}

        
bool Item::accept(SpecificItemVisitor* visitor)
{
    return visitor->visitRSS2Item(this);
}

} // namespace RSS2
} // namespace LibSyndication
