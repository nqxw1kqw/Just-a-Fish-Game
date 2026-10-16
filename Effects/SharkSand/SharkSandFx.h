#pragma once
#include "SharkSandFxCommon.h"

// Shark boss Hunt skill: sand/silt effects. One shared pool for all four emitters.
class SharkSandEffect : public SharkSandFx::FxType<1024> {
public:
    // Sand + silt plume when diving into the seabed.
    void SandDiveBurst(VECTOR pos) {
        using namespace SharkSandFx;
        for (int i = 0; i < 28; ++i) Grain(pos, VScale(ConeDir(rng, VGet(0, 1, 0), 0.9f), rng.Range(80, 220)), 0.6f, 500);
        for (int i = 0; i < 8; ++i) Silt(VAdd(pos, VGet(rng.Range(-12, 12), rng.Range(0, 10), rng.Range(-12, 12))),
                                         VScale(ConeDir(rng, VGet(0, 1, 0), 1.2f), rng.Range(15, 45)), 1.1f, 18, 46);
        for (int i = 0; i < 5; ++i) Bubble(pos);
    }
    // Continuous trail behind the dorsal fin while underground. Call every frame.
    void SandFinTrail(VECTOR pos, VECTOR vel) {
        using namespace SharkSandFx;
        VECTOR fwd = Flat(vel, VGet(0, 0, 1));
        VECTOR side = VGet(fwd.z, 0, -fwd.x);
        for (int i = 0; i < 2; ++i) {
            float sgn = (i == 0) ? -1.0f : 1.0f;
            Grain(VAdd(pos, VScale(side, sgn * 5.0f)), VAdd(VAdd(VScale(side, sgn * rng.Range(25, 55)), VScale(fwd, -rng.Range(10, 30))), VGet(0, rng.Range(30, 70), 0)), 0.45f, 420);
        }
        if (rng.Next() < 0.2f) Silt(pos, VScale(fwd, -10), 0.8f, 10, 26);
        if (rng.Next() < 0.08f) Bubble(pos);
    }
    // Sand eruption + debris ring when bursting up to bite.
    void SandEruptBurst(VECTOR pos, float radius = 75.0f) {
        using namespace SharkSandFx;
        float r = (std::max)(5.0f, radius);
        for (int i = 0; i < 32; ++i) Grain(VAdd(pos, VGet(rng.Range(-r, r) * 0.3f, 0, rng.Range(-r, r) * 0.3f)),
                                           VScale(ConeDir(rng, VGet(0, 1, 0), 0.6f), rng.Range(160, 320)), 0.8f, 520);
        for (int i = 0; i < 20; ++i) {
            float a = 2 * Pi * i / 20 + rng.Range(-0.1f, 0.1f);
            VECTOR out = VGet(std::sin(a), 0, std::cos(a));
            SharkSandFx::Particle* d = Add(VAdd(pos, VScale(out, r * 0.8f)), VAdd(VScale(out, rng.Range(60, 130)), VGet(0, rng.Range(80, 160), 0)),
                0.8f, rng.Range(4, 7), 3, 1.0f, 120, 100, 70, false);
            if (d) d->gravity = 600;
        }
        for (int i = 0; i < 8; ++i) Silt(VAdd(pos, VGet(rng.Range(-r, r) * 0.5f, 5, rng.Range(-r, r) * 0.5f)), VGet(0, rng.Range(10, 30), 0), 1.0f, 20, 50);
    }
    // Sand trickling/vibrating inside the warning ring for `duration` seconds.
    void LockRingDust(VECTOR pos, float radius = 75.0f, float duration = 0.7f) {
        if (nRings >= MaxRings) return;                          // O(1) drop
        rings[nRings++] = Ring{pos, (std::max)(5.0f, radius), (std::max)(0.05f, duration), 0, 0};
    }
    void Update(float dt) {
        using namespace SharkSandFx;
        for (int i = 0; i < nRings; ) {
            Ring& g = rings[i];
            g.t += dt;
            if (g.t >= g.dur) { rings[i] = rings[--nRings]; continue; }
            g.acc += dt * 70;
            while (g.acc >= 1) {
                g.acc -= 1;
                float a = rng.Range(-Pi, Pi), rr = g.r * std::sqrt(rng.Next());
                SharkSandFx::Particle* p = Add(VAdd(g.pos, VGet(std::sin(a) * rr, rng.Range(1, 6), std::cos(a) * rr)),
                    VGet(0, rng.Range(15, 45), 0), 0.35f, rng.Range(2, 3.5f), 1.5f, 0.85f, 200, 175, 120, false);
                if (p) { p->gravity = 250; p->wobble = 1.5f; p->phase = rng.Range(0, 6.28f); }
            }
            ++i;
        }
        running = nRings > 0;
        pool.Update(dt);
    }
    void Clear() { pool.Clear(); nRings = 0; running = false; }
private:
    void Grain(VECTOR pos, VECTOR vel, float life, float gravity) {
        SharkSandFx::Particle* p = Add(pos, vel, life * rng.Range(0.7f, 1.0f), rng.Range(2.5f, 4.5f), 2, 0.95f,
            static_cast<int>(rng.Range(185, 215)), static_cast<int>(rng.Range(160, 185)), 115, false);
        if (p) { p->gravity = gravity; p->drag = 0.8f; }
    }
    void Silt(VECTOR pos, VECTOR vel, float life, float s0, float s1) {
        SharkSandFx::Particle* p = Add(pos, vel, life, s0, s1, 0.35f, 150, 135, 100, false);
        if (p) p->drag = 1.5f;
    }
    void Bubble(VECTOR pos) {
        SharkSandFx::Particle* b = Add(VAdd(pos, VGet(rng.Range(-8, 8), 5, rng.Range(-8, 8))), VGet(0, rng.Range(50, 100), 0),
            0.6f, 3, 5, 0.75f, 210, 240, 255, false);
        if (b) { b->tex = 1; b->gravity = -80; b->drag = 2; }
    }
    struct Ring { VECTOR pos; float r, dur, t, acc; };
    static constexpr int MaxRings = 8;
    Ring rings[MaxRings]{};
    int nRings = 0;
};
