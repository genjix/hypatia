# -*- coding: utf-8 -*-
import weakref
import random

class Signal:
    class Slot:
        def __init__(self, fn):
            self.__fn = fn

        def __call__(self, accum, *args, **kwargs):
            result = self.__fn(*args, **kwargs)
            return accum(result)

    class WeakSlot:
        def __init__(self, conn, parent, fn, obj):
            self.__conn = conn
            # Avoid circular references so deleting a signal will
            # allow deletion of the signal since the slot doesn't ref
            # back to it but only weakefs back to it
            self.__parent = weakref.ref(parent)

            self.__fn = fn
            self.__obj = weakref.ref(obj, self.Cleanup)

        def __call__(self, accum, *args, **kwargs):
            obj = self.__obj()
            if obj is None:
                return True

            result = self.__fn(obj, *args, **kwargs)
            return accum(result)

        def Cleanup(self, ref):
            parent = self.__parent()
            if parent is not None:
                parent.Disconnect(self.__conn)

    class Accumulator:
        def __call__(self, *args, **kwargs):
            return True

        def Finalize(self):
            return None

    def __init__(self):
        self.__slots = {}

    # This connects a signal to a slot, but stores a strong reference so
    # The object will not be deleted as long as the signal is connected
    def Connect(self, fn, runlevel=0):
        conn = self.NewConn(runlevel)
        self.__slots[conn] = Signal.Slot(fn)
        return conn

    # This connects a signal to a slot, but store a weak reference so
    # when the object is gone the slot will not be called.  Because of
    # the implemenations, it is not possible to do WeakConnect(obj.Fn),
    # since obj.Fn is a new object and would go to 0 refcount soon after
    # the call to WeakConnect completes.  Instead we must do a call as
    # WeakConnect(ObjClass.Fn, obj)
    # Only the object is weak-referenced.  The function object is still
    # a normal reference, this ensures that as long as the object exists
    # the function will also exist.  When the object dies, the slot will
    # be removed
    def WeakConnect(self, fn, obj, runlevel=0):
        conn = self.NewConn(runlevel)
        self.__slots[conn] = Signal.WeakSlot(conn, self, fn, obj)
        return conn

    # Disconnect a slot
    def Disconnect(self, conn):
        try:
            del self.__slots[conn]
        except KeyError:
            pass

    # Disconnect all slots
    def DisconnectAll(self):
        self.__slots = {}

    # Create an accumulator.  Accumulator will be called as a callable
    # for each return value of the executed slots.  Execution of slots
    # continues as long as the reutrn value of the accumulator call is
    # True.  The 'Finalize'function will be called to get the result
    # A custom accumulator can be created by deriving from Signal and
    # Creating a custom 'Accumulator' class, or by deriving from Singal
    # and creating CreateAccumulator
    def CreateAccumulator(self):
        return self.Accumulator()

    # Execute the slots
    def __call__(self, *args, **kwargs):
        accum = self.CreateAccumulator()
        slotitems = self.__slots.items()
        slotitems.sort()
        for conn, slot in slotitems:
            if not slot(accum, *args, **kwargs):
                break
        return accum.Finalize()

    # Create a connection name
    def NewConn(self, value):
        while self.__slots.has_key(value):
            value += 1
        return value

# End Signal
if __name__ == "__main__":
    def fn1():
        print "Hello World"

    def fn2():
        print "Goodbye Space"

    class O:
        def __init__(self, value):
            self.value = value

        def Action(self):
            print "O %d" % self.value

    a = Signal()

    a.Connect(fn1)
    a.Connect(fn2)

    print "Part 1"
    a()

    a.DisconnectAll()

    o1 = O(4)
    o2 = O(12)

    a.WeakConnect(O.Action, o1)
    a.Connect(o2.Action)

    print "Part 2"
    a()

    print "Part 3"
    o1 = None
    a()

    print "Part 4"
    o2 = None
    a()

    a.DisconnectAll()

    def f1():
        print "Hello Neighbor"

    def f2():
        print "Back to Work"

    c1 = a.Connect(f1)
    c2 = a.Connect(f2)

    print "Part 5"
    a()

    print "Part 6"
    a.Disconnect(c2)
    a()

    a.DisconnectAll()

    def f1(name):
        print "Hello %s" % name

    def f2(name):
        print "Goodbye %s" % name

    a.Connect(f1, 10)
    a.Connect(f2, 2)

    print "Part 7"
    #a() # Error
    a("Sarah")

    a.DisconnectAll()

    class MySignal(Signal):
        class Accumulator:
            def __init__(self):
                self.value = 0
            def __call__(self, value):
                self.value += value
                return bool(value != 0)
            def Finalize(self):
                return self.value


    def f1(x):
        return x * x

    def f2(x):
        return x + x

    def f3(x):
        return 0

    a = MySignal()
    a.Connect(f1)
    a.Connect(f2)
    a.Connect(f3)

    print "Part 8"
    print a(5)
