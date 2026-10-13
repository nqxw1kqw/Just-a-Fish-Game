# Hopper: orange jumping frog-fish with strong hind leg-fins. Box ref: half (12,8,12) -> 24w x 16h x 24d.
import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
reset()
B = Builder("Hopper", {"Body": (240, 120, 30), "Back": (160, 60, 10), "Belly": (255, 205, 130),
                       "Spot": (120, 45, 10), "Eye": (255, 240, 120), "Pupil": (20, 20, 20), "Mouth": (90, 25, 10)})
keys = [(11, 3, 2.5, 6), (10, 7, 5, 6.5), (7, 10, 6.5, 7), (3, 11, 7, 7.5), (-2, 10.5, 6.8, 7.5), (-6, 8, 5.5, 7), (-9, 4, 3, 7), (-10, 1.5, 1.5, 7)]
B.loft(keys, 12, lambda z, c, i, s: "Back" if c > 0.55 else ("Belly" if c < -0.4 else "Body"))
# wide flat mouth
B.prism([(-6, 10.6), (6, 10.6), (5, 11.6), (-5, 11.6)], 4.8, 1.0, "Mouth")
# bulging eyes on top
for sx in (1, -1):
    B.cyl((sx*5, 11, 5), (sx*5.5, 14, 5.5), 2.2, "Body", seg=6)
    B.ico((sx*5.5, 15, 5.8), 2.6, "Eye")
    B.ico((sx*5.8, 15.4, 8.0), 1.2, "Pupil")
    # hind legs (folded, spring-loaded)
    B.cyl((sx*9, 5, -4), (sx*13, 7, -2), 2.4, "Body", seg=5, r2=2.0)
    B.cyl((sx*13, 7, -2), (sx*12, 1.5, -8), 1.8, "Back", seg=5, r2=1.4)
    B.prism([(sx*10, -8), (sx*15, -9), (sx*14, -12), (sx*10, -11)], 0.8, 1.2, "Back")
    # front stubby fins
    B.cyl((sx*7, 3, 5), (sx*9, 0.6, 7), 1.4, "Body", seg=5)
# back spots
for (x, z) in ((3, 2), (-3, -1), (0, -5), (4, -4), (-4, 4)):
    B.ico((x, 14.2, z), 1.3, "Spot", sub=1, scale=(1, 0.35, 1))
# small tail
B.prism([(-9.5, 7), (-13, 11), (-12, 7), (-13, 3)], 0, 1.2, "Back", plane="zy")
B.finish()
