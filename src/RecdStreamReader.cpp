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


#include "RecdStreamReader.h"
#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

#include "avimage.h"


GENERATE_CLASSINFO( RecdStreamReader, FLogThread ) 

RecdStreamReader::RecdStreamReader( 
				    const FString& sStreamReaderName,
				    const FString& sIpCamera
				  )
  : FLogThread( sStreamReaderName, NULL, FThread::TP_HIGHEST, -1 ),
   m_sIpCamera( sIpCamera ),
   m_bExit( FALSE ),
   m_bReading( FALSE ),
   m_pAvReader( NULL ),
   m_bGotKeyFrame( FALSE ),
   m_dStopWatchCMP( 0.0 ),
   m_pMbxRawFrames( NULL ),
   m_pMbxHighLightsFrames( NULL )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );  
  
  m_pMbxRawFrames = new FTMailbox<CAVFrame* >( "Mailbox Decoding Frames", NULL );
  if ( m_pMbxRawFrames == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Mailbox", RecdStreamReader() )
    //@todo
  }

  m_pMbxHighLightsFrames = new FTMailbox<CAVFrame* >( "Mailbox Decoding Frames", NULL );
  if ( m_pMbxHighLightsFrames == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Mailbox", RecdStreamReader() )
    //@todo
  }

  // Initialize stop watch.
  m_swStopReading.Reset();
}

RecdStreamReader::~RecdStreamReader()
{
  if ( m_pMbxRawFrames != NULL )
  {
    LOG_INFO( "Release mailbox resources", ~RecdStreamReader() )
    // Delete all allocated items
    while ( !m_pMbxRawFrames->IsEmpty() )
    {
      CAVFrame* pFrame = m_pMbxRawFrames->Read();
      delete pFrame;
    }
    
    // Release mailbox.
    delete m_pMbxRawFrames;
    m_pMbxRawFrames = NULL;
  }
  
  if ( m_pMbxHighLightsFrames != NULL )
  {
    LOG_INFO( "Release mailbox resources", ~RecdStreamReader() )
    // Delete all allocated items
    while ( !m_pMbxHighLightsFrames->IsEmpty() )
    {
      CAVFrame* pFrame = m_pMbxHighLightsFrames->Read();
      delete pFrame;
    }
    
    // Release mailbox.
    delete m_pMbxHighLightsFrames;
    m_pMbxHighLightsFrames = NULL;
  }
}

const FString&  RecdStreamReader::GetCameraName() const
{
  return m_sIpCamera;
}
  
CAVFrame* 	RecdStreamReader::GetRawFrame( DWORD dwTimeout, BOOL bRemove )
{
  if ( m_pMbxRawFrames == NULL )
    return NULL;
  
  return m_pMbxRawFrames->Read( dwTimeout, bRemove );
}

CAVFrame* 	RecdStreamReader::GetHighLightsFrame( DWORD dwTimeout, BOOL bRemove )
{
  if ( m_pMbxHighLightsFrames == NULL )
    return NULL;
  
  return m_pMbxHighLightsFrames->Read( dwTimeout, bRemove );
}

DWORD 	 	RecdStreamReader::GetRawMailboxSize() const
{
  return m_pMbxRawFrames->GetSize();
}

DWORD 	 	RecdStreamReader::GetHighLightsMailboxSize() const
{
  return m_pMbxHighLightsFrames->GetSize();
}

VOID 	 	RecdStreamReader::ReleaseFrame( CAVFrame*& pAVFrame )
{
  if ( pAVFrame == NULL )
    return ;
  
  delete pAVFrame;
  pAVFrame = NULL;
}

VOID   RecdStreamReader::SetReading( BOOL bEnable )
{
  BOOL _bWaitClose = FALSE;
  
  m_mtxReading.EnterMutex();
  
    // Both Start and Stop are affected from the following StopWatch.
    // When changing state the StopWatch will be set to ZERO, so start
    // and stop will be delayed for the specified time.
    m_swStopReading.Reset();

    if ( bEnable == TRUE )
      m_dStopWatchCMP = RecdConfig::GetInstance().GetReaderStartDelayTime( NULL );
    else
      m_dStopWatchCMP = RecdConfig::GetInstance().GetReaderStopDelayTime( NULL );
    
    if ( 
	( bEnable     == FALSE ) &&
	( m_pAvReader != NULL  )
      )
    {
      _bWaitClose = TRUE;
    }
    m_bReading = bEnable;
  
  m_mtxReading.LeaveMutex();
  
  if ( _bWaitClose == TRUE )
  {
    m_semStop.Wait();
  }
}

void   RecdStreamReader::OnVideoKeyFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst )
{
  if ( m_bReading == TRUE )
  {  
    m_bGotKeyFrame = TRUE;
  }
}

bool   RecdStreamReader::OnVideoFrame( 
					const  AVFrame* pAVFrame,
					const  AVCodecContext* pAVCodecCtx, 
					double pst 
				     )
{
  if ( m_bGotKeyFrame == TRUE )
  {
    if ( m_pMbxRawFrames->GetSize() >= RecdConfig::GetInstance().GetReaderMaxItems( m_sIpCamera, NULL ) )
    {
      ERROR_INFO( "Consumer Thread is TOO SLOW queue is full.", OnVideoFrame() )
      return true;
    }
    
    CAVImage*  _pAVRawImage      = new CAVImage();
    _pAVRawImage->init( 
	    pAVFrame, 
	    pAVCodecCtx,
	    -1,
	    -1, 
	    PIX_FMT_YUV420P, 
	    RecdConfig::GetInstance().GetReaderRescaleOptions( m_sIpCamera, NULL ) 
	);

    m_pMbxRawFrames->Write       ( new CAVFrame( _pAVRawImage      ) );

    
    CAVImage* _pHighLightsImage = new CAVImage();
    _pHighLightsImage->init( 
	    pAVFrame, 
	    pAVCodecCtx,
	    RecdConfig::GetInstance().GetHighLightsRectWidth( m_sIpCamera, NULL ), 
	    RecdConfig::GetInstance().GetHighLightsRectHeight( m_sIpCamera, NULL ), 
	    RecdConfig::GetInstance().GetHighLightsBackgroundStatus( m_sIpCamera, NULL )?PIX_FMT_RGB24:PIX_FMT_YUV420P,
	    RecdConfig::GetInstance().GetReaderRescaleOptions( m_sIpCamera, NULL )
	);
    m_pMbxHighLightsFrames->Write( new CAVFrame( _pHighLightsImage ) );
       
    
    VERBOSE_INFO( 
		  FLogMessage::VL_HIGH_PERIODIC_MESSAGE, 
		  FString( 0, "Mailboxes RAW Size=[%u] HL Size=[%u]", m_pMbxRawFrames->GetSize(), m_pMbxHighLightsFrames->GetSize() ),
		  OnVideoFrame() 
		)
  }
  
  return true;
}

bool    RecdStreamReader::OnAudioFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst )
{
  CAVSample* pRawSample = new CAVSample();
  CAVSample* pHLSample  = new CAVSample();
  
  pRawSample->init(pAVFrame, pAVCodecCtx);
  pHLSample->init (pAVFrame, pAVCodecCtx);
  
  m_pMbxRawFrames->Write       ( new CAVFrame( pRawSample ) );
  m_pMbxHighLightsFrames->Write( new CAVFrame( pHLSample  ) );
  
  return true;
}

BOOL	RecdStreamReader::Initial()
{ 
  return TRUE; 
}

BOOL	RecdStreamReader::Final()
{
  m_bExit = TRUE;
  
  return TRUE;
}
  CAVDecoder _decoder;

VOID	RecdStreamReader::Run()
{
  AVResult      _avResult  = eAVUndefined;
  
  while ( !m_bExit )
  {
    FThread::YieldThread();
    
    m_mtxReading.EnterMutex();
    
    if (
	  ( m_bReading             == FALSE           ) &&
	  ( m_swStopReading.Peek()  > m_dStopWatchCMP ) 
       )
    {
      //Release mutex
      m_mtxReading.LeaveMutex();
      
      if ( m_pAvReader == NULL )
      {
	VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	FThread::Sleep(
			RecdConfig::GetInstance().GetReaderStandByDelay( m_sIpCamera, NULL )
		      );
      }
      else
      {
	LOG_INFO( "Close Reader", Run() )
	if ( m_pAvReader->close() != eAVSucceded )
	{
	  ERROR_INFO( "Failed to close encoder.", Run() )
	}
	
	//
	m_bGotKeyFrame = FALSE;
	
	delete m_pAvReader;
	m_pAvReader = NULL;
	
	// Signal to exit from SetReading()
	m_semStop.Post();
	FThread::YieldThread();
      }
    }
    else  
    {
      if ( m_pAvReader == NULL )
      {
	  LOG_INFO( "Allocate new CAVDecoder", Run() )
	  
	  m_pAvReader = new CAVDecoder();
	  if ( m_pAvReader != NULL )
	  {
	    if ( m_pAvReader->setDecoderEvents( this, false ) != eAVSucceded )
	    {
	      ERROR_INFO( "Failed to Set Events Handler", Run() )
	      
	      delete m_pAvReader;
	      m_pAvReader = NULL;
	      
	      m_mtxReading.LeaveMutex();
	      continue;
	    }
	  }
	  else
	  {
	    ERROR_INFO( "Not Enough Memory for allocating CAVDecoder()", Run() )
	    
	    m_mtxReading.LeaveMutex();
	    continue;
	  }
	  
	  FString _sURL = RecdConfig::GetInstance().GetReaderStream( m_sIpCamera, NULL );
	  DOUBLE  _dBuf = RecdConfig::GetInstance().GetReaderBufferingTime( NULL );
	  
	  LOG_INFO( FString( 0, "Opening URL[%s]  BUF[%f]", (const char*)_sURL, _dBuf ), Run() );
	  
	  _avResult = m_pAvReader->open( (const char*)_sURL, _dBuf, AV_SET_BEST_VIDEO_CODEC );
	  if ( _avResult != eAVSucceded )
	  {
	    ERROR_INFO( FString( 0, "Failed to Open URL=[%s] Res[%i]", (const char*)_sURL, _avResult ), Run() )
	    
	    delete m_pAvReader;

	    m_pAvReader = NULL;
	    m_mtxReading.LeaveMutex();
	    continue;
	  }
	  
	  _avResult = m_pAvReader->read( AVD_EXIT_ON_VIDEO_KEY_FRAME );
	  if ( _avResult != eAVSucceded )
	  {
	    ERROR_INFO( FString( 0, "Failed to Read from input stream Res[%i]", _avResult ) , Run() )
	    
	    delete m_pAvReader;
	    m_pAvReader = NULL;

	    m_mtxReading.LeaveMutex();
	    continue;
	  }
      }
      
      _avResult = m_pAvReader->read( AVD_EXIT_ON_NEXT_VIDEO_FRAME );
      if ( _avResult != eAVSucceded )
      {
	ERROR_INFO( FString( 0, "Failed to Read from input stream Res[%i]", _avResult ) , Run() )
	
	delete m_pAvReader;
	m_pAvReader = NULL;
      }
      
      //Release mutex
      m_mtxReading.LeaveMutex();
    } // if ( m_bRecording )
  }//while ( !m_bExit )

  if ( m_pAvReader != NULL )
  {
    if ( m_pAvReader->close() != eAVSucceded )
    {
      ERROR_INFO( "Failed to Close input stream.", Run() )
    }
    
    delete m_pAvReader;
    m_pAvReader = NULL;
  }
}

DWORD   RecdStreamReader::GetLogMessageFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsTraceExceptionEnabled  ( "Reader Log Message", NULL)?FLogMessage::MT_TRACE_EXCEPTION:0;
  _dwRetVal |= RecdConfig::GetInstance().IsCatchExceptionEnabled  ( "Reader Log Message", NULL)?FLogMessage::MT_CATCH_EXCEPTION:0;
  _dwRetVal |= RecdConfig::GetInstance().IsAssertionFailureEnabled( "Reader Log Message", NULL)?FLogMessage::MT_ASSERTION_FAILURE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsErrorInfoEnabled       ( "Reader Log Message", NULL)?FLogMessage::MT_ERROR_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLoggingInfoEnabled     ( "Reader Log Message", NULL)?FLogMessage::MT_LOGGING_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsVerboseInfoEnabled     ( "Reader Log Message", NULL)?FLogMessage::MT_VERBOSE_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsRawInfoEnabled         ( "Reader Log Message", NULL)?FLogMessage::MT_RAW_INFO:0;
  _dwRetVal |= RecdConfig::GetInstance().IsEnterMethodEnabled     ( "Reader Log Message", NULL)?FLogMessage::MT_ENTER_METHOD:0;
  _dwRetVal |= RecdConfig::GetInstance().IsExitMethodEnabled      ( "Reader Log Message", NULL)?FLogMessage::MT_EXIT_METHOD:0;

  return _dwRetVal;
}


DWORD   RecdStreamReader::GetVerbosityLevelFlags() const
{
  DWORD _dwRetVal = 0;

  _dwRetVal |= RecdConfig::GetInstance().IsStartUpMessageEnabled       ( "Reader Verbosity Level", NULL)?FLogMessage::VL_START_UP_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsShutDownMessageEnabled      ( "Reader Verbosity Level", NULL)?FLogMessage::VL_SHUT_DOWN_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsLowPeriodicMessageEnabled   ( "Reader Verbosity Level", NULL)?FLogMessage::VL_LOW_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsMediumPeriodicMessageEnabled( "Reader Verbosity Level", NULL)?FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE:0;
  _dwRetVal |= RecdConfig::GetInstance().IsHighPeriodicMessageEnabled  ( "Reader Verbosity Level", NULL)?FLogMessage::VL_HIGH_PERIODIC_MESSAGE:0;

  return _dwRetVal;
}
