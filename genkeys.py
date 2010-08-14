# -*- coding: utf-8 -*-
fout = open("keys.hpp","w")
fin = open("keys", "r")
for l in fin:
    key = l[:-1]
    fout.write("case %s:\n"%(key,))
    key = key.lower()[5:]
    fout.write("    ev.add(\"str(\\\"%s\\\")\");\n"%(key,))
    fout.write("    break;\n")
