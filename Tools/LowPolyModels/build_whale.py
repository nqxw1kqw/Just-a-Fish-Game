# Low-poly WhaleBoss for Just-a-Fish-Game. Run: blender -b -P build_whale.py
# Blender axes: X = game X, Z = game Y (up), -Y = game +Z (forward). 1 BU = 1 game unit.
# Origin = bottom-center of body (= WhaleBoss::pos_), body spans game z -75..+80.
import bpy, bmesh, math, os
from mathutils import Vector, Matrix
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out"); os.makedirs(OUT, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
sc = bpy.context.scene

def mat(name, rgb):
    m = bpy.data.materials.new(name); m.use_nodes = True
    c = tuple(v/255 for v in rgb) + (1,)
    m.diffuse_color = c
    b = m.node_tree.nodes["Principled BSDF"]; b.inputs["Base Color"].default_value = c
    b.inputs["Roughness"].default_value = 0.8
    return m
M = {k: mat(k, v) for k, v in {
    "Whale_Body": (45, 70, 105), "Whale_Back": (35, 55, 85), "Whale_Belly": (80, 130, 190),
    "Whale_Groove": (60, 100, 150), "Whale_Mouth": (15, 20, 35), "Whale_Eye": (255, 60, 60),
    "Whale_Fin": (40, 62, 95)}.items()}
names = list(M)

def g(x, y, z):  # game coords -> blender
    return Vector((x, -z, y))

me = bpy.data.meshes.new("WhaleBoss"); bm = bmesh.new()
# ---- body loft: (game z, half width, half height, center y)
keys = [(84,8,8,26),(80,24,15,26),(72,40,22,27),(60,48,26,28),(45,51,28,28),(28,52,28.5,28.5),
        (10,51,28,29),(-8,49,27,29),(-25,45,25,30),(-42,38,22,31),(-58,29,18,31),(-72,20,13,31),
        (-86,12,9,31),(-98,7,6,31),(-106,5,4,31)]
N = 24; rings = []
for (z, w, h, cy) in keys:
    r = []
    for i in range(N):
        a = 2*math.pi*i/N
        s, c = math.sin(a), math.cos(a)
        hh = h*(1.0 if c > 0 else 0.92)          # flatter belly
        r.append(bm.verts.new(g(w*s, cy + hh*c, z)))
    rings.append(r)
body_faces = []
for k in range(len(rings)-1):
    for i in range(N):
        f = bm.faces.new((rings[k][i], rings[k][(i+1)%N], rings[k+1][(i+1)%N], rings[k+1][i]))
        z = keys[k][0]; a = 2*math.pi*(i+0.5)/N; c = math.cos(a)
        if c > 0.55: f.material_index = names.index("Whale_Back")
        elif c < -0.6 and z > -10: f.material_index = names.index("Whale_Groove" if i % 2 else "Whale_Belly")
        elif c < -0.6: f.material_index = names.index("Whale_Belly")
        else: f.material_index = 0
nose = bm.faces.new(list(reversed(rings[0]))); tailcap = bm.faces.new(rings[-1])
bmesh.ops.triangulate(bm, faces=[nose, tailcap])

def prism(outline, y0, thick, mi, shift=(0,0,0)):
    """outline: list of (gx,gz) in CCW from top; extruded in game Y."""
    top = [bm.verts.new(g(x+shift[0], y0+thick/2, z+shift[2])) for x, z in outline]
    bot = [bm.verts.new(g(x+shift[0], y0-thick/2, z+shift[2])) for x, z in outline]
    fs = [bm.faces.new(top), bm.faces.new(list(reversed(bot)))]
    n = len(outline)
    for i in range(n):
        fs.append(bm.faces.new((bot[i], bot[(i+1)%n], top[(i+1)%n], top[i])))
    for f in fs: f.material_index = mi
    bmesh.ops.recalc_face_normals(bm, faces=fs)
    bmesh.ops.triangulate(bm, faces=[f for f in fs if len(f.verts) > 4])
# ---- tail flukes (horizontal, notched)
fl = [(0,-100),(18,-104),(40,-112),(58,-126),(52,-134),(30,-128),(12,-126),(0,-120),
      (-12,-126),(-30,-128),(-52,-134),(-58,-126),(-40,-112),(-18,-104)]
prism(fl, 31, 5, names.index("Whale_Fin"))
# ---- pectoral fins (swept back, slight droop)
for sgn in (1, -1):
    pf = [(sgn*44,30),(sgn*62,22),(sgn*82,4),(sgn*88,-8),(sgn*74,-4),(sgn*48,8)]
    if sgn < 0: pf = list(reversed(pf))
    prism(pf, 14, 4, names.index("Whale_Fin"))
# ---- small dorsal hump
prism([(0,-40),(3,-50),(0,-62),(-3,-50)], 0, 0, 0) if False else None
dv = [bm.verts.new(g(0, 57, -40)), bm.verts.new(g(4, 49, -38)), bm.verts.new(g(0, 49, -60)),
      bm.verts.new(g(-4, 49, -38)), bm.verts.new(g(0, 47, -64))]
for tri in ((dv[0],dv[1],dv[2]),(dv[0],dv[2],dv[3]),(dv[0],dv[3],dv[1])):
    bm.faces.new(tri).material_index = names.index("Whale_Fin")
# ---- mouth line: thin dark strip along each side
def mouth_side(sgn):
    pts = [(84,23.5),(78,20),(66,17),(52,16),(40,18.5),(34,22)]
    vs = []
    for z, y in pts:
        # find body half width at z by interpolating keys
        for a, b in zip(keys, keys[1:]):
            if b[0] <= z <= a[0]:
                t = (z-b[0])/(a[0]-b[0]); w = b[1]+(a[1]-b[1])*t; cy = b[3]+(a[3]-b[3])*t; h = b[2]+(a[2]-b[2])*t
                break
        dy = (y-cy)/h; xw = w*math.sqrt(max(0.0, 1-min(dy*dy, 1.0)))*1.03 + 0.6
        vs.append((bm.verts.new(g(sgn*xw, y+1.2, z)), bm.verts.new(g(sgn*xw, y-1.2, z))))
    for (a1, b1), (a2, b2) in zip(vs, vs[1:]):
        f = bm.faces.new((a1, b1, b2, a2) if sgn > 0 else (a1, a2, b2, b1))
        f.material_index = names.index("Whale_Mouth")
mouth_side(1); mouth_side(-1)
# ---- eyes (match code: local x ±51.5, y 34, z 45)
for sgn in (1, -1):
    r = bmesh.ops.create_icosphere(bm, subdivisions=1, radius=4.0,
        matrix=Matrix.Translation(g(sgn*50.5, 34, 45)))
    for f in {f for v in r["verts"] for f in v.link_faces}: f.material_index = names.index("Whale_Eye")
bmesh.ops.recalc_face_normals(bm, faces=body_faces or bm.faces[:0])
bmesh.ops.triangulate(bm, faces=bm.faces[:])
bmesh.ops.translate(bm, verts=bm.verts[:], vec=(0, 0, -min(v.co.z for v in bm.verts)))
bm.to_mesh(me); bm.free()
for m in M.values(): me.materials.append(m)
for p in me.polygons: p.use_smooth = False
ob = bpy.data.objects.new("WhaleBoss", me); sc.collection.objects.link(ob)
print("TRIS", len(me.polygons))
import numpy as np
co = np.array([v.co for v in me.vertices]); print("BBOX_BLENDER", co.min(0), co.max(0))
bpy.ops.wm.save_as_mainfile(filepath=f"{OUT}/WhaleBoss.blend")

# ---- exports
bpy.context.view_layer.objects.active = ob; ob.select_set(True)
bpy.ops.export_scene.fbx(filepath=f"{OUT}/WhaleBoss.fbx", use_selection=True, apply_scale_options='FBX_SCALE_UNITS',
                         axis_forward='Z', axis_up='Y', mesh_smooth_type='FACE')
bpy.ops.wm.obj_export(filepath=f"{OUT}/WhaleBoss.obj", export_selected_objects=True, forward_axis='Z', up_axis='Y',
                      export_materials=True, export_normals=True, export_uv=False)
# DirectX .x written directly in DxLib's left-handed game coords (x, y, z) = (bx, bz, -by), +Z forward
def xexp(path):
    tris = list(me.polygons); vs = [me.vertices[i].co for p in tris for i in p.vertices]
    L = ["xof 0303txt 0032", "", "Material Dummy { 1;1;1;1;; 0; 0;0;0;; 0;0;0;; }", ""]
    for m in me.materials:
        r, gg, b, _ = m.diffuse_color
        L.append(f"Material {m.name} {{ {r:.4f};{gg:.4f};{b:.4f};1.0000;; 5.0; 0.0;0.0;0.0;; {r*0.3:.4f};{gg*0.3:.4f};{b*0.3:.4f};; }}")
    L += ["", "Frame WhaleBoss {", " FrameTransformMatrix { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1;; }", " Mesh WhaleBossMesh {",
          f"  {len(vs)};"]
    L.append(",\n".join(f"  {v.x:.4f};{v.z:.4f};{-v.y:.4f};" for v in vs) + ";")
    L.append(f"  {len(tris)};")
    L.append(",\n".join(f"  3;{3*k},{3*k+1},{3*k+2};" for k in range(len(tris))) + ";")
    L += ["  MeshNormals {", f"   {len(tris)};"]
    L.append(",\n".join(f"   {p.normal.x:.4f};{p.normal.z:.4f};{-p.normal.y:.4f};" for p in tris) + ";")
    L.append(f"   {len(tris)};"); L.append(",\n".join(f"   3;{k},{k},{k};" for k in range(len(tris))) + ";"); L.append("  }")
    L += ["  MeshMaterialList {", f"   {len(me.materials)};", f"   {len(tris)};"]
    L.append(",\n".join(f"   {p.material_index}" for p in tris) + ";")
    for m in me.materials: L.append(f"   {{{m.name}}}")
    L += ["  }", " }", "}"]
    open(path, "w").write("\n".join(L) + "\n")
xexp(f"{OUT}/WhaleBoss.x")

# ---- previews (Workbench, material colors, flat)
sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.light = 'STUDIO'
sc.display.shading.color_type = 'MATERIAL'; sc.display.shading.show_cavity = True
sc.render.resolution_x, sc.render.resolution_y = 1024, 768
w = bpy.data.worlds.new("W"); sc.world = w; w.color = (0.55, 0.7, 0.8)
sc.display.shading.background_type = 'WORLD' if hasattr(sc.display.shading, 'background_type') else None
cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam")); sc.collection.objects.link(cam); sc.camera = cam
cam.data.type = 'ORTHO'
tgt = Vector((0, 20, 30))
def shot(name, loc, ortho):
    cam.location = Vector(loc); cam.data.ortho_scale = ortho
    cam.rotation_euler = (tgt - cam.location).to_track_quat('-Z', 'Y').to_euler()
    sc.render.filepath = f"{OUT}/preview_{name}.png"; bpy.ops.render.render(write_still=True)
shot("front34", (220, -260, 170), 300)   # front is -Y in blender
shot("side", (400, 20, 30), 270)
shot("top", (0, 20, 400), 270)
