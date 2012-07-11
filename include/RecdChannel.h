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


#ifndef RECD_CHANNEL_H
#define RECD_CHANNEL_H

#include "FChannel.h"
#include "RCI/FRciParser.h"
#include "RCI/IRciCommand.h"
#include "RecdResult.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 */
class RecdChannel : public FChannel, public IChannelReaderEvents
{
  ENABLE_FRTTI( RecdChannel ) 
public:
  /***/
  RecdChannel( 
		IConnection*    pConnection, 
		IThreadFactory* pThreadFactory = NULL,
		LONG            nReadTimeOut   = INFINITE 
	      );

  /***/
  virtual ~RecdChannel();


/* FChannel implementation */
protected:
  /***/
  IChannelReaderEvents*   GetReaderEvents() { return this; };
  /***/
  IChannelWriterEvents*   GetWriterEvents() { return NULL; };

/* IChannelReaderEvents implementation */
protected:

  /***/
  VOID	OnReaderStarted();

  /***/
  VOID	OnDataReceived( const VOID* pData, DWORD dwDataLen );

  /***/
  VOID	OnReadTimeOut();

  /***/
  VOID	OnStopReader();


private:
  /***/
  VOID	SendResponse( RciResponse eResponse , const RecdResult& rResults );

private:
  FRciParser	m_mdcdParser;

};

#endif


