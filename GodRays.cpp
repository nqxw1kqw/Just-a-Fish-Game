#include "GodRays.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int RAY_COUNT = 9;
    constexpr float SURFACE_Y = 620.0f;     // độ cao "mặt nước"
    constexpr float SPREAD = 240.0f;        // bán kính phân bố gốc tia quanh tâm
    constexpr int CAUSTIC_RINGS = 20;
    constexpr int CAUSTIC_SEGS = 48;
    constexpr float CAUSTIC_RADIUS = 430.0f;

    // Mỗi tia: 2 mặt chéo nhau, mỗi mặt 4 tam giác (mép - giữa - mép)
    VERTEX3D s_rayVerts[RAY_COUNT * 2 * 12];
    VERTEX3D s_causticVerts[CAUSTIC_RINGS * CAUSTIC_SEGS * 6];

    VERTEX3D V(float x, float y, float z, float intensity, int r, int g, int b)
    {
        intensity = std::clamp(intensity, 0.0f, 1.0f);
        VERTEX3D v{};
        v.pos = VGet(x, y, z);
        v.norm = VGet(0.0f, 1.0f, 0.0f);
        // Chế độ ADD: cường độ mã hoá trực tiếp vào RGB
        v.dif = GetColorU8(static_cast<int>(r * intensity), static_cast<int>(g * intensity), static_cast<int>(b * intensity), 255);
        v.spc = GetColorU8(0, 0, 0, 0);
        return v;
    }

    float Hash(int i) { float s = std::sin(i * 12.9898f) * 43758.5453f; return s - std::floor(s); }
}

void GodRays::PollToggle() const
{
    bool now = CheckHitKey(KEY_INPUT_G) != 0;
    if (now && !keyPrev_) enabled_ = !enabled_;
    keyPrev_ = now;
}

void GodRays::Draw() const
{
    PollToggle();
    if (!enabled_) return;
    if (startCount_ == 0) startCount_ = GetNowHiPerformanceCount();
    float t = static_cast<float>(GetNowHiPerformanceCount() - startCount_) / 1000000.0f;

    // Hướng nghiêng chung của ánh sáng (mặt trời lệch nhẹ), lắc lư chậm
    float slantX = 0.22f + 0.04f * std::sin(t * 0.21f);
    float slantZ = 0.12f + 0.03f * std::cos(t * 0.17f);

    int n = 0;
    for (int i = 0; i < RAY_COUNT; ++i)
    {
        float ang = Hash(i) * 6.2831853f, rad = std::sqrt(Hash(i + 31)) * SPREAD;
        float phase = Hash(i + 57) * 6.2831853f;
        float topX = std::cos(ang) * rad + std::sin(t * 0.35f + phase) * 28.0f;
        float topZ = std::sin(ang) * rad + std::cos(t * 0.29f + phase) * 22.0f;
        float botX = topX - slantX * SURFACE_Y;
        float botZ = topZ - slantZ * SURFACE_Y;
        float width = 26.0f + Hash(i + 91) * 40.0f;
        float shimmer = 0.55f + 0.45f * std::sin(t * (0.6f + Hash(i + 13) * 0.6f) + phase);
        float topI = 0.20f * shimmer, botI = 0.03f * shimmer;
        int cr = 150, cg = 215, cb = 255;

        for (int k = 0; k < 2; ++k)
        {
            // 2 mặt vuông góc trên mặt phẳng XZ để nhìn từ góc nào cũng có độ dày
            float ox = (k == 0) ? width : 0.0f, oz = (k == 0) ? 0.0f : width;
            float wb = 1.6f;   // tia loe rộng hơn ở đáy
            VERTEX3D tl = V(topX - ox, SURFACE_Y, topZ - oz, 0.0f, cr, cg, cb);
            VERTEX3D tc = V(topX, SURFACE_Y, topZ, topI, cr, cg, cb);
            VERTEX3D tr = V(topX + ox, SURFACE_Y, topZ + oz, 0.0f, cr, cg, cb);
            VERTEX3D bl = V(botX - ox * wb, 0.5f, botZ - oz * wb, 0.0f, cr, cg, cb);
            VERTEX3D bc = V(botX, 0.5f, botZ, botI, cr, cg, cb);
            VERTEX3D br = V(botX + ox * wb, 0.5f, botZ + oz * wb, 0.0f, cr, cg, cb);
            VERTEX3D tri[12] = { tl, bl, bc, tl, bc, tc, tc, bc, br, tc, br, tr };
            for (const auto& v : tri) s_rayVerts[n++] = v;
        }
    }

    // Caustics: hoa văn giao thoa sóng trên sàn quanh tâm
    int m = 0;
    auto caustic = [t](float x, float z)
    {
        float a = std::sin(x * 0.045f + t * 1.1f) + std::sin(z * 0.05f - t * 0.9f)
                + std::sin((x + z) * 0.032f + t * 0.7f) + std::sin((x - z) * 0.04f - t * 1.3f);
        float v = 1.0f - std::abs(a) * 0.5f;     // đường sáng mảnh ở chỗ giao thoa
        return std::pow(std::clamp(v, 0.0f, 1.0f), 6.0f);
    };
    auto causticVertex = [&](int ring, int seg)
    {
        float r = CAUSTIC_RADIUS * ring / CAUSTIC_RINGS;
        float a = 6.2831853f * seg / CAUSTIC_SEGS;
        float x = std::cos(a) * r, z = std::sin(a) * r;
        float falloff = 1.0f - (r / CAUSTIC_RADIUS);
        return V(x, 0.8f, z, caustic(x, z) * 0.22f * falloff * falloff, 170, 230, 255);
    };
    for (int ring = 0; ring < CAUSTIC_RINGS; ++ring)
    {
        for (int seg = 0; seg < CAUSTIC_SEGS; ++seg)
        {
            VERTEX3D a = causticVertex(ring, seg), b = causticVertex(ring + 1, seg);
            VERTEX3D c = causticVertex(ring + 1, seg + 1), d = causticVertex(ring, seg + 1);
            s_causticVerts[m++] = a; s_causticVerts[m++] = b; s_causticVerts[m++] = c;
            s_causticVerts[m++] = a; s_causticVerts[m++] = c; s_causticVerts[m++] = d;
        }
    }

    SetUseLighting(FALSE);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);
    SetUseBackCulling(FALSE);
    SetDrawBlendMode(DX_BLENDMODE_ADD, 255);
    DrawPrimitive3D(s_causticVerts, m, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);
    DrawPrimitive3D(s_rayVerts, n, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}
