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

#include "model.h"
#include "model_p.h"

namespace Syndication {
namespace RDF {

long Model::ModelPrivate::idCounter = 0;

Model::Model() : d(new ModelPrivate)
{
}

Model::Model(const Model& other)
{
    *this = other;
}

Model::~Model()
{
}

Model& Model::operator=(const Model& other)
{
    d = other.d;
    return *this;
}

bool Model::operator==(const Model& other) const
{
    return *d == *(other.d);
}

PropertyPtr Model::createProperty(const QString& uri)
{
    PropertyPtr prop;

    if (d->properties.contains(uri))
    {
        prop = d->properties[uri];
    }
    else
    {
        prop = PropertyPtr( new Property(uri) );
        prop->setModel(*this);
        // if there is a resource object with the same uri, replace
        // the resource object by the new property object and reuse the id
        if (d->resources.contains(uri))
        {
            prop->setId(d->resources[uri]->id());
        }
        d->addToHashes(prop);
    }

    return prop;

}

ResourcePtr Model::createResource(const QString& uri)
{
    ResourcePtr res;

    if (d->resources.contains(uri))
    {
        res = d->resources[uri];
    }
    else
    {
        res = ResourcePtr( new Resource(uri) );
        res->setModel(*this);
        d->addToHashes(res);
    }

    return res;
}

SequencePtr Model::createSequence(const QString& uri)
{
    SequencePtr seq;

    if (d->sequences.contains(uri))
    {
        seq = d->sequences[uri];
    }
    else
    {
        seq = SequencePtr( new Sequence(uri) );
        seq->setModel(*this);
        // if there is a resource object with the same uri, replace
        // the resource object by the new sequence object and reuse the id
        if (d->resources.contains(uri))
        {
            seq->setId(d->resources[uri]->id());
        }

        d->addToHashes(seq);
    }

    return seq;
}

LiteralPtr Model::createLiteral(const QString& text)
{
    LiteralPtr lit(new Literal(text));

    d->addToHashes(lit);
    return lit;
}


void Model::removeStatement(StatementPtr statement)
{
    removeStatement(statement->subject(), statement->predicate(), statement->object());
}

void Model::removeStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object)
{
    QString key = QString("%1-%2-%3")
            .arg(QString::number(subject->id()))
            .arg(QString::number(predicate->id()))
            .arg(QString::number(object->id()));
    d->removeFromHashes(key);
}

StatementPtr Model::addStatement(ResourcePtr subject, PropertyPtr predicate, NodePtr object)
{
    d->init();
    ResourcePtr subjInternal = subject;

    if (!d->nodes.contains(subjInternal->id()))
    {
        subjInternal = ResourcePtr( subject->clone() );
        subjInternal->setModel(*this);
        d->addToHashes(subjInternal);
    }

    PropertyPtr predInternal = predicate;

    if (!d->nodes.contains(predInternal->id()))
    {
        predInternal = PropertyPtr( predicate->clone() );
        predInternal->setModel(*this);
        d->addToHashes(predInternal);
    }

    NodePtr objInternal = object;

    if (!d->nodes.contains(objInternal->id()))
    {
        objInternal = NodePtr( object->clone() );
        objInternal->setModel(*this);
        d->addToHashes(objInternal);
    }

    // TODO: avoid duplicated stmts with literal objects!

    QString key = QString("%1-%2-%3")
            .arg(QString::number(subjInternal->id()))
            .arg(QString::number(predInternal->id()))
            .arg(QString::number(objInternal->id()));

    StatementPtr stmt;

    if (!d->statements.contains(key))
    {
        stmt = StatementPtr( new Statement(subjInternal, predInternal, objInternal) );
        d->addToHashes(stmt, key);
    }
    else
    {
        stmt = d->statements[key];
    }

    return stmt;
}

bool Model::isEmpty() const
{
    return d->statements.isEmpty();
}

bool Model::resourceHasProperty(const Resource* resource, PropertyPtr property) const
{
    return d->resourceHasProperty( resource, property );
}

bool Model::ModelPrivate::resourceHasProperty(const Resource* resource, PropertyPtr property) const
{
    // resource unknown
    if (!resources.contains(resource->uri()))
        return false;

    QList<StatementPtr> stmts = stmtsBySubject[resource->uri()];
    QList<StatementPtr>::ConstIterator it = stmts.constBegin();
    QList<StatementPtr>::ConstIterator end = stmts.constEnd();

    for ( ; it != end; ++it)
    {
        if (*((*it)->predicate()) == *property)
            return true;
    }

    return false;
}

StatementPtr Model::resourceProperty(const Resource* resource, PropertyPtr property) const
{
    return d->resourceProperty(resource, property);
}

StatementPtr Model::ModelPrivate::resourceProperty(const Resource* resource, PropertyPtr property) const
{
    QList<StatementPtr> stmts = stmtsBySubject[resource->uri()];
    QList<StatementPtr>::ConstIterator it = stmts.constBegin();
    QList<StatementPtr>::ConstIterator end = stmts.constEnd();

    for ( ; it != end; ++it)
    {
        if (*((*it)->predicate()) == *property)
            return *it;
    }

    return nullStatement;
}

QList<StatementPtr> Model::resourceProperties(const Resource* resource, PropertyPtr property) const
{
    return d->resourceProperties( resource, property );
}

QList<StatementPtr> Model::ModelPrivate::resourceProperties(const Resource* resource, PropertyPtr property) const
{
    QList<StatementPtr> res;
    QList<StatementPtr> stmts = stmtsBySubject[resource->uri()];
    QList<StatementPtr>::ConstIterator it = stmts.constBegin();
    QList<StatementPtr>::ConstIterator end = stmts.constEnd();

    for ( ; it != end; ++it)
    {
        if (*((*it)->predicate()) == *property)
            res.append(*it);
    }

    return res;
}


QList<StatementPtr> Model::statements() const
{
    return d->statements.values();
}

QString Model::debugInfo() const
{
    QString info;

    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.constBegin();
    QList<StatementPtr>::ConstIterator end = stmts.constEnd();

    for ( ; it != end; ++it)
    {
        info += QString("<%1> <%2> ").arg((*it)->subject()->uri()).arg((*it)->predicate()->uri());

        if ((*it)->object()->isLiteral())
        {
            info += QString("\"%1\"\n").arg((*it)->asString());
        }
        else
        {
            info += QString("<%1>\n").arg((*it)->asResource()->uri());
        }

    }

    return info;
}

QList<ResourcePtr> Model::resourcesWithType(ResourcePtr type) const
{
    QList<ResourcePtr> list;

    QList<StatementPtr> stmts = d->statements.values();
    QList<StatementPtr>::ConstIterator it = stmts.constBegin();
    QList<StatementPtr>::ConstIterator end = stmts.constEnd();

    for ( ; it != end; ++it)
    {
        if (*((*it)->predicate()) == *(RDFVocab::self()->type()) && *((*it)->object()) == *type )
            list.append((*it)->subject());
    }

    return list;
}

NodePtr Model::nodeByID(uint id) const
{
    return d->nodeByID(id);
}

NodePtr Model::ModelPrivate::nodeByID(uint _id) const
{
    if (!nodes.contains(_id))
    {
        return nullLiteral;
    }
    else
    {
        return nodes.value(_id);
    }
}

ResourcePtr Model::resourceByID(uint id) const
{
    return d->resourceByID(id);
}

ResourcePtr Model::ModelPrivate::resourceByID(uint _id) const
{
    if (!nodes.contains(_id))
    {
        return nullResource;
    }
    else
    {
        NodePtr node = nodes.value(_id);
        if (node->isResource())
            return boost::static_pointer_cast<Resource>(node);
        else
            return nullResource;
    }
}

PropertyPtr Model::propertyByID(uint id) const
{
    return d->propertyByID(id);
}

PropertyPtr Model::ModelPrivate::propertyByID(uint _id) const
{
    if (!nodes.contains(_id))
    {
        return nullProperty;
    }
    else
    {
        NodePtr node = nodes.value(_id);
        if (node->isProperty())
            return  boost::static_pointer_cast<Property>(node);
        else
            return nullProperty;
    }
}

LiteralPtr Model::literalByID(uint id) const
{
    return d->literalByID(id);
}


LiteralPtr Model::ModelPrivate::literalByID(uint _id) const
{
    if (!nodes.contains(_id))
    {
        return nullLiteral;
    }
    else
    {
        NodePtr node = nodes.value(_id);
        if (node->isLiteral())
            return boost::static_pointer_cast<Literal>(node);
        else
            return nullLiteral;
    }
}

} // namespace RDF
} // namespace Syndication
