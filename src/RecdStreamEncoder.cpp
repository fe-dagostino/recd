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
#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

GENERATE_CLASSINFO( RecdStreamEncoder, FLogThread ) 

RecdStreamEncoder::RecdStreamEncoder( 
				      const FString&    sStreamEncoderName,
				      RecdStreamReader& rStreamReader
				    )
  : FLogThread( sStreamEncoderName, NULL, FThread::TP_CRITICAL, -1 ),
    m_bExit( FALSE ),
    m_bRecording( FALSE ),
    m_pAVEncoder( NULL ),
    m_rStreamReader( rStreamReader )
{
  (GET_LOG_MAILBOX())->SetLogMessageFlags( GetLogMessageFlags()     );
  (GET_LOG_MAILBOX())->SetVerbosityFlags ( GetVerbosityLevelFlags() );
  
  m_pMbxFrames = new FTMailbox<CAVImage* >( "Mailbox Scaled Frames", NULL );
  if ( m_pMbxFrames == NULL )
  {
    ERROR_INFO( "Not Enough Memory for allocating Mailbox", RecdStreamReader() )
    //@todo
  }

}

RecdStreamEncoder::~RecdStreamEncoder()
{
  if ( m_pMbxFrames != NULL )
  {
    LOG_INFO( "Release mailbox resources", ~RecdStreamReader() )
    // Delete all allocated items
    while ( !m_pMbxFrames->IsEmpty() )
    {
      CAVImage* pFrame = m_pMbxFrames->Read();
      delete pFrame;
    }
    
    // Release mailbox.
    delete m_pMbxFrames;
    m_pMbxFrames = NULL;
  }
}

CAVImage*  RecdStreamEncoder::GetRenderFrame( DWORD dwTimeout, BOOL bRemove )
{
  if ( m_pMbxFrames == NULL )
    return NULL;
  
  return m_pMbxFrames->Read( dwTimeout, bRemove );
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
  		  RecdConfig::GetInstance().GetRenderRectX( m_rStreamReader.GetCameraName(), NULL ), 
		  RecdConfig::GetInstance().GetRenderRectY( m_rStreamReader.GetCameraName(), NULL ), 
		  RecdConfig::GetInstance().GetRenderRectWidth( m_rStreamReader.GetCameraName(), NULL ), 
		  RecdConfig::GetInstance().GetRenderRectHeight( m_rStreamReader.GetCameraName(), NULL )
                );
}

VOID   RecdStreamEncoder::ReleaseRenderFrame( CAVImage*& pImage )
{
  if ( pImage == NULL )
    return ;
  
  delete pImage;
  pImage = NULL;
}


VOID   RecdStreamEncoder::StartRecording( const FString& sDestination )
{
  FMutexCtrl _mtxCtrl( m_mtxRecording );
  
  m_sDestination = sDestination;
  m_bRecording   = TRUE;
}

VOID   RecdStreamEncoder::StopRecording()
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
			RecdConfig::GetInstance().GetEncoderStandByDelay( m_rStreamReader.GetCameraName(), NULL )
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
    }
    else  
    {
      CAVImage* _pFrame = m_rStreamReader.GetFrame(
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
      
      if ( m_pAVEncoder == NULL )
      {
	FString _sCameraName  = m_rStreamReader.GetCameraName();
	
	FString _sOutFilename( 0, "%s/%s_%05d.mp4", (const char*)m_sDestination, (const char*)_sCameraName, 1 ); 
	
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
	
	int _iWidth  = RecdConfig::GetInstance().GetEncoderWidth ( _sCameraName, NULL );
	int _iHeight = RecdConfig::GetInstance().GetEncoderHeight( _sCameraName, NULL );
	if (_iWidth  == -1 )
	  _iWidth = _pFrame->getWidth();
	
	if (_iHeight == -1 )
	  _iHeight = _pFrame->getHeight();
	
	LOG_INFO( FString( 0, "CAM=[%s] OUT=[%s]", (const char*)_sCameraName, (const char*)_sOutFilename ), Run() )
	AVResult _avRes = m_pAVEncoder->open( 
					    _sOutFilename,
					    0,
					    _iWidth,
					    _iHeight,
					    RecdConfig::GetInstance().GetEncoderFps       ( _sCameraName, NULL ),
					    RecdConfig::GetInstance().GetEncoderGoP       ( _sCameraName, NULL ),
					    RecdConfig::GetInstance().GetEncoderBitRate   ( _sCameraName, NULL ),
					    (CodecID)RecdConfig::GetInstance().GetEncoderVideoCodec( _sCameraName, NULL )
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
	
	_dFPS =  1.0 / (double)RecdConfig::GetInstance().GetEncoderFps( _sCameraName, NULL );
      } //if ( _pAVEncoder == NULL )
      
      m_swFPS.Reset();
      
      /**
       * Each frame retrieved from reader will be resized for the final render.
       * The reason why this part of code resides here instead of Render thread 
       * is to better balance cpu(s) allocation time at kenel level and so to 
       * distribute complexity.
       */
      if ( m_pMbxFrames->GetSize() >= RecdConfig::GetInstance().GetEncoderMaxItems( m_rStreamReader.GetCameraName(), NULL ) )
      {
	ERROR_INFO( FString( 0, "Consumer Thread (RENDER) is TOO SLOW queue is full SIZE[%d].", m_pMbxFrames->GetSize() ), Run() )
      }
      else
      {
	  CAVImage* pAVImage = new CAVImage();
	  pAVImage->init( *_pFrame, 
			  RecdConfig::GetInstance().GetRenderRectWidth( m_rStreamReader.GetCameraName(), NULL ), 
			  RecdConfig::GetInstance().GetRenderRectHeight( m_rStreamReader.GetCameraName(), NULL ), 
			  PIX_FMT_RGB24,
			  RecdConfig::GetInstance().GetEncoderRescaleOptions( m_rStreamReader.GetCameraName(), NULL ) 
			);
	  m_pMbxFrames->Write( pAVImage );
      }
     
      if ( m_pAVEncoder->write( _pFrame->getFrame(), AVMEDIA_TYPE_VIDEO ) != eAVSucceded )
      {
	  ERROR_INFO( "Failed to encode frame.", Run() )
	  
	  delete m_pAVEncoder;
	  m_pAVEncoder = NULL;
      }
      
      // Release memory.
      m_rStreamReader.ReleaseFrame( _pFrame );
      //Release mutex
      m_mtxRecording.LeaveMutex();
      
      while ( m_swFPS.Peek() < _dFPS )
	FThread::YieldThread();
      
    }// if ( m_bRecording == FALSE )
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



