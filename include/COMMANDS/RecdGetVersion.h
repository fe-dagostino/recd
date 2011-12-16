/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2011  Francesco Emanuele D'Agostino <fedagostino@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef RECD_GET_VERSION_H
#define RECD_GET_VERSION_H

#include "FChannel.h"
#include "../IRecdCommand.h"
#include "FService.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdGetVersion : public IRecdCommand
{
public:
  /***/
  RecdGetVersion( const FService& rService )
    : IRecdCommand( "GET VERSION", 0 ),
      m_rService( rService )
  {}

  /***/
  virtual ~RecdGetVersion()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "get version()             Return app version in the form x.y.z.b."; 
  }

  FString            Help() const
  { 
    return " Usage:\r\n"
           "    get version();\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    _rResults.Add( new FString( m_rService.GetVersion() ) );

    return _retVal;
  }

private:
  const FService&	m_rService;
};

#endif


