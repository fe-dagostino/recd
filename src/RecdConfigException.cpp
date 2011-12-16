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


#include "RecdConfigException.h"


GENERATE_CLASSINFO( RecdConfigException, FException )


RecdConfigException::RecdConfigException( DWORD nErrorCode )
	: FException( FString(), nErrorCode )
{
	m_sErroMsg = FString( RecdConfigException::GetClassName() ) + FString(" -:- ");
	AddMessageCodeToErrorMsg( nErrorCode );
}

RecdConfigException::RecdConfigException( const FString& sErrorMsg, DWORD nErrorCode )
	: FException( FString(), nErrorCode )
{
	m_sErroMsg = FString( RecdConfigException::GetClassName() ) + FString(" -:- ");
	AddMessageCodeToErrorMsg( nErrorCode );
	m_sErroMsg += FString("-:- MSG=") + sErrorMsg;
}

RecdConfigException::~RecdConfigException()
{

}

VOID	RecdConfigException::AddMessageCodeToErrorMsg( DWORD nErrorCode )
{
  switch ( nErrorCode )
  {
    case OBJECT_NOT_INITIALIZED :
      m_sErroMsg += FMAKE_STRING( OBJECT_NOT_INITIALIZED );
    break;
    case OBJECT_ALREADY_INITIALIZED :
      m_sErroMsg += FMAKE_STRING( OBJECT_ALREADY_INITIALIZED );
    break;
    case NOT_ENOUGH_MEMORY :
      m_sErroMsg += FMAKE_STRING( NOT_ENOUGH_MEMORY );
    break;
    case CFG_FILE_NOT_FOUND_OR_INACCESSIBLE :
      m_sErroMsg += FMAKE_STRING( CFG_FILE_NOT_FOUND_OR_INACCESSIBLE );
    break;
  }
}


