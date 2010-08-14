# -*- coding: utf-8 -*-
from engine import EventSystem as engEventSystem
import signals

class IterableWrapper:
    def __init__(self, evsys):
        self.evsys = evsys
    def __iter__(self):
        return self
    def next(self):
        if self.evsys.empty():
            raise StopIteration
        ev = self.evsys.pop()
        params = []
        for p in ev.params:
            params.append(eval(p))
        return ev.type, params

class EventSystem(engEventSystem):
    def __init__(self):
        engEventSystem.__init__(self)
        self.evmap = {}
    def events(self):
        return IterableWrapper(self)
    def signal(self, type):
        if not self.evmap.has_key(type):
            self.evmap[type] = signals.Signal()
        return self.evmap[type]
    def update(self):
        for evtype, evpar in self.events():
            #print evtype, evpar
            try:
                self.evmap[evtype](evpar)
            except KeyError:
                pass
