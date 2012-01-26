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


#ifndef RECDCLIENT_H
#define RECDCLIENT_H

#include "FString.h"
#include "FMutex.h"
#include "RCI/FRciClient.h"

USING_NAMESPACE_FED
/**
 */
class RecdClient : public FObject
{
  ENABLE_FRTTI( RecdClient ) 
public:  
  /**
   */
  RecdClient( const FString& sIP, WORD wPort, LONG iReadTimeout );
  /**
   */
  virtual ~RecdClient();

  /**
   * Return Total size and amount of available size on the specified mount point.
   * Available size refers to the amount of space available for the user.
   * @param sMountPoint [input] mount point to be checked.
   * @param dTotal      [output] total size
   * @param dFree       [output] available size
   * Function return TRUE in case of success, FALSE otherwise.
   */
  bool          GetDiskSize( const FString& sMountPoint, double& dTotal, double& dFree );
  
  /**
   * Estimate available recording time based on services and amount of available space.
   * Return time in seconds, -1.0 means error. 
   */
  double 	EstimateTime( double dSize, bool bRender, bool bHighlights, bool bRaw );
  
  /**
   * Start recording for specified services.
   * @param args must contains 2 values that are respectively DestinationPath1, DestinationPath2
   * Return value will be TRUE if recording has been started, otherwise return value will be false.            
   */
  bool          StartRecording( const FArguments& args, bool bRender, bool bHighlights, bool bRaw );
  /**
   * Stop all active recording.
   * Return value will be TRUE in case of success, otherwise return value will be false.            
   */
  bool          StopRecording();

  /**
   * Retrieve percentage of buffers both at beginning and at the end of recording.
   * @param bFlushBuffers [input] true if desidered percentage refers to flush buffers and so
   *                              to the end of recording.
   * @param dPergentace   [output] will be populated with the percntage from 0 to 100 %
   * Function return TRUE if buffering operations has been completed, FALSE otherwise; this
   * mean that application level should wait until percentage reach 100% and return value is TRUE,
   * or also until return value is TRUE. Basically pecentage could be used to provide some
   * feedback to the user.
   */
  bool          GetBuffers( bool bFlushBuffers, double& dPercentage );

  /**
   *  Start HighLights on specified camera. 
   *  @param sCamera   must match with configuration value.
   *                   Could be also "NULL" and HighLights will
   *                   be starts on all cameras.
   *  Return true in case of success, false otherwise.
   */
  bool          StartHighLights( const FString& _sCamera );
  
private:
  bool 	Connect();
  bool 	Disconnect();
  
private:  
  FString      m_sRemoteAddr;
  WORD         m_wRemotePort;
  LONG         m_iReadTimeout;
  FMutex       m_mtxClient;
  FRciClient*  m_pRciClient;
};

#endif // RECDCLIENT_H
