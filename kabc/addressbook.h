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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KABC_ADDRESSBOOK_H
#define KABC_ADDRESSBOOK_H

#include <qobject.h>
#include <qptrlist.h>

#include <kresources/manager.h>

#include "addressee.h"
#include "field.h"

namespace KABC {

class ErrorHandler;
class Resource;
class Ticket;

/**
  @short Address Book
  
  This class provides access to a collection of address book entries.
*/
class AddressBook : public QObject
{
  Q_OBJECT

  friend QDataStream &operator<<( QDataStream &, const AddressBook & );
  friend QDataStream &operator>>( QDataStream &, AddressBook & );
  friend class StdAddressBook;

  public:
    /**
      @short Address Book Iterator
      
      This class provides an iterator for address book entries.
    */
    class Iterator
    {
      public:
        Iterator();
        Iterator( const Iterator & );
      	~Iterator();

      	Iterator &operator=( const Iterator & );
        const Addressee &operator*() const;
        Addressee &operator*();
        Addressee* operator->();
        Iterator &operator++();
        Iterator &operator++(int);
        Iterator &operator--();
        Iterator &operator--(int);
        bool operator==( const Iterator &it );
        bool operator!=( const Iterator &it );

      	struct IteratorData;
        IteratorData *d;
    };

    /**
      @short Address Book Const Iterator
      
      This class provides a const iterator for address book entries.
    */
    class ConstIterator
    {
      public:
        ConstIterator();
        ConstIterator( const ConstIterator & );
        ConstIterator( const Iterator & );
      	~ConstIterator();
      
      	ConstIterator &operator=( const ConstIterator & );
        const Addressee &operator*() const;
        const Addressee* operator->() const;
        ConstIterator &operator++();
        ConstIterator &operator++(int);
        ConstIterator &operator--();
        ConstIterator &operator--(int);
        bool operator==( const ConstIterator &it );
        bool operator!=( const ConstIterator &it );

      	struct ConstIteratorData;
        ConstIteratorData *d;
    };
    
    /**
      Constructs a address book object.
      
      @param format File format class.
    */
    AddressBook();
    AddressBook( const QString &config );
    virtual ~AddressBook();

    /**
      Requests a ticket for saving the addressbook. Calling this function locks
      the addressbook for all other processes. If the address book is already
      locked the function returns 0. You need the returned Ticket object
      for calling the save() function.
      
      @see save()
    */
    Ticket *requestSaveTicket( Resource *resource=0 );

    void releaseSaveTicket( Ticket *ticket );
    
    /**
      Load address book from file.
    */
    bool load();

    bool asyncLoad();

    /**
      Save address book. The address book is saved to the file, the Ticket
      object has been requested for by requestSaveTicket().
     
      @param ticket a ticket object returned by requestSaveTicket()
    */
    bool save( Ticket *ticket );

    bool asyncSave( Ticket *ticket );

    /**
      Returns a iterator for first entry of address book.
    */
    Iterator begin();

    /**
      Returns a const iterator for first entry of address book.
    */
    ConstIterator begin() const;

    /**
      Returns a iterator for first entry of address book.
    */
    Iterator end();

    /**
      Returns a const iterator for first entry of address book.
    */
    ConstIterator end() const;

    /**
      Removes all entries from address book.
    */
    void clear();
    
    /**
      Insert an Addressee object into address book. If an object with the same
      unique id already exists in the address book it it replaced by the new
      one. If not the new object is appended to the address book.
    */
    void insertAddressee( const Addressee & );

    /**
      Removes entry from the address book.
    */
    void removeAddressee( const Addressee & );

    /**
      This is like removeAddressee() just above, with the difference that
      the first element is a iterator, returned by begin().
    */
    void removeAddressee( const Iterator & );

    /**
      Find the specified entry in address book. Returns end(), if the entry
      couldn't be found.
    */
    Iterator find( const Addressee & );

    /**
      Find the entry specified by an unique id. Returns an empty Addressee
      object, if the address book does not contain an entry with this id.
    */
    Addressee findByUid( const QString & );


    /**
      Returns a list of all addressees in the address book. This list can
      be sorted with KABC::AddresseeList for example.
    */
    Addressee::List allAddressees();

    /**
      Find all entries with the specified name in the address book. Returns
      an empty list, if no entries could be found.
    */
    Addressee::List findByName( const QString & );

    /**
      Find all entries with the specified email address  in the address book.
      Returns an empty list, if no entries could be found.
    */
    Addressee::List findByEmail( const QString & );

    /**
      Find all entries which have the specified category in the address book.
      Returns an empty list, if no entries could be found.
    */
    Addressee::List findByCategory( const QString & );

    /**
      Return a string identifying this addressbook.
    */
    virtual QString identifier();

    /**
      Used for debug output.
    */
    void dump() const;

    void emitAddressBookLocked() { emit addressBookLocked( this ); }
    void emitAddressBookUnlocked() { emit addressBookUnlocked( this ); }
    void emitAddressBookChanged() { emit addressBookChanged( this ); }

    /**
      Return list of all Fields known to the address book which are associated
      with the given field category.
    */
    Field::List fields( int category = Field::All );

    /**
      Add custom field to address book.
      
      @param label    User visible label of the field.
      @param category Ored list of field categories.
      @param key      Identifier used as key for reading and writing the field.
      @param app      String used as application key for reading and writing
                      the field.
    */
    bool addCustomField( const QString &label, int category = Field::All,
                         const QString &key = QString::null,
                         const QString &app = QString::null );

    /**
      Add address book resource.
    */
    bool addResource( Resource * );

    /**
      Remove address book resource.
    */
    bool removeResource( Resource * );

    /**
      Return pointer list of all resources.
    */
    QPtrList<Resource> resources();

    /**
      Set the @p ErrorHandler, that is used by error() to
      provide GUI independent error messages.
    */
    void setErrorHandler( ErrorHandler * );

    /**
      Shows GUI independent error messages.
    */
    void error( const QString& );

    /**
      Query all resources to clean up their lock files
     */
    void cleanUp();

  signals:
    /**
      Emitted, when the address book has changed on disk.
    */
    void addressBookChanged( AddressBook * );

    /**
      Emitted, when the address book has been locked for writing.
    */
    void addressBookLocked( AddressBook * );

    /**
      Emitted, when the address book has been unlocked.
    */
    void addressBookUnlocked( AddressBook * );

    /**
      Emitted whenever the loading has finished after calling
      asyncLoad().
     */
    void loadingFinished( Resource* );

    /**
      Emitted whenever the saving has finished after calling
      asyncSave().
     */
    void savingFinished( Resource* );

  protected slots:
    void resourceLoadingFinished( Resource* );
    void resourceSavingFinished( Resource* );
    void resourceLoadingError( Resource*, const QString &errMsg );
    void resourceSavingError( Resource*, const QString &errMsg );

  protected:
    void deleteRemovedAddressees();
    void setStandardResource( Resource * );
    Resource *standardResource();
    KRES::Manager<Resource> *resourceManager();

  private:
    QPtrList<Resource> mDummy; // Remove in KDE 4
    struct AddressBookData;
    AddressBookData *d;
};

QDataStream &operator<<( QDataStream &, const AddressBook & );
QDataStream &operator>>( QDataStream &, AddressBook & );

}

#endif
