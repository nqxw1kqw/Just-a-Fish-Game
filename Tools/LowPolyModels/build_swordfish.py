# Swordfish low-poly for Just-a-Fish-Game. Reuses the Mesh/.x writer from build_bosses.py.
# Game coords: X right, Y up, +Z forward (long bill at +Z). Origin = bottom-center.
import sys, math
sys.path.insert(0, "/workspace/boss-models")
import importlib.util
spec = importlib.util.spec_from_file_location("bb", "/workspace/boss-models/build_bosses.py")
# build_bosses.py builds everything on import; instead copy minimal classes by exec of its top part
src = open("/workspace/boss-models/build_bosses.py").read()
ns = {}
exec(src, ns)
Mesh, loft, ellipsoid, prism_xz = ns['Mesh'], ns['loft'], ns['ellipsoid'], ns['prism_xz']
write_x, write_obj, render = ns['write_x'], ns['write_obj'], ns['render']

def build_swordfish():
    mats = {"body": (70, 110, 150), "back": (45, 75, 115), "belly": (170, 195, 215),
            "bill": (200, 205, 200), "fin": (55, 90, 135), "tail": (50, 85, 130),
            "eye": (12, 12, 18), "glint": (245, 245, 255)}
    m = Mesh(mats)
    # streamlined body loft: (z, half width, half height, center y), tail -95 .. nose 55
    keys = [(-95, 2, 3, 34), (-80, 7, 9, 34), (-58, 12, 17, 35), (-32, 16, 24, 36),
            (-6, 18, 28, 37), (20, 16, 25, 37), (40, 12, 18, 36), (55, 7, 10, 35)]
    loft(m, keys, 18, lambda z, c, s: "back" if c > 0.45 else ("belly" if c < -0.5 else "body"))
    # long flat bill/sword: tapered tube flattened (use tube via ellipsoid chain approximation)
    # bill as elongated flattened ellipsoid + tip cone feel via scaled ellipsoids
    ellipsoid(m, (0, 35, 78), (5.5, 2.2, 26), "bill", lon=12, lat=6)
    ellipsoid(m, (0, 35, 108), (3.8, 1.5, 24), "bill", lon=12, lat=6)
    ellipsoid(m, (0, 35, 138), (1.6, 0.8, 22), "bill", lon=7, lat=4)
    # tall sickle dorsal fin
    prism_xz(m, [(2, -28), (10, 6), (14, 38), (10, 42), (2, 10), (-2, -20)], 62, 3, "fin")
    # pectoral fins
    for sgn in (1, -1):
        out = [(sgn * 14, 18), (sgn * 34, 2), (sgn * 44, -14), (sgn * 30, -8), (sgn * 14, 2)]
        if sgn < 0: out = list(reversed(out))
        prism_xz(m, out, 24, 3, "fin")
    # crescent tail fin (vertical)
    prism_xz(m, [(0, -92), (16, -108), (26, -128), (18, -122), (6, -104), (-6, -104), (-18, -122), (-26, -128), (-16, -108)], 36, 3, "tail")
    # second small dorsal + anal finlets
    prism_xz(m, [(2, -62), (8, -50), (10, -38), (4, -48)], 58, 3, "fin")
    # eyes
    for sgn in (1, -1):
        ellipsoid(m, (sgn * 13, 40, 38), (3.2, 3.2, 3.2), "eye", lon=7, lat=5)
        ellipsoid(m, (sgn * 15, 42, 40), (1.1, 1.1, 1.1), "glint", lon=5, lat=4)
    return m

m = build_swordfish()
write_x(m, "swordfish"); write_obj(m, "swordfish"); render(m, "swordfish")
print("swordfish tris", len(__import__('numpy').array(m.verts)), len(m.faces))
