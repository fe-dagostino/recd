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


#ifndef RECDSTREAMREADER_H
#define RECDSTREAMREADER_H

#include "FTMailbox.tlh"
#include "FStopWatch.h"
#include "LOGGING/FLogThread.h"
#include "RecdMbxItem.h"
#include "avdecoder.h"
#include "avframe.h"
#include "avfps.h"


USING_NAMESPACE_FED
USING_NAMESPACE_LOGGING

using namespace libavcpp;

/**
 *  
 */
class RecdStreamReader : public FLogThread, protected IAVDecoderEvents
{
  ENABLE_FRTTI( RecdStreamReader )
public:
  /**
   * Reader status enumeration.
   */
  enum ReaderStatus
  {
    eRSUndefined=0,  // Initial status no action required.
    eRSWaiting,      // Reader stay in waiting mode so input streaming is closed
    eRSOpenStream,   // 
    eRSBuffering,    // Input streaming has been opened and streaming will be buffered 
    eRSReading,
    eRSFlushing,
    eRSReleasing
  };
  
  
  /**
   *
   */
  RecdStreamReader( 
                    const FString& sStreamReaderName,
		    const FString& sIpCamera
		  );
  /**
   *
   */
  virtual ~RecdStreamReader();
  
  /**
   * Return the name of Camera section as for configuration file.
   */
  const FString& GetCameraName() const;
  
  /**
   * Read/Dequeue first item.
   * @param bRemove  specify if the item must be removed ot not.
   * Return a pointer to an instance of CAVFrame, it bRemove was TRUE
   * returned value must be release with a call to ReleaseFrame.
   * NOTE: this method will be called from RecdReaderEncoder instance.
   */
  RecdMbxItem*   PopRawItem( DWORD dwTimeout, BOOL bRemove );
  /**
   * Read/Dequeue first item.
   * As for PopRawItem but destinated to be used from RecdHighLightsEncoder instances.
   */   
  RecdMbxItem*   PopHighLightsItem( DWORD dwTimeout, BOOL bRemove );
  /***/ 
  BOOL           PushHighLightsItem( RecdMbxItem* pMbxItem );
  /***/ 
  DWORD          GetRawMailboxSize() const;
  /***/
  DWORD          GetHighLightsMailboxSize() const;
  
  /**
   * Release specified pointer.
   * To be used for pointers returned both from GetRawFrame and GetHighLightsFrame
   */
  VOID           ReleaseMbxItem( RecdMbxItem*& pMbxItem );
   
  /**
   * Return current status for the reader.
   * @param pdElapsed if valid and current status is equal to eRSBuffering or eRSFlushing
   * it will be initialized with amount of second elapsed.
   * @param pdTotal if valid will be initialized with the upper value of pdElapsed time.
   */
  enum ReaderStatus GetStatus( DOUBLE* pdElapsed, DOUBLE* pdTotal ) const;
  /***/  
  bool              SetStatus( ReaderStatus eStatus );
  
// Implements IAVDecoderEvents interface  
protected:
  /**
   *  Event will be raised just in presence of one key frame.
   */
  virtual void   OnVideoKeyFrame( const AVFrame* pAVFrame, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double pts );

  /**
   *  Event will be raised for each frame coming from the stream.
   *  Return value true in order to continue decoding, false to interrupt.
   *  Note: this event will be raised also for key frame.
   */
  virtual bool   OnVideoFrame( const AVFrame* pAVFrame, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double pts );

  /**
   *  Event will be raised for each frame coming from the stream.
   *  Return value true in order to continue decoding, false to interrupt.
   *  Note: this event will be raised for each frame.
   */
  virtual bool   OnFilteredVideoFrame( const AVFilterBufferRef* pAVFilterBufferRef, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double pts );
  
  /**
    *  Event will be raised for each frame coming from the stream.
    *  Return value true in order to continue decoding, false to interrupt.
    */
  virtual bool   OnAudioFrame( const AVFrame* pAVFrame, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double pts );  
  
// Implements virtual method defined in FThread  
protected:
  
  /* Defined in FThread class */
  virtual BOOL		Initial();

  /* Defined in FThread class */
  virtual BOOL		Final();

  /**/
  virtual VOID		Run();
  
private:
  /**/
  DWORD   	GetLogMessageFlags() const;
  /**/
  DWORD   	GetVerbosityLevelFlags() const;
  
private:
  mutable FMutex             m_mtxReader;
  ReaderStatus volatile      m_eReaderStatus;
  FString                    m_sIpCamera;
  BOOL                       m_bExit;
  CAVDecoder*                m_pAvReader;
  BOOL                       m_bGotKeyFrame;
  FStopWatch                 m_swFps;
  DWORD                      m_dwFpsCount;
  DOUBLE                     m_dStopWatchCMP; // Used in order to compare stop watch elapsed time.
  FStopWatch                 m_swStopReading;
  CAVFps                     m_fps;
  DWORD                      m_dwReaderMaxItems;
  BOOL                       m_bFilters;
  FTMailbox<RecdMbxItem* >*  m_pMbxRawItems;
  FTMailbox<RecdMbxItem* >*  m_pMbxHighLightsItems;
};

#endif // RECDSTREAMREADER_H
