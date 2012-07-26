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


#include "RecdStreamEncoder.h"
#include "RecdReaderCollector.h"
#include "RecdEncoderCollector.h"
#include "RecdRenderEncoder.h"
#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

GENERATE_CLASSINFO( RecdStreamEncoder, FLogThread ) 

RecdStreamEncoder::RecdStreamEncoder( 
				      const FString&    sStreamEncoderName,
				      RecdStreamReader& rStreamReader
				    )
  : FLogThread( sStreamEncoderName, NULL, FThread::TP_CRITICAL, -1 ),
    m_bExit( FALSE ),
    m_eEncoderStatus( eSEUndefined ),
    m_pAVEncoder( NULL ),
    m_rStreamReader( rStreamReader )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );
  
  m_pMbxItems = new FTMailbox<RecdMbxItem* >( "Mailbox Scaled Frames", NULL );
  if ( m_pMbxItems == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Mailbox", RecdStreamReader() )
    //@todo
  }

  m_pVideoItems = new FTQueue<RecdMbxItem* >();
  if ( m_pVideoItems == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Queue", RecdStreamReader() )
    //@todo
  }
}

RecdStreamEncoder::~RecdStreamEncoder()
{
  if ( m_pMbxItems != NULL )
  {
    LOG_INFO( "Release mailbox resources", ~RecdStreamReader() )
    // Delete all allocated items
    while ( !m_pMbxItems->IsEmpty() )
    {
      RecdMbxItem* pMbxItem = m_pMbxItems->Read();
      delete pMbxItem;
    }
    
    // Release mailbox.
    delete m_pMbxItems;
    m_pMbxItems = NULL;
  }
  
  if ( m_pVideoItems != NULL )
  {
    // Release mailbox.
    delete m_pVideoItems;
    m_pVideoItems = NULL;
  }
}

RecdMbxItem*  RecdStreamEncoder::PopRenderMbxItem( DWORD dwTimeout, BOOL bRemove )
{
  if ( m_pMbxItems == NULL )
    return NULL;
  
  return m_pMbxItems->Read( dwTimeout, bRemove );
}

BOOL       RecdStreamEncoder::PushRenderMbxItem( RecdMbxItem* pMbxItem )
{
  if ( m_pMbxItems == NULL )
    return FALSE;
  
  m_pMbxItems->Write( pMbxItem );
  
  return TRUE;
}

CAVPoint   RecdStreamEncoder::GetRenderPoint() const
{
  return CAVPoint(
  		  RecdConfig::GetInstance().GetRenderRectX( m_rStreamReader.GetCameraName(), NULL ), 
		  RecdConfig::GetInstance().GetRenderRectY( m_rStreamReader.GetCameraName(), NULL )
	      );
}

CAVRect    RecdStreamEncoder::GetRenderRect() const
{
  return CAVRect(
  		  0, 
		  0, 
		  RecdConfig::GetInstance().GetRenderRectWidth( m_rStreamReader.GetCameraName(), NULL ), 
		  RecdConfig::GetInstance().GetRenderRectHeight( m_rStreamReader.GetCameraName(), NULL )
                );
}

CAVColor   RecdStreamEncoder::GetRenderColor() const
{
  return RecdConfig::GetInstance().GetRenderKeyColor( m_rStreamReader.GetCameraName(), NULL );
}

VOID   RecdStreamEncoder::ReleaseMbxItem( RecdMbxItem*& pMbxItem )
{
  if ( pMbxItem == NULL )
    return ;
  
  delete pMbxItem;
  pMbxItem = NULL;
}


VOID   RecdStreamEncoder::SetParameters( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw )
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );
  
  m_sDestination = sDestination;
  m_bRender      = bRender; 
  m_bHighlights  = bHighlights; 
  m_bRaw         = bRaw;
}

enum RecdStreamEncoder::StreamEncoderStatus RecdStreamEncoder::GetStatus() const
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );

  return m_eEncoderStatus;
}

bool RecdStreamEncoder::SetStatus( StreamEncoderStatus eStatus )
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );
  
  m_eEncoderStatus = eStatus;
  
  return true;
}

BOOL	RecdStreamEncoder::Initial()
{ 
  return TRUE; 
}

BOOL	RecdStreamEncoder::Final()
{
  m_bExit = TRUE;
  
  return TRUE;
}


VOID	RecdStreamEncoder::Run()
{
  double     _dFPS              = 0.03333;
  double     _dFpsError         = 0.0;
  DWORD      _dwEncoderMaxItems = RecdConfig::GetInstance().GetEncoderMaxItems( m_rStreamReader.GetCameraName(), NULL );
  DWORD      _dwReadingTimeout  = RecdConfig::GetInstance().GetEncoderReadingTimeout( m_rStreamReader.GetCameraName(), NULL );
  DWORD      _dwStandbyTimeout  = RecdConfig::GetInstance().GetEncoderStandByDelay( m_rStreamReader.GetCameraName(), NULL ); 
  INT        _iRenderWidth  = -1;
  INT        _iRenderHeight = -1;
  DWORD      _dwEncoderOpts =  1;
  CAVPoint   _avAlphaBlendingPos;
  CAVRect    _avAlphaBlendingRect;
  CAVImage   _avFrameRGBA;
  CAVImage   _avFrameYUV;
   
  // Update status to waiting mode.
  SetStatus( eSEWaiting );
  
  while ( !m_bExit )
  {
    FThread::YieldThread();
    
    switch ( GetStatus() )
    {
      case eSEWaiting:
      {
	RecdMbxItem* _pMbxItem = m_rStreamReader.PopRawItem( _dwReadingTimeout, TRUE );
	if ( _pMbxItem != NULL )
	{
	  if ( _pMbxItem->GetType() == RecdMbxItem::eCommandItem ) 
	  {
	    if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStartEncoding )
	    {
	      if ( m_bRender == TRUE )
		m_pMbxItems->Write( new RecdMbxItem( RecdMbxItem::eCmdStartEncoding ) );

	      SetStatus( eSEOpenStream );
	    }
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
      
      case eSEOpenStream:
      {
	FString _sCameraName  = m_rStreamReader.GetCameraName();

	// Initialize Render options before moving to next status in the automata.
	// If this values are not initialized result will be to work with defaults values.
	_dFPS          = 1.0 / (double)RecdConfig::GetInstance().GetEncoderFps( _sCameraName, NULL );
	_dFpsError     = 0.0;
	_iRenderWidth  = RecdConfig::GetInstance().GetRenderRectWidth      ( _sCameraName, NULL );
	_iRenderHeight = RecdConfig::GetInstance().GetRenderRectHeight     ( _sCameraName, NULL );
	_dwEncoderOpts = RecdConfig::GetInstance().GetEncoderRescaleOptions( _sCameraName, NULL );
	
	// Check if raw video is required.
	if ( m_bRaw == FALSE )
	{
	  SetStatus( eSEEncoding );
	  break;
	}
	
	FString _sOutFilename( 0, "%s/%s_%010d.mp4", (const char*)m_sDestination, (const char*)_sCameraName, time(NULL) ); 
	
	m_pAVEncoder = new CAVEncoder();
	if ( m_pAVEncoder == NULL )
	{
	  ERROR_INFO( "Not Enough Memory for allocating CAVEncoder()", Run() )
	  
	  SetStatus( eSEReleasing );
	  break;
	}
	
	// With and Height for the final raw video
	int _iWidth  = RecdConfig::GetInstance().GetEncoderWidth ( _sCameraName, NULL );
	int _iHeight = RecdConfig::GetInstance().GetEncoderHeight( _sCameraName, NULL );
	
	// Release background image if previously allocated.
	m_rgbaBkg.free();
	// Check if background has be enabled or not.
	if ( RecdConfig::GetInstance().GetEncoderBackgroundStatus( _sCameraName, NULL ) == TRUE )
	{
	  //Loading background image.
	  FString sBkgFilename     = RecdConfig::GetInstance().GetEncoderBackground( _sCameraName, NULL );
	  if ( m_rgbaBkg.load( (const char*)sBkgFilename, _iWidth, _iHeight, PIX_FMT_RGBA ) != eAVSucceded )
	  {
	    ERROR_INFO( FString( 0, "Error loading [%s]", (const char*)sBkgFilename ), Run() )
	  }
	
	  if (_iWidth  == -1 )
	    _iWidth  = m_rgbaBkg.getWidth();
	  
	  if (_iHeight == -1 )
	    _iHeight = m_rgbaBkg.getHeight();
	  
	  _avAlphaBlendingPos.set(   
			    RecdConfig::GetInstance().GetEncoderRectX( m_rStreamReader.GetCameraName(), NULL ), 
			    RecdConfig::GetInstance().GetEncoderRectY( m_rStreamReader.GetCameraName(), NULL )
			  );
			  
	  _avAlphaBlendingRect = CAVRect(
			    0, 
			    0, 
			    RecdConfig::GetInstance().GetEncoderRectWidth( m_rStreamReader.GetCameraName(), NULL ), 
			    RecdConfig::GetInstance().GetEncoderRectHeight( m_rStreamReader.GetCameraName(), NULL )
			  );
	}

	// If we have no skin! raw rendering will be impossible
	if ( ( _iWidth == -1 ) || (_iHeight == -1 ) )
	{
	  ERROR_INFO( "RAW Width and/or Height NOT DEFINED! CHECK YOUR CONFIGURATION", Run() )

	  break;
	}

	LOG_INFO( FString( 0, "CAM=[%s] OUT=[%s]", (const char*)_sCameraName, (const char*)_sOutFilename ), Run() )
	AVResult _avRes = m_pAVEncoder->open( 
					    _sOutFilename,
					    AV_ENCODE_VIDEO_STREAM|AV_ENCODE_AUDIO_STREAM,
					    _iWidth,
					    _iHeight,
					    RecdConfig::GetInstance().GetEncoderFps       ( _sCameraName, NULL ),
					    RecdConfig::GetInstance().GetEncoderGoP       ( _sCameraName, NULL ),
					    RecdConfig::GetInstance().GetEncoderBitRate   ( _sCameraName, NULL ),
					    (CodecID)RecdConfig::GetInstance().GetEncoderVideoCodec( _sCameraName, NULL ),
					    RecdConfig::GetInstance().GetEncoderVideoCodecProfile( _sCameraName, NULL )
					);
	if ( _avRes != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Failed to Open Encoder [%s]", (const char*)_sOutFilename ), Run() )
	  
	  SetStatus( eSEReleasing );
	  break;
	}
	
	// Invalidate fps stop watch
	m_swFPS.Invalidate();

	// Move to next state
	SetStatus( eSEEncoding );
      }; break;
      
      case eSEEncoding:
      {
	RecdMbxItem* _pMbxItem = NULL;
	
	_dwReadingTimeout = (_dFpsError>=_dFPS)?1:RecdConfig::GetInstance().GetEncoderReadingTimeout( m_rStreamReader.GetCameraName(), NULL );
	
	_pMbxItem = m_rStreamReader.PopRawItem( _dwReadingTimeout, TRUE );
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
	      if ( m_bRender == TRUE )
		m_pMbxItems->Write( new RecdMbxItem( RecdMbxItem::eCmdStopEncoding ) );
	      
	      SetStatus( eSEReleasing );
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
	      // Cumulative error
	      _dFpsError += m_swFPS.Peek() - _dFPS;
	      
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
	    
	    	    
	    if ( m_bRender == TRUE )
	    {
	      /**
	       * Each frame retrieved from reader will be resized for the final render.
	       * The reason why this part of code resides here instead of Render thread 
	       * is to better balance cpu(s) allocation time at kenel level and so to 
	       * distribute complexity.
	       */
	      if ( m_pMbxItems->GetSize() >= _dwEncoderMaxItems )
	      {
		ERROR_INFO( FString( 0, "Consumer Thread (RENDER) is TOO SLOW queue is full SIZE[%d].", m_pMbxItems->GetSize() ), Run() )
	      }
	      else
	      {
		// Video Frame will be dispatched to RecdRenderEncoder
		CAVImage* pAVImage = new CAVImage();
		pAVImage->init( *_pMbxItem->GetImage(), 
				_iRenderWidth, 
				_iRenderHeight, 
				PIX_FMT_RGBA,
				_dwEncoderOpts
			      );
		
		//Write a new frame into mailbox for rendering
		m_pMbxItems->Write( new RecdMbxItem( pAVImage ) );
	      }
	    }//if ( m_bRender == TRUE )
	    
	    
	    if ( m_bRaw == TRUE )
	    {
	      AVResult eResult;
	  
	      // If background is valid alpha blending is required.
	      if ( m_rgbaBkg.isValid() )
	      {
		_avFrameRGBA.init( 
				   *_pMbxItem->GetImage(), 
				   _avAlphaBlendingRect.getWidth(), 
				   _avAlphaBlendingRect.getHeight(), 
				   PIX_FMT_RGBA,
				   _dwEncoderOpts
			         );
	
		// In this case _pFrame is an RGBA image
		m_rgbaBkg.blend( _avAlphaBlendingPos, _avFrameRGBA );

		// initialize autput frame.
		_avFrameYUV.init( m_rgbaBkg, -1, -1, PIX_FMT_YUV420P );
	      }
	      else
	      {
		// initialize autput frame.
		_avFrameYUV.init( 
				  *_pMbxItem->GetImage(),
				  m_pAVEncoder->getVideoWidth(),
				  m_pAVEncoder->getVideoHeight(),
				  PIX_FMT_YUV420P,
				  _dwEncoderOpts
			        );
	      }
	      	      
	      // Encoding and write output frame.
	      eResult = m_pAVEncoder->write( &_avFrameYUV, AV_INTERLEAVED_VIDEO_WR );
	      
	      // Check for a possible error during encoding.
	      if ( eResult != eAVSucceded )
	      {
		  ERROR_INFO( "Failed to encode frame.", Run() )
		  
		  SetStatus( eSEReleasing );
	      }
	    }//if ( m_bRaw == TRUE )
	    
	  }; break;
	  
	  case RecdMbxItem::eSampleItem:
	  {
	    // nothing to do.
	  }; break;
	}// switch ( _pMbxItem->GetType() )
	
	// Each item must be released.
	m_rStreamReader.ReleaseMbxItem( _pMbxItem );
      
      }; break;
      
      case eSEReleasing:
      {
	// Cleaning local queue
	RecdMbxItem* _pMbxItem = NULL;
	
	while ( !m_pVideoItems->IsEmpty() )
	{
	  _pMbxItem = m_pVideoItems->Pop();
	  
	  // Each item must be released.
	  m_rStreamReader.ReleaseMbxItem( _pMbxItem );
	}
	
	LOG_INFO( "Dispatching Stop Encoding Command", Run() )
	// Writing messages for consumers
	// Event there is an error on reading or it will be terminated by user request
	// we will dispatch Stop Encoding commando to each consumer.
	if ( m_bRender == TRUE )
	{
	  m_pMbxItems->Write       ( new RecdMbxItem( RecdMbxItem::eCmdStopEncoding ) );
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
	SetStatus( eSEWaiting );
      }; break; 
    }//switch ( GetStatus() )
    
    
  }// while ( !m_bExit )
}

DWORD   RecdStreamEncoder::GetLogMessageFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsTraceExceptionEnabled  ( "Encoder Log Message", NULL)?FLogMessage::MT_TRACE_EXCEPTION:0;
  _dwRetVal |= RecdConfig::GetInstance().IsCatchExceptionEnabled  ( "Encoder Log Message", NULL)?FLogMessage::MT_CATCH_EXCEPTION:0;
  _dwRetVal |= RecdConfig::GetInstance().IsAssertionFailureEnabled( "Encoder Log Message", NULL)?FLogMessage::MT_ASSERTION_FAILURE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsErrorInfoEnabled       ( "Encoder Log Message", NULL)?FLogMessage::MT_ERROR_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLoggingInfoEnabled     ( "Encoder Log Message", NULL)?FLogMessage::MT_LOGGING_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsVerboseInfoEnabled     ( "Encoder Log Message", NULL)?FLogMessage::MT_VERBOSE_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsRawInfoEnabled         ( "Encoder Log Message", NULL)?FLogMessage::MT_RAW_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsEnterMethodEnabled     ( "Encoder Log Message", NULL)?FLogMessage::MT_ENTER_METHOD:0;
  _dwRetVal |= RecdConfig::GetInstance().IsExitMethodEnabled      ( "Encoder Log Message", NULL)?FLogMessage::MT_EXIT_METHOD:0;

  return _dwRetVal;
}


DWORD   RecdStreamEncoder::GetVerbosityLevelFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsStartUpMessageEnabled       ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_START_UP_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsShutDownMessageEnabled      ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_SHUT_DOWN_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLowPeriodicMessageEnabled   ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_LOW_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsMediumPeriodicMessageEnabled( "Encoder Verbosity Level", NULL)?FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsHighPeriodicMessageEnabled  ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_HIGH_PERIODIC_MESSAGE:0;

  return _dwRetVal;
}



