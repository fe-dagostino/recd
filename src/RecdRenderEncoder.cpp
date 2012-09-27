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


#include "RecdRenderEncoder.h"
#include "RecdEncoderCollector.h"
#include "RecdConfig.h"
#include "avencoder.h"
#include "LOGGING/FLogger.h"


class RenderThreads : public FThread
{
public:
  
  /***/
  RenderThreads( RecdRenderEncoder* pRenderEncoder, RecdStreamEncoder* pStreamEncoder )
    : FThread( NULL, FThread::TP_CRITICAL, -1 ),
      m_bExit( FALSE ),
      m_eStatus( eRTUndefined ),
      m_bGotFrame( FALSE ),
      m_pRenderEncoder( pRenderEncoder ),
      m_pStreamEncoder( pStreamEncoder )
  {
  }
  
  virtual ~RenderThreads()
  {
  }

  BOOL    GotFrame() const
  {  
    return m_bGotFrame;  
  }
  
  VOID    Reset()
  {
    m_swFPS.Invalidate();
    m_bGotFrame = FALSE;
  }
  
  inline enum RenderThreadStatus  GetStatus() const
  {
    return m_eStatus;
  }
  
  inline VOID  SetStatus( enum RenderThreadStatus eStatus )
  {
    m_eStatus = eStatus;
  }
  
protected:
  BOOL	Initial()
  { 
    return TRUE; 
  }

  BOOL	Final()
  {
    m_bExit = TRUE;

    return TRUE;
  }

  VOID	Run()
  {
    DOUBLE     _dFPS                   =  1.0 / (double)RecdConfig::GetInstance().GetRenderFps( NULL );
    DWORD      _dwRenderReadingTimeout = RecdConfig::GetInstance().GetRenderReadingTimeout( NULL );
    DWORD      _dwStandbyTimeout       = RecdConfig::GetInstance().GetRenderStandByDelay( NULL );
    
    SetStatus( eRTWaiting );
    
    while ( !m_bExit )
    {
      FThread::YieldThread();
      
      switch( GetStatus() )
      {
	case eRTWaiting:
	{
	  RecdMbxItem*   _pMbxItem = m_pStreamEncoder->PopRenderMbxItem( _dwRenderReadingTimeout, TRUE );

	  if ( _pMbxItem != NULL )
	  {
	    if ( _pMbxItem->GetType() == RecdMbxItem::eCommandItem ) 
	    {
	      if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStartEncoding )
	      {
		SetStatus( eRTOpenStream );
	      }
	    }
	    
	    // Release memory.
	    m_pStreamEncoder->ReleaseMbxItem( _pMbxItem );
	  }
	  else
	  {
	    VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	    FThread::Sleep( _dwStandbyTimeout );
	  }
	}; break;
	
	case eRTOpenStream:
	{
	  //Wait until encoder will be opened or release command will be received.
	  RecdMbxItem*   _pMbxItem = m_pStreamEncoder->PopRenderMbxItem( _dwRenderReadingTimeout, TRUE );
	  if ( _pMbxItem != NULL )
	  {
	    if ( _pMbxItem->GetType() == RecdMbxItem::eCommandItem ) 
	    {
	      if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStartRendering )
	      {
		SetStatus( eRTRendering );
	      }
	      
	      if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStopEncoding   )
	      {
		SetStatus( eRTReleasing );
	      }
	    }
	    
	    // Release memory.
	    m_pStreamEncoder->ReleaseMbxItem( _pMbxItem );
	  }
	  else
	  {
	    VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Waiting Render Encoder", Run() )
	    FThread::Sleep( _dwStandbyTimeout );
	  }
	  
	}; break;
	
	case eRTRendering:
	{
	  RecdMbxItem*   _pMbxItem = m_pStreamEncoder->PopRenderMbxItem( _dwRenderReadingTimeout, TRUE );
      
	  if ( _pMbxItem == NULL )
	  {
	    VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Timeout occurs reading from mailbox", Run() )
	    break;
	  }

	  switch ( _pMbxItem->GetType() )
	  {
	    case  RecdMbxItem::eCommandItem:
	    {
	      if (
		  ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStopRendering ) ||
		  ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStopEncoding  )
		 )
	      {
		SetStatus( eRTReleasing );
	      }
	    };break;
	    
	    case  RecdMbxItem::eImageItem:
	    {
	      
	      if (
		    ( m_swFPS.IsValid()  == FALSE ) ||
		    ( m_swFPS.Peek()     >= _dFPS )
		  )
	      {
		m_swFPS.Reset();
	      }
	      else // Skip frame
	      {
		VERBOSE_INFO( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE, "Skip frame ..", Run() )
		break;
	      } 
	  
	      m_bGotFrame = TRUE;
	      
	      m_pRenderEncoder->Blend( 
					m_pStreamEncoder->GetRenderPoint(),
					*_pMbxItem->GetImage(),
					m_pStreamEncoder->GetRenderColor()
				    );
	      
	    }; break;
	  }//switch ( _pMbxItem->GetType() )
	
	  m_pStreamEncoder->ReleaseMbxItem( _pMbxItem );
 
	}; break;
	
	case eRTReleasing:
	{
	  //Wait until encoder will be opened or release command will be received.
	  RecdMbxItem*   _pMbxItem = m_pStreamEncoder->PopRenderMbxItem( _dwRenderReadingTimeout, TRUE );
	  if ( _pMbxItem != NULL )
	  {
	    if ( _pMbxItem->GetType() == RecdMbxItem::eCommandItem ) 
	    {
	      if ( _pMbxItem->GetCommand() == RecdMbxItem::eCmdStopRendering   )
	      {
		SetStatus( eRTWaiting );
	      }
	    }
	    
	    // Release memory.
	    m_pStreamEncoder->ReleaseMbxItem( _pMbxItem );
	  }
	  else
	  {
	    VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Waiting 'Stop Rendering' from Render Encoder", Run() )
	    FThread::Sleep( _dwStandbyTimeout );
	  }
	}; break;
      }// switch( GetStatus() )

    }//while ( !m_bExit )
  }

private:
  BOOL                        m_bExit;
  RenderThreadStatus volatile m_eStatus;
  BOOL volatile               m_bGotFrame;
  FStopWatch                  m_swFPS;
  RecdRenderEncoder*          m_pRenderEncoder;
  RecdStreamEncoder*          m_pStreamEncoder;
};

  
////////////////////////////////////////////////////////////////

  
GENERATE_CLASSINFO( RecdRenderEncoder, FLogThread ) 
  
  
RecdRenderEncoder::RecdRenderEncoder( const FString& sRenderEncoderName )
  : FLogThread( sRenderEncoderName, NULL, FThread::TP_CRITICAL, -1 ),
    m_bExit( FALSE ),
    m_eEncoderStatus( eREUndefined ),
    m_pAVEncoder( NULL )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );
  
  INT iRenderThread = 0;
  m_pRenderThreads  = new RenderThreads*[RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize()];
  FTList<RecdStreamEncoder* >::Iterator _iter = RecdEncoderCollector::GetInstance().GetRawEncoders()->Begin();
  
  while ( _iter )
  {
    RecdStreamEncoder* _pStreamEncoder = *_iter;

    m_pRenderThreads[iRenderThread] = new RenderThreads( this, _pStreamEncoder );
    m_pRenderThreads[iRenderThread]->Start();

    iRenderThread++;
    // move to next item in the list.
    _iter++;
  }
}

RecdRenderEncoder::~RecdRenderEncoder()
{
  if ( m_pRenderThreads != NULL )
  {
      for ( INT iRenderThread = 0; iRenderThread < RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize(); iRenderThread++ )
      {
	m_pRenderThreads[iRenderThread]->Stop();
	delete m_pRenderThreads[iRenderThread];
	m_pRenderThreads[iRenderThread] = NULL;
      }
      
      delete [] m_pRenderThreads;
      m_pRenderThreads = NULL;
  }
  
}

VOID   RecdRenderEncoder::SetParameters( const FString& sDestination )
{
  FMutexCtrl _mtxCtrl( m_mtxEncoder );

  for ( INT iRenderThread = 0; iRenderThread < RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize(); iRenderThread++ )
  {
    m_pRenderThreads[iRenderThread]->Reset();
  }
  
  m_sDestination = sDestination;
  
  SetStatus( eREOpenStream );
}

enum RecdRenderEncoder::RenderEncoderStatus RecdRenderEncoder::GetStatus() const
{
  FMutexCtrl mtxCtrl( m_mtxEncoder );
  
  return m_eEncoderStatus;
}

bool RecdRenderEncoder::SetStatus( RenderEncoderStatus eStatus )
{
  FMutexCtrl mtxCtrl( m_mtxEncoder );
  
  m_eEncoderStatus = eStatus;
  
  return TRUE;
}

BOOL	RecdRenderEncoder::Initial()
{ 
  return TRUE; 
}

BOOL	RecdRenderEncoder::Final()
{
  m_bExit = TRUE;
  
  return TRUE;
}

VOID	RecdRenderEncoder::Run()
{
  CAVImage   _avFrameYUV; 
  double     _dFPS             = 0.03333;
  double     _dFpsError        = 0.0;
  DWORD      _dwStandbyTimeout = RecdConfig::GetInstance().GetRenderStandByDelay( NULL );

  // Update status to waiting mode.
  SetStatus( eREWaiting );
  
  while ( !m_bExit )
  {
    FThread::YieldThread();
    
    switch( GetStatus() )
    {
      case eREWaiting:
      {
	BOOL bMoveForward = TRUE;
	
	// if at least one thread didn't got jet the first frame, rendering
	// will be suspended.
	if ( CheckStatus( eRTOpenStream ) )
	{
	  SetStatus( eREOpenStream );
	}
	else
	{
	  VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	  FThread::Sleep( _dwStandbyTimeout );
	}
      }; break;
      case eREOpenStream:
      {
	FString _sOutFilename( 0, "%s/RENDER_%010d.mp4", (const char*)m_sDestination, time(NULL) ); 
	
	//Loading background image.
	FString sBkgFilename     = RecdConfig::GetInstance().GetRenderBackground( NULL );
	FString sBkgMaskFilename = RecdConfig::GetInstance().GetRenderBackgroundMask( NULL );
	
	int _iWidth  = RecdConfig::GetInstance().GetRenderWidth ( NULL );
	int _iHeight = RecdConfig::GetInstance().GetRenderHeight( NULL );
	if ( m_rgbaBkg.load( (const char*)sBkgFilename, _iWidth, _iHeight, PIX_FMT_RGBA ) != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Error loading [%s]", (const char*)sBkgFilename ), Run() )
	
	  SetStatus( eREReleasing );
	  break;
	}
	
	if (_iWidth  == -1 )
	  _iWidth  = m_rgbaBkg.getWidth();
	
	if (_iHeight == -1 )
	  _iHeight = m_rgbaBkg.getHeight();
	
	// If we have no skin rendering will be impossible
	if ( ( _iWidth == -1 ) && (_iHeight == -1 ) )
	{
	  ERROR_INFO( FString( 0, "Failed to LOAD Background [%s] CHECK YOUR CONFIGURATION", (const char*)sBkgMaskFilename ), Run() )

	  SetStatus( eREReleasing );
	  break;
	}
	
	// Initialize background mask with background image.
	if ( m_rgbaMaskBkg.load( (const char*)sBkgMaskFilename, _iWidth, _iHeight, PIX_FMT_RGBA ) != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Error loading [%s]", (const char*)sBkgMaskFilename ), Run() )
	  
	  LOG_INFO( "Initialize BACKGROUND MASK using BACKGROUND IMAGE -- Check CONFIGURATION ...", Run() )
	  m_rgbaMaskBkg.init( m_rgbaBkg, _iWidth, _iHeight, PIX_FMT_RGBA );
	}

	m_pAVEncoder = new CAVEncoder();
	if ( m_pAVEncoder == NULL )
	{
	  ERROR_INFO( "Not Enough Memory for allocating CAVEncoder()", Run() )

	  SetStatus( eREReleasing );
	  break;
	}

	LOG_INFO( FString( 0, "RENDERING OUT=[%s]", (const char*)_sOutFilename ), Run() )
	AVResult _avRes = m_pAVEncoder->open( 
					    _sOutFilename,
					    AV_ENCODE_VIDEO_STREAM,
					    _iWidth,
					    _iHeight,
					    PIX_FMT_YUV420P,
					    RecdConfig::GetInstance().GetRenderFps       ( NULL ),
					    RecdConfig::GetInstance().GetRenderGoP       ( NULL ),
					    RecdConfig::GetInstance().GetRenderBitRate   ( NULL ),
					    (CodecID)RecdConfig::GetInstance().GetRenderVideoCodec( NULL ),
					    RecdConfig::GetInstance().GetRenderVideoCodecProfile( NULL )
					);
	if ( _avRes != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Failed to Open Encoder [%s]", (const char*)_sOutFilename ), Run() )
	  
	  SetStatus( eREReleasing );
	  break;
	}
	
        _dFPS      = 1.0 / (double)RecdConfig::GetInstance().GetRenderFps( NULL );
	_dFpsError = 0.0;

	// Send Start Rendering command to each rendering thread
	PostCommand( RecdMbxItem::eCmdStartRendering );
	
	// Invalidate fps stop watch
	m_swFPS.Invalidate();

	// Move to next state
	SetStatus( eREEncoding );
      }; break;
      
      case eREEncoding:
      {
	if ( CheckStatus( eRTReleasing ) == TRUE )
	{
	  SetStatus( eREReleasing );
	  break;
	}
	
	// Wait until all reading channels will got a frame to be processed.
	BOOL _bGotFrames = TRUE;
	for ( INT iRenderThread = 0; iRenderThread < RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize(); iRenderThread++ )
	{
	  // if at least one thread didn't got jet the first frame, rendering
	  // will be suspended.
	  _bGotFrames &= m_pRenderThreads[iRenderThread]->GotFrame();
	}
	
	if ( _bGotFrames == FALSE )
	{
	  // if at least one thread didn't got a frame we will force
	  // termination of current segment of code.
	  break;
	}
	
	if (
	    ( m_swFPS.IsValid() == FALSE ) ||
	    ( m_swFPS.Peek()    >= _dFPS ) 
	  )
	{
	  // Cumulative error
	  _dFpsError += m_swFPS.Peek() - _dFPS;
	  
	  m_swFPS.Reset();
	}
	else if ( _dFpsError >= _dFPS )
	{
	  _dFpsError -= _dFPS;
	}  
	else // move to next loop
	{
	  continue;
	}

	// initialize autput frame.
	_avFrameYUV.init( m_rgbaBkg, -1, -1, PIX_FMT_YUV420P );

	if ( m_pAVEncoder->write( &_avFrameYUV, AV_INTERLEAVED_VIDEO_WR ) != eAVSucceded )
	{
	  ERROR_INFO( "Failed to encode frame.", Run() )
	  
	  SetStatus( eREReleasing );
	  break;
	}
	
      }; break;
      
      case eREReleasing:
      {

	if ( m_pAVEncoder != NULL )
	{
	  LOG_INFO( "Close Encoder", Run() )
	  if ( m_pAVEncoder->close() != eAVSucceded )
	  {
	    ERROR_INFO( "Failed to close encoder.", Run() )
	  }
	  
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
	}
	
	PostCommand( RecdMbxItem::eCmdStopRendering );
	
	if ( CheckStatus( eRTWaiting ) == TRUE )
	  SetStatus( eREWaiting );
      }; break;
      
    }//switch( GetStatus() )

  }// while ( !m_bExit )
}

BOOL    RecdRenderEncoder::Blend( const CAVPoint& rPos, const CAVImage& rFrame, const CAVColor& crKeyColor )
{
  // Use alpha blending + key color in order to map rFrame on m_rgbaBkg.
  if ( m_rgbaBkg.blend( rPos, rFrame, m_rgbaMaskBkg, crKeyColor ) != eAVSucceded )
  {
    ERROR_INFO( "Blending failed", Run() )
  }

  return TRUE;
}

BOOL    RecdRenderEncoder::CheckStatus( enum RenderThreadStatus eStatus )
{
  BOOL bRetVal = TRUE;
  
  // Monitor status for Render Thread Wait until all reading channels will got a frame to be processed.
  for ( INT iRenderThread = 0; iRenderThread < RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize(); iRenderThread++ )
  {
    // if at least one thread didn't got jet the first frame, rendering
    // will be suspended.
    if ( m_pRenderThreads[iRenderThread]->GetStatus() != eStatus )
    {
      bRetVal = FALSE;
      break;
    }
  }
  
  return bRetVal;
}

VOID    RecdRenderEncoder::PostCommand( RecdMbxItem::MbxCommandType eCommand )
{
  FTList<RecdStreamEncoder* >::Iterator _iter = RecdEncoderCollector::GetInstance().GetRawEncoders()->Begin();
  
  while ( _iter )
  {
    RecdStreamEncoder* _pStreamEncoder = *_iter;
    
    _pStreamEncoder->PushRenderMbxItem( new RecdMbxItem( eCommand ) );
    // move to next item in the list.
    _iter++;
  }
  
  FThread::YieldThread();
}

DWORD   RecdRenderEncoder::GetLogMessageFlags() const
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


DWORD   RecdRenderEncoder::GetVerbosityLevelFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsStartUpMessageEnabled       ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_START_UP_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsShutDownMessageEnabled      ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_SHUT_DOWN_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLowPeriodicMessageEnabled   ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_LOW_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsMediumPeriodicMessageEnabled( "Encoder Verbosity Level", NULL)?FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsHighPeriodicMessageEnabled  ( "Encoder Verbosity Level", NULL)?FLogMessage::VL_HIGH_PERIODIC_MESSAGE:0;

  return _dwRetVal;
}



