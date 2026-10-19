# Seabed floor model for Just-a-Fish-Game (arena 1400x1400, ground logic stays y=0).
# Reuses the Mesh/.x writer from build_bosses.py. Flat material colours (no textures).
# Layout: undulating sand plane (centre kept near-flat), rocks, coral bits, shells, seaweed.
import math, random, sys
import importlib.util
src = open("/workspace/boss-models/build_bosses.py").read()
ns = {}
exec(src, ns)  # also rebuilds the boss models (harmless)
ns['OUT'] = "/workspace/floor_build"
Mesh, ellipsoid, cone, tube = ns['Mesh'], ns['ellipsoid'], ns['cone'], ns['tube']
write_x, render = ns['write_x'], ns['render']

ARENA_HALF = 700.0
random.seed(7)

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
    mats = {"sand": (205, 196, 165), "sand_light": (222, 214, 188), "sand_dark": (176, 168, 138),
            "patch": (158, 156, 126), "rock": (118, 116, 108), "rock_dark": (100, 100, 94),
            "coral_pink": (232, 130, 120), "coral_orange": (240, 162, 92), "coral_purple": (198, 112, 152),
            "shell": (236, 226, 205), "shell_pink": (240, 198, 188),
            "weed": (66, 138, 78), "weed_dark": (48, 112, 62)}
    m = Mesh(mats)
    # --- sand plane: 22x22 cells over 1580 (past the 700 walls), gentle undulation
    N = 20
    SIZE = 1580.0
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
            if hsh % 11 == 0: mat = "sand_light"
            elif hsh % 7 == 0: mat = "sand_dark"
            elif math.sin(cx * 0.006 + 2.0) * math.cos(cz * 0.005 - 1.0) > 0.55: mat = "patch"
            m.face((idx[(i, j)], idx[(i + 1, j)], idx[(i + 1, j + 1)], idx[(i, j + 1)]), mat)
    def scatter(rmin, rmax):
        a = random.uniform(0, 2 * math.pi)
        r = random.uniform(rmin, rmax)
        return math.cos(a) * r, math.sin(a) * r
    # --- rocks (squashed ellipsoids, half buried)
    for _ in range(9):
        x, z = scatter(250, 640)
        rx = random.uniform(18, 42); ry = rx * random.uniform(0.42, 0.55); rz = rx * random.uniform(0.75, 1.0)
        ellipsoid(m, (x, ground_h(x, z) + ry * 0.25, z), (rx, ry, rz), random.choice(["rock", "rock_dark"]), lon=7, lat=4)
    # --- coral bits (cone branches)
    for _ in range(7):
        x, z = scatter(280, 620)
        base_y = ground_h(x, z)
        mat = random.choice(["coral_pink", "coral_orange", "coral_purple"])
        for k in range(4):
            dx = random.uniform(-14, 14); dz = random.uniform(-14, 14)
            h = random.uniform(16, 34)
            cone(m, (x + dx, base_y, z + dz), (x + dx * 1.7, base_y + h, z + dz * 1.7), random.uniform(3.5, 6), mat, seg=5)
        ellipsoid(m, (x, base_y + 3, z), (13, 6, 13), mat, lon=6, lat=3)
    # --- shells (small flattened ellipsoids)
    for _ in range(10):
        x, z = scatter(200, 660)
        r = random.uniform(6, 11)
        ellipsoid(m, (x, ground_h(x, z) + 1.5, z), (r, r * 0.38, r * 0.8), random.choice(["shell", "shell_pink"]), lon=6, lat=3)
    # --- seaweed tufts (tapered bent tubes)
    for _ in range(8):
        x, z = scatter(270, 630)
        base_y = ground_h(x, z)
        for k in range(3):
            dx = random.uniform(-9, 9); dz = random.uniform(-9, 9)
            h = random.uniform(30, 52)
            bend = random.uniform(-12, 12)
            tube(m, [(x + dx, base_y - 2, z + dz),
                     (x + dx + bend * 0.4, base_y + h * 0.55, z + dz + bend * 0.3),
                     (x + dx + bend, base_y + h, z + dz + bend * 0.8)],
                 [3.0, 2.0, 0.7], random.choice(["weed", "weed_dark"]), sides=5)
    return m

m = build_floor()
write_x(m, "floor")
n, mn, mx = render(m, "floor")
print(f"floor: tris={n} verts={len(m.verts)} bbox x[{mn[0]:.0f},{mx[0]:.0f}] y[{mn[1]:.0f},{mx[1]:.0f}] z[{mn[2]:.0f},{mx[2]:.0f}]")
