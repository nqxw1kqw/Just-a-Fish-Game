# Tanker: armored green sea turtle with plated shell and spikes. Box ref: half (20,16,20) -> 40w x 32h x 40d.
import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
reset()
B = Builder("Tanker", {"Shell": (50, 155, 80), "Plate": (20, 85, 40), "Rim": (110, 90, 50),
                       "Skin": (130, 170, 90), "Spike": (225, 215, 170), "Eye": (20, 20, 20), "Belly": (200, 185, 120)})
# shell dome: rings from bottom (y) up
N = 10; prof = [(4, 19.5), (9, 19), (16, 16), (22, 11), (26, 5), (27, 0.01)]
bm = B.bm; rings = []
for (y, r) in prof:
    rings.append([bm.verts.new(g(r*math.sin(2*math.pi*i/N), y, r*1.0*math.cos(2*math.pi*i/N))) for i in range(N)])
for k in range(len(rings)-1):
    for i in range(N):
        f = bm.faces.new((rings[k][(i+1)%N], rings[k][i], rings[k+1][i], rings[k+1][(i+1)%N]))
        f.material_index = B.mi("Rim" if k == 0 else ("Plate" if (i + k) % 2 else "Shell"))
f = bm.faces.new(rings[0]); f.material_index = B.mi("Belly")
bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
# armor spikes on top ridge and sides
for (x, y, z) in ((0, 26, 4), (0, 25.5, -6), (0, 22, 13), (0, 21.5, -15)):
    B.cone((x, y-1, z), (x, y+5, z), 2.6, "Spike", seg=5)
for k in range(6):
    a = 2*math.pi*(k+0.5)/6; x, z = 18.5*math.sin(a), 18.5*math.cos(a)
    B.cone((x*0.95, 10, z*0.95), (x*1.2, 13, z*1.2), 2.2, "Spike", seg=5)
# head (forward +Z) and armored brow
B.ico((0, 9, 22), 6.5, "Skin", sub=1, scale=(1, 0.85, 1.2))
B.prism([(-5.5, 23), (5.5, 23), (4, 28), (-4, 28)], 13.6, 2.0, "Plate")
for sx in (1, -1):
    B.ico((sx*4.6, 10.8, 25), 1.2, "Eye", sub=1)
    # flippers
    B.prism([(sx*17, 12), (sx*29, 8), (sx*30, 3), (sx*18, 5)], 5, 2.2, "Skin")
    B.prism([(sx*15, -12), (sx*23, -17), (sx*20, -20), (sx*14, -16)], 4, 2.0, "Skin")
B.cone((0, 5, -18), (0, 4, -25), 2.2, "Skin", seg=5)
B.finish()
