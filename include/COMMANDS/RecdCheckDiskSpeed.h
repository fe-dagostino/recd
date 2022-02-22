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


#ifndef RECD_CHECK_DISK_SPEED_H
#define RECD_CHECK_DISK_SPEED_H

#include "FChannel.h"
#include "FFile.h"
#include "FFileSystem.h"
#include "FStopWatch.h"
#include "../IRecdCommand.h"
#include "../RecdConfig.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdCheckDiskSpeed : public IRecdCommand
{
public:
  /***/
  RecdCheckDiskSpeed( )
    : IRecdCommand( "CHECK DISK SPEED", 2 )
  {}

  /***/
  virtual ~RecdCheckDiskSpeed()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "check disk speed(<path>,<size kb>);\r\n" 
           "                          Return Read/Write time speed on specified disk.\r\n"
           "                          Size should be in Kilobytes units.\r\n";
  }

  FString            Help() const
  { 
    return "\r\n"
           " Usage:\r\n"
           "    check disk speed( /mnt/usbkey1, size );\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    FString _sMountPoint  = rArgs[0].Trim();
    INT     _iSize        = (INT)rArgs[1];
    double  _dWriteTime   = 0.0;
    double  _dReadTime    = 0.0;

    
    FString 	 _sOutFilename( 0, "%s/TEST_%010d", (const char*)_sMountPoint, time(NULL) );
    DWORD        _dwSizeBytes  = 1024;
    DWORD        _dwGotBytes   = 0;
    BYTE*        _pBuffer      = new BYTE[_dwSizeBytes];
    FStopWatch   _swOperation;
    FFile  	 _ffile( NULL, _sOutFilename );
    
    FTRY
    {
      _ffile.Open( FREAD_WRITE_OPEN, 0, FCREATE_ALWAYS, 0 );
      
      //Reset SW
      _swOperation.Reset();
      
      // Write 1 Kb for _iSize times
      for ( int cntW = 0; cntW < _iSize; cntW++ )
        _ffile.Write( _pBuffer, _dwSizeBytes );
      
      _ffile.Flush();
      
      _dWriteTime = _swOperation.Peek();
      
      _ffile.SeekToBegin();
      
      //Reset SW
      _swOperation.Reset();
       
      // Read 1 Kb for _iSize times
      for ( int cntR = 0; cntR < _iSize; cntR++ )
        _ffile.Read( _pBuffer, _dwSizeBytes, &_dwGotBytes );
      
      _dReadTime  = _swOperation.Peek();
      
      //Close test file
      _ffile.Close();
      
      //File must be deleted
      //FFile::Delete( _sOutFilename, NULL );
      
      rResults.Add( new FString( _dWriteTime ) );
      rResults.Add( new FString( _dReadTime  ) );
    }
    FCATCH( FFileSystemException, ex )
    {
      TRACE_EXCEPTION_CATCH( ex, Execute() )

      rResults.Add( new FString( ex.GetErrorCode() ) );
      rResults.Add( new FString( ex.GetMessage()   ) );
      
      _retVal = rciError;
    }

    // Release tmp buffer.
    if ( _pBuffer != NULL )
    {
      delete [] _pBuffer;
      _pBuffer = NULL;
    }

    return _retVal;
  }

private:
  
};

#endif //RECD_CHECK_DISK_SPEED_H


