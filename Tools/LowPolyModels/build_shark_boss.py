# Low-poly Shark boss (Endless). Run: blender -b -P build_shark_boss.py
# Same pipeline as the other low-poly models (lp_common): game coords, +Z forward, 1 BU = 1 game unit.
# Two files: SharkBoss.x (body, origin = bottom-center) and SharkBossTail.x (caudal section,
# origin = tail pivot at game (0, TAIL_PIVOT_Y, TAIL_PIVOT_Z) of the body) so the game can swing the tail.
import os, sys; sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))); from lp_common import *
SX, SY, SZ = 3.0, 3.1, 4.4           # scale of the Charge-shark proportions -> ~230 units long boss
PIVOT_Z = -17 * SZ                   # tail pivot (game z)
PIVOT_Y = 11 * SY - 3.6              # = body center line at the pivot
MATS = {"Body": (85, 100, 120), "Back": (45, 55, 72), "Belly": (225, 228, 232), "Fin": (60, 72, 92),
        "Eye": (255, 40, 40), "Mouth": (130, 20, 30), "Teeth": (255, 255, 255), "Gill": (30, 40, 55), "Scar": (150, 160, 170)}
def S(z, w, h, cy): return (z*SZ, w*SX, h*SY, cy*SY - 3.6)
mat = lambda z, c, i, s: "Back" if c > 0.45 else ("Belly" if c < -0.45 else "Body")

reset()
B = Builder("SharkBoss", MATS)
keys = [(24, 1.0, 1.0, 11), (22, 4, 3.5, 10.5), (18, 7.5, 6, 10), (14, 9.5, 7.5, 10), (10, 11, 8.5, 10), (3, 12, 9, 10), (-4, 11, 8.5, 10.5),
        (-11, 8.5, 6.5, 11), (-17, 5, 4, 11)]
B.loft([S(*k) for k in keys], 16, mat)
# tall dorsal fin (visible above water during Circle Strafe)
B.prism([(4*SZ, 18*SY), (-6*SZ, 32*SY), (-9*SZ, 31*SY), (-7*SZ, 18*SY)], 0, 5, "Fin", plane="zy")
for sx in (1, -1):
    B.prism([(sx*7*SX, 8*SZ), (sx*21*SX, -3*SZ), (sx*19*SX, -6*SZ), (sx*7*SX, 1*SZ)], 5.5*SY, 4, "Fin")
    B.ico((sx*6.4*SX, 11.5*SY - 3.6, 15*SZ), 4.2, "Eye", sub=1)
    for k in range(4):
        z0 = (9.5 - k*1.6)*SZ
        B.prism([(z0, 7*SY), (z0 - 1.5, 12*SY), (z0 - 4, 12*SY), (z0 - 2.5, 7*SY)], sx*11.6*SX, 1.2, "Gill", plane="zy")
    # battle scar on the flank
    B.prism([(2*SZ, 12*SY), (-3*SZ, 9*SY), (-3.4*SZ, 9.6*SY), (1.6*SZ, 12.6*SY)], sx*12.2*SX, 1.0, "Scar", plane="zy")
B.prism([(-5*SX, 13*SZ), (5*SX, 13*SZ), (3.5*SX, 19.5*SZ), (-3.5*SX, 19.5*SZ)], 5.2*SY - 3.6, 4, "Mouth")
for k in range(-3, 4):
    B.cone((k*1.3*SX, 5.9*SY - 3.6, (17.8 - abs(k)*1.0)*SZ), (k*1.3*SX, 8.2*SY - 3.6, (17.8 - abs(k)*1.0)*SZ), 2.0, "Teeth", seg=4)
B.finish(ground=False)

reset()
T = Builder("SharkBossTail", MATS)
# Tail section starts 2.5 units *inside* the body (z=-14.5, smaller than the body there) so swinging
# the tail never opens a gap at the joint; the body keeps no end cap problem because the tail sleeve covers it.
tk = [(-14.5, 5.1, 4.2, 11), (-17, 5, 4, 11), (-21, 2.5, 2.5, 11), (-23, 1.2, 1.2, 11)]
T.loft([(z*SZ - PIVOT_Z, w*SX, h*SY, cy*SY - 3.6 - PIVOT_Y) for z, w, h, cy in tk], 16, mat)
T.prism([((a*SZ) - PIVOT_Z, b*SY - 3.6 - PIVOT_Y) for a, b in
         [(-20, 11), (-28, 26), (-31, 25), (-25, 11), (-29, 1), (-27, 0.5)]], 0, 4, "Fin", plane="zy")
T.finish(ground=False)
print("PIVOT", PIVOT_Y, PIVOT_Z)
# combined preview (body + tail) for review
import bpy
reset()
for n in ("SharkBoss", "SharkBossTail"):
    with bpy.data.libraries.load(f"{OUT}/{n}.blend") as (src, dst): dst.objects = [n]
    for o in dst.objects: bpy.context.scene.collection.objects.link(o)
bpy.data.objects["SharkBossTail"].location = g(0, PIVOT_Y, PIVOT_Z)
cam = setup_render(); sc = bpy.context.scene
import math
SWAY = 0.25
for name, loc, sw in (("side", (420, -60, 60), 0.0), ("front34", (300, -330, 200), 0.0),
                      ("top_swayL", (0, 0, 500), -SWAY), ("top_swayR", (0, 0, 500), SWAY), ("top_swayL_close", (0, 75, 260), -SWAY)):
    # game RotY(a) about +Y == blender rotation about +Z by -a (blender -Y = game +Z)
    bpy.data.objects["SharkBossTail"].rotation_euler = (0, 0, -sw)
    tgt = g(0, 40, 0) if not name.startswith("top") else (g(0, 30, -75) if "close" in name else g(0, 0, -10))
    cam.location = Vector(loc) if not name.startswith("top") else tgt + Vector((0, 0.01, 1)) * 400
    cam.data.ortho_scale = 120 if "close" in name else 300
    cam.rotation_euler = (tgt - cam.location).to_track_quat('-Z', 'Y').to_euler()
    sc.render.filepath = f"{OUT}/preview_SharkBoss_{name}.png"; bpy.ops.render.render(write_still=True)
