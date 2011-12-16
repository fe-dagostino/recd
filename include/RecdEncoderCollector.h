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
  BOOL		StartRecording( const FString& sDestination );
  /**
   *  Deactivate recording on each stream reader. 
   */
  BOOL		StopRecording( );
  
  /**
   */
  FTList< RecdStreamEncoder* >*   GetEncoders()
  { return &m_lstEncoders; }
  
  RecdRenderEncoder* 		  GetRenderEncoder();
  
private:
  FTList< RecdStreamEncoder* >   m_lstEncoders;
  RecdRenderEncoder*             m_pRecdRenderEncoder;
};

#endif // RECDENCODERCOLLECTOR_H
