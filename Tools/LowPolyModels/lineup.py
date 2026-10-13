import bpy, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
from mathutils import Vector
reset()
items = [(os.path.join(OUT, "WhaleBoss.blend"), "WhaleBoss", 0)] + [(f"{OUT}/{n}.blend", n, x) for n, x in
         (("Minion", -200), ("Hopper", -150), ("Tanker", -90), ("Float", 100), ("Charge", 160))]
for path, name, x in items:
    with bpy.data.libraries.load(path) as (src, dst): dst.objects = [name]
    ob = dst.objects[0]; bpy.context.scene.collection.objects.link(ob); ob.location.x = x
cam = setup_render(); sc = bpy.context.scene; sc.render.resolution_x, sc.render.resolution_y = 1600, 700
c = Vector((-10, 0, 30)); cam.location = c + Vector((0.6, -1.3, 0.55)).normalized()*800
cam.data.ortho_scale = 470; cam.rotation_euler = (c - cam.location).to_track_quat('-Z', 'Y').to_euler()
sc.render.filepath = f"{OUT}/preview_lineup.png"; bpy.ops.render.render(write_still=True)
