# Seabed floor model for Just-a-Fish-Game (arena 1400x1400, ground logic stays y=0).
# Uses the local dependency-free floor_mesh helpers. Flat material colours (no textures).
# Layout: undulating sand plane (centre kept near-flat), rocks, coral bits, shells, seaweed.
import argparse
import math
import random
from pathlib import Path
from floor_mesh import Mesh, ellipsoid, cone, tube, write_x
from floor_layout import ROCKS, GARDENS

ARENA_HALF = 700.0

def ground_h(x, z):
    r = math.hypot(x, z)
    damp = min(1.0, max(0.0, (r - 180.0) / 340.0))          # flat-ish centre (gameplay)
    h = (3.5 * math.sin(x * 0.011 + 1.7) * math.cos(z * 0.009 - 0.6)
         + 2.0 * math.sin(x * 0.023) * math.sin(z * 0.019 + 2.0)) * damp
    edge = max(abs(x), abs(z))
    if edge > ARENA_HALF:                                     # seabed falls away outside the walls
        t = min(1.0, (edge - ARENA_HALF) / 90.0)
        h -= t * t * 18.0
    return h

def build_floor():
    rng = random.Random(7)
    mats = {"sand": (205, 196, 165), "sand_light": (222, 214, 188), "sand_dark": (176, 168, 138),
            "patch": (158, 156, 126), "rock": (118, 116, 108), "rock_dark": (100, 100, 94),
            "coral_pink": (232, 130, 120), "coral_orange": (240, 162, 92), "coral_purple": (198, 112, 152),
            "shell": (236, 226, 205), "shell_pink": (240, 198, 188),
            "weed": (66, 138, 78), "weed_dark": (48, 112, 62),
            "ruin": (149, 177, 168), "ruin_cap": (184, 200, 180)}
    m = Mesh(mats)
    # --- sand plane: 20x20 cells over 1580 (past the 700 walls), gentle undulation
    N = 36
    SIZE = 1900.0
    step = SIZE / N
    idx = {}
    for j in range(N + 1):
        for i in range(N + 1):
            x = -SIZE / 2 + i * step
            z = -SIZE / 2 + j * step
            idx[(i, j)] = m.v((x, ground_h(x, z), z))
    for j in range(N):
        for i in range(N):
            cx = -SIZE / 2 + (i + 0.5) * step
            cz = -SIZE / 2 + (j + 0.5) * step
            hsh = (i * 73856093 ^ j * 19349663) & 0xffff
            mat = "sand"
            # Broad sand bands replace the former checker-like random patches.
            band = math.sin(cx * 0.009 + math.sin(cz * 0.008))
            if band > 0.78: mat = "sand_light"
            elif band < -0.86: mat = "sand_dark"
            if any(math.hypot(cx-x, cz-z) < r+28 for x,z,r,_,_ in ROCKS): mat = "patch"
            m.face((idx[(i, j)], idx[(i, j + 1)], idx[(i + 1, j + 1)], idx[(i + 1, j)]), mat)
    def scatter(rmin, rmax):
        a = rng.uniform(0, 2 * math.pi)
        r = rng.uniform(rmin, rmax)
        return math.cos(a) * r, math.sin(a) * r
    # --- rocks (squashed ellipsoids, half buried)
    for x, z, radius, height, mat in ROCKS:
        base = ground_h(x, z)
        if mat == "ruin":
            # Broken octagonal columns: an unmistakable north/south landmark.
            tube(m, [(x, base-3, z), (x, base+height-8, z), (x+3, base+height, z)],
                 [radius, radius*0.83, radius*0.72], mat, sides=8)
            tube(m, [(x, base, z), (x, base+7, z)], [radius, radius], "ruin_cap", sides=8)
        else:
            ellipsoid(m, (x, base+height*0.42, z), (radius, height*0.58, radius), mat, lon=9, lat=5)
    # --- coral bits (cone branches)
    for gx, gz in GARDENS:
        x, z = gx + 10, gz + 8
        base_y = ground_h(x, z)
        # Coral grows on the solid reef; collision belongs to the underlying rock.
        base_y += next(h*0.70 for rx,rz,_,h,_ in ROCKS if rx == gx and rz == gz)
        mat = rng.choice(["coral_pink", "coral_orange", "coral_purple"])
        for k in range(4):
            dx = rng.uniform(-14, 14); dz = rng.uniform(-14, 14)
            h = rng.uniform(16, 27)
            cone(m, (x + dx, base_y, z + dz), (x + dx * 1.7, base_y + h, z + dz * 1.7), rng.uniform(3.5, 6), mat, seg=5)
        ellipsoid(m, (x, base_y + 3, z), (13, 6, 13), mat, lon=6, lat=3)
    # --- shells (small flattened ellipsoids)
    for _ in range(24):
        x, z = scatter(200, 660)
        r = rng.uniform(6, 11)
        ellipsoid(m, (x, ground_h(x, z) + 1.5, z), (r, r * 0.38, r * 0.8), rng.choice(["shell", "shell_pink"]), lon=6, lat=3)
    # --- seaweed tufts (tapered bent tubes)
    for gx, gz in GARDENS:
        x, z = gx - 54, gz + 35
        base_y = ground_h(x, z)
        for k in range(3):
            dx = rng.uniform(-9, 9); dz = rng.uniform(-9, 9)
            h = rng.uniform(30, 52)
            bend = rng.uniform(-12, 12)
            tube(m, [(x + dx, base_y - 2, z + dz),
                     (x + dx + bend * 0.4, base_y + h * 0.55, z + dz + bend * 0.3),
                     (x + dx + bend, base_y + h, z + dz + bend * 0.8)],
                 [3.0, 2.0, 0.7], rng.choice(["weed", "weed_dark"]), sides=5)
    return m


def main():
    parser = argparse.ArgumentParser(description="Generate the low-poly seabed using Python 3.10+ (standard library only).")
    default = Path(__file__).resolve().parents[2] / "Data/Model/LowPoly/Floor.x"
    parser.add_argument("--output", type=Path, default=default, help="Output .x file (default: the game's Floor.x)")
    parser.add_argument("--collision-output", type=Path, help="Generated C++ collider header")
    args = parser.parse_args()
    mesh = build_floor()
    write_x(mesh, args.output)
    header = args.collision_output or (default.parents[3] / "MapLayout.generated.h"
                                      if args.output == default else args.output.with_suffix(".collision.h"))
    rows = ["// Generated by Tools/LowPolyModels/build_floor.py. Edit floor_layout.py instead.",
            "#pragma once", "namespace MapLayout {", "struct Rock { float x, z, radius, top; };",
            "inline constexpr Rock rocks[] = {"]
    for x, z, radius, height, mat in ROCKS:
        # top remains in model space; runtime applies FLOOR_MODEL_Y.
        # The column's small slant is enclosed by its original circular base.
        rows.append(f"    {{ {x:.4f}f, {z:.4f}f, {radius:.4f}f, {ground_h(x,z)+height:.4f}f }},")
    rows += ["};", "}", ""]
    header.write_text("\n".join(rows), encoding="ascii")
    print(f"collision: objects={len(ROCKS)} output={header.resolve()}")
    used = [mesh.verts[i] for tri, _, _ in mesh.faces for i in tri]
    mn = tuple(min(v[i] for v in used) for i in range(3))
    mx = tuple(max(v[i] for v in used) for i in range(3))
    print(f"floor: triangles={len(mesh.faces)} bbox={mn} .. {mx} output={args.output.resolve()}")


if __name__ == "__main__":
    main()
