# Charge: grey-blue shark that dashes. Box ref: half (13,11,22) -> 26w x 22h x 44d.
import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
reset()
B = Builder("Charge", {"Body": (70, 105, 135), "Back": (35, 55, 75), "Belly": (215, 225, 230),
                       "Fin": (50, 80, 105), "Eye": (15, 15, 20), "Mouth": (120, 20, 30), "Teeth": (255, 255, 255), "Gill": (30, 45, 60)})
keys = [(22, 1.5, 1.5, 9), (20, 5, 4.5, 9), (16, 8.5, 7, 9.5), (10, 11, 8.5, 10), (3, 12, 9, 10), (-4, 11, 8.5, 10.5),
        (-11, 8.5, 6.5, 11), (-17, 5, 4, 11), (-21, 2.5, 2.5, 11)]
B.loft(keys, 12, lambda z, c, i, s: "Back" if c > 0.45 else ("Belly" if c < -0.45 else "Body"))
# dorsal fin (tall, swept back)
B.prism([(4, 18), (-4, 28), (-6, 27), (-5, 18)], 0, 1.6, "Fin", plane="zy")
# crescent tail (vertical)
B.prism([(-20, 11), (-27, 22), (-29, 21), (-24, 11), (-28, 2), (-26, 1.5)], 0, 1.4, "Fin", plane="zy")
for sx in (1, -1):
    # pectoral fins (swept, drooping)
    B.prism([(sx*7, 8), (sx*19, -1), (sx*17, -3), (sx*7, 2)], 5.5, 1.4, "Fin")
    B.ico((sx*6.2, 11.5, 15), 1.3, "Eye", sub=1)
    for k in range(3):
        B.prism([(9.5 - k*1.6, 7), (9.0 - k*1.6, 12), (8.4 - k*1.6, 12), (8.9 - k*1.6, 7)], sx*11.4, 0.4, "Gill", plane="zy")
# open jaw under the snout
B.prism([(-5, 13), (5, 13), (3.5, 19), (-3.5, 19)], 5.2, 1.4, "Mouth")
for k in range(-2, 3):
    B.cone((k*1.4, 5.9, 17.5 - abs(k)*1.2), (k*1.4, 7.6, 17.5 - abs(k)*1.2), 0.6, "Teeth", seg=4)
B.finish()
