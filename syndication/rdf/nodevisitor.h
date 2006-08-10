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
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef SYNDICATION_RDF_NODEVISITOR_H
#define SYNDICATION_RDF_NODEVISITOR_H

#include "syndication.h"

namespace Syndication {

template <class T> class SharedPtr;

namespace RDF {

class Literal;
typedef SharedPtr<Literal> LiteralPtr;
class Node;
typedef SharedPtr<Node> NodePtr;
class Property;
typedef SharedPtr<Property> PropertyPtr;
class Resource;
typedef SharedPtr<Resource> ResourcePtr;
class Sequence;
typedef SharedPtr<Sequence> SequencePtr;

class SYNDICATION_EXPORT NodeVisitor
{
    public:

        virtual ~NodeVisitor();

        virtual void visit(NodePtr node);

        virtual bool visitLiteral(LiteralPtr);

        virtual bool visitNode(NodePtr);

        virtual bool visitProperty(PropertyPtr);

        virtual bool visitResource(ResourcePtr);

        virtual bool visitSequence(SequencePtr);
};

} // namespace RDF
} // namespace Syndication

#endif // SYNDICATION_RDF_NODEVISITOR_H
