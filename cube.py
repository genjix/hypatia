# -*- coding: utf-8 -*-
import game
from OpenGL.GL import *
from engine import Vector, Plane, Matrix
import verts

def ClosestOnPlane(plane, p):
    a, b, c, d = plane.a, plane.b, plane.c, plane.d
    squares = a*a + b*b + c*c
    t = (-d - a*p.x - b*p.y - c*p.z) / squares
    return Vector(p.x + a*t, p.y + b*t, p.z + c*t)

class Cube(game.Component):
    def __init__(self, updates, text, player):
        updates.Connect(self.draw, 6000)
        self.rquad = 0
        self.texture = text
        self.player = player
    def doShid(self):
        closest_idx = 0
        pp = self.player.movebody.pos
        vvx = verts.tr
        dsqlow = (pp - vvx[closest_idx][0]).normSquared()
        for i, point in enumerate(vvx):
            if i == closest_idx:
                continue
            dsqcur = (pp - point[0]).normSquared()
            if dsqcur < dsqlow:
                dsqlow = dsqcur
                closest_idx = i

        plane = Plane()
        cm = vvx[closest_idx][1]
        plane.norm.set(cm.m21, cm.m22, cm.m23)
        plane.setOrigin(vvx[closest_idx][0])
        upv = Vector(plane.norm)
        close = ClosestOnPlane(plane, pp)
        return close
    def draw(self):
        glPushMatrix()
        pp = self.doShid()
        glTranslatef(pp.x, pp.y, pp.z)
        glRotatef( self.rquad, 0.0, 1.0, 0.0 )

        glBindTexture( GL_TEXTURE_2D, self.texture )
        glBegin( GL_QUADS )
        glNormal3f( 0.0, 0.0, 1.0)                  # Normal Pointing Towards Viewer
        glTexCoord2f(0.0, 0.0)
        glVertex3f(-1.0, -1.0,  1.0)  # Point 1 (Front)
        glTexCoord2f(1.0, 0.0)
        glVertex3f( 1.0, -1.0,  1.0)  # Point 2 (Front)
        glTexCoord2f(1.0, 1.0)
        glVertex3f( 1.0,  1.0,  1.0)  # Point 3 (Front)
        glTexCoord2f(0.0, 1.0)
        glVertex3f(-1.0,  1.0,  1.0)  # Point 4 (Front)
        # Back Face
        glNormal3f( 0.0, 0.0,-1.0)                  # Normal Pointing Away From Viewer
        glTexCoord2f(1.0, 0.0)
        glVertex3f(-1.0, -1.0, -1.0)  # Point 1 (Back)
        glTexCoord2f(1.0, 1.0)
        glVertex3f(-1.0,  1.0, -1.0)  # Point 2 (Back)
        glTexCoord2f(0.0, 1.0)
        glVertex3f( 1.0,  1.0, -1.0)  # Point 3 (Back)
        glTexCoord2f(0.0, 0.0)
        glVertex3f( 1.0, -1.0, -1.0)  # Point 4 (Back)
        # Top Face
        glNormal3f( 0.0, 1.0, 0.0)                  # Normal Pointing Up
        glTexCoord2f(0.0, 1.0)
        glVertex3f(-1.0,  1.0, -1.0)  # Point 1 (Top)
        glTexCoord2f(0.0, 0.0)
        glVertex3f(-1.0,  1.0,  1.0)  # Point 2 (Top)
        glTexCoord2f(1.0, 0.0)
        glVertex3f( 1.0,  1.0,  1.0)  # Point 3 (Top)
        glTexCoord2f(1.0, 1.0)
        glVertex3f( 1.0,  1.0, -1.0)  # Point 4 (Top)
        # Bottom Face
        glNormal3f( 0.0,-1.0, 0.0)                  # Normal Pointing Down
        glTexCoord2f(1.0, 1.0)
        glVertex3f(-1.0, -1.0, -1.0)  # Point 1 (Bottom)
        glTexCoord2f(0.0, 1.0)
        glVertex3f( 1.0, -1.0, -1.0)  # Point 2 (Bottom)
        glTexCoord2f(0.0, 0.0)
        glVertex3f( 1.0, -1.0,  1.0)  # Point 3 (Bottom)
        glTexCoord2f(1.0, 0.0)
        glVertex3f(-1.0, -1.0,  1.0)  # Point 4 (Bottom)
        # Right ace
        glNormal3f( 1.0, 0.0, 0.0)                  # Normal Pointing Right
        glTexCoord2f(1.0, 0.0)
        glVertex3f( 1.0, -1.0, -1.0)  # Point 1 (Right)
        glTexCoord2f(1.0, 1.0)
        glVertex3f( 1.0,  1.0, -1.0)  # Point 2 (Right)
        glTexCoord2f(0.0, 1.0)
        glVertex3f( 1.0,  1.0,  1.0)  # Point 3 (Right)
        glTexCoord2f(0.0, 0.0)
        glVertex3f( 1.0, -1.0,  1.0)  # Point 4 (Right)
        glNormal3f(-1.0, 0.0, 0.0)                  # Normal Pointing Let
        glTexCoord2f(0.0, 0.0)
        glVertex3f(-1.0, -1.0, -1.0)  # Point 1 (Let)
        glTexCoord2f(1.0, 0.0)
        glVertex3f(-1.0, -1.0,  1.0)  # Point 2 (Let)
        glTexCoord2f(1.0, 1.0)
        glVertex3f(-1.0,  1.0,  1.0)  # Point 3 (Let)
        glTexCoord2f(0.0, 1.0)
        glVertex3f(-1.0,  1.0, -1.0)  # Point 4 (Let)
        glEnd( )
        glPopMatrix()
        self.rquad -= 0.8
