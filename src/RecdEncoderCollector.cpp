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
    FString _sRawEncoder       ( 0, "RecdStreamEncoder[%u]", _dwItem );
    FString _sHighLightsEncoder( 0, "RecdHighLightsEncoder[%u]", _dwItem );
    FString _sCamera = _pIpCameras->GetValue( _dwItem );

    RecdStreamEncoder*     _pStreamEncoder         = NULL;
    RecdHighLightsEncoder* _pRecdHighLightsEncoder = NULL;
    FTRY
    {
      _pStreamEncoder         = new RecdStreamEncoder( 
						_sRawEncoder, 
						RecdReaderCollector::GetInstance().GetStreamReader( _sCamera )
					     );
					     
      _pRecdHighLightsEncoder = new RecdHighLightsEncoder( 
						_sHighLightsEncoder, 
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
			 (const char*)_sRawEncoder, (const char*)_sCamera ), OnInitialize() )
      
      _pStreamEncoder->Start();

      LOG_INFO( FString( 0, "Keep pointer to Stream Encoder [%s]", 
			 (const char*)_sRawEncoder ), OnInitialize() )
      m_lstEncoders.PushTail( _pStreamEncoder );
    }
    
    
    if ( _pRecdHighLightsEncoder != NULL )
    {
      LOG_INFO( FString( 0, "Allocated New HighLights Encoder [%s] for Camera [%s]", 
			 (const char*)_sHighLightsEncoder, (const char*)_sCamera ), OnInitialize() )
      
      _pRecdHighLightsEncoder->Start();

      LOG_INFO( FString( 0, "Keep pointer to HighLights Encoder [%s]", 
			 (const char*)_sHighLightsEncoder ), OnInitialize() )
      m_lstHighLightsEncoders.PushTail( _pRecdHighLightsEncoder );
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
  LOG_INFO( FString( 0, "Release Running Stream Encoders [%u]", m_lstEncoders.GetSize() ), OnFinalize() )
  while ( m_lstEncoders.IsEmpty() == FALSE )
  {
    RecdStreamEncoder* _pStreamEncoder = m_lstEncoders.PopHead();
    
    _pStreamEncoder->Stop();
    
    delete _pStreamEncoder;
  }
  
  LOG_INFO( FString( 0, "Release Running HighLights Encoders [%u]", m_lstHighLightsEncoders.GetSize() ), OnFinalize() )
  while ( m_lstHighLightsEncoders.IsEmpty() == FALSE )
  {
    RecdHighLightsEncoder* _pRecdHighLightsEncoder = m_lstHighLightsEncoders.PopHead();
    
    _pRecdHighLightsEncoder->Stop();
    
    delete _pRecdHighLightsEncoder;
  }
  
  // Release Render Encoder instance
  if ( m_pRecdRenderEncoder != NULL )
  {
    m_pRecdRenderEncoder->Stop();
    
    delete m_pRecdRenderEncoder;
  }
}


BOOL	RecdEncoderCollector::SetParameters( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw )
{
  if ( bRender == TRUE )
    m_pRecdRenderEncoder->SetParameters( sDestination );

  FTList< RecdStreamEncoder* >::Iterator _iterRaw = m_lstEncoders.Begin();
  while ( _iterRaw == TRUE )
  {
    RecdStreamEncoder* _pStreamEncoder = *_iterRaw;
    
    _pStreamEncoder->SetParameters( sDestination, bRender, bHighlights, bRaw );
    
    _iterRaw++;
  }

  FTList< RecdHighLightsEncoder* >::Iterator _iterHL = m_lstHighLightsEncoders.Begin();
  
  while ( _iterHL == TRUE )
  {
    RecdHighLightsEncoder* _pHighLightsEncoder = *_iterHL;
    
    _pHighLightsEncoder->SetParameters( sDestination, bRender, bHighlights, bRaw );
    
    _iterHL++;
  }

  return TRUE;
}

BOOL	RecdEncoderCollector::ReadyForRecording() const
{
  BOOL _bRetVal = TRUE;
  
  _bRetVal = (m_pRecdRenderEncoder->GetStatus()==RecdRenderEncoder::eREWaiting);

  FTList< RecdStreamEncoder* >::Iterator _iterRaw = m_lstEncoders.Begin();
  while ( _iterRaw == TRUE )
  {
    RecdStreamEncoder* _pStreamEncoder = *_iterRaw;
    
    _bRetVal &= (_pStreamEncoder->GetStatus()==RecdStreamEncoder::eSEWaiting);
    
    _iterRaw++;
  }

  FTList< RecdHighLightsEncoder* >::Iterator _iterHL = m_lstHighLightsEncoders.Begin();
  
  while ( _iterHL == TRUE )
  {
    RecdHighLightsEncoder* _pHighLightsEncoder = *_iterHL;
    
    _bRetVal &= (_pHighLightsEncoder->GetStatus( NULL, NULL )==RecdHighLightsEncoder::eHSWaiting);
    
    _iterHL++;
  }

  return _bRetVal;
}

BOOL	RecdEncoderCollector::StartHighLights( const FString& sCamera )
{
  FTList< RecdHighLightsEncoder* >::Iterator _iterHL = m_lstHighLightsEncoders.Begin();
  
  while ( _iterHL == TRUE )
  {
    RecdHighLightsEncoder* _pHighLightsEncoder = *_iterHL;
    
    if ( sCamera == "ALL" )
      _pHighLightsEncoder->StartHighLight();
    else 
    if ( sCamera == _pHighLightsEncoder->GetStreamReader().GetCameraName() )
      _pHighLightsEncoder->StartHighLight();
    
    _iterHL++;
  }
  
  return TRUE;
}



