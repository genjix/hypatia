# -*- coding: utf-8 -*-
from engine import Vector
import time

class Body:
    def __init__(self):
        self.accs = []
        self.mass = 1.0
        self.accumtime = 0.0
        self.lasttime = time.time()
        self.stepsize = 0.001
        self.pos = Vector(0.0)
        self.acc = Vector(0.0)
        self.vel = Vector(0.0)
    def start(self):
        self.lasttime = time.time()
    def addForce(self, force, time):
        self.accs.append([force/self.mass, time])
    def simulate(self, pos, vel, oldacc, newacc, h):
        pos = pos + vel*h + oldacc*(h*h)/2.0
        velhalfw = vel + oldacc*h/2.0
        vel = velhalfw + newacc*h/2.0
        return pos, vel, newacc
    def update(self):
        newtime = time.time()
        self.accumtime += newtime - self.lasttime
        self.lasttime = newtime
        # run steps
        while self.accumtime >= self.stepsize:
            acc = Vector(0.0)
            for aid, a in enumerate(self.accs):
                t = 1.0
                if a[1] > self.stepsize:
                    self.accs[aid][1] -= self.stepsize
                else:
                    t = a[1] / self.stepsize
                    del self.accs[aid]
                acc += a[0] * t
            # simulate
            self.pos, self.vel, self.acc = \
                self.simulate(self.pos, self.vel, self.acc, acc, self.stepsize)
            self.accumtime -= self.stepsize
