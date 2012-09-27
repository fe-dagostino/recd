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


#ifndef RECD_LIST_H
#define RECD_LIST_H

#include "FChannel.h"
#include "../IRecdCommand.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdList : public IRecdCommand
{
public:
  /***/
  RecdList( FRciServer& rRciServer )
    : IRecdCommand( "LIST", 0 ),
      m_rRciServer( rRciServer )
  {}

  /***/
  virtual ~RecdList()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "list();                   Return the list of all available commands."; 
  }

  FString            Help() const
  { 
    return " Usage:\r\n"
           "    list();\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    _rResults.SetSeparator( "\r\n" );

    _rResults.Add( new FString( "COMMAND                   DESCRIPTION\r\n" ) );

    FRciCommandCollector::Iterator _cmdIter = m_rRciServer.GetCommandCollector().Begin();
    IRecdCommand* _pCmd = NULL;
    while ( _pCmd = (IRecdCommand*)_cmdIter.GetCommand() )
    {
      _rResults.Add( new FString( _pCmd->Description() ) );
    }

    return _retVal;
  }

private:
  FRciServer& m_rRciServer;
};

#endif


