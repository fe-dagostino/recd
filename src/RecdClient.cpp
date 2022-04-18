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


#include "../include/RecdClient.h"
#include "../include/RecdParser.h"

#include "FMutexCtrl.h"
#include "FTcpConnection.h"
#include "LOGGING/FLogger.h"


USING_NAMESPACE_LOGGING

GENERATE_CLASSINFO( RecdClient, FObject )

RecdClient::RecdClient( const FString& sIP, WORD wPort, LONG iReadTimeout )
  : m_pRciClient( NULL ), m_sRemoteAddr( sIP ), m_wRemotePort( wPort ),m_iReadTimeout( iReadTimeout )
{
}


RecdClient::~RecdClient()
{
  Disconnect();
}

bool    RecdClient::GetDiskSize( const FString& sMountPoint, double& dTotal, double& dFree )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  bool       _bRetVal = false;
  
  if ( Connect() == false )
  {
    return _bRetVal;
  }
  
  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( sMountPoint ) );
  
  if ( m_pRciClient->Execute( _argsRes, "GET DISK SIZE", _argsParams, m_iReadTimeout ) == rciError )
  {
    Disconnect();
    
    return _bRetVal;
  }

  if ( _argsRes[0] == "OK" )
  {
    dTotal   = (double)_argsRes[1];
    dFree    = (double)_argsRes[2];
    _bRetVal = true;
  }
   
  return _bRetVal;
}

int   RecdClient::CheckDiskSpeed( const FString& sMountPoint, int iSize, double& dWriteTime, double& dReadTime )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  bool       _iRetVal = 0;
  
  if ( Connect() == false )
  {
    _iRetVal = -1;
    return _iRetVal;
  }
  
  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( sMountPoint )    );
  _argsParams.Add( new FString( 0, "%d", iSize ) );  
  
  if ( m_pRciClient->Execute( _argsRes, "CHECK DISK SPEED", _argsParams, m_iReadTimeout * iSize ) == rciError )
  {
    Disconnect();
    
    _iRetVal = -2;
    return _iRetVal;
  }

  if ( _argsRes[0] == "OK" )
  {
    dWriteTime   = (double)_argsRes[1];
    dReadTime    = (double)_argsRes[2];
  }
  else
  {
    _iRetVal   = (int)_argsRes[1];
  } 
   
  return _iRetVal;
}

bool    RecdClient::GetBuffers( bool bFlushBuffers, double& dPercentage )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  bool       _bRetVal = false;
  
  if ( Connect() == false )
  {
    return _bRetVal;
  }
  
  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( (int)bFlushBuffers ) );
  
  if ( m_pRciClient->Execute( _argsRes, "GET BUFFERS", _argsParams, m_iReadTimeout ) == rciError )
  {
    Disconnect();
    
    return _bRetVal;
  }

  if ( _argsRes[0] == "OK" )
  {
    dPercentage = (double)_argsRes[1];
    _bRetVal    = (_argsRes[2]=="1");
  }
   
  return _bRetVal;
}

int 	RecdClient::EstimateTime( double dSize, bool bRender, bool bHighlights, bool bRaw, int &iMin, int &iMax )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  int        _iRetVal = -1;
  
  if ( Connect() == false )
  {
    return _iRetVal;
  }
  
  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( 0, "%f", dSize       ) );
  _argsParams.Add( new FString( 0, "%d", bRender     ) );
  _argsParams.Add( new FString( 0, "%d", bHighlights ) );
  _argsParams.Add( new FString( 0, "%d", bRaw        ) );
  
  if ( m_pRciClient->Execute( _argsRes, "ESTIMATE TIME", _argsParams, m_iReadTimeout ) == rciError )
  {
    ERROR_INFO( FString( 0, "FAILED To Estimate TIME [%f]", dSize), EstimateTime() )

    Disconnect();
    
    return _iRetVal;
  }

  if ( _argsRes[0] == "OK" )
  {
    _iRetVal = (int)_argsRes[1];
    iMin     = (int)_argsRes[2];
    iMax     = (int)_argsRes[3];
  }

  return _iRetVal;
}

bool    RecdClient::StartRecording( const FArguments& args, bool bRender, bool bHighlights, bool bRaw )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  bool       _bRetVal = false;
  
  if ( Connect() == false )
  {
    return _bRetVal;
  }

  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( args[0].Trim()       ) );
  _argsParams.Add( new FString( args[1].Trim()       ) );
  _argsParams.Add( new FString( 0, "%d", bRender     ) );
  _argsParams.Add( new FString( 0, "%d", bHighlights ) );
  _argsParams.Add( new FString( 0, "%d", bRaw        ) );

  if ( m_pRciClient->Execute( _argsRes, "START RECORDING", _argsParams, m_iReadTimeout ) == rciError )
  {
    Disconnect();
    
    return _bRetVal;
  }

  if ( _argsRes[0] == "OK" )
    _bRetVal = true;

  return _bRetVal;
}

bool    RecdClient::StopRecording()
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  bool       _bRetVal = false;
  
  if ( Connect() == false )
  {
    return _bRetVal;
  }

  FArguments _argsRes;
  FArguments _argsParams;
  
  if ( m_pRciClient->Execute( _argsRes, "STOP RECORDING", _argsParams, m_iReadTimeout ) == rciError )
  {
    Disconnect();
    
    return _bRetVal;
  }

  if ( _argsRes[0] == "OK" )
    _bRetVal = true;

  return _bRetVal;
}

bool          RecdClient::StartHighLights( const FString& _sCamera )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  bool       _bRetVal = false;
  
  if ( Connect() == false )
  {
    return _bRetVal;
  }

  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( _sCamera ) );

  if ( m_pRciClient->Execute( _argsRes, "START HIGHLIGHTS", _argsParams, m_iReadTimeout ) == rciError )
  {
    Disconnect();
    
    return _bRetVal;
  }

  if ( _argsRes[0] == "OK" )
    _bRetVal = true;

  return _bRetVal;
}
  
bool 	RecdClient::Connect()
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  
  if ( m_pRciClient != NULL )
  {
    LOG_INFO( "Rci Client already connected.", Connect() )
    return true;
  }
  m_pRciClient = new FRciClient( 
                                new FTcpConnection( new FTcpConnectionInfo( m_sRemoteAddr, m_wRemotePort ) ),
				new RecdParser()
                               );
  if ( m_pRciClient == NULL )
  {
    ERROR_INFO( FString( 0, "FAILED Create a new istance for FRciClient [%s] [%u]", (const CHAR*)m_sRemoteAddr, m_wRemotePort ), Connect() )
    return FALSE;
  }
  
  return m_pRciClient->Open();
}

bool 	RecdClient::Disconnect()
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  
  if ( m_pRciClient == NULL )
  {
    LOG_INFO( "Rci Client is not connected.", Disconnect() )
    return false;
  }
  
  m_pRciClient->Close();
  
  delete m_pRciClient;
  m_pRciClient = NULL;
  
  return true;
}

