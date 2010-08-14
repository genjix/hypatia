# -*- coding: utf-8 -*-
import math
import Blender

class Vector:
  def __init__(self, x=None, y=None, z=0):
    if type(self) == type(x):
      self.x = x.x
      self.y = x.y
      self.z = x.z
    elif type(x) == list or type(x) == tuple:
      self.x = x[0]
      self.y = x[1]
      self.z = x[2]
    elif x != None and y != None:
      self.x = float(x)
      self.y = float(y)
      self.z = float(z)
    elif y != None:
      self.x = self.y = self.z = float(x)
    elif x != None:
      v = x.vec[1]
      self.x = v[0]
      self.y = v[1]
      self.z = v[2]
    else:
      self.x = self.y = self.z = 0.0
  def __add__(self, v):
    r = Vector(self)
    r.x += v[0]
    r.y += v[1]
    r.z += v[2]
    return r
  def __sub__(self, v):
    r = Vector(self)
    r.x -= v[0]
    r.y -= v[1]
    r.z -= v[2]
    return r
  def __getitem__(self, idx):
    if type(idx) != int:
      raise KeyError
    if idx < 0 or idx > 2:
      raise IndexError
    if idx == 0:
      return self.x
    if idx == 1:
      return self.y
    if idx == 2:
      return self.z
  def __repr__(self):
    return "(%f,%f,%f)" % (self.x, self.y, self.z)
  def Normalize(self):
    d = self.x*self.x + self.y*self.y + self.z*self.z
    if d > 0.0000001:
      d = math.sqrt(d)
      self.x /= d
      self.y /= d
      self.z /= d
    else:
      self.x = self.y = self.z = 0
    return d
  def Tuple(self):
    return self.x, self.y, self.z

def VecBisect3(v1, v2, v3):
  d_12 = v2 - v1
  d_23 = v3 - v2
  d_12.Normalize()
  d_23.Normalize()
  out = d_12 + d_23
  out.Normalize()
  return out

def saacos(fac):
  if fac <= -1.0:
    return math.pi
  elif fac >= 1.0:
    return 0.0
  else:
    return math.acos(fac)
def saasin(fac):
  if fac <= -1.0:
    return -math.pi/2
  elif fac >= 1.0:
    return m.pi/2
  else:
    return math.asin(fac)

def QuatToMat3(q, m):
    q0 = math.sqrt(2) * q[0]
    q1 = math.sqrt(2) * q[1]
    q2 = math.sqrt(2) * q[2]
    q3 = math.sqrt(2) * q[3]

    qda = q0 * q1
    qdb = q0 * q2
    qdc = q0 * q3
    qaa = q1 * q1
    qab = q1 * q2
    qac = q1 * q3
    qbb = q2 * q2
    qbc = q2 * q3
    qcc = q3 * q3

    m[0][0] = 1.0 - qbb - qcc
    m[0][1] = qdc + qab
    m[0][2] =  -qdb + qac

    m[1][0] =  -qdc + qab
    m[1][1] = 1.0 - qaa - qcc
    m[1][2] = qda + qbc

    m[2][0] = qdb + qac
    m[2][1] =  -qda + qbc
    m[2][2] = 1.0 - qaa - qbb

def QuatMul(q, q1, q2):
    t0 = q1[0]*q2[0]-q1[1]*q2[1]-q1[2]*q2[2]-q1[3]*q2[3]
    t1 = q1[0]*q2[1]+q1[1]*q2[0]+q1[2]*q2[3]-q1[3]*q2[2]
    t2 = q1[0]*q2[2]+q1[2]*q2[0]+q1[3]*q2[1]-q1[1]*q2[3]
    q[3]= q1[0]*q2[3]+q1[3]*q2[0]+q1[1]*q2[2]-q1[2]*q2[1]
    q[0]=t0
    q[1]=t1
    q[2]=t2

def VectToQuat(vec):
  mat = \
   [[0.0, 0.0, 0.0],
    [0.0, 0.0, 0.0],
    [0.0, 0.0, 0.0]]

  q = [1.0, 0.0, 0.0, 0.0]
  q2 = q[:]

  len1 = math.sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z)
  if len1 == 0.0:
    return

  # nasty! I need a good routine for this...
  # problem is a rotation of an Y axis to the negative Y-axis for example.
  nor = Vector(-vec.y, vec.x, 0.0)

  if math.fabs(vec.x) + math.fabs(vec.y) < 0.0001:
    nor.x = 1.0

  co = vec.z
  co /= len1

  nor.Normalize()

  angle = 0.5*saacos(co)
  si = math.sin(angle)
  q[0] = math.cos(angle)
  q[1] = nor.x*si
  q[2] = nor.y*si
  q[3] = nor.z*si

  QuatToMat3(q, mat)

  fp = mat[2]
  angle = -0.5 * math.atan2(-fp[0], -fp[1])

  co= math.cos(angle)
  si= math.sin(angle) / len1
  q2[0] = co
  q2[1] = vec.x*si
  q2[2] = vec.y*si
  q2[3] = vec.z*si

  QuatMul(q,q2,q)
  return q

# Inpf returns the dot product, also called the scalar product and inner product
def Inpf(v1, v2):
  return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z

def VecLenf(v1, v2):
    x = v1.x - v2.x
    y = v1.y - v2.y
    z = v1.z - v2.z
    return math.sqrt(x*x + y*y + z*z)

def NormalizedVecAngle2(v1, v2):
  # this is the same as acos(Inpf(v1, v2)), but more accurate
  if Inpf(v1, v2) < 0.0:
    vec = Vector(-v2[0], -v2[1], -v2[2])
    return math.pi - 2.0*saasin(VecLenf(vec, v1)/2.0)
  else:
    return 2.0*saasin(VecLenf(v2, v1)/2.0)

def Crossf(a, b):
  return Vector(
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0])

def AxisAngleToQuat(q, axis, angle):
  nor = Vector()
  nor.x = axis.x
  nor.y = axis.y
  nor.z = axis.z
  nor.Normalize()

  angle /= 2
  si = math.sin(angle)
  q[0] = math.cos(angle)
  q[1] = nor[0] * si
  q[2] = nor[1] * si
  q[3] = nor[2] * si

def QuatInterpol(quat1, quat2, t):
#    float quat[4], omega, cosom, sinom, sc1, sc2;
  q = [1.0, 0.0, 0.0, 0.0]
  result = q[:]
  cosom = quat1[0]*quat2[0] + quat1[1]*quat2[1] + quat1[2]*quat2[2] + quat1[3]*quat2[3]

  #/* rotate around shortest angle */
  if cosom < 0.0:
    cosom = -cosom;
    quat[0]= -quat1[0]
    quat[1]= -quat1[1]
    quat[2]= -quat1[2]
    quat[3]= -quat1[3]
  else:
    quat[0]= quat1[0]
    quat[1]= quat1[1]
    quat[2]= quat1[2]
    quat[3]= quat1[3]

  if (1.0 - cosom) > 0.0001:
    omega = math.acos(cosom)
    sinom = math.sin(omega)
    sc1 = math.sin((1 - t) * omega) / sinom
    sc2 = math.sin(t * omega) / sinom
  else:
    sc1= 1.0 - t
    sc2= t

  result[0] = sc1 * quat[0] + sc2 * quat2[0]
  result[1] = sc1 * quat[1] + sc2 * quat2[1]
  result[2] = sc1 * quat[2] + sc2 * quat2[2]
  result[3] = sc1 * quat[3] + sc2 * quat2[3]
  return result
