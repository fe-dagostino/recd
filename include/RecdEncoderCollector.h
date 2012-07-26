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


#ifndef RECDENCODERCOLLECTOR_H
#define RECDENCODERCOLLECTOR_H

#include "FSingleton.h"
#include "FTList.tlh"
#include "RecdStreamEncoder.h"
#include "RecdHighLightsEncoder.h"
#include "RecdRenderEncoder.h"

USING_NAMESPACE_FED

/**
 * This is a singleton object manage encoders.
 */
class RecdEncoderCollector : public FSingleton
{
  ENABLE_FRTTI( RecdEncoderCollector )
  DECLARE_SINGLETON( RecdEncoderCollector )
protected:
    
  /***/
  VOID         OnInitialize();
  /***/
  VOID         OnFinalize();

public:
  
  /**
   *  Activate recording on each stream reader. 
   */
  BOOL		SetParameters( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw );
  
  /**
   *  Start HighLights on specified camera. 
   *  @param sCamera   must match with configuration value.
   *                   Could be also "ALL" and HighLights will
   *                   be starts on all cameras.
   *  Return TRUE in case of success, FALSE otherwise.
   */
  BOOL          StartHighLights( const FString& sCamera );
  
  /**
   *  Check if all encoders are ready for a new recording.
   *  Return TRUE if all encoders are ready, FALSE otherwise.
   */
  BOOL		ReadyForRecording() const;
  
  /**
   */
  inline FTList< RecdStreamEncoder* >*       GetRawEncoders()
  { return &m_lstEncoders; }
  /**
   */
  inline FTList< RecdHighLightsEncoder* >*   GetHighLightsEncoders()
  { return &m_lstHighLightsEncoders; }
  
  inline RecdRenderEncoder* 		     GetRenderEncoder()
  { return m_pRecdRenderEncoder; }
  
private:
  FTList< RecdStreamEncoder*     >   m_lstEncoders;
  FTList< RecdHighLightsEncoder* >   m_lstHighLightsEncoders;
  RecdRenderEncoder*                 m_pRecdRenderEncoder;
};

#endif // RECDENCODERCOLLECTOR_H
