#include <kdebug.h>

#include "resource.h"

using namespace KABC;

Resource::Resource( AddressBook *ab ) 
    : mAddressBook( ab )
{
  mReadOnly = true;
  mFastResource = true;
}

Resource::~Resource()
{
}

bool Resource::open()
{
  return true;
}

void Resource::close()
{
}

Ticket *Resource::requestSaveTicket()
{
  return 0;
}

bool Resource::load()
{
  return true;
}

bool Resource::save( Ticket * )
{
  return false;
}

Ticket *Resource::createTicket( Resource *resource )
{
  return new Ticket( resource );
}

QString Resource::identifier() const
{
  return "NoIdentifier";
}

void Resource::removeAddressee( const Addressee& )
{
  // do nothing
}

void Resource::setReadOnly( bool value )
{
  mReadOnly = value;
}

bool Resource::readOnly() const
{
  return mReadOnly;
}

void Resource::setFastResource( bool value )
{
  mFastResource = value;
}

bool Resource::fastResource() const
{
  return mFastResource;
}

void Resource::setName( const QString &name )
{
  mName = name;
}

QString Resource::name() const
{
  return mName;
}

QString Resource::cryptStr( const QString &str )
{
  QString result;
  for ( uint i = 0; i < str.length(); ++i )
    result += ( str[ i ].unicode() < 0x20 ) ? str[ i ] :
        QChar( 0x1001F - str[ i ].unicode() );
                
  return result;
}
