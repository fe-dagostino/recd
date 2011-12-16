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


#ifndef RECD_HELP_H
#define RECD_HELP_H

#include "FChannel.h"
#include "../IRecdCommand.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdHelp : public IRecdCommand
{
public:
  /***/
  RecdHelp( FRciServer& rRciServer )
    : IRecdCommand( "HELP", 1 ),
  m_rRciServer( rRciServer )
  {}

  /***/
  virtual ~RecdHelp()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "help()                    Provide help for specified command."; 
  }

  FString            Help() const
  { 
    return "\r\n"
           " Usage:\r\n"
           "      help( <cmd name> )\r\n"; 
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RciResponse _retVal   = rciOk;                        //Default value

    FString    _sMethod  = rArgs[0].Trim();
  
    //Convert method name to upper case.
    _sMethod.UpCase();

    const IRecdCommand*	_pRciCommand = (const IRecdCommand*)m_rRciServer.GetCommandCollector().Find( _sMethod );

    if ( _pRciCommand == NULL )
    {
      _retVal   = rciError;
      rResults.Add( new FString( "SPECIFIED_COMMAND_NOT_FOUND" ) );
    }
    else
    {
      rResults.Add( new FString( _pRciCommand->Help() ) );
    }

    return _retVal;
  }

private:
  FRciServer& m_rRciServer;
};

#endif


