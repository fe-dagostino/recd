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


#ifndef RECDRENDERENCODER_H
#define RECDRENDERENCODER_H

#include "LOGGING/FLogThread.h"
#include "RecdStreamEncoder.h"

USING_NAMESPACE_FED
USING_NAMESPACE_LOGGING


class RenderThreads;

  enum RenderThreadStatus
  {
    eRTUndefined,    // Initial status no action required.
    eRTWaiting,      // Reader stay in waiting mode so input streaming is closed
    eRTOpenStream,   // 
    eRTRendering,
    eRTReleasing
  };

/**
 * This is the consumer for RecdStreamEncoder(s).
 * One single instance of will read datas from all RecdStreamEncoder is otder to 
 * render all video frames in a single output video and to encode it.
 *
 * NOTE: In order to optimize encoding and to better distribute cpu(s) allocation
 *       resizing of Images frame as require from final render will be performed on
 *       RecdStreamEncoder thread.
 */
class RecdRenderEncoder : public FLogThread
{
  ENABLE_FRTTI( RecdRenderEncoder )
public:
  
  /**
   * High Lights Encoder Status status enumeration.
   */
  enum RenderEncoderStatus
  {
    eREUndefined,    // Initial status no action required.
    eREWaiting,      // Reader stay in waiting mode so input streaming is closed
    eREOpenStream,   // 
    eREEncoding,
    eREReleasing
  };  
  
  /**
   * 
   */
  RecdRenderEncoder( const FString& sRenderEncoderName );
  /**
   * 
   */
  virtual ~RecdRenderEncoder();
  
  /**
   *  Activate recording on stream reader. 
   */
  VOID                     SetParameters( const FString& sDestination );
  /***/
  BOOL                     Blend( const CAVPoint& rPos, const CAVImage& rFrame, const CAVColor& crKeyColor );
  
  /***/
  enum RenderEncoderStatus GetStatus() const;
  
private:  
  /***/
  bool                     SetStatus( RenderEncoderStatus eStatus );
  
// Implements virtual method defined in FThread  
protected:
  
  /* Defined in FThread class */
  virtual BOOL		Initial();

  /* Defined in FThread class */
  virtual BOOL		Final();

  /**/
  virtual VOID		Run();


private:
  /**
   */
  BOOL		WriteFrame( CAVImage& avFrameYUV );
  /**
   * Return TRUE if all Rendering Threads are in the specified status.
   */
  BOOL          CheckStatus( enum RenderThreadStatus  eStatus );

  /**
   * Put specified command in each rendering thread.
   */
  VOID          PostCommand( RecdMbxItem::MbxCommandType eCommand );
private:
  /**/
  DWORD   	GetLogMessageFlags() const;
  /**/
  DWORD   	GetVerbosityLevelFlags() const;
  
private:
  BOOL                 m_bExit;
  mutable FMutex       m_mtxEncoder;
  RenderEncoderStatus  m_eEncoderStatus;
  CAVEncoder*          m_pAVEncoder;
  FString              m_sDestination;
  CAVImage             m_rgbaBkg;
  CAVImage             m_rgbaMaskBkg;
  FStopWatch           m_swFPS;
  RenderThreads**      m_pRenderThreads;
};

#endif // RECDSTREAMENCODER_H
