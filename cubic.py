# -*- coding: utf-8 -*-
import math

def clamp(v, floor, ceil):
    if v < floor:
        return floor
    elif v > ceil:
        return ceil
    return v

def putKey(pix, x, y):
    y = 600 - y - 1
    x = clamp(x, 1, 800-2)
    y = clamp(y, 1, 600-2)
    pix[x-1, y-1] = 0xff0000
    pix[x,   y-1] = 0xff0000
    pix[x+1, y-1] = 0xff0000
    pix[x-1, y]   = 0xff0000
    pix[x,   y]   = 0xff0000
    pix[x+1, y]   = 0xff0000
    pix[x-1, y+1] = 0xff0000
    pix[x,   y+1] = 0xff0000
    pix[x+1, y+1] = 0xff0000
def putPnt(pix, x, y, col=0x0000ff):
    y = 600 - y - 1
    x = clamp(x, 0, 800-1)
    y = clamp(y, 0, 600-1)
    pix[x, y] = col

def linearInterpolate(i, y0, y1):
    return y0 + i*(y1 - y0)

class Vector:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __add__(self, v):
        return Vector(self.x + v.x, self.y + v.y)
    def __sub__(self, v):
        return Vector(self.x - v.x, self.y - v.y)
    def __mul__(self, a):
        return Vector(a * self.x, a * self.y)
    def length(self):
        return math.sqrt(self.x**2 + self.y**2)

def multiply(a, b):
    return a*b

def cubicInterpolate(t, p1, p2, p3, p4, mull=multiply):
    t2 = t * t
    t3 = t2 * t

    b1 = .5 * (  -t3 + 2*t2 - t)
    b2 = .5 * ( 3*t3 - 5*t2 + 2)
    b3 = .5 * (-3*t3 + 4*t2 + t)
    b4 = .5 * (   t3 -   t2    )

    return mull(p1,b1) + mull(p2,b2) + mull(p3,b3) + mull(p4,b4)

class CatmullRom:
    def __init__(self):
        self.pts = []
    def basis(self, i, t):
        if i == -2:
            return ((-t+2)*t-1)*t/2
        elif i == -1:
            return (((3*t-5)*t)*t+2)/2
        elif i == 0:
            return ((-3*t+4)*t+1)*t/2
        elif i == 1:
            return ((t-1)*t*t)/2

    def point(self, i, t):
        px = 0
        py = 0
        for j in xrange(-2,2,1):
            px += self.basis(j,t) * self.pts[i+j][0]
            py += self.basis(j,t) * self.pts[i+j][1]
        return (px, py)

    def paint(self, pix):
        q = self.point(2,0)
        putKey(pix, *q)

        for i in xrange(1,len(self.pts)-1,1):
            for j in xrange(1,30,1):
                q = self.point(i, j/30.0)
                putPnt(pix, *q)

# http://www.cse.unsw.edu.au/~lambert/splines/source.html
if __name__ == "__main__":
    import Image, os
    graph = Image.new("RGB", (800, 600))
    pix = graph.load()
    #key_points = [(60, 0), (100, 200), (300, 80), (400, 50), (600, 400), (800, 5), (900,5)]
    key_points = [(100, 80), (300, 300), (100, 200), (200, 200), (400, 80), (500, 80), (520, 81), (700, 5), (780,100), (780, 100)]
    #key_points = []
    #for i in xrange(10):
    #    key_points.append((i*100, 600*i**2/64.0))

    """c = CatmullRom()
    c.pts = key_points
    c.paint(pix)"""

    null = Vector(*key_points[0])
    tablp = [null, null, Vector(*key_points[0]), Vector(*key_points[1])]
    putKey(pix, *key_points[0])
    putKey(pix, *key_points[1])
    for kp in key_points[2:]:
        """tablp[0] = tablp[1]
        tablp[1] = tablp[2]
        tablp[2] = tablp[3]
        tablp[3] = Vector(*kp)"""
        tablp = tablp[1:]
        tablp.append(Vector(*kp))
        axdist = (tablp[2] - tablp[1]).length()*2
        for x in xrange(int(axdist)):
            i = x / axdist
            tweeny = linearInterpolate(i, tablp[2].y, tablp[3].y)
            #putPnt(pix, x + tablp[2].x, tweeny, 0x00ff00)
            tween = cubicInterpolate(i, *tablp)
            putPnt(pix, tween.x, tween.y)

        putKey(pix, *kp)

    graph.save("/tmp/plot.png")
    os.system("display /tmp/plot.png")
