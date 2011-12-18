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


#ifndef RECD_PARSER_H
#define RECD_PARSER_H

#include "RCI/IRciParser.h"
#include "RCI/FRciParserInfo.h"


USING_NAMESPACE_FED
USING_NAMESPACE_RCI

class RecdParser : public IRciParser
{
  ENABLE_FRTTI( RecdParser ) 
public:
  /***/
  RecdParser();

  /***/
  virtual ~RecdParser();

  /***/
  const FRciParserInfo*     PushData( const VOID* pData, DWORD dwDataLen );

public:
  enum eParserState
  { 
    epsReset,
    epsParameters
  };

public:
  FString*		m_pParameter;
  FRciParserInfo	m_rciParserInfo;
  eParserState		m_eCurrentState;

};


#endif


