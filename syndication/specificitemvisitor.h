/*
 * This file is part of the syndication library
 *
 * Copyright (C) 2006 Frank Osterfeld <frank.osterfeld@kdemail.net>
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
 * You must have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef SYNDICATION_SPECIFICITEMVISITOR_H
#define SYNDICATION_SPECIFICITEMVISITOR_H

#include "syndication.h"

namespace Syndication {

class SpecificDocument;

namespace Atom
{
    class Entry;
}

namespace RDF
{
    class Item;
}

namespace RSS2
{
    class Item;
}

class SYNDICATION_EXPORT SpecificItemVisitor
{
    public:

        virtual ~SpecificItemVisitor();

        virtual bool visit(SpecificItem* item);

        virtual bool visitRSS2Item(Syndication::RSS2::Item* item);

        virtual bool visitRDFItem(Syndication::RDF::Item* item);

        virtual bool visitAtomEntry(Syndication::Atom::Entry* item);
};

} // namespace Syndication

#endif // SYNDICATION_SPECIFICITEMVISITOR_H
