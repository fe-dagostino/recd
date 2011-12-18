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


#ifndef RECD_ESTIMATE_TIME_H
#define RECD_ESTIMATE_TIME_H

#include "FChannel.h"
#include "FFile.h"
#include "../IRecdCommand.h"
#include "../RecdConfig.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdEstimateTime : public IRecdCommand
{
public:
  /***/
  RecdEstimateTime( )
    : IRecdCommand( "ESTIMATE TIME", 4 )
  {}

  /***/
  virtual ~RecdEstimateTime()
  {}

  FString            Description() const
  {  
    //      01234567890123456789012345678901234567890123456789012345678901234567890123456789
    //      01234567890123456789012345
    return "estimate time(<size gb>,    Return the amoutn of time for required services.\n"
           "          <render>,         Size should be in Gigabite. Services should be\n"
           "          <highlights>      a value 0 (Zero) or 1 (One) where 1 meand that\n"     
           "          <raw>)            serice is required as part of estimation.\n";     
  }

  FString            Help() const
  { 
    return " Usage:\r\n"
           "    estimate time( size, 1, 0, 0 );\r\n";
  }

  RciResponse        Execute( FChannel& rChannel, const FArguments& rArgs, FArguments& rResults ) const
  {
    RecdResult& _rResults = (RecdResult&)rResults;
    RciResponse _retVal   = rciOk;                        //Default value

    FString _sSize       = rArgs[0].Trim();
    FString _sRender     = rArgs[1].Trim();
    FString _sHighlights = rArgs[2].Trim();
    FString _sRaw        = rArgs[3].Trim();

    
    double  _dAvailabeSize      = (double)_sSize;
    double  _dSizePerSec        = 0.0;
    double  _dTime              = 0.0;
    double  _dBitRates          = 0.0;
    double  _dRenderBitRate     = 0.0;
    double  _dRawBitRate        = 0.0;
    double  _dHighlightsBitRate = 0.0;
    
    if ( _sRender == "1" )
      _dRenderBitRate = RecdConfig::GetInstance().GetRenderBitRate( NULL );
    
    if ( _sRaw    == "1" )
    {
      FParameter*  _pIpCameras = RecdConfig::GetInstance().GetIpCameras( NULL );
      if ( _pIpCameras == NULL )
      {
	ERROR_INFO( "Parameter not FOUND", Execute() )
      }
      else
      {
	LOG_INFO( FString( 0, "Configured [%d] Cameras", _pIpCameras->GetCount() ), OnInitialize() )
	for ( DWORD _dwItem = 0; _dwItem < _pIpCameras->GetCount(); _dwItem++ )
	{
	  FString _sCamera = _pIpCameras->GetValue( _dwItem );

	  _dRawBitRate += (double)RecdConfig::GetInstance().GetEncoderBitRate( _sCamera, NULL );
	}
      }
    } //if ( _sRaw    == "1" )


    if ( _sHighlights == "1" )
    {
      _dHighlightsBitRate = _dRawBitRate;
    }


    _dBitRates   = _dRenderBitRate + _dHighlightsBitRate + _dRawBitRate;
    ////////////////////////////////////////////
    // Following values are repoted here in order to clarify operations 
    //
    // 1 Byte     = 8 Bits
    // 1 Kilobyte = 1024 Bytes
    // 1 Megabyte = 1048576 Bytes
    // 1 Gigabyte = 1073741824 Bytes    
    _dSizePerSec = ((_dBitRates / 8) / 1073741824);

    _dTime     = _dAvailabeSize / _dSizePerSec; 

    rResults.Add( new FString( _dTime ) );

    return _retVal;
  }

private:
  
};

#endif //RECD_ESTIMATE_SIZE_H
