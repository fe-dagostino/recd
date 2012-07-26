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


#include "RecdCmdServer.h"
#include "RecdConfig.h"
#include "RecdChannelFactory.h"

#include "FTcpConnectionListener.h"
#include "FTcpConnectionFactory.h"

#include "RCI/FRciCommandCollector.h"

#include "LOGGING/FLogger.h"

#include "COMMANDS/RecdList.h"
#include "COMMANDS/RecdHelp.h"
#include "COMMANDS/RecdGetVersion.h"
#include "COMMANDS/RecdStartRecording.h"
#include "COMMANDS/RecdStopRecording.h"
#include "COMMANDS/RecdStartHighLights.h"
#include "COMMANDS/RecdGetDiskSize.h"
#include "COMMANDS/RecdGetBuffers.h"
#include "COMMANDS/RecdEstimateTime.h"


USING_NAMESPACE_LOGGING

GENERATE_CLASSINFO( RecdCmdServer, FObject )


RecdCmdServer::RecdCmdServer( const FService& rService )
  : m_rService( rService )
{

}

RecdCmdServer::~RecdCmdServer()
{

}

BOOL RecdCmdServer::Initialize()
{
  // Register RCI commands.
  m_rciServer.GetCommandCollector().Register( new RecdList      ( m_rciServer )    );
  m_rciServer.GetCommandCollector().Register( new RecdHelp      ( m_rciServer )    );
  m_rciServer.GetCommandCollector().Register( new RecdGetVersion( m_rService  )    );
  m_rciServer.GetCommandCollector().Register( new RecdGetDiskSize()                );
  m_rciServer.GetCommandCollector().Register( new RecdGetBuffers()                 );
  m_rciServer.GetCommandCollector().Register( new RecdEstimateTime()               );
  m_rciServer.GetCommandCollector().Register( new RecdStartRecording()             );
  m_rciServer.GetCommandCollector().Register( new RecdStopRecording()              );
  m_rciServer.GetCommandCollector().Register( new RecdStartHighLights()            );
    
  IConnectionFactory*  _pIConnectionFactory	= new FTcpConnectionFactory();
  if ( _pIConnectionFactory == NULL )
  {
    ERROR_INFO( "Tcp Connection Factory is NULL", Initialize() )
    return FALSE;
  }

  IConnectionListener* _pConnectionListener	= new FTcpConnectionListener( 
							      _pIConnectionFactory, 
							      RecdConfig::GetInstance().GetCmdServerAddress( NULL ), 
							      RecdConfig::GetInstance().GetCmdServerPort   ( NULL ), 
							      TRUE 
						      );
  if ( _pIConnectionFactory == NULL )
  {
    ERROR_INFO( "Connection Factory is NULL", Initialize() )
    return FALSE;
  }

  RecdChannelFactory*  _pChannelFactory		= new RecdChannelFactory();
  if ( _pChannelFactory == NULL )
  {
    ERROR_INFO( "Channel Factory is NULL", Initialize() )
    return FALSE;
  }

  m_rciServer.AddListener( _pConnectionListener, _pChannelFactory );

  return TRUE;
}

BOOL RecdCmdServer::Finalize()
{
  return TRUE;
}
