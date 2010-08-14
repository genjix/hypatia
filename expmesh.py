# -*- coding: utf-8 -*-
import Blender
mesh = Blender.Object.GetSelected()[0].getData()
f = open("/home/genjix/out", "a")
f.write("faces %i\nverts %i\n"%(len(mesh.faces), len(mesh.verts)))
for fidx, face in enumerate(mesh.faces):
    f.write("%s\n"%(face.image.getFilename().strip("//textures/")))
    for vert in face.v:
        f.write("%i "%(vert.index,))
    f.write("|")
    for uv in face.uv:
        f.write("%f %f "%(uv[0], uv[1]))
    f.write("\n")
    f.write("%f %f %f\n"%(face.normal[0], face.normal[1], face.normal[2]))
for vert in mesh.verts:
    f.write("%f %f %f\n"%(vert.co[0], vert.co[1], vert.co[2]))
