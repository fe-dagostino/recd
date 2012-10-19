#include <iostream>

#include "avapplication.h"
#include "avdecoder.h"
#include "avencoder.h"
#include "avimage.h"

using namespace libavcpp;


class AVDecoderEventsImp : public IAVDecoderEvents
{
public:
  AVDecoderEventsImp()
  {
    
  }
  
  void  setOutputFilename( const char* pOutputFilename )
  {
    m_sOutputFilename = pOutputFilename;
  }
  
  const CAVImage*      getImage() const
  { 
    return &m_avImage;
  }
  
  virtual void   OnVideoKeyFrame( const AVFrame* pAVFrame, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double time )
  {
     printf( "Got Video KEY Frame ...\r\n" );
     m_avImage.init( pAVFrame, pAVCodecCtx, -1, -1, PIX_FMT_RGB32 );
     
     
     printf( "Opening Output file ...\r\n" );
     if ( m_avEncoder.open( m_sOutputFilename, AV_ENCODE_VIDEO_STREAM, m_avImage.getWidth(), m_avImage.getHeight(), PIX_FMT_RGB32, 1, 1, 0, CODEC_ID_PNG, 0 ) != eAVSucceded )
     {
	printf( "Opening Output file FAILED\r\n" );
	return;
     }
     
     printf( "Opening Output file  DONE\r\n" );
     
     printf( "Writing Output file ...\r\n" );
     if ( m_avEncoder.write( &m_avImage, 0 ) != eAVSucceded )
     {
	printf( "Writing Output file FAILED\r\n" );
     }
     
     printf( "Closing Output file ...\r\n" );
     if ( m_avEncoder.close() != eAVSucceded )
     {
	printf( "Closing Output file FAILED\r\n" );
     }
  }
  
  virtual bool   OnVideoFrame( const AVFrame* pAVFrame, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double time )
  {
     return true;
  }
  
  virtual bool   OnFilteredVideoFrame( const AVFilterBufferRef* pAVFilterBufferRef, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double pst )
  {
    /* Nothing to do */  
  }
  
  virtual bool   OnAudioFrame( const AVFrame* pAVFrame, const AVStream* pAVStream, const AVCodecContext* pAVCodecCtx, double time )
  {
     return true;
  }

private:
  FString          m_sOutputFilename;  
  CAVImage         m_avImage;
  CAVEncoder       m_avEncoder;
};

int main(int argc, char **argv)
{
    
    if ( argc < 2 )
    {
      printf( "CAPTURE v.1.0.0\r\n" );
      printf( "\r\n" );
      printf( "DESCRIPTION: program read from specified input stream\r\n" );
      printf( "until a keyframe will be received and then will save \r\n" );
      printf( "that frame in the specified format.\r\n" );
      printf( "\r\n" );
      printf( "USAGE:\r\n" );
      printf( "    #./capture <input stream>  <outputn file>\r\n" );
      printf( "\r\n" );
      printf( "USING EXAMPLE:\r\n" );
      printf( "    #./capture rtsp://169.254.0.10:554/axis-media/media.amp?tcp  output.png\r\n" );
      printf( "\r\n" );
      
      return -1;
    }


    const char* pInputStream = argv[1];
    const char* pOutputFile  = argv[2];

    printf( "PARAMETERS  INPUT STREAM=[%s]    OUTPUT FILE=[%s]:\r\n", pInputStream, pOutputFile );

    
    CAVApplication::initLibAVCPP();

    
    CAVDecoder         _avDecoder;
    AVDecoderEventsImp _avDecoderEvents;
    
    _avDecoderEvents.setOutputFilename( pOutputFile );
    _avDecoder.setDecoderEvents( &_avDecoderEvents, false );
    

    printf( "Opening Input Stream ...\r\n" );
    if ( _avDecoder.open( pInputStream, 0.0, AV_SET_BEST_VIDEO_CODEC ) == eAVSucceded )
    {
      printf( "Opening Input Stream   DONE\r\n" );
    }
    else
    {
      printf( "Opening Input Stream FAILED\r\n" );
      return -2;
    }
    
    printf( "Start reading from Input Stream up to first key frame ...\r\n" );
    if ( _avDecoder.read( AVD_EXIT_ON_VIDEO_KEY_FRAME ) != eAVSucceded )
    {
      printf( "Reading Input Stream FAILED\r\n" );
      return -3;
    }
    
    printf( "Releasing Input Stream ...\r\n" );
    if ( _avDecoder.close() != eAVSucceded )
    {
      printf( "Releasing Input Stream FAILED\r\n" );
      return -4;
    }
    
    CAVApplication::deinitLibAVCPP();
    
    return 0;
}
  