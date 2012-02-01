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


#include "RecdHighLightsEncoder.h"
#include "RecdReaderCollector.h"
#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

GENERATE_CLASSINFO( RecdHighLightsEncoder, FLogThread ) 

RecdHighLightsEncoder::RecdHighLightsEncoder( 
				      const FString&    sHighLightsEncoderName,
				      RecdStreamReader& rStreamReader
				    )
  : FLogThread( sHighLightsEncoderName, NULL, FThread::TP_CRITICAL, -1 ),
    m_bExit( FALSE ),
    m_bRecording( FALSE ),
    m_pAVEncoder( NULL ),
    m_rStreamReader( rStreamReader ),
    m_swHighLights( FALSE )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );
}

RecdHighLightsEncoder::~RecdHighLightsEncoder()
{

}

VOID   RecdHighLightsEncoder::StartRecording( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw )
{
  FMutexCtrl _mtxCtrl( m_mtxRecording );
  
  m_sDestination = sDestination;
  m_bRecording   = TRUE;
  m_bHighlights  = bHighlights;
  
  m_swHighLights.Invalidate();
}

VOID   RecdHighLightsEncoder::StopRecording()
{
  BOOL _bWaitClose = FALSE;
  
  m_mtxRecording.EnterMutex();
    if ( m_pAVEncoder != NULL  )
    {
      _bWaitClose = TRUE;
    }
    m_bRecording   = FALSE;
  m_mtxRecording.LeaveMutex();
  
  if ( _bWaitClose )
  {
    m_semStop.Wait();
  }
}

BOOL	RecdHighLightsEncoder::StartHighLights()
{
  FMutexCtrl _mtxCtrl( m_mtxRecording );
  
  LOG_INFO( FString( 0, "BEGIN HighLights(%s)", (const char*)m_rStreamReader.GetCameraName() ), StartHighLights() )
  m_swHighLights.Reset();
  
  return TRUE;
}

BOOL	RecdHighLightsEncoder::Initial()
{ 
  return TRUE; 
}

BOOL	RecdHighLightsEncoder::Final()
{
  m_bExit = TRUE;
  
  return TRUE;
}

VOID	RecdHighLightsEncoder::Run()
{
  double   _dFPS          = 0.03333;
  double   _dTimeSpan     = 40.0;
  CAVPoint _avRenderPos;
  CAVRect  _avRenderRect;
  CAVImage _avFrameYUV;
  
  while ( !m_bExit )
  {
    FThread::YieldThread();
    
    m_mtxRecording.EnterMutex();
    
    if ( 
	  ( m_bRecording                               == FALSE ) && 
	  ( m_rStreamReader.GetHighLightsMailboxSize() ==     0 )
       )
    {    
      //Release mutex
      m_mtxRecording.LeaveMutex();
            
      if ( m_pAVEncoder == NULL )
      {
	VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	FThread::Sleep(
			RecdConfig::GetInstance().GetEncoderStandByDelay( m_rStreamReader.GetCameraName(), NULL )
		      );
      }
      else
      {
	m_sDestination.Empty();
	m_swHighLights.Invalidate();
	
	LOG_INFO( "Close Encoder", Run() )
	if ( m_pAVEncoder->close() != eAVSucceded )
	{
	  ERROR_INFO( "Failed to close encoder.", Run() )
	}
	
	delete m_pAVEncoder;
	m_pAVEncoder = NULL;
	
	// Signal termination
	m_semStop.Post();
	FThread::YieldThread();
      }
    }
    else  
    {
      CAVFrame* _pFrame = m_rStreamReader.GetHighLightsFrame(
			  RecdConfig::GetInstance().GetEncoderReadingTimeout( m_rStreamReader.GetCameraName(), NULL ),
			  TRUE
			);
      if ( _pFrame ==  NULL ) 
      {
	VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Timeout occurs reading from mailbox", Run() )
	
	//Release mutex
	m_mtxRecording.LeaveMutex();
	continue;
      }
      
      if ( 
	   ( m_pAVEncoder   == NULL   ) &&   // Encoder must be initialized 
	   ( m_bHighlights  == TRUE   ) &&   // HighLights are enabled
	   ( m_swHighLights.IsValid() )      // StopWatch must be valid
	 )
      {
	// If the encoder is closed and we got a sample frame we will skip it
	// untile the first video frame will be readed.
	if ( _pFrame->getImage() == NULL )
	{	  
	  // Release memory.
	  m_rStreamReader.ReleaseFrame( _pFrame );

	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}
	
	
	FString _sCameraName  = m_rStreamReader.GetCameraName();
	
	FString _sOutFilename( 0, "%s/HL_%s_%010d.mp4", (const char*)m_sDestination, (const char*)_sCameraName, time(NULL) ); 
	
	m_pAVEncoder = new CAVEncoder();
	if ( m_pAVEncoder == NULL )
	{
	  ERROR_INFO( "Not Enough Memory for allocating CAVEncoder()", Run() )
	  
	  // Release memory.
	  m_rStreamReader.ReleaseFrame( _pFrame );
	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}
	
	// With and Height for the final video
	int _iWidth  = RecdConfig::GetInstance().GetHighLightsEncoderWidth ( _sCameraName, NULL );
	int _iHeight = RecdConfig::GetInstance().GetHighLightsEncoderHeight( _sCameraName, NULL );

	// Release background image if previously allocated.
	m_rgbaBkg.free();
	// Check if background has be enabled or not.
	if ( RecdConfig::GetInstance().GetHighLightsBackgroundStatus( _sCameraName, NULL ) == TRUE )
	{
	  //Loading background image.
	  FString sBkgFilename     = RecdConfig::GetInstance().GetHighLightsBackground( _sCameraName, NULL );
	  if ( m_rgbaBkg.load( (const char*)sBkgFilename, _iWidth, _iHeight, PIX_FMT_RGBA ) != eAVSucceded )
	  {
	    ERROR_INFO( FString( 0, "Error loading [%s]", (const char*)sBkgFilename ), Run() )
	  }
	
	  if (_iWidth  == -1 )
	    _iWidth  = m_rgbaBkg.getWidth();
	  
	  if (_iHeight == -1 )
	    _iHeight = m_rgbaBkg.getHeight();
	  
	  _avRenderPos.set(   
			    RecdConfig::GetInstance().GetHighLightsRectX( m_rStreamReader.GetCameraName(), NULL ), 
			    RecdConfig::GetInstance().GetHighLightsRectY( m_rStreamReader.GetCameraName(), NULL )
			  );
			  
	  _avRenderRect = CAVRect(
			    0, 
			    0, 
			    RecdConfig::GetInstance().GetHighLightsRectWidth( m_rStreamReader.GetCameraName(), NULL ), 
			    RecdConfig::GetInstance().GetHighLightsRectHeight( m_rStreamReader.GetCameraName(), NULL )
			  );
	}
	
	// If we have no skin! highlights rendering will be impossible
	if ( ( _iWidth == -1 ) && (_iHeight == -1 ) )
	{
	  ERROR_INFO( "HIGHLIGHTS Width and Height NOT DEFINED! CHECK YOUR CONFIGURATION", Run() )

	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}
	
	LOG_INFO( FString( 0, "CAM=[%s] OUT=[%s]", (const char*)_sCameraName, (const char*)_sOutFilename ), Run() )
	AVResult _avRes = m_pAVEncoder->open( 
			      _sOutFilename,
			      AV_ENCODE_VIDEO_STREAM|AV_ENCODE_AUDIO_STREAM,
			      _iWidth,
			      _iHeight,
			      RecdConfig::GetInstance().GetHighLightsEncoderFps       ( _sCameraName, NULL ),
			      RecdConfig::GetInstance().GetHighLightsEncoderGoP       ( _sCameraName, NULL ),
			      RecdConfig::GetInstance().GetHighLightsEncoderBitRate   ( _sCameraName, NULL ),
			      (CodecID)RecdConfig::GetInstance().GetHighLightsEncoderVideoCodec( _sCameraName, NULL )
			  );
	if ( _avRes != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Failed to Open Encoder [%s]", (const char*)_sOutFilename ), Run() )
	  
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
	  
	  // Release memory.
	  m_rStreamReader.ReleaseFrame( _pFrame );
	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}
	
	_dFPS          = 1.0 / (double)RecdConfig::GetInstance().GetHighLightsEncoderFps( _sCameraName, NULL );
	_dTimeSpan     = RecdConfig::GetInstance().GetHighLightsTimeSpan( _sCameraName, NULL );
      } //if ( _pAVEncoder == NULL )
      
      m_swFPS.Reset();
      
      if ( m_pAVEncoder != NULL )
      {
	AVResult eResult;
	
	// If background is valid alpha blending is required.
	if ( m_rgbaBkg.isValid() && ( _pFrame->getMediaType() == AVMEDIA_TYPE_VIDEO ) )
	{
	  // In this case _pFrame is an RGB image
	  m_rgbaBkg.blend( _avRenderPos, *_pFrame->getImage() );
	  
	  // initialize autput frame.
	  _avFrameYUV.init( m_rgbaBkg, -1, -1, PIX_FMT_YUV420P );
	  
	  eResult = m_pAVEncoder->write( &_avFrameYUV, 0 );
	}
	else
	  eResult = m_pAVEncoder->write( _pFrame, AV_INTERLEAVED_AUDIO_WR );
	
	if (  eResult != eAVSucceded )
	{
	    ERROR_INFO( "Failed to encode frame.", Run() )
	    
	    delete m_pAVEncoder;
	    m_pAVEncoder = NULL;
	}
	
	// Check if recording time is greater or equal to GetHighLightsTimeSpan()
	if ( m_swHighLights.Peek() >= _dTimeSpan )
	{
	  LOG_INFO( FString( 0, "END HighLights(%s)", (const char*)m_rStreamReader.GetCameraName() ), Run() )

	  m_swHighLights.Invalidate();
	  
	  if ( m_pAVEncoder->close() != eAVSucceded )
	  {
	    ERROR_INFO( "Failed to close encoder.", Run() )
	  }
	
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
	}
      }//if ( m_bRaw == TRUE )

      
      // Release memory.
      m_rStreamReader.ReleaseFrame( _pFrame );
      //Release mutex
      m_mtxRecording.LeaveMutex();
      
      while ( m_swFPS.Peek() < _dFPS )
	FThread::YieldThread();
      
    }// if ( m_bRecording == FALSE )
  }// while ( !m_bExit )
}

DWORD   RecdHighLightsEncoder::GetLogMessageFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsTraceExceptionEnabled  ( "HighLights Log Message", NULL)?FLogMessage::MT_TRACE_EXCEPTION:0;
  _dwRetVal |= RecdConfig::GetInstance().IsCatchExceptionEnabled  ( "HighLights Log Message", NULL)?FLogMessage::MT_CATCH_EXCEPTION:0;
  _dwRetVal |= RecdConfig::GetInstance().IsAssertionFailureEnabled( "HighLights Log Message", NULL)?FLogMessage::MT_ASSERTION_FAILURE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsErrorInfoEnabled       ( "HighLights Log Message", NULL)?FLogMessage::MT_ERROR_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLoggingInfoEnabled     ( "HighLights Log Message", NULL)?FLogMessage::MT_LOGGING_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsVerboseInfoEnabled     ( "HighLights Log Message", NULL)?FLogMessage::MT_VERBOSE_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsRawInfoEnabled         ( "HighLights Log Message", NULL)?FLogMessage::MT_RAW_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsEnterMethodEnabled     ( "HighLights Log Message", NULL)?FLogMessage::MT_ENTER_METHOD:0;
  _dwRetVal |= RecdConfig::GetInstance().IsExitMethodEnabled      ( "HighLights Log Message", NULL)?FLogMessage::MT_EXIT_METHOD:0;

  return _dwRetVal;
}


DWORD   RecdHighLightsEncoder::GetVerbosityLevelFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsStartUpMessageEnabled       ( "HighLights Verbosity Level", NULL)?FLogMessage::VL_START_UP_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsShutDownMessageEnabled      ( "HighLights Verbosity Level", NULL)?FLogMessage::VL_SHUT_DOWN_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLowPeriodicMessageEnabled   ( "HighLights Verbosity Level", NULL)?FLogMessage::VL_LOW_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsMediumPeriodicMessageEnabled( "HighLights Verbosity Level", NULL)?FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsHighPeriodicMessageEnabled  ( "HighLights Verbosity Level", NULL)?FLogMessage::VL_HIGH_PERIODIC_MESSAGE:0;

  return _dwRetVal;
}



