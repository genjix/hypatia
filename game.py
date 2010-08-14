# -*- coding: utf-8 -*-
import signals

class TestComponent:
    def __init__(self, parententity, name):
        pass
    def sendMessage(self, propname, msg):
        print propname, msg
        return "returned..."

class Entity:
    def __init__(self, name):
        self.comps = {}
    def sendMessage(self, msg):
        try:
            return self.comps[msg[0][0]].sendMessage(msg[0][1], msg[1])
        except KeyError:
            return None

class InvalidRequest(Exception):
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return "Invalid method/property request: " + str(self.msg)

class Component:
    def __init__(self):
        self.comps = {}
    def sendMessage(self, hier, msg):
        hier = list(hier[:])
        next = hier.pop()
        try:
            if len(hier) == 0:
                getattr(self, next)(*msg)
            else:
                return self.comps[next].sendMessage(hier, msg)
        except KeyError:
            raise InvalidRequest(msg)
    def findComp(self, hier):
        hier = list(hier[:])
        next = hier.pop()
        try:
            if len(hier) == 0:
                return self.comps[next]
            else:
                return self.comps[next].findComp(hier)
        except KeyError:
            raise InvalidRequest("findComp() " + next + ", " + str(hier))

    def setValue(self, name, val):
        try:
            preval = getattr(self, name)
            val = preval.__class__(val)
        except KeyError:
            raise InvalidRequest(name + "=" + str(val))
        except TypeError:
            pass
        setattr(self, name, val)

    def add(self, name, comp):
        self.comps[name] = comp

# signals slots
#properties, methods, entities
# set/get on props signals

if __name__ == "__main__":
    class A(Component):
        pass
    class B(Component):
        def __init__(self):
            Component.__init__(self)
            self.y = 110
        def foo(self, x):
            print "bar", x

    a = A()
    b = B()
    a.comps["person"] = b
    msg = ("foo", "person"), (10,)
    msg = ("setValue", "person"), ("y", 4.5)
    a.sendMessage(*msg)
    print b.y
