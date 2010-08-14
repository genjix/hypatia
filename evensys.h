#ifndef EVENTSYM_H
#define EVENTSYM_H

#include <boost/python/list.hpp>
#include <stack>
#include <string>
#include <vector>
#include "lo/lo.h"

struct SDL_keysym;

struct Event
{
    Event() {}
    Event(const char* type) : type (type) {}
    void add(const char* par) { params.append(par); }
    std::string type;
    boost::python::list params;
};

class EventSystem
{
  public:
    EventSystem();
    void pollEvents();
    Event pop();
    bool empty() { return events.empty(); }
    bool runOSCServer(int port);

    void handleKeyPress(SDL_keysym *keysym, bool down);
    void handleOSCMessage(const char *path, const char *types, lo_arg **argv,
        int argc, void *data);
    std::stack<Event> events;
};

#endif
