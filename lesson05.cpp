/*
 * This code was created by Jeff Molofee '99
 * (ported to Linux/SDL by Ti Leggett '01)
 *
 * If you've found this code useful, please let me know.
 *
 * Visit Jeff at http://nehe.gamedev.net/
 *
 * or for port-specific comments, questions, bugreports etc.
 * email to leggett@eecs.tulane.edu
 */
#include <cssysdef.h>

#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"
extern "C"
{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}
#include <csgeom/transfrm.h>
#include <csplugincommon/opengl/glhelper.h>

/* screen width, height, and bit depth */
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768
#define SCREEN_BPP     16

/* Define our booleans */
#define TRUE  1
#define FALSE 0

/* This is our SDL surface */
SDL_Surface *surface;

static GLfloat LightPosition[] = { 0.0f, 2.0f, 2.0f, 1.0f };
static GLfloat LightAmbient[]= { 0.5f, 0.5f, 0.5f, 1.0f };
static GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };

/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode )
{
    /* clean up the window */
    SDL_Quit( );

    /* and exit appropriately */
    exit( returnCode );
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
    /* Height / width ration */
    GLfloat ratio;

    /* Protect against a divide by zero */
   if ( height == 0 )
    height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    return( TRUE );
}

/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym )
{
    switch ( keysym->sym )
    {
    case SDLK_ESCAPE:
        /* ESC key was pressed */
        Quit( 0 );
        break;
    case SDLK_F1:
        /* F1 key was pressed
         * this toggles fullscreen mode
         */
        SDL_WM_ToggleFullScreen( surface );
        break;
    default:
        break;
    }

    return;
}

/* general OpenGL initialization function */
int initGL()
{

    /* Enable smooth shading */
    glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    glEnable(GL_LIGHTING);
    /* Setup The Ambient Light */
    glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );

    /* Setup The Diffuse Light */
    glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );

    /* Position The Light */
    glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );

    /* Enable Light One */
    glEnable( GL_LIGHT1 );

    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glEnable(GL_LINE_SMOOTH);
    return( TRUE );
}

GLuint LoadImage()
{
    GLuint texture;         // This is a handle to our texture object
SDL_Surface *surface;   // This surface will tell us the details of the image
GLenum texture_format;
GLint  nOfColors;

if ( (surface = SDL_LoadBMP("thefuture.bmp")) ) {

    // Check that the image's width is a power of 2
    if ( (surface->w & (surface->w - 1)) != 0 ) {
        printf("warning: image.bmp's width is not a power of 2\n");
    }

    // Also check if the height is a power of 2
    if ( (surface->h & (surface->h - 1)) != 0 ) {
        printf("warning: image.bmp's height is not a power of 2\n");
    }

        // get the number of channels in the SDL surface
        nOfColors = surface->format->BytesPerPixel;
        if (nOfColors == 4)     // contains an alpha channel
        {
                if (surface->format->Rmask == 0x000000ff)
                        texture_format = GL_RGBA;
                else
                        texture_format = GL_BGRA;
        } else if (nOfColors == 3)     // no alpha channel
        {
                if (surface->format->Rmask == 0x000000ff)
                        texture_format = GL_RGB;
                else
                        texture_format = GL_BGR;
        } else {
                printf("warning: the image is not truecolor..  this will probably break\n");
                // this error should not go unhandled
        }

    // Have OpenGL generate a texture object handle for us
    glGenTextures( 1, &texture );

    // Bind the texture object
    glBindTexture( GL_TEXTURE_2D, texture );

    // Set the texture's stretching properties
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    // Edit the texture object's image data using the information SDL_Surface gives us
    glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );
}
else {
    printf("SDL could not load image.bmp: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
}

// Free the SDL_Surface only if it was successfully created
if ( surface ) {
    SDL_FreeSurface( surface );
}
return texture;
}

GLuint LoadTextureRAW( uint8_t* data, int width, int height )
{
  GLuint texture;
  // allocate a texture name
  glGenTextures( 1, &texture);

  // select our current texture
  glBindTexture( GL_TEXTURE_2D, texture );

  // select modulate to mix texture with color for shading
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);

  // when texture area is small, bilinear filter the closest MIP map
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                   GL_LINEAR);
  // when texture area is large, bilinear filter the first MIP map
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  // if wrap is true, the texture wraps over at the edges (repeat)
  //       ... false, the texture ends at the edges (clamp)
  //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  //glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

  // build our texture MIP maps
  //gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB8, width,
    //height, GL_RGB, GL_UNSIGNED_BYTE, data );

  return texture;

}

/* Here goes our drawing code */
int drawGLScene(AVFrame* fr, int width, int height)
{
    static GLuint futtex = 110;
    if (futtex == 110)
        futtex = LoadImage();

    /* rotational vars for the triangle and quad, respectively */
    static GLfloat rtri, rquad;
    /* These are to calculate our fps */
    static GLint T0     = 0;
    static GLint Frames = 0;

        uint8_t data[width * height * 4];
    uint8_t*d = data;
    for (int y = 0; y < height; y++)
    {
      uint8_t* line = fr->data[0] + y*fr->linesize[0];
      memcpy(d, line, 4*width);
      d += 4*width;
      /*for (int x = 0; x < width; x++)
      {
        char r = line[x*4], g = line[x*4 + 1], b = line[x*4 + 2];
        d[0] = b;
        d[1] = g;
        d[2] = r;
        //d[3] = 255;
        d[3] = line[x*4 + 3];
        d += 4;
      }*/
    }
        /*for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int i = y*w + x;
                i *= 3;
                data[i] = 1;
                data[i+1] = 128;
                data[i+2] = 12;
            }
        }*/
        GLuint texture = LoadTextureRAW(data, width, height);
    glClearColor( 0.0f, 0.4f, 0.0f, 0.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         // Clear Screen And Depth Buffer
    glLoadIdentity();                           // Reset The Current Matrix
      // setup texture mapping
      glEnable( GL_TEXTURE_2D );
    glDisable(GL_LIGHTING);

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();

      glBindTexture( GL_TEXTURE_2D, texture );
    glBegin (GL_QUADS);
      glColor3f(   1.0f,  1.0f,  1.0f ); /* Red                           */
    glTexCoord2f(0, 1);
    glVertex3i (-1, -1, 1);
    glTexCoord2f(1, 1);
    glVertex3i (1, -1, 1);
    glTexCoord2f(1, 0);
    glVertex3i (1, 1, 1);
    glTexCoord2f(0, 0);
    glVertex3i (-1, 1, 1);
    glEnd ();

    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
glDeleteTextures( 1, &texture );
//SDL_GL_SwapBuffers( );
//return 0;
    /* Clear The Screen And The Depth Buffer */
    //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* Move Left 1.5 Units And Into The Screen 6.0 */

    glLoadIdentity();
    float m[16];
    csReversibleTransform camtrans;
    camtrans.SetOrigin(csVector3(0,0,0));
    camtrans.LookAt(csVector3(0, 0, 6), csVector3(0,1,0));
    makeGLMatrix (camtrans, m);
    //trans.SetOrigin(csVector3(0));
    //trans.LookAt(csVector3(0,0,-1), csVector3(0,1,0));
    glLoadMatrixf (m);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor4f(0.2f,1.0f,0.2f,0.5f);
    glBegin(GL_LINES);
        glVertex3f( -1.5f, 0.0f, -6.0f );
        glVertex3f( 1.5f, 1.0f, -6.0f );
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    //glLoadIdentity();
    glTranslatef( -1.5f, 0.0f, -6.0f );
glPushMatrix();
    /* Rotate The Triangle On The Y axis ( NEW ) */
    glRotatef( rtri, 0.0f, 1.0f, 0.0f );

    glBegin( GL_TRIANGLES );             /* Drawing Using Triangles       */
      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Front)       */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Left Of Triangle (Front)      */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Right Of Triangle (Front)     */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Right)       */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f(  1.0f, -1.0f,  1.0f ); /* Left Of Triangle (Right)      */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Right Of Triangle (Right)     */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Back)        */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f(  1.0f, -1.0f, -1.0f ); /* Left Of Triangle (Back)       */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Right Of Triangle (Back)      */

      glColor3f(   1.0f,  0.0f,  0.0f ); /* Red                           */
      glVertex3f(  0.0f,  1.0f,  0.0f ); /* Top Of Triangle (Left)        */
      glColor3f(   0.0f,  0.0f,  1.0f ); /* Blue                          */
      glVertex3f( -1.0f, -1.0f, -1.0f ); /* Left Of Triangle (Left)       */
      glColor3f(   0.0f,  1.0f,  0.0f ); /* Green                         */
      glVertex3f( -1.0f, -1.0f,  1.0f ); /* Right Of Triangle (Left)      */
    glEnd( );                            /* Finished Drawing The Triangle */
glPopMatrix();
    /* Move Right 3 Units */
    //glLoadIdentity( );
    glTranslatef( 4, 0, 0 );

    /* Rotate The Quad On The X axis ( NEW ) */
    glRotatef( rquad, 1.0f, 0.0f, 0.0f );

    /* Set The Color To Blue One Time Only */
    glColor3f( 0.5f, 0.5f, 1.0f);

      glBindTexture( GL_TEXTURE_2D, futtex );
    glBegin( GL_QUADS );                 /* Draw A Quad                      */
        // Front Face
        glNormal3f( 0.0f, 0.0f, 1.0f);                  // Normal Pointing Towards Viewer
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 1 (Front)
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 2 (Front)
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Front)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 4 (Front)
        // Back Face
        glNormal3f( 0.0f, 0.0f,-1.0f);                  // Normal Pointing Away From Viewer
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Back)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 2 (Back)
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 3 (Back)
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 4 (Back)
        // Top Face
        glNormal3f( 0.0f, 1.0f, 0.0f);                  // Normal Pointing Up
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 1 (Top)
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 2 (Top)
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Top)
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 4 (Top)
        // Bottom Face
        glNormal3f( 0.0f,-1.0f, 0.0f);                  // Normal Pointing Down
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Bottom)
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 2 (Bottom)
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 3 (Bottom)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 4 (Bottom)
        // Right face
        glNormal3f( 1.0f, 0.0f, 0.0f);                  // Normal Pointing Right
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Point 1 (Right)
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Point 2 (Right)
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Point 3 (Right)
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Point 4 (Right)
        // Left Face
        glNormal3f(-1.0f, 0.0f, 0.0f);                  // Normal Pointing Left
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Point 1 (Left)
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Point 2 (Left)
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Point 3 (Left)
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Point 4 (Left)
    glEnd( );                            /* Done Drawing The Quad            */

    /* Draw it to the screen */
    SDL_GL_SwapBuffers( );

    /* Gather our frames per second */
    Frames++;
    {
    GLint t = SDL_GetTicks();
    if (t - T0 >= 5000) {
        GLfloat seconds = (t - T0) / 1000.0;
        GLfloat fps = Frames / seconds;
        printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
        T0 = t;
        Frames = 0;
    }
    }

    /* Increase The Rotation Variable For The Triangle ( NEW ) */
    rtri  += 0.2f;
    /* Decrease The Rotation Variable For The Quad     ( NEW ) */
    rquad -=0.15f;

    return( TRUE );
}

int main( int argc, char **argv )
{
    /* Flags to pass to SDL_SetVideoMode */
    int videoFlags;
    /* main loop variable */
    int done = FALSE;
    /* used to collect events */
    SDL_Event event;
    /* this holds some info about our display */
    const SDL_VideoInfo *videoInfo;
    /* whether or not the window is active */
    int isActive = TRUE;

  AVFormatContext *pFormatCtx;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx;
  AVCodec         *pCodec;
  AVFrame         *pFrame, *pFrameRGB;
  AVPacket        packet;
  int             frameFinished;
  float           aspect_ratio;

  //SDL_Event       event;

  if(argc < 2) {
    argv[1] = "/home/genjix/media/videos/[BSS]_Genius_Party_Beyond_[DVD]/[BSS]_Genius_Party_Beyond_-_Dimension_Bomb_[1DB25A30].mkv";
  }
  // Register all formats and codecs
  av_register_all();


    /* initialize SDL */
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        fprintf( stderr, "Video initialization failed: %s\n",
             SDL_GetError( ) );
        Quit( 1 );
    }

  // Open video file
  if(av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
    return -1; // Couldn't open file

  // Retrieve stream information
  if(av_find_stream_info(pFormatCtx)<0)
    return -1; // Couldn't find stream information

  // Dump information about file onto standard error
  dump_format(pFormatCtx, 0, argv[1], 0);

  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream

  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;

  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }

  // Open codec
  if(avcodec_open(pCodecCtx, pCodec)<0)
    return -1; // Could not open codec

  // Allocate video frame
  pFrame=avcodec_alloc_frame();

// Allocate an AVFrame structure
  pFrameRGB=avcodec_alloc_frame();
  if(pFrameRGB==NULL)
    return -1;

  // Determine required buffer size and allocate buffer
  int numBytes=avpicture_get_size(PIX_FMT_RGB32, pCodecCtx->width,
                  pCodecCtx->height);
  uint8_t* buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB32,
         pCodecCtx->width, pCodecCtx->height);


    /* Fetch the video info */
    videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
    {
        fprintf( stderr, "Video query failed: %s\n",
             SDL_GetError( ) );
        Quit( 1 );
    }

    /* the flags to pass to SDL_SetVideoMode */
    videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
    videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
    videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
    videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */

    /* This checks to see if surfaces can be stored in memory */
    if ( videoInfo->hw_available )
    videoFlags |= SDL_HWSURFACE;
    else
    videoFlags |= SDL_SWSURFACE;

    /* This checks if hardware blits can be done */
    if ( videoInfo->blit_hw )
    videoFlags |= SDL_HWACCEL;

    /* Sets up OpenGL double buffering */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

    /* get a SDL surface */
    surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
                videoFlags );

    /* Verify there is a surface */
    if ( !surface )
    {
        fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
        Quit( 1 );
    }

    /* initialize OpenGL */
    initGL( );

    /* resize the initial window */
    resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );

    /* wait for events */
    while ( av_read_frame(pFormatCtx, &packet)>=0 || !done)
    {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video(pCodecCtx, pFrame, &frameFinished,
               packet.data, packet.size);

      // Did we get a video frame?
      if(frameFinished) {
    /*SDL_LockYUVOverlay(bmp);

    AVPicture pict;
    pict.data[0] = bmp->pixels[0];
    pict.data[1] = bmp->pixels[2];
    pict.data[2] = bmp->pixels[1];

    pict.linesize[0] = bmp->pitches[0];
    pict.linesize[1] = bmp->pitches[2];
    pict.linesize[2] = bmp->pitches[1];*/

    // Convert the image into YUV format that SDL uses
    //img_convert(&pict, PIX_FMT_YUV420P,
      //              (AVPicture *)pFrame, pCodecCtx->pix_fmt,
        //    pCodecCtx->width, pCodecCtx->height);
      SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
          pCodecCtx->pix_fmt,
          pCodecCtx->width, pCodecCtx->height,
          PIX_FMT_RGB32,
          SWS_BICUBIC, NULL, NULL, NULL);

      sws_scale(img_convert_ctx, pFrame->data,
          pFrame->linesize, 0, pCodecCtx->height,
          pFrameRGB->data, pFrameRGB->linesize);

      sws_freeContext(img_convert_ctx);

    //SDL_UnlockYUVOverlay(bmp);

                                                                                                                                                                                                                                                                drawGLScene(pFrameRGB, pCodecCtx->width, pCodecCtx->height);
      }
    }

    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
        /* handle the events in the queue */

        while ( SDL_PollEvent( &event ) )
        {
            switch( event.type )
            {
            case SDL_ACTIVEEVENT:
                /* Something's happend with our focus
                 * If we lost focus or we are iconified, we
                 * shouldn't draw the screen
                 */
                if ( event.active.gain == 0 )
                isActive = FALSE;
                else
                isActive = TRUE;
                break;
            case SDL_VIDEORESIZE:
                /* handle resize event */
                surface = SDL_SetVideoMode( event.resize.w,
                            event.resize.h,
                            16, videoFlags );
                if ( !surface )
                {
                    fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
                    Quit( 1 );
                }
                resizeWindow( event.resize.w, event.resize.h );
                break;
            case SDL_KEYDOWN:
                /* handle key presses */
                handleKeyPress( &event.key.keysym );
                break;
            case SDL_QUIT:
                /* handle quit requests */
                done = TRUE;
                break;
            default:
                break;
            }
        }

    }

    /* clean ourselves up and exit */
    Quit( 0 );

    /* Should never get here */
    return( 0 );
}
