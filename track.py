# -*- coding: utf-8 -*-
from OpenGL.GL import *
import game
from engine import Vector, Matrix, matByFloat, TrackPart
import verts
from cubic import cubicInterpolate
import pickle

class Track(game.Component, TrackPart):
    def __init__(self, updates, text):
        TrackPart.__init__(self)
        updates.Connect(self.draw, 6002)
        f = open("track.dat", "r")
        v, m = pickle.load(f)
        self.load(v, m, text)
        self.resol = 0

if __name__ == "__main__":
    print "bager!"
