"""Small dependency-free, flat-shaded mesh helpers for the seabed generator."""
import math
from pathlib import Path


def sub(a, b):
    return tuple(x - y for x, y in zip(a, b))


def cross(a, b):
    return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])


def unit(v):
    length = math.sqrt(sum(x*x for x in v))
    if length < 1e-10:
        raise ValueError("Cannot normalize a zero-length direction")
    return tuple(x / length for x in v)


def basis(direction):
    direction = unit(direction)
    u = unit(cross(direction, (0, 0, 1) if abs(direction[2]) < 0.9 else (1, 0, 0)))
    return u, cross(direction, u)


class Mesh:
    def __init__(self, materials):
        self.materials = materials
        self.verts = []
        self.faces = []

    def v(self, position):
        self.verts.append(position)
        return len(self.verts) - 1

    def face(self, indices, material):
        if material not in self.materials:
            raise KeyError(material)
        for i in range(1, len(indices) - 1):
            tri = (indices[0], indices[i], indices[i + 1])
            a, b, c = (self.verts[j] for j in tri)
            normal = cross(sub(b, a), sub(c, a))
            if sum(x*x for x in normal) > 1e-12:
                self.faces.append((tri, material, unit(normal)))


def ellipsoid(mesh, center, radii, material, lon=8, lat=4):
    rings = []
    for j in range(lat + 1):
        phi = -math.pi/2 + math.pi*j/lat
        rings.append([mesh.v((center[0] + radii[0]*math.cos(phi)*math.cos(2*math.pi*i/lon),
                              center[1] + radii[1]*math.sin(phi),
                              center[2] + radii[2]*math.cos(phi)*math.sin(2*math.pi*i/lon)))
                      for i in range(lon)])
    for j in range(lat):
        for i in range(lon):
            k = (i + 1) % lon
            mesh.face((rings[j][i], rings[j+1][i], rings[j+1][k], rings[j][k]), material)


def cone(mesh, base, tip, radius, material, seg=6):
    u, v = basis(sub(tip, base))
    ring = [mesh.v(tuple(base[k] + radius*(u[k]*math.cos(2*math.pi*i/seg) + v[k]*math.sin(2*math.pi*i/seg))
                         for k in range(3))) for i in range(seg)]
    top = mesh.v(tip)
    for i in range(seg):
        mesh.face((ring[i], ring[(i+1) % seg], top), material)
    mesh.face(tuple(reversed(ring)), material)


def tube(mesh, points, radii, material, sides=5):
    if len(points) != len(radii) or len(points) < 2:
        raise ValueError("A tube needs matching points and radii")
    rings = []
    for i, point in enumerate(points):
        direction = sub(points[min(i+1, len(points)-1)], points[max(0, i-1)])
        u, v = basis(direction)
        rings.append([mesh.v(tuple(point[k] + radii[i]*(u[k]*math.cos(2*math.pi*j/sides) + v[k]*math.sin(2*math.pi*j/sides))
                                   for k in range(3))) for j in range(sides)])
    for i in range(len(rings)-1):
        for j in range(sides):
            k = (j + 1) % sides
            mesh.face((rings[i][j], rings[i][k], rings[i+1][k], rings[i+1][j]), material)
    mesh.face(tuple(reversed(rings[0])), material)
    mesh.face(tuple(rings[-1]), material)


def write_x(mesh, path, name="floor"):
    """Export DirectX text .x with one normal per triangle, in game coordinates."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    materials = list(mesh.materials)
    count = len(mesh.faces)
    lines = ["xof 0303txt 0032", ""]
    for key, rgb in mesh.materials.items():
        r, g, b = (x/255 for x in rgb)
        lines.append(f"Material {name}_{key} {{ {r:.4f};{g:.4f};{b:.4f};1.0000;; 5.0; 0;0;0;; {r*0.3:.4f};{g*0.3:.4f};{b*0.3:.4f};; }}")
    lines += ["", f"Frame {name} {{", " FrameTransformMatrix { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1;; }", f" Mesh {name}Mesh {{", f"  {count*3};"]
    vertices = [mesh.verts[i] for tri, _, _ in mesh.faces for i in tri]
    lines.append(",\n".join(f"  {x:.4f};{y:.4f};{z:.4f};" for x, y, z in vertices) + ";")
    lines += [f"  {count};", ",\n".join(f"  3;{i*3},{i*3+1},{i*3+2};" for i in range(count)) + ";", "  MeshNormals {", f"   {count};"]
    lines.append(",\n".join(f"   {n[0]:.4f};{n[1]:.4f};{n[2]:.4f};" for _, _, n in mesh.faces) + ";")
    lines += [f"   {count};", ",\n".join(f"   3;{i},{i},{i};" for i in range(count)) + ";", "  }", "  MeshMaterialList {", f"   {len(materials)};", f"   {count};"]
    lines.append(",\n".join(f"   {materials.index(material)}" for _, material, _ in mesh.faces) + ";")
    lines += [f"   {{{name}_{key}}}" for key in materials]
    lines += ["  }", " }", "}"]
    path.write_text("\n".join(lines) + "\n", encoding="ascii", newline="\r\n")
