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


#ifndef RECD_GET_BUFFERS_H
#define RECD_GET_BUFFERS_H

#include "FChannel.h"
#include "FFile.h"
#include "../IRecdCommand.h"
#include "RecdReaderCollector.h"
#include "RecdEncoderCollector.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdGetBuffers : public IRecdCommand
{
public:
  /***/
  RecdGetBuffers( )
    : IRecdCommand( "GET BUFFERS", 1 )
  {}

  /***/
  virtual ~RecdGetBuffers()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "get buffers( <bflush> );  retrieve buffers status. bflush can be 0 or 1.\r\n";

  }

  FString            Help() const
  { 
    return "\r\n"
           " Usage:\r\n"
           "    get buffers( 0 );\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    FString _sFlush      = rArgs[0].Trim();
    DOUBLE  _dPercentage = 0.0;
    BOOL    _bRetVal     = FALSE;
    if ( _sFlush == "0" )
    {
	_bRetVal = RecdReaderCollector::GetInstance().GetBuffers( RecdStreamReader::eRSBuffering, &_dPercentage );
    }
    else
    {
	_bRetVal = RecdReaderCollector::GetInstance().GetBuffers( RecdStreamReader::eRSFlushing, &_dPercentage );
	
	_bRetVal &= RecdEncoderCollector::GetInstance().ReadyForRecording();
    }

    rResults.Add( new FString( _dPercentage ) );
    rResults.Add( new FString( _bRetVal     ) );
        
    return _retVal;
  }

private:
  
};

#endif //RECD_GET_BUFFERS_H


