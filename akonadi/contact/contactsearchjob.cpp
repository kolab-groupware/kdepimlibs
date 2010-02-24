/*
    This file is part of Akonadi Contact.

    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "contactsearchjob.h"

#include <akonadi/itemfetchscope.h>

using namespace Akonadi;

class ContactSearchJob::Private
{
  public:
    int mLimit;
};

ContactSearchJob::ContactSearchJob( QObject * parent )
  : ItemSearchJob( QString(), parent ), d( new Private() )
{
  fetchScope().fetchFullPayload();
  d->mLimit = -1;

  // by default search for all contacts
  ItemSearchJob::setQuery( QLatin1String( ""
                                          "prefix nco:<http://www.semanticdesktop.org/ontologies/2007/03/22/nco#>"
                                          "SELECT ?r WHERE { ?r a nco:PersonContact }" ) );
}

ContactSearchJob::~ContactSearchJob()
{
  delete d;
}

void ContactSearchJob::setQuery( Criterion criterion, const QString &value )
{
  QString query = QString::fromLatin1(
            "prefix nco:<http://www.semanticdesktop.org/ontologies/2007/03/22/nco#>" );

  if ( criterion == Name ) {
    query += QString::fromLatin1(
        "SELECT DISTINCT ?r "
        "WHERE { "
        "  graph ?g { "
        "    ?r a nco:PersonContact . "
        "    ?r nco:fullname \"%1\"^^<http://www.w3.org/2001/XMLSchema#string>. "
        "  } "
        "} " );
  } else if ( criterion == Email ) {
    query += QString::fromLatin1(
        "SELECT DISTINCT ?person "
        "WHERE { "
        "  graph ?g { "
        "    ?person a nco:PersonContact ; "
        "            nco:hasEmailAddress ?email . "
        "    ?email nco:emailAddress \"%1\"^^<http://www.w3.org/2001/XMLSchema#string> . "
        "  } "
        "}" );
  } else if ( criterion == NickName ) {
    query += QString::fromLatin1(
        "SELECT DISTINCT ?r "
        "WHERE { "
        "  graph ?g { "
        "    ?r a nco:PersonContact . "
        "    ?r nco:nickname ?v . "
        "    ?v bif:contains \"'%1'\" . "
        "  } "
        "}" );
  }

  if ( d->mLimit != -1 ) {
    query += QString::fromLatin1( " LIMIT %1" ).arg( d->mLimit );
  }
  query = query.arg( value );

  ItemSearchJob::setQuery( query );
}

void ContactSearchJob::setLimit( int limit )
{
  d->mLimit = limit;
}

KABC::Addressee::List ContactSearchJob::contacts() const
{
  KABC::Addressee::List contacts;

  foreach ( const Item &item, items() ) {
    if ( item.hasPayload<KABC::Addressee>() )
      contacts.append( item.payload<KABC::Addressee>() );
  }

  return contacts;
}

#include "contactsearchjob.moc"
