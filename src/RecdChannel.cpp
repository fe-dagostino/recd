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


#include "RecdChannel.h"
#include "RCI/FRciServer.h"
#include "RCI/FRciDefines.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_LOGGING

GENERATE_CLASSINFO( RecdChannel, FChannel ) 


RecdChannel::RecdChannel( 
			  IConnection*    pConnection, 
			  IThreadFactory* pThreadFactory,
			  LONG            nReadTimeOut 
			)
  : FChannel( pConnection, pThreadFactory, nReadTimeOut )
{
}

RecdChannel::~RecdChannel()
{

}

VOID	RecdChannel::OnDataReceived( const VOID* pData, DWORD dwDataLen )
{
  const FRciParserInfo* _pRciParserInfo = m_mdcdParser.PushData( pData, dwDataLen );
  if ( _pRciParserInfo )
  {
    RciResponse   _response = rciOk;
    RecdResult    _results;
    FString       _sMethod  = _pRciParserInfo->GetMethod();
     
    //Convert method name to upper case.
    _sMethod.UpCase();

    const IRciCommand*	_pRciCommand = ((const FRciServer*)GetServer())->GetCommandCollector().Find( _sMethod );

    if ( !_pRciCommand )
    {
      _results.Add( new FString("UNKNOWN_COMMAND") );
      _response = rciError;
    }
    else
    if ( _pRciCommand->GetRequiredArguments() != _pRciParserInfo->GetArguments().GetCount() )
    {
      _results.Add( new FString("PARAMETERS_NOT_MATCH") );
      _response = rciError;
    }
    else
    {
      _response = _pRciCommand->Execute( *this, _pRciParserInfo->GetArguments(), _results );
    }

    FTRY
    {
      SendResponse( _response, _results );
    }
    FCATCH( FChannelException, ex )
    {
      TRACE_EXCEPTION_CATCH( ex, OnDataReceived() );

      Finalize();
    }

  }//if ( _pRciParserInfo )
}

VOID	RecdChannel::OnReaderStarted()
{
}

VOID	RecdChannel::OnStopReader()
{
}

VOID	RecdChannel::OnReadTimeOut()
{

}

VOID	RecdChannel::SendResponse( RciResponse eResponse , const RecdResult& rResults )
{
  FString _sResponse;

  if ( eResponse == rciError )
    _sResponse = "KO";
  else
    _sResponse = "OK";

  for ( DWORD _dwIndex = 0; _dwIndex < rResults.GetCount(); _dwIndex++ )
  {
    _sResponse += rResults.GetSeparator();
    _sResponse += rResults[_dwIndex];
  }
  _sResponse += "\r\n";

  GetConnection()->Write( FData( _sResponse.GetBuffer(), (DWORD)_sResponse.GetLen() ) );
}

