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

#ifndef KABC_ADDRESS_H
#define KABC_ADDRESS_H

#include <qstring.h>
#include <qvaluelist.h>

namespace KABC {

/*
  @short Postal address information.
  
  This class represents information about a postal address.
*/
class Address
{
    friend QDataStream &operator<<( QDataStream &, const Address & );
    friend QDataStream &operator>>( QDataStream &, Address & );

  public:
    /**
     * List of addresses.
     */
    typedef QValueList<Address> List;
  
    /**
     * Address types:
     *
     * @li @p Dom -    domestic
     * @li @p Intl -   international
     * @li @p Postal - postal
     * @li @p Parcel - parcel
     * @li @p Home -   home address
     * @li @p Work -   address at work
     * @li @p Pref -   preferred address
    */
    enum Type { Dom = 1, Intl = 2, Postal = 4, Parcel = 8, Home = 16, Work = 32,
           Pref = 64 };

    /**
     * Constructor that creates an empty Address, which is initialized
     * with a unique id (see @ref id()).
     */
    Address();
  
    /**
     * This is like @ref Address() just above, with the difference
     * that you can specify the type.
     */
    Address( int );

    bool operator==( const Address & ) const;
    bool operator!=( const Address & ) const;
  
    /**
     * Returns, if the address is empty.
     */
    bool isEmpty() const;

    /**
     * Clears all entries of the address.
     */
    void clear();

    /**
     * Sets the unique id.
     */
    void setId( const QString & );

    /*
     * Returns the unique id.
     */
    QString id() const;

    /**
     * Sets the type of address. See enum for definiton of types. 
     *
     * @param type type, can be a bitwise or of multiple types.
     */
    void setType( int type );

    /**
     * Returns the type of address. Can be a bitwise or of multiple types.
     */
    int type() const;

    /**
     * Sets the post office box.
     */
    void setPostOfficeBox( const QString & );

    /**
     * Returns the post office box.
     */
    QString postOfficeBox() const;

    /**
     * Returns the translated label for post office box field.
     */
    QString postOfficeBoxLabel() const;

    /**
     * Sets the extended address information.
     */
    void setExtended( const QString & );

    /**
     * Returns the extended address information.
     */
    QString extended() const;

    /**
     * Returns the translated label for extended field.
     */
    QString extendedLabel() const;
    
    /**
     * Sets the street (including number).
     */
    void setStreet( const QString & );

    /**
     * Returns the street.
     */
    QString street() const;

    /**
     * Returns the translated label for street field.
     */
    QString streetLabel() const;

    /**
     * Sets the locality, e.g. city.
     */
    void setLocality( const QString & );

    /**
     * Returns the locality.
     */
    QString locality() const;

    /**
     * Returns the translated label for locality field.
     */
    QString localityLabel() const;

    /**
     * Sets the region, e.g. state.
     */
    void setRegion( const QString & );

    /**
     * Returns the region.
     */
    QString region() const;

    /**
     * Returns the translated label for region field.
     */
    QString regionLabel() const;
 
    /**
     * Sets the postal code.
     */
    void setPostalCode( const QString & );

    /**
     * Returns the postal code.
     */
    QString postalCode() const;

    /**
     * Returns the translated label for postal code field.
     */
    QString postalCodeLabel() const;

    /**
     * Sets the country.
     */
    void setCountry( const QString & );

    /**
     * Returns the country.
     */
    QString country() const;

    /**
     * Returns the translated label for country field.
     */
    QString countryLabel() const;

    /**
     * Sets the delivery label. This is the literal text to be used as label.
     */
    void setLabel( const QString & );

    /**
     * Returns the delivery label.
     */
    QString label() const;

    /**
     * Returns the translated label for delivery label field.
     */
    QString labelLabel() const;

    /**
     * Used for debug output.
     */
    void dump() const;

  private:
    bool mEmpty;
  
    QString mId;
    int mType;
  
    QString mPostOfficeBox;
    QString mExtended;
    QString mStreet;
    QString mLocality;
    QString mRegion;
    QString mPostalCode;
    QString mCountry;
    QString mLabel;
};

QDataStream &operator<<( QDataStream &, const Address & );
QDataStream &operator>>( QDataStream &, Address & );

}
#endif
