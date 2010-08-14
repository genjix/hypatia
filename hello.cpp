//  Copyright Joel de Guzman 2002-2004. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)
//  Hello World Example from the tutorial
//  [Joel de Guzman 10/9/2002]

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/class.hpp>
#include <boost/python/operators.hpp>
#include <cssysdef.h>
#include <csgeom/transfrm.h>
#include <csgeom/quaternion.h>
#include <csgeom/matrix3.h>
#include <csgeom/plane3.h>
#include <csplugincommon/opengl/glhelper.h>
#include <csgeom/vector3.h>
#include <string>
#include <boost/lexical_cast.hpp>
#include "videoplayer.h"
#include "trackpar.h"
#include "evensys.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <lo/lo.h>

void quit( int returnCode )
{
    /* clean up the window */
    SDL_Quit( );

    /* and exit appropriately */
    exit( returnCode );
}
/* function to reset our viewport after a window resize */
bool resizeWindow( int width, int height )
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

    return true;
}
void appInit()
{
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        fprintf( stderr, "Video initialization failed: %s\n",
             SDL_GetError( ) );
        quit( 1 );
    }
    /* Fetch the video info */
    const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo( );

    if ( !videoInfo )
    {
        fprintf( stderr, "Video query failed: %s\n",
             SDL_GetError( ) );
        quit( 1 );
    }

    int videoFlags;
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
    SDL_Surface* surface = SDL_SetVideoMode( 1024, 768, 32,
                videoFlags );

    /* Verify there is a surface */
    if ( !surface )
    {
        fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
        quit( 1 );
    }
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

    // needed for blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    /*glAlphaFunc(GL_GREATER,0.1);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);*/

    /* Really Nice Perspective Calculations */
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    glEnable(GL_LIGHTING);

    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
    glEnable(GL_LINE_SMOOTH);
    resizeWindow(1024, 768);
}
GLuint LoadImage(const char* name)
{
    GLuint texture;         // This is a handle to our texture object
SDL_Surface *surface;   // This surface will tell us the details of the image
GLenum texture_format;
GLint  nOfColors;

    SDL_Surface* Surf_Temp = NULL;

    if((Surf_Temp = IMG_Load(name)) == NULL) {
        return 0;
    }

    surface = SDL_DisplayFormatAlpha(Surf_Temp);
    SDL_FreeSurface(Surf_Temp);

if (surface) {

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

    // select modulate to mix texture with color for shading
    //glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

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

void setupCamera(const csReversibleTransform& camtrans)
{
    float m[16];
    makeGLMatrix (camtrans, m);
    glLoadMatrixf (m);
}
void loadMatrix(const csMatrix3& q)
{
    float m[16];
    makeGLMatrix(q, m);
    glLoadMatrixf (m);
}
void setupMatrix(const csMatrix3& q)
{
    float m[16];
    makeGLMatrix(q, m);
    glMultMatrixf (m);
}
std::string VecRepr(const csVector3& v)
{
    return std::string("(") + boost::lexical_cast<std::string>(v.x) + ", " +
        boost::lexical_cast<std::string>(v.y) + ", " +
        boost::lexical_cast<std::string>(v.z) + ")";
}

class OSC
{
  public:
    bool connect(const char* serv, int port)
    {
        disconnect();
        std::string portstr = boost::lexical_cast<std::string>(port);
        t = lo_address_new(serv, portstr.c_str());
        return true;
    }
    bool disconnect()
    {
        lo_address_free(t);
        return true;
    }
    int send(const char* path, boost::python::list params)
    {
        lo_message msg = lo_message_new();

        boost::python::ssize_t n = boost::python::len(params);
        for(boost::python::ssize_t i = 0; i < n; i++)
        {
            // the most evil piece of shit i ever typed in my life
            boost::python::object elem = params[i];
            {
                boost::python::extract<int> get(elem);
                if(get.check()) {
                    lo_message_add_int32(msg, get());
                    continue;
                }
            }
            {
                boost::python::extract<float> get(elem);
                if(get.check()) {
                    lo_message_add_float(msg, get());
                    continue;
                }
            }
            {
                boost::python::extract<const char*> get(elem);
                if(get.check()) {
                    lo_message_add_string(msg, get());
                    continue;
                }
            }
            {
                boost::python::extract<bool> get(elem);
                if(get.check()) {
                    if(get())
                        lo_message_add_true(msg);
                    else
                        lo_message_add_false(msg);
                    continue;
                }
            }
            // oh no!
            lo_message_add_nil(msg);
        }
        int ret = lo_send_message(t, path, msg);
        lo_message_free(msg);

        return ret;
    }
    lo_address t;
};

static csVector3 TransformsOrigin(const csReversibleTransform& t)
{
    return t.GetOrigin();
}
static csQuaternion MultipleyQuat(const csQuaternion& q1, const csQuaternion& q2)
{
    return q1 * q2;
}
static csMatrix3 MultiplyMatrix(const csMatrix3&m, const csMatrix3 &m1)
{
    return m * m1;
}
static csMatrix3 AddMatrix(const csMatrix3&m, const csMatrix3 &m1)
{
    return m + m1;
}
static csMatrix3 MultiplyMatrixFloat(const csMatrix3&m, const float f)
{
    return m * f;
}
static csVector3 MatByVec(const csMatrix3&m, const csVector3&v)
{
    return m * v;
}
static csVector3 addVector(const csVector3&v, const csVector3&v1)
{
    return v + v1;
}
static csVector3 subVector(const csVector3&v, const csVector3&v1)
{
    return v - v1;
}
static csVector3 raddVector(csVector3&v, const csVector3&v1)
{
    v += v1;
    return v;
}
static csVector3 rsubVector(csVector3&v, const csVector3&v1)
{
    v -= v1;
    return v;
}
static csVector3 vecDivide(const csVector3&v, float x)
{
    return v / x;
}
static csVector3 vecMULL(const csVector3&v, float x)
{
    return v * x;
}
csVector3 ThisToOtherVector(const csReversibleTransform& t, const csVector3&v)
{
    return t.This2Other(v);
}
csVector3 ThisToOtherRelativeVector(const csReversibleTransform& t, const csVector3&v)
{
    return t.This2OtherRelative(v);
}
float VNorm(const csVector3& v)
{
    return v.Norm();
}
csVector3 crossVec(const csVector3 &a, const csVector3 &b)
{
    return a%b;
}
static float GetPlaneAPart(const csPlane3 &p)
{
    return p.A();
}
static void SetPlaneAPart(csPlane3 &p, float x)
{
    p.norm.x = x;
}
static float GetPlaneBPart(const csPlane3 &p)
{
    return p.B();
}
static void SetPlaneBPart(csPlane3 &p, float x)
{
    p.norm.y = x;
}
static float GetPlaneCPart(const csPlane3 &p)
{
    return p.C();
}
static void SetPlaneCPart(csPlane3 &p, float x)
{
    p.norm.z = x;
}
static float GetPlaneDPart(const csPlane3 &p)
{
    return p.D();
}
static void SetPlaneDPart(csPlane3 &p, float x)
{
    p.DD = x;
}

BOOST_PYTHON_MODULE(engine)
{
    using namespace boost::python;
    def("init", appInit);
    def("gluLookAt", gluLookAt);
    def("quit", quit);
    def("flipBuffers", SDL_GL_SwapBuffers);
    def("delay", SDL_Delay);
    def("loadTexture", LoadImage);
    def("getTicks", SDL_GetTicks);
    def("setupTransform", setupCamera);
    def("loadMatrix", loadMatrix);
    def("setupMatrix", setupMatrix);
    def("matByFloat", MultiplyMatrixFloat);
    void (csVector3::*Set1)(float, float, float) = &csVector3::Set;
    class_<csVector3>("Vector", init<>())
        .def(init<const csVector3&>())
        .def(init<float>())
        .def(init<float, float, float>())
        .def("set", Set1)
        .def("normalize", &csVector3::Normalize)
        .def("cross", &crossVec)
        .def("length", &VNorm)
        .def("normSquared", &csVector3::SquaredNorm)
        .def_readwrite("x", &csVector3::x)
        .def_readwrite("y", &csVector3::y)
        .def_readwrite("z", &csVector3::z)
        //.def(class_<csVector3>::self - other<csVector3>())
        .def("__repr__", &VecRepr)
        .def("__add__", &addVector)
        .def("__sub__", &subVector)
        .def("__radd__", &raddVector)
        .def("__rsub__", &rsubVector)
        .def("__div__", &vecDivide)
        .def("__mul__", &vecMULL)
    ;
    class_<csMatrix3>("Matrix", init<>())
        .def(init<const csMatrix3&>())
        .def(init<float, float, float, float, float, float, float, float, float>())
        .def_readwrite("m11",  &csMatrix3::m11)
        .def_readwrite("m21",  &csMatrix3::m21)
        .def_readwrite("m31",  &csMatrix3::m31)
        .def_readwrite("m12",  &csMatrix3::m12)
        .def_readwrite("m22",  &csMatrix3::m22)
        .def_readwrite("m32",  &csMatrix3::m32)
        .def_readwrite("m13",  &csMatrix3::m13)
        .def_readwrite("m23",  &csMatrix3::m23)
        .def_readwrite("m33",  &csMatrix3::m33)
        .def("__mul__", &MultiplyMatrix)
        .def("__add__", &AddMatrix)
        .def("applyVector", &MatByVec)
    ;
    void (csReversibleTransform::*RotateThisVector)(const csVector3&, float) =
        &csReversibleTransform::RotateThis;
    class_<csReversibleTransform>("Transform", init<>())
        .def(init<const csMatrix3 &, const csVector3 &>())
        .def("LookAt", &csReversibleTransform::LookAt)
        .def("SetOrigin", &csReversibleTransform::SetOrigin)
        .def("Translate", &csReversibleTransform::Translate)
        .def("RotateThis",  RotateThisVector)
        .def("GetOrigin",  &TransformsOrigin)
        .def("ToOther", &ThisToOtherVector)
        .def("ToOtherRel", &ThisToOtherRelativeVector)
    ;
    class_<csQuaternion>("Quaternion", init<>())
        .def(init<const csVector3&, float>())
        .def(init<const csQuaternion &>())
        .def(init<float, float, float, float>())
        .def("SetAxisAngle", &csQuaternion::SetAxisAngle)
        .def("GetMatrix", &csQuaternion::GetMatrix)
        .def("__mul__", &MultipleyQuat)
        .def_readwrite("v",  &csQuaternion::v)
        .def_readwrite("w",  &csQuaternion::w)
    ;
    class_<csPlane3>("Plane", init<>())
        .def(init<float, float, float, float>())
        .def("setOrigin", &csPlane3::SetOrigin)
        .add_property("a", &GetPlaneAPart, &SetPlaneAPart)
        .add_property("b", &GetPlaneBPart, &SetPlaneBPart)
        .add_property("c", &GetPlaneCPart, &SetPlaneCPart)
        .add_property("d", &GetPlaneDPart, &SetPlaneDPart)
        .def_readwrite("norm",  &csPlane3::norm)
    ;
    class_<VideoPlayer>("VideoPlayer")
        .def("init", &VideoPlayer::init)
        .def("run", &VideoPlayer::run)
        .def_readwrite("die",  &VideoPlayer::time2die)
        .def("display", &VideoPlayer::display)
    ;
    class_<Event>("Event")
        .def_readonly("type", &Event::type)
        .def_readonly("params", &Event::params)
    ;
    class_<EventSystem>("EventSystem")
        .def("pollEvents", &EventSystem::pollEvents)
        .def("pop", &EventSystem::pop)
        .def("empty", &EventSystem::empty)
        .def("runOSCServer", &EventSystem::runOSCServer)
    ;
    class_<OSC>("OpenSoundControl")
        .def("connect", &OSC::connect)
        .def("disconnect", &OSC::disconnect)
        .def("send", &OSC::send)
    ;
    class_<Track>("TrackPart", init<>())
        .def_readwrite("verts",  &Track::verts)
        .def_readwrite("mats",  &Track::mats)
        .def_readwrite("resol",  &Track::resol)
        .def("load", &Track::load)
        .def("draw", &Track::draw)
        .def("attachPlayer", &Track::attachToTrack)
    ;
}
