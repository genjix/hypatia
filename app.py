# -*- coding: utf-8 -*-
from OpenGL.GL import *
import engine as en, time
import evsys as ev
import signals
import game
from bind import *
import harimau, track
import cube

en.init()
root = game.Component()

def lookAt(dir, up):
    m = en.Matrix()
    z = en.Vector(dir)
    z.normalize()
    x = en.Vector(up.cross(z))
    x.normalize()
    y = en.Vector(z.cross(x))
    m.m11, m.m12, m.m13 = x.x, x.y, x.z
    m.m21, m.m22, m.m23 = y.x, y.y, y.z
    m.m31, m.m32, m.m33 = z.x, z.y, z.z
    return m

class Camera(game.Component):
    def __init__(self, evsys, updates, player):
        game.Component.__init__(self)
        #evsys.signal("keydown_left").Connect(self.move(-1, 0, 0))
        """evsys.signal("keydown_left").Connect(bind(self.startMove, 1, 0, 0))
        evsys.signal("keyup_left").Connect(bind(self.stopMove, 1, 0, 0))
        evsys.signal("keydown_right").Connect(bind(self.startMove, -1, 0, 0))
        evsys.signal("keyup_right").Connect(bind(self.stopMove, -1, 0, 0))
        evsys.signal("keydown_up").Connect(bind(self.startMove, 0, 0, 1))
        evsys.signal("keyup_up").Connect(bind(self.stopMove, 0, 0, 1))
        evsys.signal("keydown_down").Connect(bind(self.startMove, 0, 0, -1))
        evsys.signal("keyup_down").Connect(bind(self.stopMove, 0, 0, -1))
        evsys.signal("keydown_pageup").Connect(bind(self.startMove, 0, -1, 0))
        evsys.signal("keyup_pageup").Connect(bind(self.stopMove, 0, -1, 0))
        evsys.signal("keydown_pagedown").Connect(bind(self.startMove, 0, 1, 0))
        evsys.signal("keyup_pagedown").Connect(bind(self.stopMove, 0, 1, 0))
        evsys.signal("keydown_lalt").Connect(bind(self.altPress, True))
        evsys.signal("keyup_lalt").Connect(bind(self.altPress, False))"""
        self.pos = en.Vector(0,-5,-16)
        self.lookat = en.Vector(0,-1,1)
        self.up = en.Vector(0,1,0)
        self.camtrans = en.Transform()
        self.updateTransform()
        updates.Connect(self.applyCamera, 5000)
        self.move = None
        self.alt = False
        self.player = player
    def altPress(self, alt):
        self.alt = alt
    def startMove(self, x, y, z):
        self.move = en.Vector(x, y, z)
    def stopMove(self, x, y, z):
        self.move = None
    def applyCamera(self):
        if self.move:
            if self.alt:
                self.camtrans.RotateThis(self.move, 0.1)
            else:
                self.camtrans.Translate(self.move)
        pm = self.player.m
        dir = en.Vector(pm.m31,pm.m32,pm.m33)
        dir.normalize()
        #dir.z *= -1
        #dir = en.Vector(0,0,-1)
        #print dir
        #self.lookat = self.player.pos - self.pos
        #self.pos = en.Vector(0,-5,-16) + self.player.pos
        #print self.player.pos
        #self.lookat = self.pos - dir*100
        #self.updateTransform()
        #m = lookAt(dir, en.Vector(0,1,0))
        glLoadIdentity()
        #en.setupTransform(self.camtrans)
        #glTranslatef(self.pos.x, self.pos.y, self.pos.z)
        #en.setupMatrix(m)
        pp = self.player.movebody.pos
        p = pp - dir*12 + en.Vector(0,5,0)
        pp += dir*50
        u = self.player.upv
        en.gluLookAt(p.x, p.y, p.z, pp.x, pp.y, pp.z, u.x, u.y, u.z)
    def updateTransform(self):
        self.camtrans.SetOrigin(self.pos)
        """dir = en.Vector(self.pos)
        dir.x -= self.eye.x
        dir.y -= self.eye.y
        dir.z -= self.eye.z"""
        #v = en.Vector(0,-1,1)
        self.camtrans.LookAt(self.lookat, self.up)

class Video(game.Component):
    def __init__(self, updater):
        self.v = en.VideoPlayer()
        self.v.init("/home/genjix/media/videos/[BSS]_Genius_Party_Beyond_[DVD]/[BSS]_Genius_Party_Beyond_-_Dimension_Bomb_[1DB25A30].mkv")
        self.v.run()
        updater.Connect(self.v.display, 4000)
    def die(self):
        self.v.die = True

LightPosition  = [0.0, 10.0, 2.0, 1.0]
LightAmbient = [0.5, 0.5, 0.5, 1.0]
LightDiffuse = [1.0, 1.0, 1.0, 1.0]
glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient )
glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse )
glLightfv( GL_LIGHT1, GL_POSITION, LightPosition )
glEnable( GL_LIGHT1 )

#evsys = EventSystem()
evsys = ev.EventSystem()
evsys.runOSCServer(7779)

tex = en.loadTexture("textures/thefuture.png")
updater = signals.Signal()
tr = track.Track(updater, tex)
vid = Video(updater)
s = harimau.Ship(evsys, updater, tr)
cam = Camera(evsys, updater, s)
#cube.Cube(updater, tex, s)

finish = False
def stopLoop(p):
    global finish
    finish = True
evsys.signal("keydown_escape").Connect(stopLoop)

while not finish:
    try:
        updater()
        en.flipBuffers()
        en.delay(50)
        evsys.pollEvents()
        evsys.update()
    except KeyboardInterrupt:
        vid.die()
        break
vid.die()
en.quit(0)
