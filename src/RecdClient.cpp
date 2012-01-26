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

double 	RecdClient::EstimateTime( double dSize, bool bRender, bool bHighlights, bool bRaw )
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  double     _dRetVal = -1.0;
  
  if ( Connect() == false )
  {
    return _dRetVal;
  }
  
  FArguments _argsRes;
  FArguments _argsParams;
  
  _argsParams.Add( new FString( 0, "%f", dSize       ) );
  _argsParams.Add( new FString( 0, "%d", bRender     ) );
  _argsParams.Add( new FString( 0, "%d", bHighlights ) );
  _argsParams.Add( new FString( 0, "%d", bRaw        ) );
  
  if ( m_pRciClient->Execute( _argsRes, "ESTIMATE TIME", _argsParams, m_iReadTimeout ) == rciError )
  {
    Disconnect();
    
    return _dRetVal;
  }

  if ( _argsRes[0] == "OK" )
  {
    _dRetVal = (double)_argsRes[1];
  }

  return _dRetVal;
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
    return true;
  
  m_pRciClient = new FRciClient( 
                                new FTcpConnection( new FTcpConnectionInfo( m_sRemoteAddr, m_wRemotePort ) ),
				new RecdParser()
                               );
  if ( m_pRciClient == NULL )
    return FALSE;
  
  return m_pRciClient->Open();
}

bool 	RecdClient::Disconnect()
{
  FMutexCtrl _mtxCtrl( m_mtxClient );
  
  if ( m_pRciClient == NULL )
    return false;
  
  m_pRciClient->Close();
  
  delete m_pRciClient;
  m_pRciClient = NULL;
  
  return true;
}

