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


#ifndef __RECD_CONFIG_H__
#define __RECD_CONFIG_H__

#include "FSingleton.h"
#include "FConfigFileEx.h"
#include "RecdConfigException.h"
#include "avcolor.h"

USING_NAMESPACE_FED

using namespace libavcpp;

/**
 * This is a singleton object used to facilitates access 
 * to configuration parameters.
 */
class RecdConfig : public FSingleton
{
  ENABLE_FRTTI( RecdConfig )
  DECLARE_SINGLETON( RecdConfig )
protected:

  /***/
  VOID         OnInitialize();
  /***/
  VOID         OnFinalize();

public:
  /////////////////////////////
  // IP CAMERAS SECTION 
  ///////////////////////
  
  /**
   * Returns ..
   */
  FParameter*   GetIpCameras( BOOL* pbStored ) const;

  /**
   * Return stream buffer size.
   * Default value 40.0 seconds.
   */
  DOUBLE 	GetReaderBufferingTime( BOOL* pbStored ) const;

  /**
   * Return stream url for the specified ip camera.
   * Default value is empty string, so an error will be raised.
   * @param sIPCamera must match with a section in configuration file.
   */
  FString	GetReaderStream( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return max number of items that can be buffered from ip cam reader.
   * Default value is 5 that means 5/30 seconds of buffered, where 30 is the 
   * maximum number of frames per second expexted from video stream.
   * @param sIPCamera must match with a section in configuration file.
   */
  DWORD 	GetReaderMaxItems( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return delay time for the reader during stand-by mode.
   * Default value is 30 ms.
   * @param sIPCamera must match with a section in configuration file.
   */
  DWORD 	GetReaderStandByDelay( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return rescaling options.
   * Default value is 1 than means SWS_FAST_BILINEAR, 2 = SWS_BILINEAR, 4 = SWS_BICUBIC.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetReaderRescaleOptions( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return limit for incoming fps.
   * Default value is 25fps.
   * @param sIPCamera must match with a section in configuration file.
   */
  DOUBLE 	GetReaderFpsLimits( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return max number of items that can be buffered from encoder thread.
   * Default value is 5 that means 5/30 seconds of buffered, where 30 is the 
   * maximum number of frames per second expexted from video stream.
   * @param sIPCamera must match with a section in configuration file.
   */
  DWORD 	GetEncoderMaxItems( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return delay time for the encoder during stand-by mode.
   * Default value is 30 ms.
   * @param sIPCamera must match with a section in configuration file.
   */
  DWORD 	GetEncoderStandByDelay( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return timeout for used as max waiting time during reading from Frames Mailbox populated
   * by StreamReader instance.
   * Default value is 30 ms.
   * @param sIPCamera must match with a section in configuration file.
   */
  DWORD 	GetEncoderReadingTimeout( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rescaling options.
   * Default value is 1 than means SWS_FAST_BILINEAR, 2 = SWS_BILINEAR, 4 = SWS_BICUBIC.
   * @param sIPCamera must match with a section in configuration file.
   */
  DWORD 	GetEncoderRescaleOptions( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return encoded video width.
   * Default value is 1280
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetEncoderWidth( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return encoded video height.
   * Default value is 720 
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetEncoderHeight( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return encoded video fps.
   * Default value is 25fps
   * @param sIPCamera must match with a section in configuration file.
   */
  DOUBLE	GetEncoderFps( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return encoded video gop (group of pictures).
   * Default value is 10.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetEncoderGoP( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return encoded video bit rate.
   * Default value is 4000000.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetEncoderBitRate( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return encoded video codec.
   * Default value is 13 ( CODEC_ID_MPEG4 ).
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetEncoderVideoCodec( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return timespan to be used for the HighLights on the specified camera.
   * Default value is 40.0 seconds ( check GetReaderBufferingTime() )
   * @param sIPCamera must match with a section in configuration file.
   */
  DOUBLE	GetHighLightsTimeSpan( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return encoded video width to be used for HighLights.
   * Default value is -1 ( keep input size )
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetHighLightsEncoderWidth( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return encoded video height to be used for HighLights.
   * Default value is -1 ( keep input size )
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetHighLightsEncoderHeight( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return encoded video fps to be used for HighLights.
   * Default value is 25fps
   * @param sIPCamera must match with a section in configuration file.
   */
  DOUBLE	GetHighLightsEncoderFps( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return encoded video gop (group of pictures) to be used for HighLights.
   * Default value is 10.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetHighLightsEncoderGoP( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return encoded video bit rate to be used for HighLights.
   * Default value is 8000000.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetHighLightsEncoderBitRate( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return encoded video codec to be used for HighLights.
   * Default value is 13 ( CODEC_ID_MPEG4 ).
   * @param sIPCamera must match with a section in configuration file.
   */
  INT		GetHighLightsEncoderVideoCodec( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return TRUE if a background image should be used for rendering highlights; FALSE in order to 
   * write highlights like raw video.
   * Default value is FALSE
   * @param sIPCamera must match with a section in configuration file.
   */
  BOOL          GetHighLightsBackgroundStatus( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return background image to be used for rendering highlights.
   * Default value is /etc/recd/default-skin-highlight.png
   * @param sIPCamera must match with a section in configuration file.
   */
  FString  	GetHighLightsBackground( const FString& sIPCamera, BOOL* pbStored ) const;

  /**
   * Return rect destination X position on final highlights render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetHighLightsRectX( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rect destination Y position on final highlights render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetHighLightsRectY( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rect destination Widht on final highlights render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetHighLightsRectWidth( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rect destination Height on final highlights render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetHighLightsRectHeight( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return rect destination X position on final render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetRenderRectX( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rect destination Y position on final render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetRenderRectY( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rect destination Widht on final render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetRenderRectWidth( const FString& sIPCamera, BOOL* pbStored ) const;
  /**
   * Return rect destination Height on final render.
   * Default value is 0.
   * @param sIPCamera must match with a section in configuration file.
   */
  INT 		GetRenderRectHeight( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /**
   * Return key color for the specified camera.
   * Default value is CAVColor().
   * @param sIPCamera must match with a section in configuration file.
   */
  CAVColor 	GetRenderKeyColor( const FString& sIPCamera, BOOL* pbStored ) const;
  
  /////////////////////////////
  // RENDER SECTION 
  ///////////////////////
  
  /**
   * Return delay time for the encoder during stand-by mode.
   * Default value is 30 ms.
   */
  DWORD 		GetRenderStandByDelay( BOOL* pbStored ) const;
  /**
   * Return timeout for used as max waiting time during reading from Frames Mailbox populated
   * by StreamEncoder instance.
   * Default value is 30 ms.
   */
  DWORD 		GetRenderReadingTimeout( BOOL* pbStored ) const;

  /**
   * Return background filename to be used during renering.
   */
  FString  		GetRenderBackground( BOOL* pbStored ) const;
  /**
   * Return filename with key colors.
   */
  FString  		GetRenderBackgroundMask( BOOL* pbStored ) const;
  
  /**
   * Return encoded video width.
   * Default value is -1 ( keep background size )
   */
  INT 			GetRenderWidth( BOOL* pbStored ) const;
  /**
   * Return encoded video height.
   * Default value is -1 ( keep background size )
   */
  INT			GetRenderHeight( BOOL* pbStored ) const;
  /**
   * Return encoded video fps.
   * Default value is 25fps
   */
  DOUBLE		GetRenderFps( BOOL* pbStored ) const;

  /**
   * Return encoded video gop (group of pictures).
   * Default value is 10.
   */
  INT			GetRenderGoP(BOOL* pbStored ) const;

  /**
   * Return encoded video bit rate.
   * Default value is 4000000.
   */
  INT			GetRenderBitRate( BOOL* pbStored ) const;

  /**
   * Return encoded video codec.
   * Default value is 13 ( CODEC_ID_MPEG4 ).
   */
  INT			GetRenderVideoCodec( BOOL* pbStored ) const;
  
  /////////////////////////////
  // GENERAL SECTION 
  ///////////////////////

  /**
   * Return destination log path on file system. Format must be supported from host OS, so 
   * do not use something like c:\var\log under linux.
   */
  FString	GetLogDiskPath( BOOL* pbStored ) const;
  /**
   * Return the prefix used to form all log filenames.
   */
  FString	GetLogDiskPrefix( BOOL* pbStored ) const;
  /**
   * Return the extension to be appended to each log file.
   */
  FString	GetLogDiskExtension( BOOL* pbStored ) const;
  /**
   */
  DWORD		GetLogDiskFiles( BOOL* pbStored ) const;
  /**
   */
  DWORD		GetLogDiskFileSizeLimit( BOOL* pbStored ) const;
  
  /**
   */
  FString	GetLogServerAddress( BOOL* pbStored ) const;
  /**
   */
  WORD		GetLogServerPort   ( BOOL* pbStored ) const;
   
   
  /**
   */
  FString	GetCmdServerAddress( BOOL* pbStored ) const;
  /**
   */
  WORD		GetCmdServerPort   ( BOOL* pbStored ) const;
   

  /////////////////////////////
  // LOG INFO
  ///////////////////////
  /***/
  BOOL		IsTraceExceptionEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetTraceExceptionStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsCatchExceptionEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetCatchExceptionStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsAssertionFailureEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetAssertionFailureStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsErrorInfoEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetErrorInfoStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsLoggingInfoEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetLoggingInfoStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsVerboseInfoEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetVerboseInfoStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsRawInfoEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetRawInfoStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsEnterMethodEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetEnterMethodStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsExitMethodEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetExitMethodStatus( const FString& sSection, BOOL bStatus );


  /////////////////////////////
  // LOG VERBOSITY
  ///////////////////////

  /***/
  BOOL		IsStartUpMessageEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetStartUpMessageStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsShutDownMessageEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetShutDownMessageStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsLowPeriodicMessageEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetLowPeriodicMessageStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsMediumPeriodicMessageEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetMediumPeriodicMessageStatus( const FString& sSection, BOOL bStatus );
  /***/
  BOOL		IsHighPeriodicMessageEnabled( const FString& sSection, BOOL* pbStored ) const;
  VOID		SetHighPeriodicMessageStatus( const FString& sSection, BOOL bStatus );


private:
  FConfigFileEx        m_cfg;

};

#endif

