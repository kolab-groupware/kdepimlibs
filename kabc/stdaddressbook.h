/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KABC_STDADDRESSBOOK_H
#define KABC_STDADDRESSBOOK_H

#include "addressbook.h"

namespace KABC {

/**
  Standard KDE address book

  This class provides access to the standard KDE address book shared by all
  applications.

  It's implemented as a singleton. Use self() to get the address book
  object. On the first self() call the address book also gets loaded.

  Example:

  \code
  KABC::AddressBook *ab = KABC::StdAddressBook::self();

  AddressBook::Ticket *ticket = ab->requestSaveTicket();

  if ( ticket ) {
    KABC::AddressBook::Iterator it;
    for ( it = ab->begin(); it != ab->end(); ++it ) {
      kDebug( 5700 ) << "UID=" << ( *it ).uid();

      // do some other stuff
    }

    KABC::StdAddressBook::save( ticket );
  }
  \endcode

  @deprecated Port to libakonadi-kontact. For instance using Akonadi::ContactSearchJob.
  See http://techbase.kde.org/Development/AkonadiPorting/AddressBook for details.
*/
class KABC_DEPRECATED_EXPORT StdAddressBook : public AddressBook
{
  public:

    /**
      Destructor.
     */
    ~StdAddressBook();

    /**
      Returns the standard addressbook object. It also loads all resources of
      the users standard address book synchronously.
     */
    static StdAddressBook *self();

    /**
      This is the same as above, but with specified behaviour of resource loading.

      @param asynchronous When true, the resources are loaded asynchronous, that
                          means you have the data foremost the addressBookChanged()
                          signal has been emitted. So connect to this signal when
                          using this method!
     */
    static StdAddressBook *self( bool asynchronous );

    /**
      Saves the standard address book to disk.

      @deprecated Use AddressBook::save( Ticket* ) instead
     */
    static KABC_DEPRECATED bool save();

    /**
      Returns the default file name for vcard-based addressbook
     */
    static QString fileName();

    /**
      Returns the default directory name for vcard-based addressbook
     */
    static QString directoryName();

    /**
      Sets the automatic save property of the address book.

      @param state If true, the address book is saved automatically
                   at destruction time, otherwise you have to call
                   AddressBook::save( Ticket* ).
     */
    static void setAutomaticSave( bool state );

    /**
      Closes the address book. Depending on automaticSave() it will
      save the address book first.
    */
    static void close();

    /**
      Returns whether the address book is saved at destruction time.
      See also setAutomaticSave().
     */
    static bool automaticSave();

    /**
      Returns the contact, that is associated with the owner of the
      address book. This contact should be used by other programs
      to access user specific data.
     */
    Addressee whoAmI() const;

    /**
      Sets the users contact. See whoAmI() for more information.

      @param addr The users contact.
     */
    void setWhoAmI( const Addressee &addr );

  protected:
    StdAddressBook();
    StdAddressBook( bool asynchronous );

  private:
    // needed another constructor for delaying Private::init() to right
    // after the instance creation. Cannot change the other two since they
    // are protected and might be called by subclasses
    StdAddressBook( bool asynchronous, bool doInit );

    class Private;
    Private *const d;
};

}

#endif

