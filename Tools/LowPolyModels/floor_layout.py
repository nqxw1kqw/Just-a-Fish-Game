"""Single source for visible reef rocks and generated runtime collision volumes."""
# x, z, footprint radius, visible height, material
# Central disk and cardinal escape lanes remain open. Reefs are deliberately separated.
ROCKS = [
    (-355, -315, 49, 54, "rock"), (-418, -352, 28, 29, "rock_dark"),
    (350, -325, 52, 61, "rock_dark"), (414, -362, 27, 28, "rock"),
    (-350, 325, 48, 52, "rock_dark"), (-414, 362, 27, 27, "rock"),
    (360, 320, 50, 57, "rock"), (425, 352, 26, 26, "rock_dark"),
    (-548, 0, 35, 34, "rock"), (548, 0, 35, 34, "rock_dark"),
    (-100, 505, 25, 76, "ruin"), (100, 505, 25, 62, "ruin"),
    (-85, -525, 24, 36, "ruin"), (100, -525, 27, 48, "ruin"),
]

# Decorative clusters have soft branches and do not block swimming.
GARDENS = [(-355, -315), (350, -325), (-350, 325), (360, 320),
           (-548, 0), (548, 0), (-100, 505), (100, -525)]
