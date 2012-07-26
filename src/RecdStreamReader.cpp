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
#include "avfiltergraph.h"


GENERATE_CLASSINFO( RecdStreamReader, FLogThread ) 

RecdStreamReader::RecdStreamReader( 
				    const FString& sStreamReaderName,
				    const FString& sIpCamera
				  )
  : FLogThread( sStreamReaderName, NULL, FThread::TP_HIGHEST, -1 ),
   m_eReaderStatus( eRSUndefined ),
   m_sIpCamera( sIpCamera ),
   m_bExit( FALSE ),
   m_pAvReader( NULL ),
   m_bGotKeyFrame( FALSE ),
   m_dStopWatchCMP( 0.0 ),
   m_dwReaderMaxItems( 0 ),
   m_pMbxRawItems( NULL ),
   m_pMbxHighLightsItems( NULL )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );  
  
  m_pMbxRawItems = new FTMailbox<RecdMbxItem* >( "Mailbox Decoding Frames", NULL );
  if ( m_pMbxRawItems == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Mailbox", RecdStreamReader() )
    //@todo
  }

  m_pMbxHighLightsItems = new FTMailbox<RecdMbxItem* >( "Mailbox Decoding Frames", NULL );
  if ( m_pMbxHighLightsItems == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Mailbox", RecdStreamReader() )
    //@todo
  }

  // Initialize stop watch.
  m_swStopReading.Reset();
}

RecdStreamReader::~RecdStreamReader()
{
  if ( m_pMbxRawItems != NULL )
  {
    LOG_INFO( "Release mailbox resources", ~RecdStreamReader() )
    // Delete all allocated items
    while ( !m_pMbxRawItems->IsEmpty() )
    {
      RecdMbxItem* pFrame = m_pMbxRawItems->Read();
      delete pFrame;
    }
    
    // Release mailbox.
    delete m_pMbxRawItems;
    m_pMbxRawItems = NULL;
  }
  
  if ( m_pMbxHighLightsItems != NULL )
  {
    LOG_INFO( "Release mailbox resources", ~RecdStreamReader() )
    // Delete all allocated items
    while ( !m_pMbxHighLightsItems->IsEmpty() )
    {
      RecdMbxItem* pFrame = m_pMbxHighLightsItems->Read();
      delete pFrame;
    }
    
    // Release mailbox.
    delete m_pMbxHighLightsItems;
    m_pMbxHighLightsItems = NULL;
  }
}

const FString&  RecdStreamReader::GetCameraName() const
{
  return m_sIpCamera;
}
  
RecdMbxItem* 	RecdStreamReader::PopRawItem( DWORD dwTimeout, BOOL bRemove )
{
  if ( m_pMbxRawItems == NULL )
    return NULL;
  
  return m_pMbxRawItems->Read( dwTimeout, bRemove );
}

RecdMbxItem* 	RecdStreamReader::PopHighLightsItem( DWORD dwTimeout, BOOL bRemove )
{
  if ( m_pMbxHighLightsItems == NULL )
    return NULL;
  
  return m_pMbxHighLightsItems->Read( dwTimeout, bRemove );
}

BOOL            RecdStreamReader::PushHighLightsItem( RecdMbxItem* pMbxItem )
{
  if ( m_pMbxHighLightsItems == NULL )
    return FALSE;
  
  m_pMbxHighLightsItems->Write( pMbxItem );
  
  return TRUE;
}

DWORD 	 	RecdStreamReader::GetRawMailboxSize() const
{
  return m_pMbxRawItems->GetSize();
}

DWORD 	 	RecdStreamReader::GetHighLightsMailboxSize() const
{
  return m_pMbxHighLightsItems->GetSize();
}

VOID 	 	RecdStreamReader::ReleaseMbxItem( RecdMbxItem*& pMbxItem )
{
  if ( pMbxItem == NULL )
    return ;
  
  delete pMbxItem;
  pMbxItem = NULL;
}

enum RecdStreamReader::ReaderStatus RecdStreamReader::GetStatus( DOUBLE* pdElapsed, DOUBLE* pdTotal ) const
{
  FMutexCtrl  _mtxCtrl( m_mtxReader );

  if ( pdElapsed != NULL )
  {
    *pdElapsed = 0.0;
    if (
	( m_eReaderStatus == eRSBuffering ) ||
	( m_eReaderStatus == eRSFlushing  )
      )
    {
      *pdElapsed = m_swStopReading.Peek();
      if ( *pdElapsed > m_dStopWatchCMP )
	*pdElapsed = m_dStopWatchCMP;

      if ( pdTotal != NULL )
	*pdTotal = m_dStopWatchCMP;
    }
  }
  
  VERBOSE_INFO( 
		FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE, 
		FString( 0, "Current Readers Status [%d][%.2f][%.2f]", m_eReaderStatus, m_swStopReading.Peek(), m_dStopWatchCMP ), 
	        GetStatus() 
	      )

  return m_eReaderStatus;
}

bool              RecdStreamReader::SetStatus( ReaderStatus eStatus )
{
  FMutexCtrl  _mtxCtrl( m_mtxReader );

  if (
       ( eStatus == eRSOpenStream ) ||
       ( eStatus == eRSFlushing   )
     )
  {
    m_dStopWatchCMP = RecdConfig::GetInstance().GetReaderBufferingTime( NULL );
    m_swStopReading.Reset();
  }
  
  m_eReaderStatus = eStatus;
  
  return true;
}

void   RecdStreamReader::OnVideoKeyFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst )
{
  m_bGotKeyFrame = TRUE;
  
  if ( m_bGotKeyFrame )
  {
    m_fps.init( 
		1/av_q2d(pAVCodecCtx->time_base),
		(DOUBLE)RecdConfig::GetInstance().GetReaderFpsLimits( GetCameraName(), NULL )
	      );
  }
}

bool   RecdStreamReader::OnVideoFrame( 
					const  AVFrame* pAVFrame,
					const  AVCodecContext* pAVCodecCtx, 
					double pst 
				     )
{
  // If filters has been enabled all operation must be performed in OnFilteredVideoFrame() event
  if ( m_bFilters == TRUE )
    return true;
  
  // Video processing will be avoided until the first Key frame will be received.
  if ( m_bGotKeyFrame == FALSE )
    return true;
  
  // Check if current frame must be delete.
  // This condition could happens when source fps greater than whished fps
  if ( m_fps.bDrop() )
  {
    m_fps.reset();
    
    VERBOSE_INFO( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE, "Drop Current Frame ..", Run() )

    return true;    
  }

  if ( m_pMbxRawItems->GetSize()       <= m_dwReaderMaxItems )
  {
    CAVImage*  _pAVRawImage      = new CAVImage();
    
    // Initialized to original W:H
    _pAVRawImage->init( 
	    pAVFrame, 
	    pAVCodecCtx,
	    -1,
	    -1, 
	    PIX_FMT_RGBA, 
	    RecdConfig::GetInstance().GetReaderRescaleOptions( m_sIpCamera, NULL ) 
	);

    m_pMbxRawItems->Write       ( new RecdMbxItem( _pAVRawImage      ) );
  }
    
  if ( m_pMbxHighLightsItems->GetSize() <= m_dwReaderMaxItems )
  {
    CAVImage* _pHighLightsImage = new CAVImage();
    BOOL      _bBkgStatus       = RecdConfig::GetInstance().GetHighLightsBackgroundStatus( m_sIpCamera, NULL );

    // Initialized to HIGHLIGHTS VIDEO SETTINGS W:H when background is not active
    // Initialized to HIGHLIGHTS RECT           W:H when background is active
    _pHighLightsImage->init( 
	    pAVFrame, 
	    pAVCodecCtx,
	    _bBkgStatus?RecdConfig::GetInstance().GetHighLightsRectWidth( m_sIpCamera, NULL ):RecdConfig::GetInstance().GetHighLightsEncoderWidth( m_sIpCamera, NULL ), 
	    _bBkgStatus?RecdConfig::GetInstance().GetHighLightsRectHeight( m_sIpCamera, NULL ):RecdConfig::GetInstance().GetHighLightsEncoderHeight( m_sIpCamera, NULL ), 
	    _bBkgStatus?PIX_FMT_RGBA:PIX_FMT_YUV420P,
	    RecdConfig::GetInstance().GetReaderRescaleOptions( m_sIpCamera, NULL )
	);
    m_pMbxHighLightsItems->Write( new RecdMbxItem( _pHighLightsImage ) );
  }       
    
  VERBOSE_INFO( 
		FLogMessage::VL_HIGH_PERIODIC_MESSAGE, 
		FString( 0, "Mailboxes RAW Size=[%u] HL Size=[%u]", m_pMbxRawItems->GetSize(), m_pMbxHighLightsItems->GetSize() ),
		OnVideoFrame() 
	      )
  
  
  return true;
}

bool    RecdStreamReader::OnFilteredVideoFrame( const AVFilterBufferRef* pAVFilterBufferRef, const AVCodecContext* pAVCodecCtx, double pst )
{
  // Video processing will be avoided until the first Key frame will be received.
  if ( m_bGotKeyFrame == FALSE )
    return true;
    
  // Check if current frame must be delete.
  // This condition could happens when source fps greater than whished fps
  if ( m_fps.bDrop() )
  {
    m_fps.reset();
    
    VERBOSE_INFO( FLogMessage::VL_MEDIUM_PERIODIC_MESSAGE, "Drop Current Frame ..", Run() )

    return true;    
  }
  
  if ( m_pMbxRawItems->GetSize()       <= m_dwReaderMaxItems )
  {
    CAVImage*  _pAVRawImage      = new CAVImage();
    
    // Initialized to original W:H
    _pAVRawImage->init( pAVFilterBufferRef, pAVCodecCtx->width, pAVCodecCtx->height ); 

    m_pMbxRawItems->Write       ( new RecdMbxItem( _pAVRawImage      ) );
  }
  
  if ( m_pMbxHighLightsItems->GetSize() <= m_dwReaderMaxItems )
  {
    CAVImage* _pHighLightsImage = new CAVImage();
    BOOL      _bBkgStatus       = RecdConfig::GetInstance().GetHighLightsBackgroundStatus( m_sIpCamera, NULL );

    // Initialized to HIGHLIGHTS VIDEO SETTINGS W:H when background is not active
    // Initialized to HIGHLIGHTS RECT           W:H when background is active
    _pHighLightsImage->init( 
	    pAVFilterBufferRef, 
	    pAVCodecCtx->width, pAVCodecCtx->height,
	    _bBkgStatus?RecdConfig::GetInstance().GetHighLightsRectWidth( m_sIpCamera, NULL ):RecdConfig::GetInstance().GetHighLightsEncoderWidth( m_sIpCamera, NULL ), 
	    _bBkgStatus?RecdConfig::GetInstance().GetHighLightsRectHeight( m_sIpCamera, NULL ):RecdConfig::GetInstance().GetHighLightsEncoderHeight( m_sIpCamera, NULL ), 
	    _bBkgStatus?PIX_FMT_RGBA:PIX_FMT_YUV420P,
	    RecdConfig::GetInstance().GetReaderRescaleOptions( m_sIpCamera, NULL )
	);
    m_pMbxHighLightsItems->Write( new RecdMbxItem( _pHighLightsImage ) );
  }
  
  return true;
}

bool    RecdStreamReader::OnAudioFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst )
{
  if ( m_pMbxRawItems->GetSize()        <= m_dwReaderMaxItems )
  {
    CAVSample* pRawSample = new CAVSample();
    
    pRawSample->init(pAVFrame, pAVCodecCtx);
    
    m_pMbxRawItems->Write       ( new RecdMbxItem( pRawSample ) );
  }
    
  if ( m_pMbxHighLightsItems->GetSize() <= m_dwReaderMaxItems )
  {
    CAVSample* pHLSample  = new CAVSample();
    
    pHLSample->init (pAVFrame, pAVCodecCtx);
    
    m_pMbxHighLightsItems->Write( new RecdMbxItem( pHLSample  ) );
  }  
  
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
  
VOID	RecdStreamReader::Run()
{
  AVResult      _avResult         = eAVUndefined;
  DWORD         _dwStandbyTimeout = RecdConfig::GetInstance().GetReaderStandByDelay( m_sIpCamera, NULL );
  // Update status to waiting mode.
  SetStatus( eRSWaiting );
  
  while ( !m_bExit )
  {
    FThread::YieldThread();

    switch ( GetStatus( NULL, NULL ) )
    {
      case eRSWaiting:
      {
	VERBOSE_INFO( FLogMessage::VL_HIGH_PERIODIC_MESSAGE, "Recording Disabled ", Run() )
	FThread::Sleep( _dwStandbyTimeout );
      }; break;
      
      case eRSOpenStream:
      {
	LOG_INFO( "Dispatching Start Encoding Command", Run() )

	// Writing messages for consumers
	m_pMbxRawItems->Write       ( new RecdMbxItem( RecdMbxItem::eCmdStartEncoding ) );

	// Keep value into member variable in order to avoid continue conversions.
	m_dwReaderMaxItems = RecdConfig::GetInstance().GetReaderMaxItems( m_sIpCamera, NULL );
	
	// TRUE if filters has been enabled False otherwise.
	m_bFilters         = RecdConfig::GetInstance().GetReaderFiltersStatus( m_sIpCamera, NULL );
	
	CAVFilterGraph* _pAVFilterGraph = NULL;
	if ( m_bFilters == TRUE )
	{
	  FString _sSectionName = RecdConfig::GetInstance().GetReaderFiltersSettings( m_sIpCamera, NULL );
	  FString _sFiltersConf = RecdConfig::GetInstance().GetReaderFiltersConfiguration( _sSectionName, NULL );
	  
	  LOG_INFO( FString( 0, "FilterGraph Setting = [%s]", (const char*)_sFiltersConf ), Run() )
	  _pAVFilterGraph =  new CAVFilterGraph( _sFiltersConf );
	}
	
	LOG_INFO( "Allocate new CAVDecoder", Run() )
	  
	m_pAvReader = new CAVDecoder();
	if ( m_pAvReader == NULL )
	{
	  ERROR_INFO( "Not Enough Memory for allocating CAVDecoder()", Run() )
	  
	  SetStatus( eRSReleasing );
	  break;
	}

	if ( m_pAvReader->setDecoderEvents( this, false ) != eAVSucceded )
	{
	  ERROR_INFO( "Failed to Set Events Handler", Run() )

	  SetStatus( eRSReleasing );
	  break;
	}

	if ( _pAVFilterGraph )
	{
	  LOG_INFO( "Enabling Reader FilterGraph", Run() )
	  
	  m_pAvReader->setFilterGraph( _pAVFilterGraph );
	}
		
	FString _sURL = RecdConfig::GetInstance().GetReaderStream( m_sIpCamera, NULL );
	DOUBLE  _dBuf = RecdConfig::GetInstance().GetReaderBufferingTime( NULL );

	LOG_INFO( FString( 0, "Opening URL[%s]  BUF[%f]", (const char*)_sURL, _dBuf ), Run() );
	  
	_avResult = m_pAvReader->open( (const char*)_sURL, _dBuf, AV_SET_BEST_VIDEO_CODEC );
	if ( _avResult != eAVSucceded )
	{
	  ERROR_INFO( FString( 0, "Failed to Open URL=[%s] Res[%i]", (const char*)_sURL, _avResult ), Run() )
	  
	  SetStatus( eRSReleasing );
	  break;
	}
	  
	_avResult = m_pAvReader->read( AVD_EXIT_ON_VIDEO_KEY_FRAME|AVD_EXIT_ON_BUFFERING );
	if ( _avResult == eAVBuffering )
	{
	  SetStatus( eRSBuffering );
	} 
	else if ( _avResult == eAVSucceded )
	{
	  // Reset stop watch in order to avoid delay introduced by opening the stream.
	  m_swStopReading.Peek();

	  SetStatus( eRSReading   );
	}
	
	if ( 
	     ( _avResult != eAVSucceded  ) &&
	     ( _avResult != eAVBuffering )
	   )
	{
	  ERROR_INFO( FString( 0, "Failed to Read from input stream Res[%i]", _avResult ) , Run() )

	  SetStatus( eRSReleasing );
	  break;
	}
      }; break;
      
      case eRSBuffering:
      {
	_avResult = m_pAvReader->read( AVD_EXIT_ON_NEXT_VIDEO_FRAME|AVD_EXIT_ON_NEXT_AUDIO_FRAME );
	if ( _avResult == eAVSucceded )
	{
	  
	  SetStatus( eRSReading );
	}
	else if ( _avResult != eAVBuffering )
	{
	  ERROR_INFO( FString( 0, "Failed during buffering input stream Res[%i]", _avResult ) , Run() )

	  SetStatus( eRSReleasing );
	}
      }; break;	
      case eRSReading:
      {
	_avResult = m_pAvReader->read( AVD_EXIT_ON_NEXT_VIDEO_FRAME|AVD_EXIT_ON_NEXT_AUDIO_FRAME );
	if ( _avResult != eAVSucceded  ) 
	{
	  ERROR_INFO( FString( 0, "Failed to Read from input stream Res[%i]", _avResult ) , Run() )

	  SetStatus( eRSReleasing );
	}
      }; break;
      
      // This is not a real flush; reader just dalay close accordingly with configuration and
      // then move to next state.
      case eRSFlushing:
      {
	// Delay time before starting reading
	if ( m_swStopReading.Peek() < m_dStopWatchCMP )
	{
	  _avResult = m_pAvReader->read( AVD_EXIT_ON_NEXT_VIDEO_FRAME|AVD_EXIT_ON_NEXT_AUDIO_FRAME );
	  if ( _avResult != eAVSucceded )
	  {
	    SetStatus( eRSReleasing );
	  }
	  
	  break;
	}

	// Move to next state in order to release allocated object.
	SetStatus( eRSReleasing );
      }; break;
	
      case eRSReleasing:
      {	
	LOG_INFO( "Dispatching Stop Encoding Command", Run() )
	// Writing messages for consumers
	// Event there is an error on reading or it will be terminated by user request
	// we will dispatch Stop Encoding commando to each consumer.
	m_pMbxRawItems->Write       ( new RecdMbxItem( RecdMbxItem::eCmdStopEncoding ) );
	// Message will be dispatched also to Highlight thread in order to force closing
	// encoder.
	m_pMbxHighLightsItems->Write( new RecdMbxItem( RecdMbxItem::eCmdStopEncoding ) );
	
	LOG_INFO( "Close and Release CAVDecoder instance", Run() )

	if ( m_pAvReader != NULL )
	{
	  if ( m_pAvReader->close() != eAVSucceded )
	  {
	    ERROR_INFO( "Failed to close encoder.", Run() )
	  }
	  
	  delete m_pAvReader;
	  m_pAvReader = NULL;
	}
	
	// Reset key fram waiting
	m_bGotKeyFrame = FALSE;
	
	// Update status to waiting mode.
	SetStatus( eRSWaiting );
      }; break;
      
    };
 
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
