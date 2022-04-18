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


#include "RecdEvents.h"
#include "Recd.h"

#include "avapplication.h"
#include "RecdReaderCollector.h"
#include "RecdEncoderCollector.h"


#include "LOGGING/FLogger.h"
#include "LOGGING/FLogThread.h"
#include "LOGGING/FLogDeviceCollector.h"
#include "LOGGING/FLogMessageColorization.h"
#include "LOGGING/FTcpLogDevice.h"
#include "LOGGING/FDiskLogDevice.h"
#include "LOGGING/FConsoleLogDevice.h"

GENERATE_CLASSINFO( RecdEvents, FServiceEventInterface )

USING_NAMESPACE_FED
USING_NAMESPACE_LOGGING


RecdEvents::RecdEvents( const FService& rService, const FString& sCfgFile )
 : m_cmdServer( rService ), m_sCfgFile(sCfgFile)
{
  
}

RecdEvents::~RecdEvents()
{
  
}

BOOL RecdEvents::OnStart( const FService* pService )
ENTER( OnStart() )
  const Recd* _pRecdSrv = (const Recd*)pService;

  //////////////
  //  Initialize Recd configuration
  FTRY
  {
    RecdConfig::Initialize();
    RecdConfig::GetInstance().Load(m_sCfgFile);
  }
  FCATCH( FException, ex )
  {
    TRACE_EXCEPTION_CATCH( ex, OnStart() )
    
    printf( (const char*)ex.GetMessage() );
    printf( "\r\n" );
    
    exit(0);
  }

  ///////////////////
  // Initialize the log device collector
  FLogDeviceCollector::Initialize();

  ///////////////////
  // Create message colorozation class instance used to colorize
  // streamed messages on the consolle and on the tcp.
  const ILogMessageColorization* pLogMessageColorization  = new FLogMessageColorization();

  ///////////////////
  // Create an instace for a Log device over TCP.
  // The device will accept row tcp connection on port 10000 on each active interface.
  ILogDevice* _pLogDevice0 = new FTcpLogDevice( 
			      "TCP", 
			      RecdConfig::GetInstance().GetLogServerAddress( NULL ), // Default "127.0.0.1" 
			      RecdConfig::GetInstance().GetLogServerPort( NULL ),    // Default 55930 
			      pLogMessageColorization 
			);

  ILogDevice* _pLogDevice1 = new FDiskLogDevice( 
			      "DSK", 
			      0xFFFFFFFF,
			      RecdConfig::GetInstance().GetLogDiskPath( NULL ),
			      RecdConfig::GetInstance().GetLogDiskPrefix( NULL ),
			      RecdConfig::GetInstance().GetLogDiskExtension( NULL ),
			      RecdConfig::GetInstance().GetLogDiskFiles( NULL ),
			      RecdConfig::GetInstance().GetLogDiskFileSizeLimit( NULL )
			);
#ifdef _DEBUG 
  ILogDevice* _pLogDevice2 = new FConsoleLogDevice( "CON", pLogMessageColorization );
#endif
  ///////////////////
  // Add the new devices to the list of available devices.
  FLogDeviceCollector::GetInstance().AddDevice( _pLogDevice0 );
  FLogDeviceCollector::GetInstance().AddDevice( _pLogDevice1 );
#ifdef _DEBUG 
  FLogDeviceCollector::GetInstance().AddDevice( _pLogDevice2 );
#endif
  ///////////////////
  // Initialize the Logger
  FLogger::Initialize();

  ///////////////////
  // Register the devices for message dispatching.
  FLogger::GetInstance().RegisterDevice( "TCP" );
  FLogger::GetInstance().RegisterDevice( "DSK" );
#ifdef _DEBUG
  FLogger::GetInstance().RegisterDevice( "CON" );
#endif
  (GET_LOG_MAILBOX())->SetLogMessageFlags( _pRecdSrv->GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( _pRecdSrv->GetVerbosityLevelFlags() );

  //Initialize member variable that control the program exit
  m_bExit = FALSE;

  LOG_INFO( "Initialize Command Server", OnStart() )
  if ( m_cmdServer.Initialize() == FALSE )
  {

  }

  // Initialize LIB AV CPP
  CAVApplication::initLibAVCPP();

  // Initialize IP Camera Readers
  LOG_INFO( "Initialize IP Camera Readers", OnStart() )
  RecdReaderCollector::Initialize();
  
  // Initialze Camera Encoders
  LOG_INFO( "Initialize IP Camera Readers", OnStart() )
  RecdEncoderCollector::Initialize();
  
  return TRUE;
EXIT

VOID RecdEvents::OnRun(const FService* pService)
ENTER( OnRun() )

  Recd* _pService =  (Recd*)pService;

  while ( !m_bExit )
  {
    LOG_INFO( "Run ..", OnRun() )

    // Wait for number of seconds in configuration
    FThread::Sleep( 10000 );
  }//while ( !m_bExit )

EXIT

VOID RecdEvents::OnStop(const FService* pService)
ENTER( OnStop() )

  LOG_INFO( "Finalize Command Server", OnStop() )
  if ( m_cmdServer.Finalize() == FALSE )
  {

  }

  // Finalize IP Camera Readers
  LOG_INFO( "Finalize IP Camera Readers", OnStop() )
  RecdReaderCollector::GetInstance().Finalize();

  // Deinitialize LIB AV CPP
  CAVApplication::deinitLibAVCPP();
  
  //
  m_bExit = TRUE;
EXIT

VOID RecdEvents::OnInterrogate(const FService* pService)
ENTER( OnInterrogate() )

EXIT

VOID RecdEvents::OnPause(const FService* pService)
ENTER( OnPause() )

EXIT

VOID RecdEvents::OnContinue(const FService* pService)
ENTER( OnContinue() )

EXIT

VOID RecdEvents::OnShutdown(const FService* pService)
ENTER( OnShutdown() )

EXIT

VOID RecdEvents::OnNetBindAdd(const FService* pService)
ENTER( OnNetBindAdd() )

EXIT

VOID RecdEvents::OnNetBindDisable(const FService* pService)
ENTER( OnNetBindDisable() )

EXIT

VOID RecdEvents::OnNetBindEnable(const FService* pService)
ENTER( OnNetBindEnable() )

EXIT

VOID RecdEvents::OnNetBindRemove(const FService* pService)
ENTER( OnNetBindRemove() )

EXIT

VOID RecdEvents::OnParamChange(const FService* pService)
ENTER( OnParamChange() )

EXIT

BOOL RecdEvents::OnUserControl(const FService* pService, DWORD dwOpcode)
ENTER( OnUserControl() )
  return TRUE;
EXIT

