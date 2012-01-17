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


#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_LOGGING

GENERATE_CLASSINFO( RecdConfig, FSingleton )
IMPLEMENT_SINGLETON( RecdConfig )

#ifdef _WIN32
# define CFG_FILENAME  "recd.cfg"
#else
# define CFG_FILENAME  "/etc/recd/recd.cfg"
#endif


/////////////////////////////////////
  //
   //
    ////////////////////////////////////////////////////////////////////////////////////////////
   //
  //		Disable warning 4715 : "The specified function can potentially not return a value."
 //
//////////
#pragma warning( disable : 4715 )

/////////////////
////	IP DEVICES
//////

FParameter*    RecdConfig::GetIpCameras( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetParameter( "IP CAMERAS", "DEVICES", pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetIpCameras() );
  }

  //Return default value
  return NULL;
}

DOUBLE 	RecdConfig::GetReaderBufferingTime( BOOL* pbStored ) const
{
  FTRY
  {
    return (DOUBLE)m_cfg.GetValue( "IP CAMERAS", "BUFFERING TIME", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderBufferingTime() );
  }

  //Return default value
  return 40.0;
}

DOUBLE 	RecdConfig::GetReaderStartDelayTime( BOOL* pbStored ) const
{
  FTRY
  {
    return (DOUBLE)m_cfg.GetValue( "IP CAMERAS", "START STOP DELAY", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderStartDelayTime() );
  }
  
  //Return default value
  return 0.0;
}

DOUBLE 	RecdConfig::GetReaderStopDelayTime( BOOL* pbStored ) const
{
  FTRY
  {
    return (DOUBLE)m_cfg.GetValue( "IP CAMERAS", "START STOP DELAY", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderStopDelayTime() );
  }
  
  //Return default value
  return 40.0;
}

FString	RecdConfig::GetReaderStream( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sIPCamera, "URL", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderStream() );
  }

  //Return default value
  return FString();
}

DWORD RecdConfig::GetReaderMaxItems( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "READER SETTINGS", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderMaxItems() );
  }

  //Return default value
  return 5;
}

DWORD RecdConfig::GetReaderStandByDelay( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "READER SETTINGS", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderStandByDelay() );
  }

  //Return default value
  return 30;
}

INT   RecdConfig::GetReaderRescaleOptions( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "READER SETTINGS", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetReaderRescaleOptions() );
  }

  //Return default value
  return 1;
}

DWORD RecdConfig::GetEncoderMaxItems( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "ENCODER SETTINGS", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderMaxItems() );
  }

  //Return default value
  return 5;
}


DWORD RecdConfig::GetEncoderStandByDelay( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "ENCODER SETTINGS", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderStandByDelay() );
  }

  //Return default value
  return 30;
}

DWORD RecdConfig::GetEncoderReadingTimeout( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "ENCODER SETTINGS", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderReadingTimeout() );
  }

  //Return default value
  return 30;
}

DWORD RecdConfig::GetEncoderRescaleOptions( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( sIPCamera, "ENCODER SETTINGS", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderRescaleOptions() );
  }

  //Return default value
  return 1;
}

INT RecdConfig::GetEncoderWidth( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "ENCODER VIDEO SETTINGS", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderWidth() );
  }

  //Return default value
  return -1;
}

INT RecdConfig::GetEncoderHeight( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "ENCODER VIDEO SETTINGS", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderHeight() );
  }

  //Return default value
  return -1;
}

INT	RecdConfig::GetEncoderFps( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "ENCODER VIDEO SETTINGS", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderFps() );
  }

  //Return default value
  return 30;
}

INT	RecdConfig::GetEncoderGoP( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "ENCODER VIDEO SETTINGS", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderGoP() );
  }

  //Return default value
  return 10;
}

INT	RecdConfig::GetEncoderBitRate( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "ENCODER VIDEO SETTINGS", 4, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderBitRate() );
  }

  //Return default value
  return 4000000;
}

INT	RecdConfig::GetEncoderVideoCodec( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "ENCODER VIDEO SETTINGS", 5, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetEncoderVideoCodec() );
  }

  //Return default value
  return 13;
}

DOUBLE RecdConfig::GetHighLightsTimeSpan( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (DOUBLE)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS SETTING", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsTimeSpan() );
  }

  //Return default value
  return 40.0;
}

INT RecdConfig::GetHighLightsEncoderWidth( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS VIDEO SETTINGS", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsEncoderWidth() );
  }

  //Return default value
  return -1;
}

INT RecdConfig::GetHighLightsEncoderHeight( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS VIDEO SETTINGS", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsEncoderHeight() );
  }

  //Return default value
  return -1;
}

INT	RecdConfig::GetHighLightsEncoderFps( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS VIDEO SETTINGS", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsEncoderFps() );
  }

  //Return default value
  return 30;
}

INT	RecdConfig::GetHighLightsEncoderGoP( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS VIDEO SETTINGS", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsEncoderGoP() );
  }

  //Return default value
  return 10;
}

INT	RecdConfig::GetHighLightsEncoderBitRate( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS VIDEO SETTINGS", 4, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsEncoderBitRate() );
  }

  //Return default value
  return 8000000;
}

INT	RecdConfig::GetHighLightsEncoderVideoCodec( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS VIDEO SETTINGS", 5, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsEncoderVideoCodec() );
  }

  //Return default value
  return 13;
}

BOOL  RecdConfig::GetHighLightsBackgroundStatus( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return ((INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS BACKGROUND", 0, pbStored )==1);
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsBackground() );
  }
  
  return FALSE;
}

FString  RecdConfig::GetHighLightsBackground( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sIPCamera, "HIGHLIGHTS BACKGROUND", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsBackground() );
  }
  
  return "/etc/recd/default-skin-highlight.png";
}

INT 	RecdConfig::GetHighLightsRectX( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS RECT", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsRectX() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetHighLightsRectY( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS RECT", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsRectY() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetHighLightsRectWidth( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS RECT", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsRectWidth() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetHighLightsRectHeight( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "HIGHLIGHTS RECT", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetHighLightsRectHeight() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetRenderRectX( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "RENDER RECT", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderRectX() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetRenderRectY( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "RENDER RECT", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderRectY() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetRenderRectWidth( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "RENDER RECT", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderRectWidth() );
  }

  //Return default value
  return 0;
}

INT 	RecdConfig::GetRenderRectHeight( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return (INT)m_cfg.GetValue( sIPCamera, "RENDER RECT", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderRectHeight() );
  }

  //Return default value
  return 0;
}

CAVColor RecdConfig::GetRenderKeyColor( const FString& sIPCamera, BOOL* pbStored ) const
{
  FTRY
  {
    return CAVColor( 
		      (INT)m_cfg.GetValue( sIPCamera, "KEY COLOR", 0, pbStored ),
		      (INT)m_cfg.GetValue( sIPCamera, "KEY COLOR", 1, pbStored ),
		      (INT)m_cfg.GetValue( sIPCamera, "KEY COLOR", 2, pbStored ),
		      255
		   );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderKeyColor() );
  }

  //Return default value
  return CAVColor();
}


/////////////////////////////
// RENDER SECTION 
///////////////////////
  
DWORD 	RecdConfig::GetRenderStandByDelay( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "SETTINGS", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderStandByDelay() );
  }

  //Return default value
  return 30;
}

DWORD 	RecdConfig::GetRenderReadingTimeout( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "SETTINGS", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderReadingTimeout() );
  }

  //Return default value
  return 30;
}

FString  RecdConfig::GetRenderBackground( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "RENDER", "BACKGROUND", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderBackground() );
  }
  
  return "/etc/recd/default-skin-alpha.png";
}

FString RecdConfig::GetRenderBackgroundMask( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "RENDER", "BACKGROUND", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderBackground() );
  }
  
  return "/etc/recd/default-skin-chroma-keys.png";
}

INT 	RecdConfig::GetRenderWidth( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "VIDEO SETTINGS", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderWidth() );
  }

  //Return default value
  return -1;
}

INT	RecdConfig::GetRenderHeight( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "VIDEO SETTINGS", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderHeight() );
  }

  //Return default value
  return -1;
}

INT	RecdConfig::GetRenderFps( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "VIDEO SETTINGS", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderFps() );
  }

  //Return default value
  return 30;
}

INT	RecdConfig::GetRenderGoP(BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "VIDEO SETTINGS", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderGoP() );
  }

  //Return default value
  return 10;
}

INT	RecdConfig::GetRenderBitRate( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "VIDEO SETTINGS", 4, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderBitRate() );
  }

  //Return default value
  return 4000000;
}

INT	RecdConfig::GetRenderVideoCodec( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "RENDER", "VIDEO SETTINGS", 5, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetRenderVideoCodec() );
  }

  //Return default value
  return 13;
}


/////////////////
////	GENERAL
//////

FString	RecdConfig::GetLogDiskPath( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "GENERAL", "LOG_DISK", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogDiskPath() );
  }

  //Return default value
  return "/var/log/recd/";
}

FString	RecdConfig::GetLogDiskPrefix( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "GENERAL", "LOG_DISK", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogDiskPrefix() );
  }

  //Return default value
  return "recd_";
}

FString	RecdConfig::GetLogDiskExtension( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "GENERAL", "LOG_DISK", 2, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogDiskExtension() );
  }

  //Return default value
  return "log";
}

DWORD	RecdConfig::GetLogDiskFiles( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "GENERAL", "LOG_DISK", 3, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogDiskFiles() );
  }

  //Return default value
  return 10;
}

DWORD	RecdConfig::GetLogDiskFileSizeLimit( BOOL* pbStored ) const
{
  FTRY
  {
    return (DWORD)m_cfg.GetValue( "GENERAL", "LOG_DISK", 4, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogDiskFileSizeLimit() );
  }

  //Return default value
  return 5000;  
}

FString		RecdConfig::GetLogServerAddress( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "GENERAL", "LOG_SERVER", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogServerAddress() );
  }

  //Return default value
  return "127.0.0.1";
}

WORD		RecdConfig::GetLogServerPort( BOOL* pbStored ) const
{
  FTRY
  {
    return (WORD)m_cfg.GetValue( "GENERAL", "LOG_SERVER", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetLogServerPort() );
  }

  //Return default value
  return 55930;
}

FString		RecdConfig::GetCmdServerAddress( BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( "GENERAL", "CMD_SERVER", 0, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetCmdServerAddress() );
  }

  //Return default value
  return "127.0.0.1";
}

WORD	RecdConfig::GetCmdServerPort( BOOL* pbStored ) const
{
  FTRY
  {
    return (WORD)m_cfg.GetValue( "GENERAL", "CMD_SERVER", 1, pbStored );
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, GetCmdServerPort() );
  }

  //Return default value
  return 31280;
}


////////////////////////////////////////

VOID     RecdConfig::OnInitialize()
{
  m_cfg.SetFileName( CFG_FILENAME );

  FTRY
  {
    m_cfg.LoadFile();
  }
  FCATCH( FConfigFileException, ex )
  {
    switch( ex.GetErrorCode() )
    {
	case FConfigFileException::CFG_FILE_NOT_FOUND :
	{
	  THROW_MSG_EXCEPTION( RecdConfigException, FString( 0,"  Filed to open [%s]", (const char*)m_cfg.GetFileName() ), RecdConfigException::CFG_FILE_NOT_FOUND_OR_INACCESSIBLE
  , OnInitialize()  )
	}; break;
    }
  }
}

VOID	RecdConfig::OnFinalize()
{

}

BOOL	RecdConfig::IsTraceExceptionEnabled( const FString& sSection, BOOL* pbStored ) const
{
	FTRY
	{
		return m_cfg.GetValue( sSection,"MT_TRACE_EXCEPTION", 0, pbStored ) == "ENABLED";
	}
	FCATCH( FConfigFileException, fexception )
	{
		TRACE_EXCEPTION_CATCH( fexception, IsTraceExceptionEnabled() );
	}
	return TRUE;
}

VOID	RecdConfig::SetTraceExceptionStatus( const FString& sSection, BOOL bStatus )
ENTER( SetTraceExceptionStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"MT_TRACE_EXCEPTION", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_TRACE_EXCEPTION );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_TRACE_EXCEPTION );
    }
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetTraceExceptionStatus() );
  }
EXIT

BOOL	RecdConfig::IsCatchExceptionEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_CATCH_EXCEPTION", 0, pbStored  ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsCatchExceptionEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetCatchExceptionStatus( const FString& sSection, BOOL bStatus )
ENTER( SetCatchExceptionStatus() )
    FTRY
    {
      m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection, "MT_CATCH_EXCEPTION", 0 );
      if ( GET_LOG_MAILBOX() )
      {
	if ( bStatus )
	  (GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_CATCH_EXCEPTION );
	else
	  (GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_CATCH_EXCEPTION );
      }
    }
    FCATCH( FConfigFileException, fexception  )
    {
      TRACE_EXCEPTION_CATCH( fexception, SetCatchExceptionStatus() );
    }
EXIT

BOOL	RecdConfig::IsAssertionFailureEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_ASSERTION_FAILURE", 0, pbStored  ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsAssertionFailureEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetAssertionFailureStatus( const FString& sSection, BOOL bStatus ) 
ENTER( SetAssertionFailureStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"MT_ASSERTION_FAILURE", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_ASSERTION_FAILURE );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_ASSERTION_FAILURE );
    }
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetAssertionFailureStatus() );
  }
EXIT

BOOL	RecdConfig::IsErrorInfoEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_ERROR_INFO", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsErrorInfoEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetErrorInfoStatus( const FString& sSection, BOOL bStatus )
ENTER( SetErrorInfoStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection, "MT_ERROR_INFO", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_ERROR_INFO );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_ERROR_INFO );
    }
  }
  FCATCH( FConfigFileException, fexception )
  {
  TRACE_EXCEPTION_CATCH( fexception, SetErrorInfoStatus() );
  }
EXIT

BOOL	RecdConfig::IsLoggingInfoEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_LOGGING_INFO", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsLoggingInfoEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetLoggingInfoStatus( const FString& sSection, BOOL bStatus )
ENTER( SetLoggingInfoStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection, "MT_LOGGING_INFO", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_LOGGING_INFO );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_LOGGING_INFO );
    }
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetLoggingInfoStatus() );
  }
EXIT

BOOL	RecdConfig::IsVerboseInfoEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_VERBOSE_INFO", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsVerboseInfoEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetVerboseInfoStatus( const FString& sSection, BOOL bStatus )
ENTER( SetVerboseInfoStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection, "MT_VERBOSE_INFO", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_VERBOSE_INFO );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_VERBOSE_INFO );
    }
  }
  FCATCH( FConfigFileException, fexception )
  {
  TRACE_EXCEPTION_CATCH( fexception, SetVerboseInfoStatus() );
  }
EXIT

BOOL	RecdConfig::IsRawInfoEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_RAW_INFO", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsRawInfoEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetRawInfoStatus( const FString& sSection, BOOL bStatus )
ENTER( SetRawInfoStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"MT_RAW_INFO", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_RAW_INFO );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_RAW_INFO );
    }
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetRawInfoStatus() );
  }
EXIT

BOOL	RecdConfig::IsEnterMethodEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "MT_ENTER_METHOD", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsEnterMethodEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetEnterMethodStatus( const FString& sSection, BOOL bStatus )
ENTER( SetEnterMethodStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection, "MT_ENTER_METHOD", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_ENTER_METHOD );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_ENTER_METHOD );
    }
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetEnterMethodStatus() );
  }
EXIT

BOOL	RecdConfig::IsExitMethodEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection,"MT_EXIT_METHOD", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsExitMethodEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetExitMethodStatus( const FString& sSection, BOOL bStatus )
ENTER( SetExitMethodStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"MT_EXIT_METHOD", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddLogMessageFlag( FLogMessage::MT_EXIT_METHOD );
      else
	(GET_LOG_MAILBOX())->SubLogMessageFlag( FLogMessage::MT_EXIT_METHOD );
    }
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetExitMethodStatus() );
  }
EXIT

BOOL	RecdConfig::IsStartUpMessageEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "VL_START_UP_MESSAGE", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsStartUpMessageEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetStartUpMessageStatus( const FString& sSection, BOOL bStatus )
ENTER( SetStartUpMessageStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"VL_START_UP_MESSAGE", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddVerbosityFlag( FLogMessage::VL_START_UP_MESSAGE );
      else
	(GET_LOG_MAILBOX())->SubVerbosityFlag( FLogMessage::VL_START_UP_MESSAGE );
    }
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetStartUpMessageStatus() );
  }
EXIT

BOOL	RecdConfig::IsShutDownMessageEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection,"VL_SHUT_DOWN_MESSAGE", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsShutDownMessageEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetShutDownMessageStatus( const FString& sSection, BOOL bStatus )
ENTER( SetShutDownMessageStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"VL_SHUT_DOWN_MESSAGE", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddVerbosityFlag( FLogMessage::VL_SHUT_DOWN_MESSAGE );
      else
	(GET_LOG_MAILBOX())->SubVerbosityFlag( FLogMessage::VL_SHUT_DOWN_MESSAGE );
    }
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetShutDownMessageStatus() );
  }
EXIT

BOOL	RecdConfig::IsLowPeriodicMessageEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection, "VL_LOW_PERIODIC_MESSAGE", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsLowPeriodicMessageEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetLowPeriodicMessageStatus( const FString& sSection, BOOL bStatus )
ENTER( SetLowPeriodicMessageStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"VL_LOW_PERIODIC_MESSAGE", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddVerbosityFlag( FLogMessage::VL_LOW_PERIODIC_MESSAGE );
      else
	(GET_LOG_MAILBOX())->SubVerbosityFlag( FLogMessage::VL_LOW_PERIODIC_MESSAGE );
    }
  }
  FCATCH( FConfigFileException, fexception )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetLowPeriodicMessageStatus() );
  }
EXIT

BOOL	RecdConfig::IsMediumPeriodicMessageEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection,"VL_MEDIUM_PERIODIC_MESSAGE", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsMediumPeriodicMessageEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetMediumPeriodicMessageStatus( const FString& sSection, BOOL bStatus )
ENTER( SetMediumPeriodicMessageStatus() )
    FTRY
    {
      m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"VL_MEDIUM_PERIODIC_MESSAGE", 0 );
      if ( GET_LOG_MAILBOX() )
      {
	if ( bStatus )
	  (GET_LOG_MAILBOX())->AddVerbosityFlag( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE );
	else
	  (GET_LOG_MAILBOX())->SubVerbosityFlag( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE );
      }
    }
    FCATCH( FConfigFileException, fexception )
    {
      TRACE_EXCEPTION_CATCH( fexception, SetMediumPeriodicMessageStatus() );
    }
EXIT

BOOL	RecdConfig::IsHighPeriodicMessageEnabled( const FString& sSection, BOOL* pbStored ) const
{
  FTRY
  {
    return m_cfg.GetValue( sSection,"VL_HIGH_PERIODIC_MESSAGE", 0, pbStored ) == "ENABLED";
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, IsHighPeriodicMessageEnabled() );
  }
  return TRUE;
}

VOID	RecdConfig::SetHighPeriodicMessageStatus( const FString& sSection, BOOL bStatus )
ENTER( SetHighPeriodicMessageStatus() )
  FTRY
  {
    m_cfg.SetValue( (bStatus)?"ENABLED":"DISABLED", sSection,"VL_HIGH_PERIODIC_MESSAGE", 0 );
    if ( GET_LOG_MAILBOX() )
    {
      if ( bStatus )
	(GET_LOG_MAILBOX())->AddVerbosityFlag( FLogMessage::VL_HIGH_PERIODIC_MESSAGE );
      else
	(GET_LOG_MAILBOX())->SubVerbosityFlag( FLogMessage::VL_HIGH_PERIODIC_MESSAGE );
    }
  }
  FCATCH( FConfigFileException, fexception  )
  {
    TRACE_EXCEPTION_CATCH( fexception, SetHighPeriodicMessageStatus() );
  }
EXIT

