#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <GL/gl.h>

class VideoPlayer
{
  public:
    VideoPlayer();
    bool init(const char* filename);
    void run();
    bool time2die;
    void display();

    AVFormatContext *pFormatCtx;
    int             videoStream;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    AVFrame         *pFrame, *pFrameRGB;
    AVPacket        packet;
    int             frameFinished;
    uint8_t *data;

    GLuint makeTexture();
    void execloop();
    void build();
};

#endif
