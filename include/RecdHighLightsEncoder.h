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


#ifndef RECDHIGHLIGHTSENCODER_H
#define RECDHIGHLIGHTSENCODER_H

#include "LOGGING/FLogThread.h"
#include "RecdStreamReader.h"
#include "avencoder.h"

USING_NAMESPACE_FED
USING_NAMESPACE_LOGGING

/**
 * This is the consumer for RecdStreamReader.
 * Each instance of RecdHighLightsEncoder is intended in order to 
 * read datas from RecdStreamReader and to encode an output video.
 */
class RecdHighLightsEncoder : public FLogThread
{
  ENABLE_FRTTI( RecdHighLightsEncoder )
public:
  /**
   * 
   */
  RecdHighLightsEncoder( 
		      const FString&    sHighLightsEncoderName,
		      RecdStreamReader& rStreamReader
		   );
  /**
   * 
   */
  virtual ~RecdHighLightsEncoder();
  
  /**
   *  Activate recording on stream reader. 
   */
  VOID			StartRecording( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw );
  /**
   *  Deactivate recording on stream reader. 
   */
  VOID			StopRecording();
    
  /**
   *  Start writing Highlight. 
   *  If called before the end of first Highlight stopwatch will be
   *  reinitialized with the effect to have the final render longer
   *  than buffering time specified in configuration.
   */
  BOOL			StartHighLights();
  
  /***/
  const RecdStreamReader&    GetStreamReader() const
  { return m_rStreamReader; }
  
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
  BOOL                m_bExit;
  FMutex              m_mtxRecording;  
  BOOL                m_bRecording;
  BOOL                m_bHighlights;
  CAVEncoder*         m_pAVEncoder;
  FSemaphore          m_semStop;
  FString             m_sDestination;
  CAVImage            m_rgbaBkg;
  RecdStreamReader&   m_rStreamReader;
  FStopWatch          m_swFPS;
  FStopWatch          m_swHighLights;
};

#endif // RECDSTREAMENCODER_H