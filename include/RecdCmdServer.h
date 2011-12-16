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

#ifndef __RECD_CMD_SERVER_H__
#define __RECD_CMD_SERVER_H__

#include "FConfigFileEx.h"
#include "RecdConfigException.h"
#include "RCI/FRciServer.h"
#include "FService.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

/**
 * 
 */
class RecdCmdServer : public FObject
{
  ENABLE_FRTTI( RecdCmdServer )
public:
  /**
    */
  RecdCmdServer( const FService& rService );
  /**
    */
  virtual ~RecdCmdServer();
  /**
    */
  BOOL Initialize();
  /**
    */
  BOOL Finalize();

protected:

private:
  const FService&   m_rService;
  FRciServer        m_rciServer;

};

#endif

