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


#ifndef RECDREADERCOLLECTOR_H
#define RECDREADERCOLLECTOR_H

#include "FSingleton.h"
#include "FTList.tlh"
#include "RecdStreamReader.h"

USING_NAMESPACE_FED

/**
 * This is a singleton object manage readers.
 */
class RecdReaderCollector : public FSingleton
{
  ENABLE_FRTTI( RecdReaderCollector )
  DECLARE_SINGLETON( RecdReaderCollector )
protected:
    
  /***/
  VOID         OnInitialize();
  /***/
  VOID         OnFinalize();

public:
  /**
   * Retrieve stream reader instance for the specifed camera.
   */
  RecdStreamReader& 	GetStreamReader( const FString& sCamera );
  
  /**
   * Activate/Deactivate reading on each camera. 
   */
  BOOL			SetReading( BOOL bEnable );
  
  /**
   * Return percentage for reading buffers.
   * @param eStatus if eRSBuffering function will return TRUE only when all readers are fully buffered and
   *                currently in reading status.
   *                if eRSFlushing  function will return TRUE only when all readers are fully flushed and
   *                currently status is waiting.
   * @param pdPercentage could be NULL. If valid will be updated with current buffer percentage. 
   */
  BOOL                  GetBuffers( RecdStreamReader::ReaderStatus eStatus, DOUBLE* pdPercentage ) const;

  /**
   *  Check if all readers are in the specified status.
   */
  BOOL                  CheckStatus( RecdStreamReader::ReaderStatus eStatus ) const;

  /**
   */
  BOOL                  IsEnabled() const;
  
private:
  mutable FMutex                m_mtxReaders;
  FTList< RecdStreamReader* >   m_lstReaders;
  BOOL                          m_bEnabled;
};

#endif // RECDREADERCOLLECTOR_H
