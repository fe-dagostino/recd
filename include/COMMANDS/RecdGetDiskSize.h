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


#ifndef RECD_GET_DISK_SIZE_H
#define RECD_GET_DISK_SIZE_H

#include "FChannel.h"
#include "FFileSystem.h"
#include "../IRecdCommand.h"
#include "../RecdConfig.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdGetDiskSize : public IRecdCommand
{
public:
  /***/
  RecdGetDiskSize( )
    : IRecdCommand( "GET DISK SIZE", 1 )
  {}

  /***/
  virtual ~RecdGetDiskSize()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "get disk size(<path>);    Return the amount of total and free space";
  }

  FString            Help() const
  { 
    return "\r\n"
           " Usage:\r\n"
           "    get disk size( /mnt/usbkey1 );\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    FString _sMountPoint    = rArgs[0].Trim();
    double  _dTotalSize     = 0.0;
    double  _dAvailableSize = 0.0;

    FTRY
    {
      FDiskInfo  _nfoDisk;
      FFileSystem::GetDiskInfo( _sMountPoint, _nfoDisk );

      ////////////////////////////////////////////
      // Following values are repoted here in order to clarify operations
      //
      // 1 Byte     = 8 Bits
      // 1 Kilobyte = 1024 Bytes
      // 1 Megabyte = 1048576 Bytes
      // 1 Gigabyte = 1073741824 Bytes

      _dTotalSize     = double(_nfoDisk.GetTotalBytes()    ) / 1073741824;
      _dAvailableSize = double(_nfoDisk.GetAvailableBytes()) / 1073741824;
    }
    FCATCH( FFileSystemException, ex )
    {
      TRACE_EXCEPTION_CATCH( ex, Execute() )
      
      _retVal = rciError;
    }
    
    rResults.Add( new FString( _dTotalSize     ) );
    rResults.Add( new FString( _dAvailableSize ) );

    return _retVal;
  }

private:
  
};

#endif //RECD_GET_DISK_SIZE_H


