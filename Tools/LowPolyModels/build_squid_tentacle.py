# SquidTentacle for Just-a-Fish-Game. Run: python build_squid_tentacle.py
# Writes a tapered low-poly tentacle (along +Z from origin, length 322, vertex colours
# matching SquidBoss) as a DirectX .x next to this script in out/. Copy to Data/Model/LowPoly/.
import math, os
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out"); os.makedirs(OUT, exist_ok=True)
MATS = {"arm": (186, 80, 105), "club": (232, 160, 168), "belly": (196, 105, 128)}
PROF = [(0, 17), (40, 15), (80, 12.5), (120, 10), (160, 8), (200, 6.5), (240, 5), (275, 4), (300, 3)]
N = 7
verts, tris, matidx = [], [], []
def vtx(x, y, z):
    verts.append((x, y, z)); return len(verts) - 1
rings = []
for z, r in PROF:
    rings.append([vtx(r * math.cos(2 * math.pi * i / N), r * math.sin(2 * math.pi * i / N), z) for i in range(N)])
for k in range(len(rings) - 1):
    for i in range(N):
        tris.append((rings[k][i], rings[k][(i + 1) % N], rings[k + 1][(i + 1) % N])); matidx.append(0)
        tris.append((rings[k][i], rings[k + 1][(i + 1) % N], rings[k + 1][i])); matidx.append(0)
tip = vtx(0, 0, 322)
for i in range(N):
    tris.append((rings[-1][i], rings[-1][(i + 1) % N], tip)); matidx.append(1)
base = vtx(0, 0, 0)
for i in range(N):
    tris.append((base, rings[0][(i + 1) % N], rings[0][i])); matidx.append(0)
names = list(MATS)
L = ["xof 0303txt 0032", "", "Material Dummy { 1;1;1;1;; 0; 0;0;0;; 0;0;0;; }", ""]
for n, (r, g, b) in MATS.items():
    L.append(f"Material {n} {{ {r/255:.4f};{g/255:.4f};{b/255:.4f};1.0000;; 5.0; 0.0;0.0;0.0;; {r/255*0.3:.4f};{g/255*0.3:.4f};{b/255*0.3:.4f};; }}")
L += ["", "Frame SquidTentacle {", " FrameTransformMatrix { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1;; }", " Mesh SquidTentacleMesh {", f"  {len(verts)};"]
L.append(",\n".join(f"  {x:.2f};{y:.2f};{z:.2f};" for x, y, z in verts) + ";")
L.append(f"  {len(tris)};"); L.append(",\n".join(f"  3;{a},{b},{c};" for a, b, c in tris) + ";")
L += ["  MeshMaterialList {", f"   {len(names)};", f"   {len(tris)};"]
L.append(",\n".join(f"   {m}" for m in matidx) + ";")
for n in names: L.append(f"   {{{n}}}")
L += ["  }", " }", "}"]
open(os.path.join(OUT, "SquidTentacle.x"), "w").write("\n".join(L) + "\n")
print("SquidTentacle.x", len(verts), "verts", len(tris), "tris")