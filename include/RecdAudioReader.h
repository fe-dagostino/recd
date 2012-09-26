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


#ifndef RECDAUDIOREADER_H
#define RECDAUDIOREADER_H

#include "FTMailbox.tlh"
#include "FStopWatch.h"
#include "LOGGING/FLogThread.h"
#include "RecdMbxItem.h"
#include "avdecoder.h"
#include "avframe.h"


USING_NAMESPACE_FED
USING_NAMESPACE_LOGGING

using namespace libavcpp;

/**
 *  
 */
class RecdAudioReader : public FLogThread
{
  ENABLE_FRTTI( RecdAudioReader )
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
  RecdAudioReader( const FString& sStreamReaderName );
  /**
   *
   */
  virtual ~RecdAudioReader();


  /**
   * Return current status for the reader.
   */
  enum ReaderStatus GetStatus( DOUBLE* pdElapsed, DOUBLE* pdTotal ) const;
  /***/  
  bool              SetStatus( ReaderStatus eStatus );
  
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
  BOOL                       m_bExit;

};

#endif // RECDAUDIOREADER_H
