#include "TsunamiWaterEffect.h"
#include "Consts.h"
#include "DxConv.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    constexpr float PI = 3.14159265f;
    constexpr float TWO_PI = 6.2831853f;
    constexpr float MAX_RADIUS = 1100.0f;  // khớp ngưỡng hủy vòng trong Boss.cpp
    constexpr float FADE_LENGTH = 150.0f;  // mờ dần trong 150 unit cuối

    float Rnd(float a, float b)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return a + r * (b - a);
    }
    float Smooth(float e0, float e1, float x)
    {
        float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }
    float WrapAngle(float a)
    {
        while (a > PI) a -= TWO_PI;
        while (a < -PI) a += TWO_PI;
        return a;
    }
    bool InGap(const TsunamiWaveState& w, float angle, float margin = 0.0f)
    {
        return w.gapHalfRad > 0.0f && std::abs(WrapAngle(angle - w.gapCenterRad)) < w.gapHalfRad + margin;
    }
    float WaveFade(const TsunamiWaveState& w)
    {
        float inner = w.radius - w.thickness * 0.5f;
        return std::clamp((MAX_RADIUS - inner) / FADE_LENGTH, 0.0f, 1.0f);
    }

    // Lưới tĩnh: không cấp phát động trong frame
    constexpr int WAVE_RADIAL = 14;
    constexpr int WAVE_ANGULAR = 180;
    constexpr int MOUND_RADIAL = 12;
    constexpr int MOUND_ANGULAR = 64;
    VERTEX3D s_grid[(WAVE_RADIAL + 1) * (WAVE_ANGULAR + 1)];
    VERTEX3D s_mesh[WAVE_RADIAL * WAVE_ANGULAR * 6];

    VERTEX3D MakeVertex(float x, float y, float z, float foam, float water, float alpha)
    {
        foam = std::clamp(foam, 0.0f, 1.0f);
        water = std::clamp(water, 0.0f, 1.0f);
        // Nước nền -> xanh sóng thần (35,150,245) -> bọt trắng (230,245,255)
        float br = 18.0f + (35.0f - 18.0f) * water;
        float bg = 92.0f + (150.0f - 92.0f) * water;
        float bb = 132.0f + (245.0f - 132.0f) * water;
        VERTEX3D v{};
        v.pos = VGet(x, y, z);
        v.norm = VGet(0.0f, 1.0f, 0.0f);
        v.dif = GetColorU8(static_cast<int>(br * (1.0f - foam) + 230.0f * foam),
                           static_cast<int>(bg * (1.0f - foam) + 245.0f * foam),
                           static_cast<int>(bb * (1.0f - foam) + 255.0f * foam),
                           static_cast<int>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
        v.spc = GetColorU8(0, 0, 0, 0);
        v.u = 0.0f; v.v = 0.0f; v.su = 0.0f; v.sv = 0.0f;
        return v;
    }

    int BuildMesh(int radial, int angular)
    {
        int n = 0;
        for (int r = 0; r < radial; ++r)
        {
            for (int a = 0; a < angular; ++a)
            {
                int i = r * (angular + 1) + a, j = i + angular + 1;
                s_mesh[n++] = s_grid[i]; s_mesh[n++] = s_grid[j]; s_mesh[n++] = s_grid[j + 1];
                s_mesh[n++] = s_grid[i]; s_mesh[n++] = s_grid[j + 1]; s_mesh[n++] = s_grid[i + 1];
            }
        }
        return n;
    }

    int MakeGlowTexture()
    {
        BASEIMAGE image{};
        if (CreateARGB8ColorBaseImage(64, 64, &image) < 0) return -1;
        for (int y = 0; y < 64; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                float dx = (x - 31.5f) / 31.5f, dy = (y - 31.5f) / 31.5f;
                float rad = std::sqrt(dx * dx + dy * dy);
                float alpha = std::pow((std::max)(0.0f, 1.0f - rad), 1.6f);
                SetPixelBaseImage(&image, x, y, 255, 255, 255, static_cast<int>(alpha * 255.0f));
            }
        }
        int handle = CreateGraphFromBaseImage(&image);
        ReleaseBaseImage(&image);
        return handle;
    }
}

TsunamiWaterEffect::~TsunamiWaterEffect()
{
    Shutdown();
}

void TsunamiWaterEffect::Shutdown()
{
    if (glowTexture_ >= 0)
    {
        DeleteGraph(glowTexture_);
        glowTexture_ = -1;
    }
}

void TsunamiWaterEffect::Reset()
{
    for (auto& p : particles_) p.active = false;
    sprayTimer_.fill(0.0f);
    suckTimer_ = 0.0f;
    chargeAge_ = 0.0f;
    moundAge_ = 10.0f;
}

void TsunamiWaterEffect::EnsureTexture()
{
    if (glowTexture_ < 0) glowTexture_ = MakeGlowTexture();
}

void TsunamiWaterEffect::Add(const TsunamiParticle& src)
{
    for (std::size_t k = 0; k < MAX_PARTICLES; ++k)
    {
        std::size_t idx = (nextSlot_ + k) % MAX_PARTICLES;
        if (!particles_[idx].active)
        {
            particles_[idx] = src;
            particles_[idx].active = true;
            particles_[idx].age = 0.0f;
            nextSlot_ = (idx + 1) % MAX_PARTICLES;
            return;
        }
    }
    // Pool đầy: ghi đè hạt cũ nhất theo vòng
    particles_[nextSlot_] = src;
    particles_[nextSlot_].active = true;
    particles_[nextSlot_].age = 0.0f;
    nextSlot_ = (nextSlot_ + 1) % MAX_PARTICLES;
}

std::size_t TsunamiWaterEffect::ActiveParticleCount() const
{
    std::size_t n = 0;
    for (const auto& p : particles_) if (p.active) ++n;
    return n;
}

void TsunamiWaterEffect::OnLaunch(const TsunamiWaveState& w)
{
    EnsureTexture();
    moundAge_ = 0.0f;
    moundCenter_ = w.center;
    const float start = GameConsts::TSUNAMI_START_RADIUS;
    for (int i = 0; i < 72; ++i)
    {
        float angle = TWO_PI * (i + Rnd(0.0f, 1.0f)) / 72.0f;
        if (InGap(w, angle)) continue;
        float sp = Rnd(210.0f, 450.0f);
        float sa = std::sin(angle), ca = std::cos(angle);
        TsunamiParticle p;
        p.pos = Vec3{ w.center.x + sa * start, 18.0f, w.center.z + ca * start };
        p.vel = Vec3{ sa * sp, Rnd(108.0f, 240.0f), ca * sp };
        p.gravity = 420.0f;
        p.lifetime = Rnd(0.5f, 0.9f);
        p.sizeStart = Rnd(4.8f, 9.6f); p.sizeEnd = p.sizeStart * 1.6f;
        p.r = 200; p.g = 235; p.b = 255; p.opacity = 0.85f;
        Add(p);
    }
}

void TsunamiWaterEffect::Update(float dt, bool charging, const Vec3& bossPos,
                                const std::array<TsunamiWaveState, MAX_WAVES>& waves)
{
    EnsureTexture();
    constexpr int SEGMENTS = 64;       // ngân sách hạt dọc mép trước
    constexpr float SPRAY_TICK = 0.05f;

    // Gồng: giọt nước bị hút vào tâm
    if (charging)
    {
        chargeAge_ += dt;
        suckTimer_ += dt;
        while (suckTimer_ >= 0.025f)
        {
            suckTimer_ -= 0.025f;
            for (int i = 0; i < 3; ++i)
            {
                float angle = Rnd(-PI, PI), r = Rnd(120.0f, 228.0f), life = Rnd(0.35f, 0.6f);
                float sa = std::sin(angle), ca = std::cos(angle);
                float v = r * 0.9f / life;
                TsunamiParticle p;
                p.pos = Vec3{ bossPos.x + sa * r, Rnd(3.0f, 18.0f), bossPos.z + ca * r };
                p.vel = Vec3{ -sa * v, Rnd(0.0f, 30.0f), -ca * v };
                p.lifetime = life; p.sizeStart = Rnd(2.4f, 4.8f); p.sizeEnd = 1.2f;
                p.r = 190; p.g = 230; p.b = 255; p.opacity = 0.75f; p.fadeIn = 0.2f;
                Add(p);
            }
        }
    }
    else
    {
        chargeAge_ = 0.0f;
        suckTimer_ = 0.0f;
    }
    moundAge_ += dt;

    // Bọt / giọt phun dọc mép trước
    for (int k = 0; k < MAX_WAVES; ++k)
    {
        const auto& w = waves[k];
        if (!w.active) { sprayTimer_[k] = 0.0f; continue; }
        float outer = w.radius + w.thickness * 0.5f;
        float fade = WaveFade(w);
        sprayTimer_[k] += dt;
        while (sprayTimer_[k] >= SPRAY_TICK)
        {
            sprayTimer_[k] -= SPRAY_TICK;
            if (fade < 0.05f) continue;
            for (int s = 0; s < SEGMENTS; ++s)   // 1 hạt / đoạn / tick
            {
                float angle = TWO_PI * (s + Rnd(0.0f, 1.0f)) / SEGMENTS;
                if (InGap(w, angle, 0.05f)) continue;
                float r = outer - w.thickness * 0.06f;
                float sp = w.speed * Rnd(1.0f, 1.35f);
                float sa = std::sin(angle), ca = std::cos(angle);
                TsunamiParticle p;
                p.pos = Vec3{ w.center.x + sa * r, GameConsts::TSUNAMI_HEIGHT * fade * Rnd(0.6f, 1.0f), w.center.z + ca * r };
                p.vel = Vec3{ sa * sp, Rnd(36.0f, 108.0f), ca * sp };
                p.gravity = 300.0f;
                p.lifetime = Rnd(0.3f, 0.55f);
                p.sizeStart = Rnd(3.6f, 7.2f); p.sizeEnd = p.sizeStart * 1.8f;
                p.r = 230; p.g = 245; p.b = 255; p.opacity = 0.8f * fade;
                Add(p);
            }
        }
    }

    for (auto& p : particles_)
    {
        if (!p.active) continue;
        p.age += dt;
        if (p.age >= p.lifetime) { p.active = false; continue; }
        p.vel.y -= p.gravity * dt;
        p.pos = p.pos + p.vel * dt;
        if (p.pos.y < 0.5f) { p.pos.y = 0.5f; p.vel.y = 0.0f; }
    }
}

void TsunamiWaterEffect::Draw(bool charging, float chargeRatio, const Vec3& bossPos,
                              const std::array<TsunamiWaveState, MAX_WAVES>& waves) const
{
    SetUseLighting(FALSE);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);

    // 1. Khối nước trung tâm: phồng x1.08 + rung khi gồng, xẹp xuống sau khi phóng
    if (charging || moundAge_ < 0.8f)
    {
        Vec3 c = charging ? bossPos : moundCenter_;
        constexpr float MOUND_R = 170.0f;
        for (int a = 0; a <= MOUND_ANGULAR; ++a)
        {
            float angle = TWO_PI * a / MOUND_ANGULAR;
            float sa = std::sin(angle), ca = std::cos(angle);
            for (int r = 0; r <= MOUND_RADIAL; ++r)
            {
                float radius = MOUND_R * r / MOUND_RADIAL;
                float h, water;
                if (charging)
                {
                    float ch = std::clamp(chargeRatio, 0.0f, 1.0f);
                    float jitter = 2.4f * std::sin(chargeAge_ * 61.0f + angle * 3.0f) * ch;
                    float s = 54.0f * (1.0f + 0.08f * ch);
                    h = (15.0f + 12.0f * ch + jitter) * std::exp(-0.5f * radius * radius / (s * s));
                }
                else
                {
                    h = 27.0f * std::exp(-moundAge_ * 7.0f) * std::exp(-0.5f * radius * radius / (54.0f * 54.0f));
                }
                water = std::clamp(h / 20.0f, 0.0f, 1.0f);
                float alpha = 0.8f * water * std::clamp((MOUND_R - radius) / 40.0f, 0.0f, 1.0f);
                s_grid[r * (MOUND_ANGULAR + 1) + a] = MakeVertex(c.x + sa * radius, 0.6f + h, c.z + ca * radius, 0.0f, water, alpha);
            }
        }
        int n = BuildMesh(MOUND_RADIAL, MOUND_ANGULAR);
        DrawPrimitive3D(s_mesh, n, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);
    }

    // 2. Mặt nước vành khuyên 360°: cao chỉ theo bán kính, đuôi mỏng, mép trước dốc
    for (const auto& w : waves)
    {
        if (!w.active) continue;
        float inner = w.radius - w.thickness * 0.5f;
        float travel = (w.speed > 0.0f) ? (w.radius - GameConsts::TSUNAMI_START_RADIUS) / w.speed : 1.0f;
        float grow = Smooth(0.0f, 0.18f, travel);
        float fade = WaveFade(w);
        float rMin = (std::max)(0.0f, inner - 36.0f);
        float rMax = w.radius + w.thickness * 0.5f + 18.0f;
        for (int a = 0; a <= WAVE_ANGULAR; ++a)
        {
            float angle = -PI + TWO_PI * a / WAVE_ANGULAR;
            float sa = std::sin(angle), ca = std::cos(angle);
            float gapEdge = 1.0f;
            if (w.gapHalfRad > 0.0f)
                gapEdge = Smooth(0.0f, 0.08f, std::abs(WrapAngle(angle - w.gapCenterRad)) - w.gapHalfRad);
            float k = grow * fade * gapEdge;
            for (int r = 0; r <= WAVE_RADIAL; ++r)
            {
                float radius = rMin + (rMax - rMin) * r / WAVE_RADIAL;
                float u = (radius - inner) / w.thickness;   // 0 mép trong .. 1 mép trước
                float body = Smooth(-0.35f, 0.85f, u) * (1.0f - Smooth(0.92f, 1.08f, u));
                float lead = std::exp(-0.5f * std::pow((u - 0.95f) / 0.07f, 2.0f));
                float h = GameConsts::TSUNAMI_HEIGHT * body * k;
                float water = body * k;
                float foam = lead * k;
                float edge = (r == 0 || r == WAVE_RADIAL) ? 0.0f : 1.0f;
                float alpha = edge * (0.25f + 0.55f * water + 0.2f * foam) * k;
                s_grid[r * (WAVE_ANGULAR + 1) + a] = MakeVertex(w.center.x + sa * radius, 0.6f + h, w.center.z + ca * radius, foam, water, alpha);
            }
        }
        int n = BuildMesh(WAVE_RADIAL, WAVE_ANGULAR);
        DrawPrimitive3D(s_mesh, n, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE);
    }

    // 3. Hạt bọt / giọt nước (billboard)
    if (glowTexture_ >= 0)
    {
        for (const auto& p : particles_)
        {
            if (!p.active) continue;
            float t = p.age / p.lifetime;
            float alpha = p.opacity * (1.0f - t);
            if (p.fadeIn > 0.0f) alpha *= (std::min)(1.0f, t / p.fadeIn);
            float size = p.sizeStart + (p.sizeEnd - p.sizeStart) * t;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(std::clamp(alpha, 0.0f, 1.0f) * 255.0f));
            SetDrawBright(p.r, p.g, p.b);
            DrawBillboard3D(DxConv::ToVECTOR(p.pos), 0.5f, 0.5f, size, 0.0f, glowTexture_, TRUE);
        }
    }

    SetDrawBright(255, 255, 255);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}
