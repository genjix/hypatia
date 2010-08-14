#include "evensys.h"

#include <SDL/SDL.h>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
//#include <boost/foreach.hpp>
//#define foreach BOOST_FOREACH

static std::string oscTypePython(char type)
{
    switch(type)
    {
    /** 32 bit signed integer. */
    case (LO_INT32):
        return "int";
    /** 32 bit IEEE-754 float. */
    case (LO_FLOAT):
        return "float";
    /** Standard C, NULL terminated string. */
    case (LO_STRING):
        return "str";
    /** OSC binary blob type. Accessed using the lo_blob_*() functions. */
    case (LO_BLOB):
    //params.GetParameter (i+1).Set (v->i32);
        break;
    /* extended OSC types */
    /** 64 bit signed integer. */
    case (LO_INT64):
        return "int";
    /** OSC TimeTag type, represented by the lo_timetag structure. */
    case (LO_TIMETAG):
    //params.GetParameter (i+1).Set (v->i32);
        break;
    /** 64 bit IEEE-754 double. */
    case (LO_DOUBLE):
        return "float";
    /** Standard C, NULL terminated, string. Used in systems which
    * distinguish strings and symbols. */
    case (LO_SYMBOL):
        return "str";
    /** Standard C, 8 bit, char variable. */
    case (LO_CHAR):
        return "str";
    /** A 4 byte MIDI packet. */
    case (LO_MIDI):
    //params.GetParameter (i+1).Set (v->i32);
/*printf("MIDI [");
for (i=0; i<4; i++) {
    printf("0x%02x", *((uint8_t *)(data) + i));
    if (i+1 < 4) printf(" ");
}
printf("]");*/
        break;
    /** Symbol representing the value True. */
    case (LO_TRUE):
        return "bool";
    /** Symbol representing the value False. */
    case (LO_FALSE):
        return "bool";
    /** Symbol representing the value Nil. */
    case (LO_NIL):
        return "int";
    /** Symbol representing the value Infinitum. */
    case (LO_INFINITUM):
        return "int";
    default:
        break;
    }
    return "";
}
static std::string oscValueStr(char type, lo_arg* v)
{
    #define cast(x, y) boost::lexical_cast<std::string>((x)y)
    switch (type)
    {
    /** 32 bit signed integer. */
    case (LO_INT32):
        return cast(int32_t, v->i32);
    /** 32 bit IEEE-754 float. */
    case (LO_FLOAT):
        return cast(float,v->f32);
    /** Standard C, NULL terminated string. */
    case (LO_STRING):
        return cast(const char*,v);
    /** OSC binary blob type. Accessed using the lo_blob_*() functions. */
    case (LO_BLOB):
    //params.GetParameter (i+1).Set (v->i32);
        break;
    /* extended OSC types */
    /** 64 bit signed integer. */
    case (LO_INT64):
        return cast(int64_t,v->i64);
    /** OSC TimeTag type, represented by the lo_timetag structure. */
    case (LO_TIMETAG):
    //params.GetParameter (i+1).Set (v->i32);
        break;
    /** 64 bit IEEE-754 double. */
    case (LO_DOUBLE):
        return cast(double,v->f64);
    /** Standard C, NULL terminated, string. Used in systems which
    * distinguish strings and symbols. */
    case (LO_SYMBOL):
        return cast(const char*,v);
    /** Standard C, 8 bit, char variable. */
    case (LO_CHAR):
        return cast(char,v->c);
    /** A 4 byte MIDI packet. */
    case (LO_MIDI):
    //params.GetParameter (i+1).Set (v->i32);
/*printf("MIDI [");
for (i=0; i<4; i++) {
    printf("0x%02x", *((uint8_t *)(data) + i));
    if (i+1 < 4) printf(" ");
}
printf("]");*/
        break;
    /** Symbol representing the value True. */
    case (LO_TRUE):
        return cast(const char*,"True");
    /** Symbol representing the value False. */
    case (LO_FALSE):
        return cast(const char*,"False");
    /** Symbol representing the value Nil. */
    case (LO_NIL):
        return cast(int,0);
    /** Symbol representing the value Infinitum. */
    case (LO_INFINITUM):
        return cast(int,99999999);
    default:
        break;
    }
    #undef cast
    return "None";
}
static int generic_handler(const char *path, const char *types, lo_arg **argv,
            int argc, void *data, void *user_data)
{
    EventSystem* evsys = reinterpret_cast<EventSystem*>(user_data);
    if (!evsys)
        return 1;
    evsys->handleOSCMessage(path, types, argv, argc, data);
    return 0;
}
static void loerror(int num, const char *msg, const char *path)
{
    fprintf(stderr, "liblo server error %d in path %s: %s\n", num, path, msg);
    fflush(stderr);
}
static void threadOSC(int port, EventSystem* evsys)
{
    std::string portstr = boost::lexical_cast<std::string>(port);
    lo_server_thread st = lo_server_thread_new(portstr.c_str(), loerror);
    lo_server_thread_add_method(st, NULL, NULL, generic_handler, evsys);
    lo_server_thread_start(st);
}

EventSystem::EventSystem()
{
}
Event EventSystem::pop()
{
    Event ev = events.top();
    events.pop();
    return ev;
}
bool EventSystem::runOSCServer(int port)
{
    boost::thread thr(boost::bind(&threadOSC, port, this));
    return true;
}
void EventSystem::handleKeyPress(SDL_keysym *keysym, bool down)
{
    std::string evtype(down ? "keydown_" : "keyup_");
    switch (keysym->sym)
    {
  #include "keys.hpp"
    default:
        break;
    }
    Event ev(evtype.c_str());
    events.push(ev);
}
void EventSystem::handleOSCMessage(const char *path, const char *types, lo_arg **argv,
    int argc, void *data)
{
    Event ev("osc_msg");
    std::string par("\"");
    par += path;
    par += "\"";
    ev.add(par.c_str());
    for (int i = 0; i < argc; i++) {
        par = oscTypePython(types[i]);
        par += "(\"";
        par += oscValueStr(types[i], argv[i]);
        par += "\")";
        ev.add(par.c_str());
    }
    events.push(ev);
}
void EventSystem::pollEvents()
{
    Event* ev;
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        switch(event.type)
        {
        case SDL_ACTIVEEVENT:
            ev = new Event("window_focus");
            if(event.active.gain == 0)
                ev->add("bool(False)");
            else
                ev->add("bool(True)");
            events.push(*ev);
            delete ev;
            break;
        case SDL_VIDEORESIZE:
            events.push(Event("window_resize"));
            break;
        case SDL_KEYDOWN:
            handleKeyPress(&event.key.keysym, true);
            break;
        case SDL_KEYUP:
            handleKeyPress(&event.key.keysym, false);
            break;
        case SDL_QUIT:
            events.push(Event("window_quit"));
            break;
        default:
            break;
        }
    }
}
