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


#ifndef RECD_MBX_ITEM_H
#define RECD_MBX_ITEM_H

#include "FChannel.h"
#include "RCI/IRciCommand.h"
#include "avframe.h"

USING_NAMESPACE_FED
USING_NAMESPACE_RCI

using namespace libavcpp;

/**
 */
class RecdMbxItem 
{
public:
  /***/
  enum MbxItemType 
  {
    eUndefinedItem,
    eCommandItem,
    eImageItem,
    eSampleItem
  };
  
  enum MbxCommandType
  {
    eCmdUndefined,
    eCmdStartEncoding,
    eCmdStopEncoding,
    eCmdStartRendering,
    eCmdStopRendering
  };

  /***/
  RecdMbxItem( MbxCommandType eMbxCommand )
   : m_eItemType( eCommandItem ), m_eMbxCommand( eMbxCommand ), m_dPTS(0.0)
  {
  }
  
  /***/
  RecdMbxItem( CAVImage* pAVImage, double pts )
   : m_eItemType( eImageItem ), m_pAVImage( pAVImage ), m_dPTS(pts)
  {
  }
  
  /***/
  RecdMbxItem( CAVSample* pAVSample, double pts )
   : m_eItemType( eSampleItem ), m_pAVSample( pAVSample ), m_dPTS(pts)
  { 
  }

  /***/
  ~RecdMbxItem()
  {
    if ( (m_eItemType == eImageItem) && (m_pAVImage != NULL) ) 
    {
       delete m_pAVImage;
       m_pAVImage = NULL;
    }
    if ( (m_eItemType == eSampleItem) && (m_pAVSample != NULL) ) 
    {
       delete m_pAVSample;
       m_pAVSample = NULL;
    }
  }

  /**
   * Return type for current instance.
   */
  inline MbxItemType     GetType() const
  { return m_eItemType; }
  inline double          GetPTS() const
  { return m_dPTS; }
  /***/
  inline MbxCommandType  GetCommand() const
  { return m_eMbxCommand; }
  /***/
  inline CAVImage*       GetImage() const
  { return m_pAVImage; }
  /***/
  inline CAVSample*      GetSample() const
  { return m_pAVSample; }
  
private:
  enum MbxItemType  m_eItemType;
  double            m_dPTS;
  
  union {
  CAVImage*        m_pAVImage;
  CAVSample*       m_pAVSample;
  MbxCommandType   m_eMbxCommand;
  };
};

#endif
