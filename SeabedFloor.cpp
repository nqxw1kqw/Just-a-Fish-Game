#include "SeabedFloor.h"
#include "DxLib.h"

namespace
{
    int s_texture = -2;                 // -2: chưa nạp, -1: không có file
    constexpr float TILE_WORLD = 350.0f; // 1 ô texture phủ 350 unit (sàn 1400 = 4x4 ô)
    constexpr int GRID = 8;              // chia lưới để fog/ánh sáng theo đỉnh mượt hơn

    int Texture()
    {
        if (s_texture == -2)
        {
            const wchar_t* path = L"Data/Texture/Stage/sand_floor.png";
            s_texture = (FileRead_size(path) > 0) ? LoadGraph(path) : -1;
        }
        return s_texture;
    }
}

bool SeabedFloor::Draw(float half)
{
    int tex = Texture();
    if (tex < 0) return false;

    // Màu nhân (tint) cho cát dưới nước: tối và ngả xanh, ánh sáng/fog hiện có vẫn áp dụng
    const COLOR_U8 tint = GetColorU8(120, 138, 150, 255);
    const COLOR_U8 spc = GetColorU8(0, 0, 0, 0);
    static VERTEX3D verts[GRID * GRID * 6];
    int n = 0;
    const float size = half * 2.0f;
    for (int iz = 0; iz < GRID; ++iz)
    {
        for (int ix = 0; ix < GRID; ++ix)
        {
            float x0 = -half + size * ix / GRID, x1 = -half + size * (ix + 1) / GRID;
            float z0 = -half + size * iz / GRID, z1 = -half + size * (iz + 1) / GRID;
            float u0 = (x0 + half) / TILE_WORLD, u1 = (x1 + half) / TILE_WORLD;
            float v0 = (z0 + half) / TILE_WORLD, v1 = (z1 + half) / TILE_WORLD;
            VERTEX3D q[4];
            float xs[4] = { x0, x1, x1, x0 }, zs[4] = { z0, z0, z1, z1 };
            float us[4] = { u0, u1, u1, u0 }, vs[4] = { v0, v0, v1, v1 };
            for (int k = 0; k < 4; ++k)
            {
                q[k].pos = VGet(xs[k], 0.0f, zs[k]);
                q[k].norm = VGet(0.0f, 1.0f, 0.0f);
                q[k].dif = tint;
                q[k].spc = spc;
                q[k].u = us[k]; q[k].v = vs[k];
                q[k].su = 0.0f; q[k].sv = 0.0f;
            }
            // Thứ tự đỉnh để mặt trên hướng lên (+Y)
            verts[n++] = q[0]; verts[n++] = q[2]; verts[n++] = q[1];
            verts[n++] = q[0]; verts[n++] = q[3]; verts[n++] = q[2];
        }
    }

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    SetTextureAddressMode(DX_TEXADDRESS_WRAP);
    SetDrawMode(DX_DRAWMODE_ANISOTROPIC);
    SetUseBackCulling(FALSE);
    DrawPolygon3D(verts, n / 3, tex, FALSE);
    SetDrawMode(DX_DRAWMODE_NEAREST);
    SetTextureAddressMode(DX_TEXADDRESS_CLAMP);
    return true;
}

void SeabedFloor::Release()
{
    if (s_texture >= 0) DeleteGraph(s_texture);
    s_texture = -2;
}
