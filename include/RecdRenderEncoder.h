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
  VOID			StartRecording( const FString& sDestination );
  /**
   *  Deactivate recording on stream reader. 
   */
  VOID			StopRecording();
    
  
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
  BOOL            m_bExit;
  FMutex          m_mtxRecording;
  BOOL            m_bRecording;
  CAVEncoder*     m_pAVEncoder;
  FSemaphore      m_semStop;  
  FString         m_sDestination;
  CAVImage        m_rgbaBkg;
  FStopWatch      m_swFPS;
};

#endif // RECDSTREAMENCODER_H
