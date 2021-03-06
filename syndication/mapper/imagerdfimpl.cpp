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

#include "imagerdfimpl.h"

#include <QtCore/QString>

namespace Syndication {

ImageRDFImpl::ImageRDFImpl(const Syndication::RDF::Image& image)
    : m_image(image)
{}
        
bool ImageRDFImpl::isNull() const
{
    return m_image.isNull();
}

QString ImageRDFImpl::url() const
{
    return m_image.url();
}

QString ImageRDFImpl::title() const
{
    return m_image.title();
}

QString ImageRDFImpl::link() const
{
    return m_image.link();
}

QString ImageRDFImpl::description() const
{
    return QString();
}

uint ImageRDFImpl::height() const
{
    return 0;
}
 
 uint ImageRDFImpl::width() const
{
    return 0;
}

} // namespace Syndication
