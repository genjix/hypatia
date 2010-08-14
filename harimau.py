# -*- coding: utf-8 -*-
from OpenGL.GL import *
import engine as en
from engine import Vector, Matrix, Plane
from bind import *
import math
import physics, verts

class Face:
    def __init__(self):
        self.v = [0] * 3
        self.tex = 0
        self.uvs = [(0,0)] * 3
        self.normal = [0] * 3

def ClosestOnPlane(plane, p):
    a, b, c, d = plane.a, plane.b, plane.c, plane.d
    squares = a*a + b*b + c*c
    t = (-d - a*p.x - b*p.y - c*p.z) / squares
    return Vector(p.x + a*t, p.y + b*t, p.z + c*t)

def BezierInterpolate(q0, q1, q2, q3, i):
    return i**3*q3 + 3*i**2*q2 - 3*i**3*q2 + 3*i*q1 - 6*i**2*q1 + 3*i**3*q1 + q0 - 3*i*q0 + 3*i**2*q0 - i**3*q0

def cubicInterpolate(y0, y1, y2, y3, mu):
    mu2 = mu*mu
    a0 = y3 - y2 - y0 + y1
    a1 = y0 - y1 - a0
    a2 = y2 - y0
    a3 = y1

    return a0*mu*mu2 + a1*mu2 + a2*mu + a3

def fetchUp(idx):
    cm = verts.tr[idx][1]
    return Vector(cm.m21, cm.m22, cm.m23)

class Ship():
    def __init__(self, evsys, updater, track):
        self.track = track
        f = open("models/harimau")
        l = f.readline()[:-1]
        if not "faces" in l:
            print "errror"
        faclen = int(l.split(" ")[1])
        l = f.readline()[:-1]
        if not "verts" in l:
            print "errror"
        vertlen = int(l.split(" ")[1])
        self.texref = []
        self.faces = []
        self.verts = []
        for i in xrange(faclen):
            ltex = f.readline()[:-1].split(" ")[0].strip()
            ldat = f.readline()[:-1].split("|")
            verts = ldat[0].split(" ")[:-1]
            uvs = ldat[1].split(" ")
            normal = f.readline()[:-1].split(" ")
            face = Face()
            face.v = [int(v) for v in verts]
            face.normal = [float(v) for v in normal]
            tmuvs = [float(uv) for uv in uvs]
            face.uvs[0] = tmuvs[0], tmuvs[1]
            face.uvs[1] = tmuvs[2], tmuvs[3]
            face.uvs[2] = tmuvs[4], tmuvs[5]
            if ltex not in self.texref:
                self.texref.append(ltex)
            face.tex = self.texref.index(ltex)
            self.faces.append(face)
        for i in xrange(vertlen):
            normal = f.readline()[:-1].split(" ")
            vert = [float(v) for v in normal]
            self.verts.append(vert)
        self.texglref = []
        for fname in self.texref:
            self.texglref.append(en.loadTexture("textures/" + fname))
        evsys.signal("keydown_left").Connect(bind(self.startMove, 0, -1, 0.8))
        evsys.signal("keyup_left").Connect(bind(self.stopMove, 0, -1, 0.8))
        evsys.signal("keydown_right").Connect(bind(self.startMove, 0, 1, -0.8))
        evsys.signal("keyup_right").Connect(bind(self.stopMove, 0, 1, -0.8))
        evsys.signal("keydown_up").Connect(bind(self.startMove, 1, 0, 0))
        evsys.signal("keyup_up").Connect(bind(self.stopMove, 1, 0, 0))
        evsys.signal("keydown_down").Connect(bind(self.startMove, -1, 0, 0))
        evsys.signal("keyup_down").Connect(bind(self.stopMove, -1, 0, 0))
        evsys.signal("keydown_pageup").Connect(bind(self.startMove, 0, 0, 1))
        evsys.signal("keyup_pageup").Connect(bind(self.stopMove, 0, 0, 1))
        evsys.signal("keydown_pagedown").Connect(bind(self.startMove, 0, 0, -1))
        evsys.signal("keyup_pagedown").Connect(bind(self.stopMove, 0, 0, -1))
        evsys.signal("keydown_x").Connect(bind(self.setForward, True))
        evsys.signal("keyup_x").Connect(bind(self.setForward, False))
        updater.Connect(self.draw, 6000)
        self.rot = en.Vector(0,0,0)
        self.osc = en.OpenSoundControl()
        self.osc.connect("localhost", 9002)
        self.move = True
        self.moverot = en.Vector(0)
        self.movebody = physics.Body()
        self.rotbody = physics.Body()
        self.m = en.Matrix()
        self.forward = False
        self.upv = Vector(0,1,0)
    def setForward(self, ison):
        self.forward = ison
    def computeForward(self):
        if not self.forward:
            return
        dir = en.Vector(self.m.m31,self.m.m32,self.m.m33)
        self.movebody.addForce(dir*60, 0.1)
    def computeTurn(self):
        if self.moverot.length() < 0.00001:
            return
        self.rotbody.addForce(self.moverot*20, 0.1)
    def computeDrag(self):
        veldir = en.Vector(self.movebody.vel)
        veldir.normalize()
        speed = self.movebody.vel.length()
        self.movebody.addForce(veldir*speed*speed*-1, 0.01)

        turndir = en.Vector(self.rotbody.vel)
        turndir.normalize()
        speed = self.rotbody.vel.length()
        self.rotbody.addForce(turndir*speed*speed*-40, 0.01)
    def startMove(self, x, y, z):
        if x:
            self.moverot.x = x
        if y:
            self.moverot.y = y
        if z:
            self.moverot.z = z
        self.osc.send("/test", [True])
    def stopMove(self, x, y, z):
        self.osc.send("/test", [False])
        if abs(x) > 0.00001:
            self.moverot.x = 0
        if abs(y) > 0.00001:
            self.moverot.y = 0
        if abs(z) > 0.00001:
            self.moverot.z = 0
        if abs(self.moverot.x) < 0.00001 and abs(self.moverot.y) < 0.00001 and abs(self.moverot.z) < 0.00001:
            self.osc.send("/test", [False])
    def computeUp(self, closi, pp):
        # to find out how far along the current quad we are, we
        # get the vector for closi1 -> closi2 (F)
        # then find closi1 -> closest point on F
        # ratios of distances of vectors = % along face
        # use cubic curve to interpolate up vector and the points on ground
        if abs(closi[0] - closi[1]) != 1 and closi[0] != 0 and closi[1] != 0:
            raise Exception("closest_idx in computeUp- they're not apart by onli 1!!!")
        vvx = verts.tr
        dir = closi[1] - closi[0]
        next = closi[1] + dir
        prev = closi[0] - dir
        if next > len(vvx)-1:
            next -= len(vvx)
        if prev > len(vvx)-1:
            prev -= len(vvx)
        dists = (pp - vvx[closi[0]][0]).norm(), (vvx[closi[0]][0] - vvx[closi[1]][0]).norm()
        i = dists[0] / dists[1]
        #return (fetchUp(closi-2) + fetchUp(closi-1) + fetchUp(closi) + \
        #    fetchUp(next1) + fetchUp(next2)) / 5.0
        print i
        return cubicInterpolate(fetchUp(prev), fetchUp(closi[0]), fetchUp(closi[1]), \
            fetchUp(next), i)
    def attachToTrack(self, playerpos):
        self.track.attachPlayer(playerpos)
        closest_idx = [0, 1]
        pp = playerpos
        vvx = verts.tr
        dsqlow = [(pp - vvx[closest_idx[0]][0]).normSquared(), \
            (pp - vvx[closest_idx[1]][0]).normSquared()]
        for i, point in enumerate(vvx):
            dsqcur = (pp - point[0]).normSquared()
            if dsqcur < dsqlow[0]:
                dsqlow[0] = dsqcur
                closest_idx[0] = i
            elif dsqcur < dsqlow[1]:
                dsqlow[1] = dsqcur
                closest_idx[1] = i

        plane = Plane()
        cm = vvx[closest_idx[0]][1]
        plane.norm.set(cm.m21, cm.m22, cm.m23)
        plane.setOrigin(vvx[closest_idx[0]][0])
        upv = Vector(plane.norm)
        close = ClosestOnPlane(plane, pp)
        drawpos = close + upv*2
        # find closest point from player pos on line segment closi1->2
        # use that value to interpolate upv from 1->2
        #return drawpos, self.computeUp(closest_idx, close)
        #return drawpos, upv
        return drawpos, Vector(0,1,0)
    def draw(self):
        savedpos = en.Vector(self.movebody.pos)
        savedrot = en.Vector(self.rotbody.pos)
        #savedpos = en.Vector(self.pos)
        self.computeForward()
        self.computeTurn()
        self.computeDrag()
        self.movebody.update()
        self.rotbody.update()
        # clamp z rot
        if self.moverot.y < -0.1:
            if self.rotbody.pos.z > 0.6:
                self.rotbody.pos.z = 0.6
                self.rotbody.vel.z = 0
                self.rotbody.acc.z = 0
                for v in self.rotbody.accs:
                    v[0].z = 0
        elif self.moverot.y > 0.1:
            if self.rotbody.pos.z < -0.6:
                self.rotbody.pos.z = -0.6
                self.rotbody.vel.z = 0
                self.rotbody.acc.z = 0
                for v in self.rotbody.accs:
                    v[0].z = 0
        else: # abs(self.moverot.y) < 0.1:
            self.rotbody.addForce(en.Vector(0,0,-6*self.rotbody.pos.z), 0.1)

        speed = self.movebody.vel.length()
        self.osc.send("/test/speed", [speed])

        self.movebody.pos, self.upv = self.attachToTrack(self.movebody.pos)
        """self.rot.x += self.moverot.x
        self.rot.y += self.moverot.y
        self.rot.z += self.moverot.z"""
        self.rot = savedrot*0.5 + self.rotbody.pos*0.5
        q = en.Quaternion()
        axis = en.Vector(self.upv)
        q.SetAxisAngle(axis, self.rot.y)

        m = q.GetMatrix()
        q2 = en.Quaternion()
        axis = en.Vector(m.m11,m.m12,m.m13)
        q2.SetAxisAngle(axis, self.rot.x)

        m = m * q2.GetMatrix()
        q3 = en.Quaternion()
        axis = en.Vector(m.m31,m.m32,m.m33)
        q3.SetAxisAngle(axis, self.rot.z)
        m = m * q3.GetMatrix()
        #q = q2.__mull__(q)
        #m = q.GetMatrix()
        glPushMatrix()
        #glRotatef(180, 0, 1, 0)
        #glRotatef( -90, 1, 0, 0 )
        #en.setupMatrix(q3.GetMatrix())
        #en.setupMatrix(q2.GetMatrix())
        #en.setupMatrix(q.GetMatrix())
        #pos = m.applyVector(self.pos)
        pos = savedpos
        #trans = en.Transform(m, en.Vector(0))
        #pos = trans.ThisToOtherVector(self.pos)
        glTranslatef( pos.x, pos.y, pos.z )
        self.m = m
        en.setupMatrix(m)
        #glTranslatef( 0, 0, 1 )
        #glTranslatef( self.pos.x, self.pos.y, self.pos.z )

        texbefore = self.texglref[0]
        glBindTexture( GL_TEXTURE_2D, texbefore )
        glBegin( GL_TRIANGLES )
        for f in self.faces:
            if texbefore != self.texglref[f.tex]:
                glEnd()
                texbefore = self.texglref[f.tex]
                glBindTexture( GL_TEXTURE_2D, texbefore )
                glBegin( GL_TRIANGLES )
            glNormal3f(f.normal[0], f.normal[1], f.normal[2])
            for x in xrange(3):
                glTexCoord2f(f.uvs[x][0], f.uvs[x][1])
                v1 = self.verts[f.v[x]]
                glVertex3f(v1[0], v1[1], v1[2])
        glEnd( )
        glPopMatrix()


if __name__ == "__main__":
    pass
