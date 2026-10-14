#pragma once
// Shared fixed-size particle pool for whale boss FX. Game units (Consts.h), Y up.
// Portable: depends only on DxLib. No per-frame heap allocation.
#include "DxLib.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace WhaleFx {
constexpr float Pi = 3.14159265f;

struct Particle {
    VECTOR pos{}, vel{};
    VECTOR target{};            // used when pull > 0 (suction toward a point)
    float age = 0, life = 1, size0 = 10, size1 = 10, alpha = 1;
    float gravity = 0;          // +down (u/s^2); negative = buoyant bubbles
    float drag = 0;             // velocity damping per second
    float pull = 0;             // acceleration toward target (u/s^2)
    float killRadius = 0;       // die when this close to target
    float wobble = 0;           // horizontal wobble amplitude (bubbles)
    unsigned char r = 255, g = 255, b = 255;
    bool additive = false, bubble = false, alive = false;
};

class Rng {
public:
    explicit Rng(uint32_t seed = 0x9E3779B9u) : s(seed ? seed : 1) {}
    float Next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return (s & 0xFFFFFF) / 16777216.0f; }
    float Range(float a, float b) { return a + (b - a) * Next(); }
private:
    uint32_t s;
};

inline VECTOR Dir(float yaw) { return VGet(std::cos(yaw), 0, std::sin(yaw)); }
inline float Yaw(VECTOR d) { return std::atan2(d.z, d.x); }
inline float WrapAngle(float a) { while (a > Pi) a -= 2 * Pi; while (a < -Pi) a += 2 * Pi; return a; }

template <int N>
class Pool {
public:
    void Clear() { for (auto& p : items) p.alive = false; alive = 0; cursor = 0; }
    int Count() const { return alive; }
    // Returns nullptr when full (budget reached).
    Particle* Spawn() {
        for (int i = 0; i < N; ++i) {
            int k = (cursor + i) % N;
            if (!items[k].alive) {
                cursor = (k + 1) % N; items[k] = Particle{}; items[k].alive = true; ++alive; return &items[k];
            }
        }
        return nullptr;
    }
    void Update(float dt, float floorY = -1e9f) {
        alive = 0;
        for (auto& p : items) {
            if (!p.alive) continue;
            p.age += dt;
            if (p.age >= p.life) { p.alive = false; continue; }
            if (p.pull > 0) {
                VECTOR d = VSub(p.target, p.pos);
                float len = std::sqrt(VSquareSize(d));
                if (len < p.killRadius) { p.alive = false; continue; }
                p.vel = VAdd(p.vel, VScale(d, p.pull * dt / (std::max)(len, 0.001f)));
            }
            p.vel.y -= p.gravity * dt;
            if (p.drag > 0) p.vel = VScale(p.vel, (std::max)(0.0f, 1 - p.drag * dt));
            p.pos = VAdd(p.pos, VScale(p.vel, dt));
            if (p.pos.y < floorY && p.vel.y < 0) { p.pos.y = floorY; p.vel.y *= -0.15f; p.vel.x *= 0.6f; p.vel.z *= 0.6f; }
            ++alive;
        }
    }
    // glowTex: soft dot; bubbleTex: ring bubble (may be -1 -> glow used).
    void Draw(int glowTex, int bubbleTex) const {
        SetUseLighting(FALSE); SetUseZBuffer3D(TRUE); SetWriteZBuffer3D(FALSE);
        for (int pass = 0; pass < 2; ++pass) {
            for (const auto& p : items) {
                if (!p.alive || p.additive != (pass == 1)) continue;
                float t = p.age / p.life;
                float a = p.alpha * (std::min)(1.0f, t / 0.08f) * (std::min)(1.0f, (1 - t) / 0.35f);
                if (a <= 0.003f) continue;
                SetDrawBlendMode(p.additive ? DX_BLENDMODE_ADD : DX_BLENDMODE_ALPHA, static_cast<int>(255 * (std::min)(1.0f, a)));
                SetDrawBright(p.r, p.g, p.b);
                VECTOR pos = p.pos;
                if (p.wobble > 0) { pos.x += std::sin(p.age * 11 + p.size0) * p.wobble; pos.z += std::cos(p.age * 9 + p.size0) * p.wobble; }
                int tex = (p.bubble && bubbleTex >= 0) ? bubbleTex : glowTex;
                DrawBillboard3D(pos, 0.5f, 0.5f, p.size0 + (p.size1 - p.size0) * t, 0, tex, TRUE);
            }
        }
        SetDrawBright(255, 255, 255); SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0); SetWriteZBuffer3D(TRUE);
    }
private:
    Particle items[N];
    int alive = 0, cursor = 0;
};

// Procedural bubble sprite (thin bright rim + highlight). Call after DxLib_Init; DeleteGraph when done.
inline int CreateBubbleTexture(int size = 64) {
    int soft = MakeARGB8ColorSoftImage(size, size);
    if (soft < 0) return -1;
    float c = (size - 1) * 0.5f;
    for (int y = 0; y < size; ++y) for (int x = 0; x < size; ++x) {
        float dx = (x - c) / c, dy = (y - c) / c, d = std::sqrt(dx * dx + dy * dy);
        float rim = std::exp(-std::pow((d - 0.86f) / 0.09f, 2.0f));
        float fill = d < 0.9f ? 0.12f : 0;
        float hx = dx + 0.35f, hy = dy + 0.4f;
        float hi = std::exp(-(hx * hx + hy * hy) / 0.03f);
        float a = std::clamp(rim + fill + hi, 0.0f, 1.0f) * (d < 1 ? 1.0f : 0.0f);
        DrawPixelSoftImage(soft, x, y, 255, 255, 255, static_cast<int>(a * 255));
    }
    int g = CreateGraphFromSoftImage(soft);
    DeleteSoftImage(soft);
    return g;
}

// Common colors
constexpr unsigned char WaterR = 120, WaterG = 200, WaterB = 255;
constexpr unsigned char FoamR = 230, FoamG = 245, FoamB = 255;
}
