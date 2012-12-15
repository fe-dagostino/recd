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
    m_eEncoderStatus( eHSUndefined ),
    m_pAVEncoder( NULL ),
    m_rStreamReader( rStreamReader ),
    m_swHighLights( FALSE )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );
  
  m_pVideoItems = new FTQueue<RecdMbxItem* >();
  if ( m_pVideoItems == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Queue", RecdStreamReader() )
    //@todo
  }
}

RecdHighLightsEncoder::~RecdHighLightsEncoder()
{
  if ( m_pVideoItems != NULL )
  {
    // Release mailbox.
    delete m_pVideoItems;
    m_pVideoItems = NULL;
  }
}

VOID   RecdHighLightsEncoder::SetParameters( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw )
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );
  
  m_sDestination = sDestination;
  m_bHighlights  = bHighlights;
  
  m_swHighLights.Invalidate();
}

BOOL	RecdHighLightsEncoder::StartHighLight()
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );
  
  if ( m_bHighlights == FALSE )
    return FALSE;

  if ( GetStatus( NULL, NULL ) == eHSWaiting )
  {
    LOG_INFO( FString( 0, "BEGIN HighLight(%s)", (const char*)m_rStreamReader.GetCameraName() ), StartHighLights() )
    m_rStreamReader.PushHighLightsItem( new RecdMbxItem( RecdMbxItem::eCmdStartEncoding ) );
  }
  else
  {
    LOG_INFO( FString( 0, "CONTINUE HighLight(%s)", (const char*)m_rStreamReader.GetCameraName() ), StartHighLights() )
    m_swHighLights.Reset();
  }
  
  return TRUE;
}

enum RecdHighLightsEncoder::HighLightsEncoderStatus RecdHighLightsEncoder::GetStatus( DOUBLE* pdElapsed, DOUBLE* pdTotal ) const
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );

  if ( pdElapsed != NULL )
  {
    *pdElapsed = 0.0;
    
    *pdElapsed = m_swHighLights.Peek();
    if ( *pdElapsed > m_dHighLightDuration )
      *pdElapsed = m_dHighLightDuration;
    
    if ( pdTotal != NULL )
    {
      *pdTotal = m_dHighLightDuration;
    }
  }
  
  return m_eEncoderStatus;
}

bool                         RecdHighLightsEncoder::SetStatus( HighLightsEncoderStatus eStatus )
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );
  
  m_eEncoderStatus = eStatus;
  
  return true;
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
  double     _dFPS              = 0.03333;
  double     _dFpsCountDown     = 0;   // used to skip old frames in queue
  double     _dFpsError         = 0.0; 
  DWORD      _dwEncoderMaxItems = RecdConfig::GetInstance().GetEncoderMaxItems( m_rStreamReader.GetCameraName(), NULL );
  DWORD      _dwReadingTimeout  = RecdConfig::GetInstance().GetEncoderReadingTimeout( m_rStreamReader.GetCameraName(), NULL );
  DWORD      _dwStandbyTimeout  = RecdConfig::GetInstance().GetEncoderStandByDelay( m_rStreamReader.GetCameraName(), NULL );
  FStopWatch _swFpsCount;
  DWORD      _dwFpsCount = 0;
  CAVPoint   _avRenderPos;
  CAVRect    _avRenderRect;
  CAVImage   _avFrameYUV;
  
  // Update status to waiting mode.
  SetStatus( eHSWaiting );
  
  while ( !m_bExit )
  {
    FThread::YieldThread();
    
    switch ( GetStatus( NULL, NULL ) )
    {
      case eHSWaiting:
      {
	RecdMbxItem* _pMbxItem = m_rStreamReader.PopHighLightsItem( _dwReadingTimeout, TRUE );	
	if ( _pMbxItem != NULL )
	{
	  if ( _pMbxItem->GetType() == RecdMbxItem::eCommandItem ) 
	  {
	    if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStartEncoding )
	      SetStatus( eHSOpenStream );
	  }

	  // Release memory.
	  m_rStreamReader.ReleaseMbxItem( _pMbxItem );
	}
	else
	{
	  VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	  FThread::Sleep( _dwStandbyTimeout );
	}
      }; break;
      
      case eHSOpenStream:
      {
	FString   _sCameraName  = m_rStreamReader.GetCameraName();
	FDateTime _dtNow( TRUE, TRUE );
	FString   _sOutFilename( 0, "%s/HL_%s_%s.mp4", (const char*)m_sDestination, (const char*)_sCameraName, (const char*)_dtNow.GetDateTime( "%A%-%M%-%G%_%h%%m%%s%" ) ); 
	
	m_pAVEncoder = new CAVEncoder();
	if ( m_pAVEncoder == NULL )
	{
	  ERROR_INFO( "Not Enough Memory for allocating CAVEncoder()", Run() )
	  break;
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
	if ( ( _iWidth == -1 ) || (_iHeight == -1 ) )
	{
	  ERROR_INFO( "HIGHLIGHTS Width and/or Height NOT DEFINED! CHECK YOUR CONFIGURATION", Run() )

	  break;
	}
	
	LOG_INFO( FString( 0, "CAM=[%s] OUT=[%s]", (const char*)_sCameraName, (const char*)_sOutFilename ), Run() )
	AVResult _avRes = m_pAVEncoder->open( 
			      _sOutFilename,
			      AV_ENCODE_VIDEO_STREAM|AV_ENCODE_AUDIO_STREAM,
			      _iWidth,
			      _iHeight,
			      PIX_FMT_YUV420P,
			      RecdConfig::GetInstance().GetHighLightsEncoderFps       ( _sCameraName, NULL ),
			      RecdConfig::GetInstance().GetHighLightsEncoderGoP       ( _sCameraName, NULL ),
			      RecdConfig::GetInstance().GetHighLightsEncoderBitRate   ( _sCameraName, NULL ),
			      (CodecID)RecdConfig::GetInstance().GetHighLightsEncoderVideoCodec( _sCameraName, NULL ),
			      RecdConfig::GetInstance().GetHighLightsEncoderVideoCodecProfile( _sCameraName, NULL )
			  );
	if ( _avRes != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Failed to Open Encoder [%s]", (const char*)_sOutFilename ), Run() )
	  
	  SetStatus( eHSReleasing );
	  break;
	}
	
	_dFPS                 = 1.0 / (double)RecdConfig::GetInstance().GetHighLightsEncoderFps( _sCameraName, NULL );
	_dFpsCountDown        = RecdConfig::GetInstance().GetHighLightsEncoderFps( _sCameraName, NULL );
	_dFpsError            = 0.0;
	m_dHighLightDuration  = RecdConfig::GetInstance().GetHighLightsTimeSpan( _sCameraName, NULL );

	// Invalidate fps stop watch
	m_swFPS.Invalidate();

	// Reset stop watch in order to recovery time lost opening the encoder.
	m_mtxEncoder.EnterMutex();
	 m_swHighLights.Reset();
	m_mtxEncoder.LeaveMutex();
	
	// Reset variables for avg fps counter
	_swFpsCount.Reset();
	_dwFpsCount = 0;
	
	// Update status to encoding
	SetStatus( eHSEncoding );
      }; break;
      
      case eHSEncoding:
      {
	RecdMbxItem* _pMbxItem = NULL;
	
	_dwReadingTimeout = (_dFpsError>=_dFPS)?0:RecdConfig::GetInstance().GetEncoderReadingTimeout( m_rStreamReader.GetCameraName(), NULL );
	
	_pMbxItem = m_rStreamReader.PopHighLightsItem( _dwReadingTimeout, TRUE );
	if ( _pMbxItem == NULL )   
	{
	  if ( m_pVideoItems->IsEmpty() )
	  {
	    VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Timeout occurs reading from mailbox", Run() )
	    continue;
	  }
	  else
	  {
	    _pMbxItem = m_pVideoItems->Pop( false );
	  }
	}

	switch ( _pMbxItem->GetType() )
	{
	  
	  case RecdMbxItem::eCommandItem:
	  {	    
	    if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStopEncoding )
	    {
	      SetStatus( eHSReleasing );
	    }
	  }; break;
	  
	  case RecdMbxItem::eImageItem:
	  {
	    // It is necessary to check if queue is empty in order to 
	    // avoid FTQueue exception raising.
	    // Checking that current frame is a new frame and do not
	    // match with first element in queue.
	    if (
	        ( m_pVideoItems->IsEmpty()                 ) ||
		( _pMbxItem != m_pVideoItems->Pop( FALSE ) )
	       )
	    {
	      m_pVideoItems->Push( _pMbxItem );
	    }
	    
	    // Just in case enqueued frames is bigger than max
	    // we will drop the older frame and then the one 
	    // after will be processed.
	    if ( m_pVideoItems->GetSize() > _dwEncoderMaxItems )
	    {
	      _pMbxItem = m_pVideoItems->Pop();
	      
	      // Release frame.
	      m_rStreamReader.ReleaseMbxItem( _pMbxItem );
	      
	      VERBOSE_INFO( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE, "Drop Current Frame ..", Run() )
	    }
	    
	    if (
		( m_swFPS.IsValid() == FALSE ) ||
		( m_swFPS.Peek()    >= _dFPS ) 
	      )
	    {
	      if ( m_swFPS.IsValid() )
	      {
		// Cumulative error
		_dFpsError += m_swFPS.Peek() - _dFPS;
	      }
	      
	      m_swFPS.Reset();
	      
	      _pMbxItem = m_pVideoItems->Pop();
	    }
	    else if ( _dFpsError >= _dFPS )
	    {
	      _dFpsError -= _dFPS;
	      
	      _pMbxItem = m_pVideoItems->Pop();
	    }  
	    else // move to next loop
	    {
	      continue;
	    }


	    // Used to drop old frames enqueued during output streaming opening operation.
	    if ( _dFpsCountDown > 0 )
	    {
	      _dFpsCountDown -= 1;
	      
	      m_swHighLights.Reset();
	    }
	    else
	    {
	      AVResult eResult;

	      // If background is valid alpha blending is required.
	      if ( m_rgbaBkg.isValid() )
	      {
		// In this case _pFrame is an RGB image
		m_rgbaBkg.blend( _avRenderPos, *_pMbxItem->GetImage() );
		
		// initialize autput frame.
		_avFrameYUV.init( m_rgbaBkg, -1, -1, PIX_FMT_YUV420P );
		
		eResult = m_pAVEncoder->write( &_avFrameYUV, AV_INTERLEAVED_VIDEO_WR );
	      }
	      else
	      {
		eResult = m_pAVEncoder->write( _pMbxItem->GetImage(), AV_INTERLEAVED_VIDEO_WR );
	      }

	      if (  eResult != eAVSucceded )
	      {
	  	ERROR_INFO( "Failed to encode frame.", Run() )
		
		SetStatus( eHSReleasing );
		break;
	      }
	    }
	  
	    // Check if recording time is greater or equal to GetHighLightsTimeSpan()
	    if ( m_swHighLights.Peek() >= m_dHighLightDuration )
	    {
	      LOG_INFO( FString( 0, "END HighLights(%s)", (const char*)m_rStreamReader.GetCameraName() ), Run() )

	      SetStatus( eHSReleasing );
	    }
	
	    // Just count and log average fps
	    if (_swFpsCount.Peek() >= 1.0 )
	    {
	      VERBOSE_INFO( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE, FString( 0, "AVG FPS [%d]",  _dwFpsCount ), Run() )
	      
	      _swFpsCount.Reset();
	      _dwFpsCount = 0;
	    }
	    else
	    {
	      _dwFpsCount++;
	    }
	    
	  }; break;
	  
	  case RecdMbxItem::eSampleItem:
	  {
	    // nothing to do
	  }; break;
	    
	} //switch ( _pMbxItem->GetType() )
	
	// Each item must be released.
	m_rStreamReader.ReleaseMbxItem( _pMbxItem );
	
      }; break;
      
      case eHSReleasing:
      {
	// Cleaning local queue
	RecdMbxItem* _pMbxItem = NULL;
	
	while ( !m_pVideoItems->IsEmpty() )
	{
	  _pMbxItem = m_pVideoItems->Pop();
	  
	  // Each item must be released.
	  m_rStreamReader.ReleaseMbxItem( _pMbxItem );
	}
	
	LOG_INFO( "Close and Release CAVEncoder instance", Run() )

	if ( m_pAVEncoder != NULL )
	{
	  if ( m_pAVEncoder->close() != eAVSucceded )
	  {
	    ERROR_INFO( "Failed to close encoder.", Run() )
	  }
	  
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
	}
	
	// Move to initial state  
	SetStatus( eHSWaiting );
      }; break;
    } // switch ( GetStatus() )
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



