# -*- coding: utf-8 -*-
import engine as en

osc = en.OpenSoundControl()
osc.connect("localhost", 9002)
osc.send("/test", [True])
