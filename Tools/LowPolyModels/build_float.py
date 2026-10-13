# Float: purple floating jellyfish (bell + trailing tentacles). Box ref: half (14,13,14) -> 28w x 26h x 28d.
import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
reset()
B = Builder("Float", {"Bell": (185, 75, 235), "Top": (235, 160, 255), "Rim": (130, 40, 180),
                      "Glow": (255, 230, 255), "Tentacle": (210, 120, 245), "Eye": (30, 10, 40)})
N = 12; prof = [(10, 14), (13, 13.8), (17, 12.5), (21, 9.5), (24, 5.5), (26, 0.01)]
bm = B.bm; rings = []
for (y, r) in prof:
    rings.append([bm.verts.new(g(r*math.sin(2*math.pi*i/N), y, r*math.cos(2*math.pi*i/N))) for i in range(N)])
for k in range(len(rings)-1):
    for i in range(N):
        f = bm.faces.new((rings[k][(i+1)%N], rings[k][i], rings[k+1][i], rings[k+1][(i+1)%N]))
        f.material_index = B.mi("Top" if k >= 3 else "Bell")
# frilled rim (alternating points downward)
inner = [bm.verts.new(g(9*math.sin(2*math.pi*i/N), 11, 9*math.cos(2*math.pi*i/N))) for i in range(N)]
for i in range(N):
    j = (i+1) % N
    f = bm.faces.new((rings[0][i], rings[0][j], inner[j], inner[i])); f.material_index = B.mi("Rim")
    a = 2*math.pi*(i+0.5)/N
    tip = bm.verts.new(g(14.5*math.sin(a), 8, 14.5*math.cos(a)))
    f = bm.faces.new((rings[0][j], rings[0][i], tip)); f.material_index = B.mi("Rim")
cf = bm.faces.new(list(reversed(inner))); cf.material_index = B.mi("Glow")
bmesh.ops.recalc_face_normals(bm, faces=bm.faces[:])
# glowing core
B.ico((0, 17, 0), 5, "Glow", sub=1)
# angry eyes on front of bell
for sx in (1, -1):
    B.ico((sx*4.5, 17, 12.3), 1.7, "Eye", sub=1)
# tentacles: zig-zag tapered segments hanging to the ground
for k in range(6):
    a = 2*math.pi*k/6; x0, z0 = 7*math.sin(a), 7*math.cos(a)
    pts = [(x0, 11, z0), (x0*1.15, 7, z0*1.15), (x0*0.95, 3.5, z0*0.95), (x0*1.2, 0, z0*1.2)]
    for t in range(3):
        B.cyl(pts[t], pts[t+1], 1.4 - 0.4*t, "Tentacle", seg=4, r2=1.0 - 0.3*t)
# oral arms in the middle
for k in range(3):
    a = 2*math.pi*k/3 + 0.5
    B.cyl((2*math.sin(a), 11, 2*math.cos(a)), (3.5*math.sin(a), 2, 3.5*math.cos(a)), 1.8, "Glow", seg=4, r2=0.6)
B.finish()
