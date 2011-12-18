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


#include "RecdParser.h"


GENERATE_CLASSINFO( RecdParser, IRciParser ) 


RecdParser::RecdParser( )
  : m_pParameter( NULL ), m_eCurrentState( epsReset )
{
}

RecdParser::~RecdParser( )
{

}

const FRciParserInfo*	RecdParser::PushData( const VOID* pData, DWORD dwDataLen )
{
  FRciParserInfo*	_pRetVal = NULL;
  const CHAR*		_pCh = (const CHAR*)pData;

  if ( m_eCurrentState == epsReset )
  {
    m_eCurrentState = epsParameters;
    m_rciParserInfo.Reset();
    m_pParameter    = new FString();
  }

  
  while ( dwDataLen && !_pRetVal )
  {
    switch( m_eCurrentState )
    {
      case epsParameters :
      {
	switch ( *_pCh )
	{
	  case '\t' : 
	  {
	    //skip
	  };break;
	  case '\r' :
	    m_eCurrentState = epsReset;
	    
	    if ( m_pParameter )
	    {
	      if ( !m_pParameter->IsEmpty() )
		m_rciParserInfo.m_args.Add( m_pParameter );

	      m_pParameter = NULL;
	    }
	    
	    _pRetVal        = &m_rciParserInfo;
	  break;
	  case ',' :
	    m_rciParserInfo.m_args.Add( m_pParameter );
	    m_pParameter = new FString();
	  break;
	  default :
	    *m_pParameter += *_pCh;
	  break;
	}//switch ( *_pCh )
      }; break;
    }

    _pCh++;
    dwDataLen--;
  }

  return _pRetVal;
}



