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


#ifndef RECD_START_RECORDING_H
#define RECD_START_RECORDING_H

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
class RecdStartRecording : public IRecdCommand
{
public:
  /***/
  RecdStartRecording( )
    : IRecdCommand( "START RECORDING", 5 )
  {}

  /***/
  virtual ~RecdStartRecording()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "start recording( ,   Return the list of all available commands.\n"
           "            <destination>)";     
  }

  FString            Help() const
  { 
    return " Usage:\r\n"
           "    start recording( destination );\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    FString _sDstPath1   = rArgs[0].Trim();
    FString _sDstPath2   = rArgs[1].Trim();
    FString _sRender     = rArgs[2].Trim();
    FString _sHighlights = rArgs[3].Trim();
    FString _sRaw        = rArgs[4].Trim();

    
    BOOL     _bExist   = TRUE;
    
    FTRY
    {
      if ( _sDstPath1 != "NULL" )
	_bExist = FFile::Exist( _sDstPath1, NULL );
      
      if ( (_bExist == TRUE) && ( _sDstPath2 != "NULL" ) )
	_bExist = FFile::Exist( _sDstPath2, NULL );
    }
    FCATCH( FFileSystemException, ex )
    {
      TRACE_EXCEPTION_CATCH( ex, Execute() );
      _bExist = FALSE;
    }

    // If destination path exist both reading and recording will 
    // starts and return value will be rciOk.
    if ( _bExist == TRUE )
    {
	// Start reading each cam
	RecdReaderCollector::GetInstance().SetReading   ( TRUE );
	// Start rendering both single video and mixed video.
	RecdEncoderCollector::GetInstance().StartRecording( _sDstPath1 );
    }
    else
    {
      _retVal = rciError;
    }

    return _retVal;
  }

private:
  
};

#endif //RECD_START_RECORDING_H


