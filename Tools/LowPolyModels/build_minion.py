# Minion: small red grunt piranha. Box ref: half (10,9,10) -> ~20w x 18h x 20d. Run: blender -b -P build_minion.py
import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
reset()
B = Builder("Minion", {"Body": (225, 45, 45), "Back": (140, 20, 20), "Belly": (255, 170, 140),
                       "Fin": (170, 30, 30), "Eye": (250, 250, 240), "Pupil": (20, 20, 20), "Teeth": (255, 255, 255)})
keys = [(10, 2, 2, 8), (9, 5, 5, 8), (6, 7.5, 7.5, 8.5), (2, 8.5, 8.5, 9), (-2, 8, 8, 9), (-5, 6, 6.5, 9), (-7.5, 3, 3.5, 9), (-8.5, 1.5, 2, 9)]
B.loft(keys, 12, lambda z, c, i, s: "Back" if c > 0.6 else ("Belly" if c < -0.5 else "Body"))
B.prism([(0, -7.5), (0, -11), (0, -11.5), (0, -7.5)], 0, 0, "Fin") if False else None
# tail fin (vertical, extruded along X)
B.prism([(-7.5, 9), (-11.5, 17), (-10, 9), (-11.5, 1)], 0, 1.6, "Fin", plane="zy")
# dorsal fin
B.prism([(3, 16.5), (-1, 19.5), (-4, 16)], 0, 1.4, "Fin", plane="zy")
# side fins
for sx in (1, -1):
    B.prism([(sx*7.5, 2), (sx*11.5, -2), (sx*7.5, -1)], 6, 1.2, "Fin")
    B.ico((sx*5.2, 11.5, 6.5), 2.2, "Eye", sub=1)
    B.ico((sx*6.4, 11.8, 7.6), 1.1, "Pupil", sub=1)
# teeth row under the mouth
for k in range(-2, 3):
    B.cone((k*1.3, 7.2, 9.6), (k*1.3, 5.6, 9.9), 0.7, "Teeth", seg=4)
B.finish()
