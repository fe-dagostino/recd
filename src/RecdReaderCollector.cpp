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


#include "RecdReaderCollector.h"
#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_LOGGING

GENERATE_CLASSINFO( RecdReaderCollector, FSingleton )
IMPLEMENT_SINGLETON( RecdReaderCollector )


VOID	RecdReaderCollector::OnInitialize()
{
  FMutexCtrl  _mtxCtrl( m_mtxReaders );

  FParameter*  _pIpCameras = RecdConfig::GetInstance().GetIpCameras( NULL );
  if ( _pIpCameras == NULL )
  {
    ERROR_INFO( "Parameter not FOUND", OnInitialize() )
    return;
  }
  
  LOG_INFO( FString( 0, "Configured [%d] Cameras", _pIpCameras->GetCount() ), OnInitialize() )
  for ( DWORD _dwItem = 0; _dwItem < _pIpCameras->GetCount(); _dwItem++ )
  {
    FString _sReader( 0, "RecdStreamReader[%u]", _dwItem );
    FString _sCamera = _pIpCameras->GetValue( _dwItem );

    RecdStreamReader* _pStreamReader = NULL;
    FTRY
    {
      _pStreamReader = new RecdStreamReader( _sReader, _sCamera );
    }
    FCATCH( FException, ex )
    {
      TRACE_EXCEPTION_CATCH( ex, OnInitialize() )
    }
    
    if ( _pStreamReader != NULL )
    {
      LOG_INFO( FString( 0, "Allocated New Stream Reader [%s] for Camera [%s]", 
			 (const char*)_sReader, (const char*)_sCamera ), OnInitialize() )
      
      _pStreamReader->Start();

      LOG_INFO( FString( 0, "Keep pointer to Stream Reader [%s]", 
			 (const char*)_sReader ), OnInitialize() )
      m_lstReaders.PushTail( _pStreamReader );
    }
  }
  
}

VOID	RecdReaderCollector::OnFinalize()
{
  LOG_INFO( FString( 0, "Release Running Stream Readers [%u]", m_lstReaders.GetSize() ), OnFinalize() )
  
  FMutexCtrl  _mtxCtrl( m_mtxReaders );
  
  while ( m_lstReaders.IsEmpty() == FALSE )
  {
    RecdStreamReader* _pStreamReader = m_lstReaders.PopHead();
    
    _pStreamReader->Stop();
    
    delete _pStreamReader;
  }
}

RecdStreamReader& 	RecdReaderCollector::GetStreamReader( const FString& sCamera )
{
  FMutexCtrl  _mtxCtrl( m_mtxReaders );

  FTList< RecdStreamReader* >::Iterator _iter = m_lstReaders.Begin();
  
  while ( _iter == TRUE )
  {
    RecdStreamReader* _pStreamReader = *_iter;
    
    if ( _pStreamReader->GetCameraName() == sCamera )
    {
	LOG_INFO( FString( 0, "Specified StreamReader [%s] FOUND", (const char*)sCamera ), GetStreamReader() )
	return *_pStreamReader;
    }
    
    _iter++;
  }
  
  ERROR_INFO( FString( 0, "Specified StreamReader [%s] NOT FOUND", (const char*)sCamera ), GetStreamReader() )
}

BOOL	RecdReaderCollector::SetReading( BOOL bEnable )
{
  FMutexCtrl  _mtxCtrl( m_mtxReaders );

  FTList< RecdStreamReader* >::Iterator _iter = m_lstReaders.Begin();
  
  while ( _iter == TRUE )
  {
    RecdStreamReader* _pStreamReader = *_iter;
    
    _pStreamReader->SetStatus( bEnable?(RecdStreamReader::eRSOpenStream):(RecdStreamReader::eRSFlushing) );
    
    _iter++;
  }

  m_bEnabled = bEnable;
  
  return TRUE;
}

BOOL	RecdReaderCollector::GetBuffers( RecdStreamReader::ReaderStatus eStatus, DOUBLE* pdPercentage ) const
{
  FMutexCtrl  _mtxCtrl( m_mtxReaders );

  FTList< RecdStreamReader* >::Iterator _iter = m_lstReaders.Begin();
  
  DOUBLE _dPercentage = 100.0;
  
  while ( _iter == TRUE )
  {
    RecdStreamReader* _pStreamReader = *_iter;
    
    DOUBLE dElapsed = 0.0;
    DOUBLE dTotal   = 0.0;
    RecdStreamReader::ReaderStatus _eStatus = _pStreamReader->GetStatus( &dElapsed, &dTotal );
    if (
        ( _eStatus == RecdStreamReader::eRSBuffering ) || 
        ( _eStatus == RecdStreamReader::eRSFlushing  )
       )
    {
      DOUBLE _dTmp = (dElapsed / dTotal) * 100;
      
      if ( _dTmp < _dPercentage )
	_dPercentage = _dTmp;
    }
    _iter++;
  }

  if ( pdPercentage != NULL )
    *pdPercentage = _dPercentage; 

  if ( eStatus == RecdStreamReader::eRSBuffering )
    return CheckStatus( RecdStreamReader::eRSReading );
  
  if ( eStatus == RecdStreamReader::eRSFlushing  )
    return CheckStatus( RecdStreamReader::eRSWaiting );
  
  return FALSE;
}

BOOL	RecdReaderCollector::CheckStatus( RecdStreamReader::ReaderStatus eStatus ) const
{
  FMutexCtrl  _mtxCtrl( m_mtxReaders );

  FTList< RecdStreamReader* >::Iterator _iter = m_lstReaders.Begin();
  
  while ( _iter == TRUE )
  {
    RecdStreamReader* _pStreamReader = *_iter;
    
    if ( _pStreamReader->GetStatus( NULL, NULL ) != eStatus ) 
      return FALSE;
    
    _iter++;
  }

  return TRUE;
}

BOOL	RecdReaderCollector::IsEnabled() const
{ 
  FMutexCtrl  _mtxCtrl( m_mtxReaders );
  
  return m_bEnabled;
}
