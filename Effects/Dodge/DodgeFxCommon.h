#pragma once
// Shared helpers for DodgeFx. Game units (Consts.h), Y up. Header-only, DxLib only.
// LivePool keeps live particles packed in [0, live): Update/Draw touch only live ones,
// dead ones are swap-removed, and Spawn on a full pool returns nullptr in O(1).
#include "DxLib.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace DodgeFx {
constexpr float Pi = 3.14159265f;

struct Particle {
    VECTOR pos{}, vel{};
    float age = 0, life = 1, size0 = 4, size1 = 4, alpha = 1;
    float gravity = 0, drag = 0, wobble = 0, stretch = 0;   // stretch > 0 => streak (length = |vel| * stretch)
    float spin = 0, phase = 0;                              // glitter twinkle
    unsigned char r = 255, g = 255, b = 255;
    unsigned char r1 = 255, g1 = 255, b1 = 255;           // end color (used when tint = true)
    bool tint = false;
    // orbit mode (vortex): position follows a shrinking spiral around orbitCenter
    bool orbit = false;
    VECTOR orbitCenter{};
    float orbitRadius = 0, orbitAngle = 0, orbitSpeed = 0, inwardSpeed = 0, riseSpeed = 0;
    unsigned char tex = 0;                                  // 0 glow, 1 bubble
    bool additive = true;
};

class Rng {
public:
    explicit Rng(uint32_t seed = 0xD0D6E5A1u) : s(seed ? seed : 1) {}
    float Next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s & 0xFFFFFF) / 16777216.0f; }
    float Range(float a, float b) { return a + (b - a) * Next(); }
private:
    uint32_t s;
};

inline VECTOR SafeNorm(VECTOR v, VECTOR fallback = VGet(0, 1, 0)) {
    float l = std::sqrt(VSquareSize(v));
    return (l > 1e-5f && std::isfinite(l)) ? VScale(v, 1.0f / l) : fallback;
}
// Game convention: yaw = atan2(x, z); forward(yaw) = (sin yaw, 0, cos yaw).
inline float Yaw(VECTOR d) { return (d.x == 0 && d.z == 0) ? 0.0f : std::atan2(d.x, d.z); }
inline VECTOR Forward(float yaw) { return VGet(std::sin(yaw), 0, std::cos(yaw)); }
inline VECTOR Flat(VECTOR v, VECTOR fallback = VGet(0, 0, 1)) { return SafeNorm(VGet(v.x, 0, v.z), fallback); }
inline bool Finite(VECTOR v) { return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z); }
// Random direction inside a cone of half-angle `spread` (radians) around `axis`.
inline VECTOR ConeDir(Rng& rng, VECTOR axis, float spread) {
    axis = SafeNorm(axis);
    VECTOR t = std::fabs(axis.y) < 0.95f ? VGet(0, 1, 0) : VGet(1, 0, 0);
    VECTOR u = SafeNorm(VCross(axis, t)), w = VCross(axis, u);
    float a = rng.Range(-Pi, Pi), c = std::cos(rng.Next() * spread), s = std::sqrt((std::max)(0.0f, 1 - c * c));
    return VAdd(VScale(axis, c), VAdd(VScale(u, std::cos(a) * s), VScale(w, std::sin(a) * s)));
}

template <int N>
class LivePool {
public:
    int Count() const { return live; }
    void Clear() { live = 0; }
    DodgeFx::Particle* Spawn() {                       // O(1); nullptr when full (spawn dropped)
        if (live >= N) return nullptr;
        items[live] = DodgeFx::Particle{};
        return &items[live++];
    }
    void Update(float dt) {
        int i = 0;
        while (i < live) {
            DodgeFx::Particle& p = items[i];
            p.age += dt;
            if (p.age >= p.life) { items[i] = items[--live]; continue; }   // swap-remove, re-check slot i
            if (p.orbit) {
                p.orbitRadius = (std::max)(0.0f, p.orbitRadius - p.inwardSpeed * dt);
                p.orbitAngle += p.orbitSpeed * dt * (1.0f + 60.0f / (p.orbitRadius + 20.0f));   // faster near the center
                p.pos = VGet(p.orbitCenter.x + std::sin(p.orbitAngle) * p.orbitRadius, p.pos.y + p.riseSpeed * dt,
                             p.orbitCenter.z + std::cos(p.orbitAngle) * p.orbitRadius);
                if (p.orbitRadius <= 2.0f) p.age = (std::max)(p.age, p.life - 0.15f);       // swallowed: fade quickly
                ++i; continue;
            }
            p.vel.y -= p.gravity * dt;
            if (p.drag > 0) p.vel = VScale(p.vel, (std::max)(0.0f, 1 - p.drag * dt));
            p.pos = VAdd(p.pos, VScale(p.vel, dt));
            ++i;
        }
    }
    const DodgeFx::Particle& operator[](int i) const { return items[i]; }
private:
    DodgeFx::Particle items[N];
    int live = 0;
};

// Procedural textures (create lazily after DxLib_Init).
inline int MakeTexture(int size, float (*alphaAt)(float dx, float dy)) {
    int soft = MakeARGB8ColorSoftImage(size, size);
    if (soft < 0) return -1;
    float c = (size - 1) * 0.5f;
    for (int y = 0; y < size; ++y) for (int x = 0; x < size; ++x) {
        float a = alphaAt((x - c) / c, (y - c) / c);
        DrawPixelSoftImage(soft, x, y, 255, 255, 255, static_cast<int>(std::clamp(a, 0.0f, 1.0f) * 255));
    }
    int g = CreateGraphFromSoftImage(soft);
    DeleteSoftImage(soft);
    return g;
}
inline float GlowAlpha(float dx, float dy) { float d2 = dx * dx + dy * dy; return d2 >= 1 ? 0.0f : std::exp(-d2 * 4.5f) * (1 - d2); }
inline float BubbleAlpha(float dx, float dy) {
    float d = std::sqrt(dx * dx + dy * dy);
    if (d >= 1) return 0;
    float rim = std::exp(-std::pow((d - 0.86f) / 0.09f, 2.0f));
    float hx = dx + 0.35f, hy = dy + 0.4f;
    return rim + 0.12f * (d < 0.9f) + std::exp(-(hx * hx + hy * hy) / 0.03f);
}

// Draws live particles: billboards, or camera-facing streak quads when stretch > 0.
// Uses one fixed 6-vertex buffer (no allocation). Restores lighting ON afterwards.
template <int N>
inline void DrawPool(const LivePool<N>& pool, int glowTex, int bubbleTex) {
    SetUseLighting(FALSE); SetUseZBuffer3D(TRUE); SetWriteZBuffer3D(FALSE);
    VECTOR cam = GetCameraPosition();
    VERTEX3D quad[6];
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < pool.Count(); ++i) {
            const DodgeFx::Particle& p = pool[i];
            if (p.additive != (pass == 1)) continue;
            float t = p.age / p.life;
            float a = p.alpha * (std::min)(1.0f, t / 0.05f) * (std::min)(1.0f, (1 - t) / 0.4f);
            if (p.spin > 0) a *= 0.45f + 0.55f * std::fabs(std::sin(p.age * p.spin + p.phase));
            if (a <= 0.004f) continue;
            int tex = p.tex == 1 && bubbleTex >= 0 ? bubbleTex : glowTex;
            float size = p.size0 + (p.size1 - p.size0) * t;
            int R = p.r, G = p.g, B = p.b;
            if (p.tint) { R += static_cast<int>((p.r1 - p.r) * t); G += static_cast<int>((p.g1 - p.g) * t); B += static_cast<int>((p.b1 - p.b) * t); }
            SetDrawBlendMode(p.additive ? DX_BLENDMODE_ADD : DX_BLENDMODE_ALPHA, static_cast<int>(255 * (std::min)(1.0f, a)));
            VECTOR pos = p.pos;
            if (p.wobble > 0) { pos.x += std::sin(p.age * 12 + p.phase) * p.wobble; pos.z += std::cos(p.age * 10 + p.phase) * p.wobble; }
            if (p.stretch > 0) {
                VECTOR tail = VSub(pos, VScale(p.vel, p.stretch));
                VECTOR side = SafeNorm(VCross(VSub(pos, tail), VSub(cam, pos)), VGet(1, 0, 0));
                side = VScale(side, size * 0.5f);
                COLOR_U8 col = GetColorU8(R, G, B, 255);
                VECTOR c0 = VAdd(pos, side), c1 = VSub(pos, side), c2 = VSub(tail, side), c3 = VAdd(tail, side);
                VECTOR cs[6] = {c0, c1, c2, c0, c2, c3};
                float us[6] = {1, 1, 0, 1, 0, 0}, vs[6] = {0, 1, 1, 0, 1, 0};
                for (int k = 0; k < 6; ++k) {
                    VERTEX3D& v = quad[k];
                    v.pos = cs[k]; v.norm = VGet(0, 1, 0); v.dif = col; v.spc = GetColorU8(0, 0, 0, 0);
                    // stretch the soft dot along the streak: sample a horizontal band of the glow
                    v.u = us[k] > 0 ? 0.85f : 0.15f; v.v = vs[k]; v.su = 0; v.sv = 0;
                }
                DrawPolygon3D(quad, 2, tex, TRUE);
            } else {
                SetDrawBright(R, G, B);
                DrawBillboard3D(pos, 0.5f, 0.5f, size, 0, tex, TRUE);
                SetDrawBright(255, 255, 255);
            }
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}

// Base for one effect type: one shared pool + lazily created textures.
template <int N>
class FxType {
public:
    FxType() = default;
    FxType(const FxType&) = delete;
    FxType& operator=(const FxType&) = delete;
    ~FxType() { if (glowTex >= 0) DeleteGraph(glowTex); if (bubbleTex >= 0) DeleteGraph(bubbleTex); }
    void Update(float dt) { pool.Update(dt); }
    void Draw() const {
        if (pool.Count() == 0) return;
        if (glowTex < 0) glowTex = MakeTexture(32, GlowAlpha);
        if (bubbleTex < 0) bubbleTex = MakeTexture(48, BubbleAlpha);
        DrawPool(pool, glowTex, bubbleTex);
    }
    void Clear() { pool.Clear(); running = false; }
    int ActiveCount() const { return pool.Count(); }
    bool IsActive() const { return running || pool.Count() > 0; }
    int Capacity() const { return N; }
protected:
    LivePool<N> pool;
    Rng rng;
    mutable int glowTex = -1, bubbleTex = -1;
    bool running = false;
    // Generic spawn helper (returns nullptr when full).
    DodgeFx::Particle* Add(VECTOR pos, VECTOR vel, float life, float s0, float s1, float alpha, int r, int g, int b, bool additive) {
        DodgeFx::Particle* p = pool.Spawn();
        if (!p) return nullptr;
        p->pos = pos; p->vel = vel; p->life = life; p->size0 = s0; p->size1 = s1; p->alpha = alpha;
        p->r = static_cast<unsigned char>(r); p->g = static_cast<unsigned char>(g); p->b = static_cast<unsigned char>(b); p->additive = additive;
        return p;
    }
};
}
