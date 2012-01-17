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
  RenderThreads( RecdRenderEncoder* pRenderEncoder, RecdStreamEncoder* pStreamEncoder )
    : FThread( NULL, FThread::TP_CRITICAL, -1 ),
      m_bExit( FALSE ),
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
    m_bGotFrame = FALSE;
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
    while ( !m_bExit )
    {
      CAVFrame*   _pFrame = m_pStreamEncoder->GetRenderFrame( 
                                                              RecdConfig::GetInstance().GetRenderReadingTimeout( NULL ),
                                                              TRUE
                                                            );
      if ( _pFrame != NULL )
      {
	m_bGotFrame = TRUE;
	
	if ( _pFrame->getImage() != NULL )
	{
	  m_pRenderEncoder->Blend( 
				    m_pStreamEncoder->GetRenderPoint(),
				    *_pFrame->getImage(),
				    m_pStreamEncoder->GetRenderColor()
				);
	}			       
			       
	m_pStreamEncoder->ReleaseRenderFrame( _pFrame );
      }
      else
      {
	VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Timeout occurs reading from mailbox", Run() )
	
	FThread::YieldThread();	
      }
    }//while ( !m_bExit )
  }

private:
  BOOL               m_bExit;
  BOOL               m_bGotFrame;
  RecdRenderEncoder* m_pRenderEncoder;
  RecdStreamEncoder* m_pStreamEncoder;
};

  
////////////////////////////////////////////////////////////////

  
GENERATE_CLASSINFO( RecdRenderEncoder, FLogThread ) 
  
  
RecdRenderEncoder::RecdRenderEncoder( const FString& sRenderEncoderName )
  : FLogThread( sRenderEncoderName, NULL, FThread::TP_CRITICAL, -1 ),
    m_bExit( FALSE ),
    m_bRecording( FALSE ),
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

VOID   RecdRenderEncoder::StartRecording( const FString& sDestination )
{
  FMutexCtrl _mtxCtrl( m_mtxRecording );

  for ( INT iRenderThread = 0; iRenderThread < RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize(); iRenderThread++ )
  {
    m_pRenderThreads[iRenderThread]->Reset();
  }
  
  m_sDestination = sDestination;
  m_bRecording   = TRUE;
}

VOID   RecdRenderEncoder::StopRecording()
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
  CAVImage    _avFrameYUV; 
  double      _dFPS = 0.03333;
  
  while ( !m_bExit )
  {
    FThread::YieldThread();
    
    m_mtxRecording.EnterMutex();
    
    if ( m_bRecording == FALSE )
    {
      //Release mutex
      m_mtxRecording.LeaveMutex();
      
      if ( m_pAVEncoder == NULL )
      {
	VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	FThread::Sleep(
			RecdConfig::GetInstance().GetRenderStandByDelay( NULL )
		      );
      }
      else
      {
	m_sDestination.Empty();
	
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
    }//if ( m_bRecording == FALSE )
    else  
    { 
      if ( m_pAVEncoder == NULL )
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
	  
	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}
	
	if (_iWidth  == -1 )
	  _iWidth  = m_rgbaBkg.getWidth();
	
	if (_iHeight == -1 )
	  _iHeight = m_rgbaBkg.getHeight();
	
	// If we have no skin rendering will be impossible
	if ( ( _iWidth == -1 ) && (_iHeight == -1 ) )
	{
	  ERROR_INFO( FString( 0, "Failed to LOAD Background [%s] CHECK YOUR CONFIGURATION", (const char*)sBkgMaskFilename ), Run() )

	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
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

	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}

	LOG_INFO( FString( 0, "RENDERING OUT=[%s]", (const char*)_sOutFilename ), Run() )
	AVResult _avRes = m_pAVEncoder->open( 
					    _sOutFilename,
					    AV_ENCODE_VIDEO_STREAM,
					    _iWidth,
					    _iHeight,
					    RecdConfig::GetInstance().GetRenderFps       ( NULL ),
					    RecdConfig::GetInstance().GetRenderGoP       ( NULL ),
					    RecdConfig::GetInstance().GetRenderBitRate   ( NULL ),
					    (CodecID)RecdConfig::GetInstance().GetRenderVideoCodec( NULL )
					);
	if ( _avRes != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Failed to Open Encoder [%s]", (const char*)_sOutFilename ), Run() )
	  
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
	  
	  //Release mutex
	  m_mtxRecording.LeaveMutex();
	  continue;
	}
	
        _dFPS =  1.0 / (double)RecdConfig::GetInstance().GetRenderFps( NULL );
	
	// Wait until all reading channels will got a frame to be processed.
	for ( INT iRenderThread = 0; iRenderThread < RecdEncoderCollector::GetInstance().GetRawEncoders()->GetSize(); iRenderThread++ )
	{
	  while ( !m_pRenderThreads[iRenderThread]->GotFrame() )
	    FThread::Sleep(1);
	}
      } //if ( _pAVEncoder == NULL )
      

      m_swFPS.Reset();

      // initialize autput frame.
      _avFrameYUV.init( m_rgbaBkg, -1, -1, PIX_FMT_YUV420P );

      CAVFrame  _avFrame( &_avFrameYUV ); 
      if ( m_pAVEncoder->write( &_avFrame, 0 ) != eAVSucceded )
      {
	  ERROR_INFO( "Failed to encode frame.", Run() )
	  
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
      }
      _avFrame.detach();

      //Release mutex
      m_mtxRecording.LeaveMutex();
      
      while ( m_swFPS.Peek() < _dFPS )
	FThread::YieldThread();
    }// else if ( m_bRecording == FALSE )
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



