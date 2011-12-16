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


#ifndef __RECD_EVENTS_H__
#define __RECD_EVENTS_H__

#include "FServiceEventInterface.h"
#include "RecdCmdServer.h"

USING_NAMESPACE_FED

/**
 * Interface to be implement to handle service events.
 */
class RecdEvents : public FServiceEventInterface
{
  ENABLE_FRTTI( RecdEvents )
public:
  /**
   */
  RecdEvents( const FService& rService );
  /**
   */
  virtual ~RecdEvents();

protected:
  
  /**
    * Called before starting service.
    */
  virtual BOOL OnStart( const FService* pService );

  /**
    * This is service thread body.
    */
  virtual VOID OnRun(const FService* pService);

  /**
    * Called before stopping service.
    */
  virtual VOID OnStop(const FService* pService);

  /***/
  virtual VOID OnInterrogate(const FService* pService);

  /***/
  virtual VOID OnPause(const FService* pService);

  /***/
  virtual VOID OnContinue(const FService* pService);

  /***/
  virtual VOID OnShutdown(const FService* pService);

  /***/
  virtual VOID OnNetBindAdd(const FService* pService);

  /***/	
  virtual VOID OnNetBindDisable(const FService* pService);

  /***/
  virtual VOID OnNetBindEnable(const FService* pService);

  /***/
  virtual VOID OnNetBindRemove(const FService* pService);

  /***/
  virtual VOID OnParamChange(const FService* pService);

  /***/
  virtual BOOL OnUserControl(const FService* pService, DWORD dwOpcode);

private:
  BOOL            m_bExit;
  RecdCmdServer   m_cmdServer;
};

#endif

