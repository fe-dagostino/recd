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
#include "avdecoder.h"
#include "avframe.h"


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
   * Read/Dequeue first frame.
   * @param bRemove  specify if the item must be removed ot not.
   * Return a pointer to an instance of CAVFrame, it bRemove was TRUE
   * returned value must be release with a call to ReleaseFrame.
   * NOTE: this method will be called from RecdReaderEncoder instance.
   */
  CAVFrame* 	 GetRawFrame( DWORD dwTimeout, BOOL bRemove );
  /**
   * Read/Dequeue first frame.
   * As for GetRawFrame but destinated to be used from RecdHighLightsEncoder instances.
   */   
  CAVFrame* 	 GetHighLightsFrame( DWORD dwTimeout, BOOL bRemove );
  /***/ 
  DWORD 	 GetRawMailboxSize() const;
  /***/
  DWORD 	 GetHighLightsMailboxSize() const;
  
  /**
   * Release specified pointer.
   * To be used for pointers returned both from GetRawFrame and GetHighLightsFrame
   */
  VOID 	 ReleaseFrame( CAVFrame*& pAVFrame );
   
  /**
   * Start/Stop Reading.
   */
  VOID 	 SetReading( BOOL bEnable );
   
// Implements IAVDecoderEvents interface  
protected:
  /**
   *  Event will be raised just in presence of one key frame.
   */
  virtual void   OnVideoKeyFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst );

  /**
   *  Event will be raised for each frame coming from the stream.
   *  Return value true in order to continue decoding, false to interrupt.
   *  Note: this event will be raised also for key frame.
   */
  virtual bool   OnVideoFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst );

  /**
    *  Event will be raised for each frame coming from the stream.
    *  Return value true in order to continue decoding, false to interrupt.
    */
  virtual bool   OnAudioFrame( const AVFrame* pAVFrame, const AVCodecContext* pAVCodecCtx, double pst );  
  
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
  FString                 m_sIpCamera;
  BOOL                    m_bExit;
  CAVDecoder* 		  m_pAvReader;
  FMutex                  m_mtxReading;
  BOOL                    m_bReading;
  FSemaphore              m_semStop;
  BOOL                    m_bGotKeyFrame;
  DOUBLE		  m_dStopWatchCMP; // Used in order to compare stop watch elapsed time.
  FStopWatch		  m_swStopReading;
  FTMailbox<CAVFrame* >*  m_pMbxRawFrames;
  FTMailbox<CAVFrame* >*  m_pMbxHighLightsFrames;
};

#endif // RECDSTREAMREADER_H
