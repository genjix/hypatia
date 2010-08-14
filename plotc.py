# -*- coding: utf-8 -*-
import Blender, bpy
import arith as ar
import math, solver

resolu = 0

def InterpolateBezier(q0, q1, q2, q3, it):
  f = it
  rt0 = q0
  rt1 = 3.0*(q1-q0)/f
  f *= f
  rt2 = 3.0*(q0-2.0*q1+q2)/f
  f *= it
  rt3 = (q3-q0+3.0*(q1-q2))/f

  """verts = []
  Q0 = q0
  Q1 = (q3 + 3.0*(q1 - q2) - q0)/it**3 + 3*(q2 - 2*q1 + q0)/it**2 + 3*(q1 - q0)/it
  Q2 = 6*(q3 + 3*(q1 - q2) - q0)/it**3 + 6*(q2 - 2*q1 + q0)/it**2
  Q3 = 6*(q3 + 3*(q1 - q2) - q0)/it**3

  for a in xrange(it+1):
    sumsq = lambda n: (n - 2)*(2*n - 3)*(n - 1)/3
    sumn = lambda n: (n - 2)*(n - 1)
    pred4 = lambda n: (sumsq(n) + sumn(n))/4
    pred3 = lambda n: n*(n - 1)/2
    pred2 = lambda n: n
    pred1 = lambda n: 1
    #print pred1(a)*Q0 + pred2(a)*Q1 + pred3(a)*Q2 + pred4(a)*Q3
    #print (a**3*q3 + 3*a**2*it*q2 - 3*a**3*q2 + 3*a*it**2*q1 - 6*a**2*it*q1 + 3*a**3*q1 + it**3*q0 - 3*a*it**2*q0 + 3*a**2*it*q0 - a**3*q0)/it**3
    i = float(a) / it
    #print i**3*q3 + 3*i**2*q2 - 3*i**3*q2 + 3*i*q1 - 6*i**2*q1 + 3*i**3*q1 + q0 - 3*i*q0 + 3*i**2*q0 - i**3*q0
    verts.append(i**3*q3 + 3*i**2*q2 - 3*i**3*q2 + 3*i*q1 - 6*i**2*q1 + 3*i**3*q1 + q0 - 3*i*q0 + 3*i**2*q0 - i**3*q0)"""

  q0 = rt0
  q1 = rt1+rt2+rt3
  q2 = 2*rt2+6*rt3
  q3 = 6*rt3

  verts = []
  for a in xrange(it+1):
    verts.append(q0)
    #print "B", q0

    q0 += q1
    q1 += q2
    q2 += q3
  return verts

def InterpolateTilt(t0, t1, it):
  td = (t1 - t0) / it
  tilts = []
  tilt = t0
  for a in xrange(it+1):
    tilts.append(tilt)
    tilt += td
  return tilts

def calc_bevel_sin_cos(x1, y1, x2, y2):
  t01 = math.sqrt(x1*x1 + y1*y1)
  t02 = math.sqrt(x2*x2 + y2*y2)
  if t01 == 0.0:
    t01 = 1.0
  if t02 == 0.0:
    t02 = 1.0

  x1 /= t01
  y1 /= t01
  x2 /= t02
  y2 /= t02

  t02 = x1*x2 + y1*y2
  if math.fabs(t02) >= 1.0:
    t02 = 0.5*math.pi
  else:
    t02 = ar.saacos(t02)/2.0

  t02 = math.sin(t02)
  if t02 == 0.0:
    t02 = 1.0

  x3 = x1 - x2
  y3 = y1 - y2
  if x3 == 0 and y3 == 0:
    x3 = y1
    y3 = -x1
  else:
    t01 = math.sqrt(x3*x3 + y3*y3)
    x3 /=t01
    y3 /=t01

  sina = -y3/t02
  cosa = x3/t02
  return (sina, cosa)

def PlotPoints(points, matrices, clear=True):
  verts = []
  faces = []
  for i in xrange(len(points)):
    x = points[i][0]
    y = points[i][1]
    z = points[i][2]
    fac = 1
    ox = fac*matrices[i][0][0]
    oy = fac*matrices[i][0][1]
    oz = fac*matrices[i][0][2]

    dx = -fac*matrices[i][1][0]
    dy = -fac*matrices[i][1][1]
    dz = -fac*matrices[i][1][2]
    verts.extend([[x,y,z], [x-ox-dx,y-oy-dy,z-oz-dz], [x+ox-dx, y+oy-dy,z+oz-dz]])
    vi = len(faces)*3
    faces.append([vi,vi+1,vi+2])

  editmode = Blender.Window.EditMode()
  if editmode: Blender.Window.EditMode(0)

  me = bpy.data.meshes['c']
  if clear:
    me.verts.delete(range(len(me.verts)))

  me.verts.extend(verts)
  me.faces.extend(faces)

  me.vertexColors = 1
  me.faces[0].col[0].r = 255
  me.faces[0].col[1].g = 255
  me.faces[0].col[2].b = 255
  me.update()
  Blender.Window.EditMode(1)
  Blender.Window.EditMode(0)

def PlotStrip(p, up):
  editmode = Blender.Window.EditMode()
  if editmode: Blender.Window.EditMode(0)

  o = 0.1
  me = bpy.data.meshes['c']
  verts = [[p[0], p[1], p[2]], [p[0]+up[0], p[1]+up[1], p[2]+up[2]]]
  verts.append([verts[1][0]+o, verts[1][1]+o, verts[1][2]])
  last = len(me.verts)
  me.verts.extend(verts)
  me.faces.extend([[last,last+1,last+2]])

  me.vertexColors = 1
  me.faces[0].col[0].r = 255
  me.faces[0].col[1].g = 255
  me.faces[0].col[2].b = 255
  me.update()
  Blender.Window.EditMode(1)
  Blender.Window.EditMode(0)

#--------------------------

c = bpy.data.curves["c"]
if not resolu:
  resolu = c.getResolu()
c = c[0]
pv = c[0]
verts = []
tilts = []
for pi in xrange(1, len(c), 1):
    p1 = pv.vec
    p2 = c[pi].vec
    xverts = InterpolateBezier(p1[1][0], p1[2][0], p2[0][0], p2[1][0], resolu)
    yverts = InterpolateBezier(p1[1][1], p1[2][1], p2[0][1], p2[1][1], resolu)
    zverts = InterpolateBezier(p1[1][2], p1[2][2], p2[0][2], p2[1][2], resolu)
    t = InterpolateTilt(pv.tilt, c[pi].tilt, resolu)
    if pi > 1:
        oldp = verts.pop()
        #if pi == 1:
        #    verts = []
        xverts[0] += oldp[0]
        xverts[0] /= 2.0
        yverts[0] += oldp[1]
        yverts[0] /= 2.0
        zverts[0] += oldp[2]
        zverts[0] /= 2.0
        oldt = tilts.pop()
        #if pi == 1:
        #    tilts = []
        t[0] += oldt
        t[0] /= 2.0
    """if pi >= len(c) - 1:
        verts[0][0] += xverts.pop()
        verts[0][0] /= 2.0
        verts[0][1] += yverts.pop()
        verts[0][1] /= 2.0
        verts[0][2] += zverts.pop()
        verts[0][2] /= 2.0
        tilts[0] += t.pop()
        tilts[0] /= 2.0"""
    tilts.extend(t)
    for i in xrange(len(xverts)):
        verts.append([xverts[i], yverts[i], zverts[i]])
    pv = c[pi]

q = [1.0, 0.0, 0.0, 0.0]
matrices = []
for i in xrange(len(verts)):
  bevp2 = ar.Vector(verts[i])
  bevp1 = ar.Vector(verts[i-1])
  bevp0 = ar.Vector(verts[i-2])
  vec = ar.VecBisect3(bevp0, bevp1, bevp2)
  quat = ar.VectToQuat(vec)
  quat_prev = quat[:]
  vec_prev = ar.Vector(vec)
  ar.AxisAngleToQuat(q, vec, tilts[i-1])
  ar.QuatMul(quat, q, quat)
  mat = \
   [[1.0, 0.0, 0.0],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0]]
  ar.QuatToMat3(quat, mat)
  matrices.append(mat)
  x1 = bevp1.x - bevp0.x
  x2 = bevp1.x - bevp2.x
  y1 = bevp1.y - bevp0.y
  y2 = bevp1.y - bevp2.y

  sina, cosa = calc_bevel_sin_cos(x1, y1, x2, y2)

matrices[0] = matrices[1]
PlotPoints(verts, matrices)

"""cube = bpy.data.objects["Cube"]
l = cube.loc
dist_sq = 1000
point = None
pidx = None
for i in xrange(len(verts)):
  p = verts[i]
  d = (p[0] - l[0])**2 + (p[1] - l[1])**2 + (p[2] - l[2])**2
  if d < dist_sq:
    dist_sq = d
    point = p[:]
    pidx = i

pidx = int(math.floor(pidx/float(resolu)))
if pidx >= len(c) - 1:
  pidx = 0
currp = c[pidx].vec
nextp = c[pidx+1].vec
x0 = currp[1][0]
y0 = currp[1][1]
z0 = currp[1][2]
x1 = currp[2][0]
y1 = currp[2][1]
z1 = currp[2][2]
x2 = nextp[0][0]
y2 = nextp[0][1]
z2 = nextp[0][2]
x3 = nextp[1][0]
y3 = nextp[1][1]
z3 = nextp[1][2]

i = solver.Solve(l[0], l[1], l[2], x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3)
loc = solver.Plot(i, x0, x1, x2, x3), solver.Plot(i, y0, y1, y2, y3), solver.Plot(i, z0, z1, z2, z3)
locp = solver.Plot(i-0.1, x0, x1, x2, x3), solver.Plot(i-0.1, y0, y1, y2, y3), solver.Plot(i-0.1, z0, z1, z2, z3)
locn = solver.Plot(i+0.1, x0, x1, x2, x3), solver.Plot(i+0.1, y0, y1, y2, y3), solver.Plot(i+0.1, z0, z1, z2, z3)
#loc = solver.Plot(0.5, x0, x1, x2, x3), solver.Plot(0.5, y0, y1, y2, y3), solver.Plot(0.5, z0, z1, z2, z3)
#print "x0 =", x0, ",x1 =", x1, ",x2 =", x2, ",x3 =", x3, ",y0 =", y0, ",y1 =", y1, ",y2 =", y2, ",y3 =", y3, \
#  ",z0 =", z0, ",z1 =", z1, ",z2 =", z2, ",z3 =", z3, ",i =", 0.5, ",x =", loc[0], ",y =", loc[1], ",z =", loc[2]

cube.setLocation(loc)
bpy.data.objects["curr"].setLocation(currp[1])
bpy.data.objects["next"].setLocation(nextp[1])
Blender.Window.Redraw()

PlotStrip(loc, (0,0,1))
t0, t1 = c[pidx].tilt, c[pidx+1].tilt
tilt = i*(t1 - t0) + t0

bevp2 = ar.Vector(locn)
bevp1 = ar.Vector(loc)
bevp0 = ar.Vector(locp)
vec = ar.VecBisect3(bevp0, bevp1, bevp2)
quat = ar.VectToQuat(vec)
q = [1.0, 0.0, 0.0, 0.0]
#ar.AxisAngleToQuat(q, vec, tilt)
#ar.QuatMul(quat, q, quat)
mat = \
  [[0.0, 0.0, 0.0],
  [0.0, 0.0, 0.0],
  [0.0, 0.0, 0.0]]
ar.QuatToMat3(quat, mat)
PlotPoints((loc,), (mat,), False)"""

if __name__ == "__main__":
    print len(verts), len(matrices)
    prefix = "/home/genjix/media/programs/hypatia/"
    f = open(prefix + "verts.py", "w")
    f.write("from engine import Vector, Matrix\n\ntr = [")
    comma = False
    for v, m in zip(verts, matrices):
        if comma:
            f.write(",\n")
        #f.write("(" + str(v) + ", " + str(m) + ")")
        comma = True
        f.write("(Vector(%f, %f, %f),\n"%(v[0], v[2], v[1]))
        f.write("""Matrix(
    %f, %f, %f,
    %f, %f, %f,
    %f, %f, %f))"""%(
        m[0][0], m[0][2], m[0][1],
        m[1][0], m[1][2], m[1][1],
        #0, 1, 0,
        m[2][0], m[2][2], m[2][1]))
    f.write("]\n")

    import pickle
    f = open(prefix + "track.dat", "w")
    verts = [[v[0], v[2], v[1]] for v in verts]
    corr = lambda v: [v[0], v[2], v[1]]
    matrices = [corr(v[0]) + corr(v[1]) + corr(v[2]) for v in matrices]
    pickle.dump((verts, matrices), f)
"""  x = verts[i][0]
  y = verts[i][1]
  z = verts[i][2]
  f.write("\nvec[%i].Set(%f, %f, %f);\n"%(i, x, z, y))
f.write("\n}\n")
"""
