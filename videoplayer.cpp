#include "videoplayer.h"
#include <boost/bind.hpp>
#include <SDL/SDL.h>

static boost::mutex mutex;

VideoPlayer::VideoPlayer()
{
    pFormatCtx = NULL;
    videoStream =1;
    pCodecCtx = NULL;
    pCodec = NULL;
    pFrame = NULL;
    pFrameRGB = NULL;
    frameFinished = false;
    time2die = false;
    data = NULL;
}
bool VideoPlayer::init(const char* filename)
{
    // Register all formats and codecs
    av_register_all();

    // Open video file
    if(av_open_input_file(&pFormatCtx, filename, NULL, 0, NULL)!=0)
        return false; // Couldn't open file

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0)
        return false; // Couldn't find stream information

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, filename, 0);

    // Find the first video stream
    videoStream=-1;
    for(size_t i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
        videoStream=i;
        break;
        }
    if(videoStream==-1)
        return false; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        fprintf(stderr, "Unsupported codec!\n");
        return false; // Codec not found
    }

    // Open codec
    if(avcodec_open(pCodecCtx, pCodec)<0)
        return false; // Could not open codec

    // Allocate video frame
    pFrame=avcodec_alloc_frame();

    // Allocate an AVFrame structure
    pFrameRGB=avcodec_alloc_frame();
    if(pFrameRGB==NULL)
        return false;

    // Determine required buffer size and allocate buffer
    int numBytes=avpicture_get_size(PIX_FMT_RGB32, pCodecCtx->width,
                    pCodecCtx->height);
    uint8_t* buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB32,
            pCodecCtx->width, pCodecCtx->height);

    return true;
}
void VideoPlayer::run()
{
    time2die = false;
    boost::thread thr(boost::bind(&VideoPlayer::execloop, this));
}
void VideoPlayer::display()
{
    GLuint tex = makeTexture();
    glClearColor( 0.0, 0.4, 0.0, 0.0 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glEnable( GL_TEXTURE_2D );
    glDisable(GL_LIGHTING);

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();

    glBindTexture( GL_TEXTURE_2D, tex );
    glBegin (GL_QUADS);
    glColor3f(1, 1, 1);
    glTexCoord2f(0, 1);
    glVertex3i(-1, -1, 1);
    glTexCoord2f(1, 1);
    glVertex3i(1, -1, 1);
    glTexCoord2f(1, 0);
    glVertex3i(1, 1, 1);
    glTexCoord2f(0, 0);
    glVertex3i(-1, 1, 1);
    glEnd();

    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    glDeleteTextures(1, &tex);
    glEnable(GL_LIGHTING);
}
GLuint VideoPlayer::makeTexture()
{
  GLuint texture;
  // allocate a texture name
  glGenTextures( 1, &texture);

  // select our current texture
  glBindTexture( GL_TEXTURE_2D, texture );

  // select modulate to mix texture with color for shading
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    mutex.lock();
    const int width = pCodecCtx->width, height = pCodecCtx->height;
glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    mutex.unlock();

  // when texture area is small, bilinear filter the closest MIP map
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR);
  // when texture area is large, bilinear filter the first MIP map
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    return texture;
}
void VideoPlayer::build()
{
    const int width = pCodecCtx->width, height = pCodecCtx->height;
    mutex.lock();
    delete data;
    data = new uint8_t[width * height * 4];
    uint8_t* d = data;
    for (int y = 0; y < height; y++)
    {
      const uint8_t* line = pFrameRGB->data[0] + y*pFrameRGB->linesize[0];
      memcpy(d, line, 4*width);
      d += 4*width;
    }
    mutex.unlock();
}
void VideoPlayer::execloop()
{
    while(!time2die)
    {
        while ( av_read_frame(pFormatCtx, &packet)>=0 && !time2die)
        {
            // Is this a packet from the video stream?
            if(packet.stream_index==videoStream) {
                // Decode video frame
                avcodec_decode_video(pCodecCtx, pFrame, &frameFinished,
                    packet.data, packet.size);

                // Did we get a video frame?
                if(frameFinished) {
      SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
          pCodecCtx->pix_fmt,
          pCodecCtx->width, pCodecCtx->height,
          PIX_FMT_RGB32,
          SWS_BICUBIC, NULL, NULL, NULL);

      sws_scale(img_convert_ctx, pFrame->data,
          pFrame->linesize, 0, pCodecCtx->height,
          pFrameRGB->data, pFrameRGB->linesize);

      sws_freeContext(img_convert_ctx);
      build();
      //SDL_Delay(42);
                }
            }
        }
        // stream finished, seek to beginning
    }
}
