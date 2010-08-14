# -*- coding: utf-8 -*-

# verlet
acc = 0
vel = 0
pos = 0

for t in xrange(10):
    h = 1.0
    pos = pos + vel*h + acc*(h*h)/2.0
    velhalfw = vel + acc*h/2.0
    acc = (t + 1) * 2 / 5.0
    if acc > 2:
        acc = 2
    vel = velhalfw + acc*h/2.0
    print pos, vel, acc

# euler
acc = 0
vel = 0
pos = 0
for t in xrange(10):
    oldvel = vel
    for h in xrange(100):
        acc = (t + 1) * 2 / 5.0
        if acc > 2:
            acc = 2
        vel += acc*0.01
        pos += oldvel*0.01
    print pos, vel, acc
