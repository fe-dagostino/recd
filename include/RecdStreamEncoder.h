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


#ifndef RECDSTREAMENCODER_H
#define RECDSTREAMENCODER_H

#include "LOGGING/FLogThread.h"
#include "RecdStreamReader.h"
#include "avencoder.h"
#include "avimage.h"

USING_NAMESPACE_FED
USING_NAMESPACE_LOGGING

/**
 * This is the consumer for RecdStreamReader.
 * Each instance of RecdStreamEncoder is intended on otder to 
 * read datas from RecdStreamReader and to encode an output video.
 */
class RecdStreamEncoder : public FLogThread
{
  ENABLE_FRTTI( RecdStreamEncoder )
public:
  
  /**
   * Stream Encoder Status status enumeration.
   */
  enum StreamEncoderStatus
  {
    eSEUndefined,    // Initial status no action required.
    eSEWaiting,      // Reader stay in waiting mode so input streaming is closed
    eSEOpenStream,   // 
    eSEEncoding,
    eSEReleasing
  };
    
  /**
   * 
   */
  RecdStreamEncoder( 
		      const FString&    sStreamEncoderName,
		      RecdStreamReader& rStreamReader
		   );
  /**
   * 
   */
  virtual ~RecdStreamEncoder();
  
  /**
   * Read/Dequeue first frame.
   * @param bRemove  specify if the item must be removed ot not.
   * Return a pointer to an instance of CAVFrame, it bRemove was TRUE
   * returned value must be release with a call to ReleaseFrame.
   */
  RecdMbxItem*             PopRenderMbxItem( DWORD dwTimeout, BOOL bRemove );
  /***/
  BOOL                     PushRenderMbxItem( RecdMbxItem* pMbxItem );
  /**
   * Return rendering starting point.
   */
  CAVPoint                 GetRenderPoint() const;
  /**
   * Return render bounding rect in relative coordinate system.
   */
  CAVRect                  GetRenderRect() const;
  /**
   * Return render key color.
   */
  CAVColor                 GetRenderColor() const;

  /**
   * Release specified pointer.
   */
  VOID                     ReleaseMbxItem( RecdMbxItem*& pMbxItem );
  
  /**
   *  Updating recording parameters. 
   */
  VOID			   SetParameters( const FString& sDestination, BOOL bRender, BOOL bHighlights, BOOL bRaw );
    
  /***/
  const RecdStreamReader&  GetStreamReader() const
  { return m_rStreamReader; }
 
  /***/
  enum StreamEncoderStatus GetStatus( ) const;
  
private:  
  /***/
  bool                     SetStatus( StreamEncoderStatus eStatus );
  
// Implements virtual method defined in FThread  
protected:
  
  /* Defined in FThread class */
  virtual BOOL		Initial();

  /* Defined in FThread class */
  virtual BOOL		Final();

  /**/
  virtual VOID		Run();

private:
  /**/
  DWORD   	GetLogMessageFlags() const;
  /**/
  DWORD   	GetVerbosityLevelFlags() const;
  
private:
  BOOL                       m_bExit;
  mutable FMutex             m_mtxEncoder;
  StreamEncoderStatus        m_eEncoderStatus;  
  CAVEncoder*                m_pAVEncoder;
  FString                    m_sDestination;
  RecdStreamReader&          m_rStreamReader;
  FTMailbox<RecdMbxItem* >*  m_pMbxItems;
  FTQueue<RecdMbxItem* >*    m_pVideoItems;
  FStopWatch                 m_swFPS;
  BOOL                       m_bRender;
  BOOL                       m_bHighlights; 
  BOOL                       m_bRaw;
  CAVImage                   m_rgbaBkg;
};

#endif // RECDSTREAMENCODER_H
