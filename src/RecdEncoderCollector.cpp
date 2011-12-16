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


#include "RecdEncoderCollector.h"
#include "RecdReaderCollector.h"
#include "RecdConfig.h"
#include "LOGGING/FLogger.h"

USING_NAMESPACE_LOGGING

GENERATE_CLASSINFO( RecdEncoderCollector, FSingleton )
IMPLEMENT_SINGLETON( RecdEncoderCollector )


VOID	RecdEncoderCollector::OnInitialize()
{
  FParameter*  _pIpCameras = RecdConfig::GetInstance().GetIpCameras( NULL );
  if ( _pIpCameras == NULL )
  {
    ERROR_INFO( "Parameter not FOUND", OnInitialize() )
    return;
  }
  
  LOG_INFO( FString( 0, "Configured [%d] Cameras", _pIpCameras->GetCount() ), OnInitialize() )
  for ( DWORD _dwItem = 0; _dwItem < _pIpCameras->GetCount(); _dwItem++ )
  {
    FString _sEncoder( 0, "RecdStreamEncoder[%u]", _dwItem );
    FString _sCamera = _pIpCameras->GetValue( _dwItem );

    RecdStreamEncoder* _pStreamEncoder = NULL;
    FTRY
    {
      _pStreamEncoder = new RecdStreamEncoder( 
						_sEncoder, 
						RecdReaderCollector::GetInstance().GetStreamReader( _sCamera )
					     );
    }
    FCATCH( FException, ex )
    {
      TRACE_EXCEPTION_CATCH( ex, OnInitialize() )
    }
    
    if ( _pStreamEncoder != NULL )
    {
      LOG_INFO( FString( 0, "Allocated New Stream Encoder [%s] for Camera [%s]", 
			 (const char*)_sEncoder, (const char*)_sCamera ), OnInitialize() )
      
      _pStreamEncoder->Start();

      LOG_INFO( FString( 0, "Keep pointer to Stream Encoder [%s]", 
			 (const char*)_sEncoder ), OnInitialize() )
      m_lstEncoders.PushTail( _pStreamEncoder );
    }
  }
 

  FTRY
  {
    LOG_INFO( "Allocation Render Encoder", OnInitialize() )
    m_pRecdRenderEncoder = new RecdRenderEncoder( "Rendering Encoder" );
  }
  FCATCH( FException, ex )
  {
    TRACE_EXCEPTION_CATCH( ex, OnInitialize() )
  }

  if ( m_pRecdRenderEncoder != NULL )
  {
      LOG_INFO( "Allocated Render Encoder", OnInitialize() )
      
      m_pRecdRenderEncoder->Start();
  }
}

VOID	RecdEncoderCollector::OnFinalize()
{
  LOG_INFO( FString( 0, "Release Running Stream Encoders [%u]", m_lstEncoders.GetSize() ), OnInitialize() )
  while ( m_lstEncoders.IsEmpty() == FALSE )
  {
    RecdStreamEncoder* _pStreamEncoder = m_lstEncoders.PopHead();
    
    _pStreamEncoder->Stop();
    
    delete _pStreamEncoder;
  }
  
  // Release Render Encoder instance
  if ( m_pRecdRenderEncoder != NULL )
  {
    m_pRecdRenderEncoder->Stop();
    
    delete m_pRecdRenderEncoder;
  }
}


BOOL	RecdEncoderCollector::StartRecording( const FString& sDestination  )
{
  FTList< RecdStreamEncoder* >::Iterator _iter = m_lstEncoders.Begin();
  
  while ( _iter == TRUE )
  {
    RecdStreamEncoder* _pStreamEncoder = *_iter;
    
    _pStreamEncoder->StartRecording( sDestination );
    
    _iter++;
  }
  
  m_pRecdRenderEncoder->StartRecording( sDestination );
  
  return TRUE;
}

BOOL	RecdEncoderCollector::StopRecording()
{
  FTList< RecdStreamEncoder* >::Iterator _iter = m_lstEncoders.Begin();
  
  while ( _iter == TRUE )
  {
    RecdStreamEncoder* _pStreamEncoder = *_iter;
    
    _pStreamEncoder->StopRecording();
    
    _iter++;
  }
  
  m_pRecdRenderEncoder->StopRecording();
  
  return TRUE;
}
