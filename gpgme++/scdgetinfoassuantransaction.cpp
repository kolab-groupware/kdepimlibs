/*
  scdgetinfoassuantransaction.cpp - Assuan Transaction to get information from scdaemon
  Copyright (C) 2009 Klarälvdalens Datakonsult AB

  This file is part of GPGME++.

  GPGME++ is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  GPGME++ is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with GPGME++; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <config-gpgme++.h>

#include "scdgetinfoassuantransaction.h"
#include "error.h"
#include "data.h"
#include "util.h"

#ifndef _WIN32_WCE
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#endif
#include <boost/static_assert.hpp>

#include <sstream>

using namespace GpgME;
using namespace boost;

ScdGetInfoAssuanTransaction::ScdGetInfoAssuanTransaction( InfoItem item )
    : AssuanTransaction(),
      m_item( item ),
      m_command(),
      m_data()
{

}

ScdGetInfoAssuanTransaction::~ScdGetInfoAssuanTransaction() {}

static std::vector<std::string> to_reader_list( const std::string & s ) {
    std::vector<std::string> result;
#ifdef _WIN32_WCE
    for ( std::string::size_type b = 0, e = s.find( '\n', 0 ) ; e != std::string::npos ; b = e+1, e = s.find( '\n', b ) )
        if ( e > b )
            result.push_back( s.substr( b, e-b ) );
    return result;
#else
    return split( result, s, is_any_of( "\n" ), token_compress_on );
#endif
}

static std::vector<std::string> to_app_list( const std::string & s ) {
    return to_reader_list( s );
}

std::string ScdGetInfoAssuanTransaction::version() const {
    if ( m_item == Version )
        return m_data;
    else
        return std::string();
}

unsigned int ScdGetInfoAssuanTransaction::pid() const {
    if ( m_item == Pid )
        return to_pid( m_data );
    else
        return 0U;
}

std::string ScdGetInfoAssuanTransaction::socketName() const {
    if ( m_item == SocketName )
        return m_data;
    else
        return std::string();
}

char ScdGetInfoAssuanTransaction::status() const {
    if ( m_item == Status && !m_data.empty() )
        return m_data[0];
    else
        return '\0';
}

std::vector<std::string> ScdGetInfoAssuanTransaction::readerList() const {
    if ( m_item == ReaderList )
        return to_reader_list( m_data );
    else
        return std::vector<std::string>();
}

std::vector<std::string> ScdGetInfoAssuanTransaction::applicationList() const {
    if ( m_item == ApplicationList )
        return to_app_list( m_data );
    else
        return std::vector<std::string>();
}

static const char * tokens[] = {
    "version",
    "pid",
    "socket_name",
    "status",
    "reader_list",
    "deny_admin",
    "app_list",
};
BOOST_STATIC_ASSERT(( sizeof tokens / sizeof *tokens == ScdGetInfoAssuanTransaction::LastInfoItem ));

void ScdGetInfoAssuanTransaction::makeCommand() const {
    assert( m_item >= 0 );
    assert( m_item < LastInfoItem );
    m_command = "SCD GETINFO ";
    m_command += tokens[m_item];
}

const char * ScdGetInfoAssuanTransaction::command() const {
    makeCommand();
    return m_command.c_str();
}

Error ScdGetInfoAssuanTransaction::data( const char * data, size_t len ) {
    m_data.append( data, len );
    return Error();
}

Data ScdGetInfoAssuanTransaction::inquire( const char * name, const char * args, Error & err ) {
    (void)name; (void)args; (void)err;
    return Data::null;
}

Error ScdGetInfoAssuanTransaction::status( const char * status, const char * args ) {
    (void)status; (void)args;
    return Error();
}
