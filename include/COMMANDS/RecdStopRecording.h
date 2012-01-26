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


#ifndef RECD_STOP_RECORDING_H
#define RECD_STOP_RECORDING_H

#include "FChannel.h"
#include "../IRecdCommand.h"
#include "RecdReaderCollector.h"
#include "RecdEncoderCollector.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdStopRecording : public IRecdCommand
{
public:
  /***/
  RecdStopRecording( )
    : IRecdCommand( "STOP RECORDING", 0 )
  {}

  /***/
  virtual ~RecdStopRecording()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "stop recording()          Terminate all streaming files.";
  }

  FString            Help() const
  { 
    return "\r\n"
           " Usage:\r\n"
           "    stop recording( TRUE|FALSE );\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    // Terminate reading from each cam
    RecdReaderCollector::GetInstance().SetReading   ( FALSE );
    
    return _retVal;
  }

private:
  
};

#endif //RECD_STOP_RECORDING_H


