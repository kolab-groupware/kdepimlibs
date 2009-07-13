/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef MESSAGECOMPOSER_MESSAGEPART_H
#define MESSAGECOMPOSER_MESSAGEPART_H

#include "behaviour.h"
#include "messagecomposer_export.h"

#include <QtCore/QObject>

#include <kmime/kmime_headers.h>

namespace MessageComposer {

class MessagePartPrivate;

/**
*/
class MESSAGECOMPOSER_EXPORT MessagePart : public QObject
{
  Q_OBJECT

  public:
    MessagePart( QObject *parent = 0 );
    virtual ~MessagePart();

    bool isAutoTransferEncoding() const;
    KMime::Headers::contentEncoding overrideTransferEncoding() const;
    void setOverrideTransferEncoding( KMime::Headers::contentEncoding cte );

  private:
    class Private;
    Private *const d;
};

}

#endif
