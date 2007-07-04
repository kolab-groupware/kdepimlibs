/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2006 Frank Osterfeld <osterfeld@kde.org>
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

#include "link.h"
#include "constants.h"

#include <QtCore/QString>

namespace Syndication {
namespace Atom {

Link::Link() : ElementWrapper()
{
}

Link::Link(const QDomElement& element) : ElementWrapper(element)
{
}

QString Link::href() const
{
    return completeURI(attribute(QString::fromUtf8("href")));
}

QString Link::rel() const
{
    //"alternate" is default
    return attribute(QString::fromUtf8("rel"), QString::fromUtf8("alternate"));
}

QString Link::type() const
{
    return attribute(QString::fromUtf8("type"));
}

QString Link::hrefLanguage() const
{
    return attribute(QString::fromUtf8("hreflang"));
}

QString Link::title() const
{
    return attribute(QString::fromUtf8("title"));
}

uint Link::length() const
{
    QString lengthStr = attribute(QString::fromUtf8("length"));
    
    bool ok;
    uint c = lengthStr.toUInt(&ok);
    return ok ? c : 0;
}

QString Link::debugInfo() const
{
    QString info;
    info += "### Link: ###################\n";
    if (!title().isEmpty())
        info += "title: #" + title() + "#\n";
    if (!href().isEmpty())
        info += "href: #" + href() + "#\n";
    if (!rel().isEmpty())
        info += "rel: #" + rel() + "#\n";
    if (!type().isEmpty())
        info += "type: #" + type() + "#\n";
    if (length() != 0)
        info += "length: #" + QString::number(length()) + "#\n";
    if (!hrefLanguage().isEmpty())
        info += "hrefLanguage: #" + hrefLanguage() + "#\n";
    info += "### Link end ################\n";
    return info;
}

} // namespace Atom
} //namespace Syndication
