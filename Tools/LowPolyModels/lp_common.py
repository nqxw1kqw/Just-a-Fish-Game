# Shared helpers for Just-a-Fish-Game low-poly creatures. Game coords: X right, Y up, +Z forward.
# Blender: X = game X, Z = game Y, -Y = game +Z. Origin = bottom center (enemy pos). 1 BU = 1 game unit.
import bpy, bmesh, math, os
from mathutils import Vector, Matrix
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "out"); os.makedirs(OUT, exist_ok=True)

def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)

def g(x, y, z): return Vector((x, -z, y))

class Builder:
    def __init__(self, name, mats):
        self.name = name; self.bm = bmesh.new(); self.M = {}
        for k, rgb in mats.items():
            m = bpy.data.materials.new(f"{name}_{k}"); m.use_nodes = True
            c = tuple(v/255 for v in rgb) + (1,); m.diffuse_color = c
            b = m.node_tree.nodes["Principled BSDF"]; b.inputs["Base Color"].default_value = c
            b.inputs["Roughness"].default_value = 0.8
            self.M[k] = m
        self.names = list(self.M)
    def mi(self, k): return self.names.index(k)

    def loft(self, keys, N, matfn, cap=True):
        """keys: (z, halfW, halfH, cy). matfn(z, angle_cos, i, sin) -> material key"""
        bm = self.bm; rings = []
        for (z, w, h, cy) in keys:
            rings.append([bm.verts.new(g(w*math.sin(2*math.pi*i/N), cy + h*math.cos(2*math.pi*i/N), z)) for i in range(N)])
        fs = []
        for k in range(len(rings)-1):
            for i in range(N):
                f = bm.faces.new((rings[k][i], rings[k][(i+1)%N], rings[k+1][(i+1)%N], rings[k+1][i]))
                a = 2*math.pi*(i+0.5)/N
                f.material_index = self.mi(matfn(keys[k][0], math.cos(a), i, math.sin(a))); fs.append(f)
        if cap:
            for r, rev in ((rings[0], True), (rings[-1], False)):
                f = bm.faces.new(list(reversed(r)) if rev else r)
                f.material_index = fs[0].material_index if rev else fs[-1].material_index
        return rings

    def prism(self, outline, y0, thick, key, plane="xz"):
        """outline of 2D pts extruded: plane 'xz' extrudes along Y; 'zy' (side fin) extrudes along X at x=y0."""
        bm = self.bm
        if plane == "xz":
            top = [bm.verts.new(g(a, y0+thick/2, b)) for a, b in outline]
            bot = [bm.verts.new(g(a, y0-thick/2, b)) for a, b in outline]
        else:
            top = [bm.verts.new(g(y0+thick/2, b, a)) for a, b in outline]
            bot = [bm.verts.new(g(y0-thick/2, b, a)) for a, b in outline]
        fs = [bm.faces.new(top), bm.faces.new(list(reversed(bot)))]
        n = len(outline)
        for i in range(n): fs.append(bm.faces.new((bot[i], bot[(i+1)%n], top[(i+1)%n], top[i])))
        for f in fs: f.material_index = self.mi(key)
        bmesh.ops.recalc_face_normals(self.bm, faces=fs)

    def ico(self, center, radius, key, sub=1, scale=(1,1,1)):
        r = bmesh.ops.create_icosphere(self.bm, subdivisions=sub, radius=1.0,
            matrix=Matrix.Translation(g(*center)) @ Matrix.Diagonal((scale[0], scale[2], scale[1], 1)) @ Matrix.Scale(radius, 4))
        for f in {f for v in r["verts"] for f in v.link_faces}: f.material_index = self.mi(key)

    def cone(self, base, tip, r, key, seg=6):
        """cone from game point base (radius r) to tip"""
        b, t = g(*base), g(*tip); d = (t-b); L = d.length
        q = d.normalized().to_track_quat('Z', 'Y')
        res = bmesh.ops.create_cone(self.bm, cap_ends=True, segments=seg, radius1=r, radius2=0, depth=L,
            matrix=Matrix.Translation((b+t)/2) @ q.to_matrix().to_4x4())
        for f in {f for v in res["verts"] for f in v.link_faces}: f.material_index = self.mi(key)

    def cyl(self, base, tip, r, key, seg=6, r2=None):
        b, t = g(*base), g(*tip); d = (t-b); L = d.length
        q = d.normalized().to_track_quat('Z', 'Y')
        res = bmesh.ops.create_cone(self.bm, cap_ends=True, segments=seg, radius1=r, radius2=(r if r2 is None else r2), depth=L,
            matrix=Matrix.Translation((b+t)/2) @ q.to_matrix().to_4x4())
        for f in {f for v in res["verts"] for f in v.link_faces}: f.material_index = self.mi(key)

    def finish(self, ground=True, cam_target=None, ortho=None):
        bm = self.bm
        bmesh.ops.triangulate(bm, faces=bm.faces[:])
        if ground:
            bmesh.ops.translate(bm, verts=bm.verts[:], vec=(0, 0, -min(v.co.z for v in bm.verts)))
        me = bpy.data.meshes.new(self.name); bm.to_mesh(me); bm.free()
        for m in self.M.values(): me.materials.append(m)
        for p in me.polygons: p.use_smooth = False
        ob = bpy.data.objects.new(self.name, me); bpy.context.scene.collection.objects.link(ob)
        import numpy as np
        co = np.array([v.co for v in me.vertices]); mn, mx = co.min(0), co.max(0)
        print(f"TRIS {self.name} {len(me.polygons)}")
        print(f"BBOX_GAME {self.name} x[{mn[0]:.1f},{mx[0]:.1f}] y[{mn[2]:.1f},{mx[2]:.1f}] z[{-mx[1]:.1f},{-mn[1]:.1f}]")
        N = self.name
        bpy.ops.wm.save_as_mainfile(filepath=f"{OUT}/{N}.blend")
        bpy.context.view_layer.objects.active = ob; ob.select_set(True)
        bpy.ops.export_scene.fbx(filepath=f"{OUT}/{N}.fbx", use_selection=True, apply_scale_options='FBX_SCALE_UNITS',
                                 axis_forward='Z', axis_up='Y', mesh_smooth_type='FACE')
        xexp(me, N, f"{OUT}/{N}.x")
        preview(N, mn, mx)
        return ob

def xexp(me, N, path):
    tris = list(me.polygons); vs = [me.vertices[i].co for p in tris for i in p.vertices]
    L = ["xof 0303txt 0032", "", "Material Dummy { 1;1;1;1;; 0; 0;0;0;; 0;0;0;; }", ""]
    for m in me.materials:
        r, gg, b, _ = m.diffuse_color
        L.append(f"Material {m.name} {{ {r:.4f};{gg:.4f};{b:.4f};1.0000;; 5.0; 0.0;0.0;0.0;; {r*0.3:.4f};{gg*0.3:.4f};{b*0.3:.4f};; }}")
    L += ["", f"Frame {N} {{", " FrameTransformMatrix { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1;; }", f" Mesh {N}Mesh {{", f"  {len(vs)};"]
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
    open(path, "w", newline="\r\n").write("\n".join(L) + "\n")

def setup_render():
    sc = bpy.context.scene
    sc.render.engine = 'BLENDER_WORKBENCH'; sc.display.shading.light = 'STUDIO'
    sc.display.shading.color_type = 'MATERIAL'; sc.display.shading.show_cavity = True
    sc.render.resolution_x, sc.render.resolution_y = 800, 600
    w = bpy.data.worlds.new("W"); sc.world = w; w.color = (0.55, 0.7, 0.8)
    cam = bpy.data.objects.new("Cam", bpy.data.cameras.new("Cam")); sc.collection.objects.link(cam); sc.camera = cam
    cam.data.type = 'ORTHO'; return cam

def preview(N, mn, mx):
    cam = setup_render(); sc = bpy.context.scene
    c = Vector(((mn[0]+mx[0])/2, (mn[1]+mx[1])/2, (mn[2]+mx[2])/2)); size = max(mx-mn)
    cam.location = c + Vector((1.1, -1.3, 0.8)).normalized()*size*4   # front-right 3/4 (front = -Y)
    cam.data.ortho_scale = size*1.5
    cam.rotation_euler = (c - cam.location).to_track_quat('-Z', 'Y').to_euler()
    sc.render.filepath = f"{OUT}/preview_{N}.png"; bpy.ops.render.render(write_still=True)
