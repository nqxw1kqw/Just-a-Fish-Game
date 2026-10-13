#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "WaterSplash.h"
#include "DxConv.h"

namespace
{
    constexpr float Pi = 3.14159265f;

    float RandomFloat(float minVal, float maxVal)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + r * (maxVal - minVal);
    }

    // Sinh texture procedurally trực tiếp trong RAM (không cần file ảnh ngoài)
    int MakeTexture(bool drop)
    {
        BASEIMAGE image{};
        if (CreateARGB8ColorBaseImage(128, 128, &image) < 0) return -1;
        for (int y = 0; y < 128; ++y)
        {
            for (int x = 0; x < 128; ++x)
            {
                float dx = (x - 63.5f) / 63.5f;
                float dy = (y - 63.5f) / 63.5f;
                float radius = std::sqrt(dx * dx * (drop ? 2.8f : 1.0f) + dy * dy);
                float alpha = drop ? std::clamp((1.0f - radius) * 8.0f, 0.0f, 1.0f)
                                   : std::pow((std::max)(0.0f, 1.0f - radius), 2.0f);
                float highlight = drop ? std::exp(-((dx + 0.15f) * (dx + 0.15f) * 90.0f + (dy + 0.3f) * (dy + 0.3f) * 20.0f)) : 1.0f;
                int light = static_cast<int>(210.0f + 45.0f * highlight);
                SetPixelBaseImage(&image, x, y, light, light, light, static_cast<int>(alpha * 255.0f));
            }
        }
        int handle = CreateGraphFromBaseImage(&image);
        ReleaseBaseImage(&image);
        return handle;
    }

    // Các buffer đỉnh tĩnh cố định: TUYỆT ĐỐI KHÔNG CẤP PHÁT ĐỘNG TRONG FRAME
    constexpr int CROWN_SEGMENTS = 48;
    constexpr int RING_SEGMENTS = 48;
    VERTEX3D s_crownMesh[CROWN_SEGMENTS * 12];
    VERTEX3D s_ringMesh[RING_SEGMENTS * 12];
}

WaterSplashSystem::~WaterSplashSystem()
{
    Shutdown();
}

void WaterSplashSystem::Init()
{
    if (dropTexture_ < 0)
    {
        dropTexture_ = MakeTexture(true);
    }
    if (glowTexture_ < 0)
    {
        glowTexture_ = MakeTexture(false);
    }
    Reset();
}

void WaterSplashSystem::Reset()
{
    for (auto& d : drops_) d.active = false;
    for (auto& r : ripples_) r.active = false;
    for (auto& c : crowns_) c.active = false;
    for (auto& f : foams_) f.active = false;
}

void WaterSplashSystem::Shutdown()
{
    if (dropTexture_ >= 0)
    {
        DeleteGraph(dropTexture_);
        dropTexture_ = -1;
    }
    if (glowTexture_ >= 0)
    {
        DeleteGraph(glowTexture_);
        glowTexture_ = -1;
    }
}

void WaterSplashSystem::EmitImpact(const Vec3& origin, float radius, float strength)
{
    strength = std::clamp(strength, 0.7f, 1.5f);
    Vec3 basePos = origin;
    basePos.y = 0.05f;

    // 1. Sinh Vành nước 3D dựng đứng hình vương miện (Splash Crown)
    for (auto& c : crowns_)
    {
        if (!c.active)
        {
            c.center = basePos;
            c.age = 0.0f;
            c.lifetime = 0.38f;
            c.startRadius = radius * 0.18f;
            c.endRadius = radius * 0.95f;
            c.maxHeight = (std::min)(32.0f, 22.0f * strength);
            c.strength = strength;
            c.active = true;
            break;
        }
    }

    // 2. Sinh 3 Vòng sóng nước mở rộng dần (Expanding Ripples) với nhịp trễ
    int rippleCount = 0;
    for (auto& r : ripples_)
    {
        if (!r.active)
        {
            r.center = basePos;
            r.age = -static_cast<float>(rippleCount) * 0.07f; // Trễ nhịp
            r.lifetime = 0.70f + static_cast<float>(rippleCount) * 0.12f;
            r.startRadius = radius * 0.12f;
            r.endRadius = radius * (1.15f - static_cast<float>(rippleCount) * 0.15f);
            r.opacity = 0.85f - static_cast<float>(rippleCount) * 0.15f;
            r.active = true;
            rippleCount++;
            if (rippleCount >= 3) break;
        }
    }

    // 3. Sinh 40-50 hạt giọt nước 3D bắn tỏa 360 độ parabol
    int dropsToSpawn = static_cast<int>(45.0f * strength);
    for (int i = 0; i < dropsToSpawn; ++i)
    {
        for (auto& d : drops_)
        {
            if (!d.active)
            {
                float angle = (2.0f * Pi * static_cast<float>(i)) / static_cast<float>(dropsToSpawn)
                              + RandomFloat(-0.25f, 0.25f);
                float speed = RandomFloat(45.0f, 105.0f) * strength;
                float startR = RandomFloat(3.0f, radius * 0.25f);

                d.pos = Vec3{
                    basePos.x + std::cos(angle) * startR,
                    basePos.y + RandomFloat(1.5f, 4.0f),
                    basePos.z + std::sin(angle) * startR
                };
                d.vel = Vec3{
                    std::cos(angle) * speed,
                    RandomFloat(65.0f, 140.0f) * strength,
                    std::sin(angle) * speed
                };
                d.gravity = 300.0f;
                d.surfaceY = 0.1f;
                d.age = 0.0f;
                d.lifetime = RandomFloat(0.55f, 0.85f);
                d.sizeStart = RandomFloat(4.5f, 7.5f) * strength;
                d.sizeEnd = d.sizeStart * 0.40f;
                d.r = 175;
                d.g = 230;
                d.b = 255;
                d.active = true;
                break;
            }
        }
    }

    // 4. Sinh bọt nước trắng dạt nhanh trên sàn
    int foamsToSpawn = static_cast<int>(20.0f * strength);
    for (int i = 0; i < foamsToSpawn; ++i)
    {
        for (auto& f : foams_)
        {
            if (!f.active)
            {
                float a = RandomFloat(0.0f, 2.0f * Pi);
                float speed = RandomFloat(25.0f, 65.0f) * strength;
                float dist = RandomFloat(2.0f, radius * 0.35f);

                f.pos = Vec3{
                    basePos.x + std::cos(a) * dist,
                    basePos.y + 0.15f,
                    basePos.z + std::sin(a) * dist
                };
                f.vel = Vec3{
                    std::cos(a) * speed,
                    0.0f,
                    std::sin(a) * speed
                };
                f.age = 0.0f;
                f.lifetime = RandomFloat(0.40f, 0.65f);
                f.sizeStart = RandomFloat(4.0f, 6.5f);
                f.sizeEnd = 1.2f;
                f.r = 220;
                f.g = 245;
                f.b = 255;
                f.active = true;
                break;
            }
        }
    }
}

void WaterSplashSystem::SpawnSecondaryRipple(const Vec3& pos)
{
    for (auto& r : ripples_)
    {
        if (!r.active)
        {
            r.center = pos;
            r.center.y = 0.08f;
            r.age = 0.0f;
            r.lifetime = 0.38f;
            r.startRadius = 1.0f;
            r.endRadius = 9.0f;
            r.opacity = 0.35f;
            r.active = true;
            break;
        }
    }
}

void WaterSplashSystem::Update(float dt)
{
    // 1. Cập nhật Vành nước Crown
    for (auto& c : crowns_)
    {
        if (!c.active) continue;
        c.age += dt;
        if (c.age >= c.lifetime)
        {
            c.active = false;
        }
    }

    // 2. Cập nhật Sóng nước Ripples
    for (auto& r : ripples_)
    {
        if (!r.active) continue;
        r.age += dt;
        if (r.age >= r.lifetime)
        {
            r.active = false;
        }
    }

    // 3. Cập nhật Giọt nước bay parabol
    for (auto& d : drops_)
    {
        if (!d.active) continue;
        d.age += dt;
        if (d.age >= d.lifetime)
        {
            d.active = false;
            continue;
        }

        // Tích hợp trọng lực
        d.pos.x += d.vel.x * dt;
        d.pos.y += d.vel.y * dt - 0.5f * d.gravity * dt * dt;
        d.pos.z += d.vel.z * dt;
        d.vel.y -= d.gravity * dt;

        // Chạm đất: sinh ripple mini phụ rồi tan biến
        if (d.pos.y <= d.surfaceY)
        {
            SpawnSecondaryRipple(d.pos);
            d.active = false;
        }
    }

    // 4. Cập nhật Bọt nước trôi dạt
    for (auto& f : foams_)
    {
        if (!f.active) continue;
        f.age += dt;
        if (f.age >= f.lifetime)
        {
            f.active = false;
            continue;
        }

        f.pos.x += f.vel.x * dt;
        f.pos.z += f.vel.z * dt;
        // Giảm tốc ma sát mặt nước
        float decay = (std::max)(0.0f, 1.0f - dt * 3.5f);
        f.vel.x *= decay;
        f.vel.z *= decay;
    }
}

void WaterSplashSystem::Draw() const
{
    SetUseLighting(FALSE);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);
    SetUseBackCulling(DX_CULLING_NONE);

    // 1. VẼ CÁC VÒNG SÓNG NƯỚC (RIPPLES) TRÊN SÀN
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
    for (const auto& r : ripples_)
    {
        if (!r.active || r.age < 0.0f) continue;
        float t = std::clamp(r.age / r.lifetime, 0.0f, 1.0f);
        float radius = r.startRadius + (r.endRadius - r.startRadius) * (1.0f - std::pow(1.0f - t, 1.25f));
        float width = 3.2f + t * 4.5f;
        float opacity = r.opacity * std::pow(1.0f - t, 1.3f);
        if (opacity <= 0.01f) continue;

        int vertIdx = 0;
        for (int i = 0; i < RING_SEGMENTS; ++i)
        {
            float a = i * (2.0f * Pi / static_cast<float>(RING_SEGMENTS));
            float b = (i + 1) * (2.0f * Pi / static_cast<float>(RING_SEGMENTS));
            float ra = radius + 0.35f * std::sin(a * 7.0f + r.age * 3.0f);
            float rb = radius + 0.35f * std::sin(b * 7.0f + r.age * 3.0f);

            for (int band = 0; band < 2; ++band)
            {
                float r0 = -width + static_cast<float>(band) * width;
                float r1 = r0 + width;
                int alpha0 = (band == 0) ? 0 : static_cast<int>(opacity * 255.0f);
                int alpha1 = (band == 0) ? static_cast<int>(opacity * 255.0f) : 0;

                auto makeV = [&](float ang, float rad, int alp) {
                    VERTEX3D v{};
                    v.pos = VGet(r.center.x + std::cos(ang) * (std::max)(0.0f, rad),
                                 r.center.y + 0.12f,
                                 r.center.z + std::sin(ang) * (std::max)(0.0f, rad));
                    v.norm = VGet(0, 1, 0);
                    v.dif = GetColorU8(175, 230, 250, std::clamp(alp, 0, 255));
                    return v;
                };

                VERTEX3D v0 = makeV(a, ra + r0, alpha0);
                VERTEX3D v1 = makeV(b, rb + r0, alpha0);
                VERTEX3D v2 = makeV(b, rb + r1, alpha1);
                VERTEX3D v3 = makeV(a, ra + r1, alpha1);

                s_ringMesh[vertIdx++] = v0;
                s_ringMesh[vertIdx++] = v1;
                s_ringMesh[vertIdx++] = v2;

                s_ringMesh[vertIdx++] = v0;
                s_ringMesh[vertIdx++] = v2;
                s_ringMesh[vertIdx++] = v3;
            }
        }
        DrawPrimitive3D(s_ringMesh, vertIdx, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);
    }

    // 2. VẼ VÀNH NƯỚC 3D VƯƠNG MIỆN (SPLASH CROWNS)
    for (const auto& c : crowns_)
    {
        if (!c.active) continue;
        float t = std::clamp(c.age / c.lifetime, 0.0f, 1.0f);
        float radius = c.startRadius + (c.endRadius - c.startRadius) * t;
        float height = std::pow(std::sin(Pi * t), 0.7f) * (1.0f - t) * c.maxHeight;
        float alpha = std::pow(1.0f - t, 1.2f);
        if (alpha <= 0.01f) continue;

        auto makeCrownV = [&](float angle, float row) {
            float peak = 0.55f + 0.45f * std::pow(0.5f + 0.5f * std::sin(angle * 12.0f), 3.0f);
            float rOut = radius + row * (2.5f + t * 3.5f);
            float hOut = row * height * peak;

            VERTEX3D v{};
            v.pos = VGet(c.center.x + std::cos(angle) * rOut,
                         c.center.y + 0.2f + hOut,
                         c.center.z + std::sin(angle) * rOut);
            v.norm = VGet(0, 1, 0);

            int cr = static_cast<int>(80.0f + row * 115.0f);
            int cg = static_cast<int>(170.0f + row * 70.0f);
            int cb = static_cast<int>(210.0f + row * 45.0f);
            int ca = static_cast<int>(alpha * (90.0f + row * 95.0f));
            v.dif = GetColorU8(cr, cg, cb, std::clamp(ca, 0, 255));
            return v;
        };

        int vertIdx = 0;
        for (int i = 0; i < CROWN_SEGMENTS; ++i)
        {
            float a = i * (2.0f * Pi / static_cast<float>(CROWN_SEGMENTS));
            float b = (i + 1) * (2.0f * Pi / static_cast<float>(CROWN_SEGMENTS));

            for (int row = 0; row < 2; ++row)
            {
                float lo = static_cast<float>(row) * 0.5f;
                float hi = lo + 0.5f;

                VERTEX3D v0 = makeCrownV(a, lo);
                VERTEX3D v1 = makeCrownV(b, lo);
                VERTEX3D v2 = makeCrownV(b, hi);
                VERTEX3D v3 = makeCrownV(a, hi);

                s_crownMesh[vertIdx++] = v0;
                s_crownMesh[vertIdx++] = v1;
                s_crownMesh[vertIdx++] = v2;

                s_crownMesh[vertIdx++] = v0;
                s_crownMesh[vertIdx++] = v2;
                s_crownMesh[vertIdx++] = v3;
            }
        }
        DrawPrimitive3D(s_crownMesh, vertIdx, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);
    }

    // 3. VẼ CÁC HẠT GIỌT NƯỚC 3D (BILLBOARDS)
    if (dropTexture_ >= 0)
    {
        for (const auto& d : drops_)
        {
            if (!d.active) continue;
            float t = d.age / d.lifetime;
            float alpha = (std::min)(1.0f, (1.0f - t) * 3.0f);
            float curSize = d.sizeStart + (d.sizeEnd - d.sizeStart) * t;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha * 240.0f));
            SetDrawBright(d.r, d.g, d.b);
            DrawBillboard3D(DxConv::ToVECTOR(d.pos), 0.5f, 0.5f, curSize, 0.0f, dropTexture_, TRUE);
        }
    }

    // 4. VẼ CÁC HẠT BỌT NƯỚC TRÊN MẶT SÀN
    if (glowTexture_ >= 0)
    {
        for (const auto& f : foams_)
        {
            if (!f.active) continue;
            float t = f.age / f.lifetime;
            float alpha = 1.0f - t;
            float curSize = f.sizeStart + (f.sizeEnd - f.sizeStart) * t;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha * 200.0f));
            SetDrawBright(f.r, f.g, f.b);
            DrawBillboard3D(DxConv::ToVECTOR(f.pos), 0.5f, 0.5f, curSize, 0.0f, glowTexture_, TRUE);
        }
    }

    // Khôi phục lại trạng thái vẽ chuẩn của DxLib
    SetDrawBright(255, 255, 255);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}
