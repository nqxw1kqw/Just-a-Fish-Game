#pragma once
#include "SharkFxCommon.h"

// Vortex: swirling water + bubbles spiralling into the center; spawn tapers off at the end so it fades.
class SharkVortexFx : public SharkFx::FxType<2048> {
public:
    void Start(VECTOR c, float radius = 240.0f, float duration = 3.0f) {
        center = c; rad = (std::max)(20.0f, radius); dur = (std::max)(0.1f, duration); time = 0; acc = 0; running = true;
    }
    void Update(float dt) {
        using namespace SharkFx;
        time += dt;
        if (running && time < dur) {
            float ramp = (std::min)(1.0f, time / 0.4f) * (std::min)(1.0f, (dur - time) / 0.6f);
            acc += dt * 480 * ramp;
            while (acc >= 1) {
                acc -= 1;
                bool bubble = rng.Next() < 0.4f;
                float r0 = rad * (bubble ? rng.Range(0.6f, 1.0f) : std::sqrt(rng.Next()));
                SharkFx::Particle* p = Add(VGet(center.x, center.y + rng.Range(2, bubble ? 20.0f : 8.0f), center.z), VGet(0, 0, 0),
                                           rng.Range(1.0f, 1.8f), bubble ? rng.Range(4, 8) : rng.Range(10, 18), bubble ? rng.Range(3, 5) : rng.Range(18, 28),
                                           bubble ? 0.85f : 0.45f, bubble ? 210 : 120, bubble ? 240 : 200, 255, false);
                if (!p) break;
                p->orbit = true; p->orbitCenter = center; p->orbitRadius = r0; p->orbitAngle = rng.Range(-Pi, Pi);
                p->orbitSpeed = rng.Range(1.6f, 2.4f);
                p->inwardSpeed = bubble ? rng.Range(90, 160) : rng.Range(15, 45);
                p->riseSpeed = bubble ? rng.Range(5, 25) : 0;
                if (bubble) p->tex = 1; else if (rng.Next() < 0.3f) { p->r = 235; p->g = 248; p->b = 255; p->alpha = 0.6f; }   // foam streaks
            }
        } else running = false;
        pool.Update(dt);
    }
private:
    VECTOR center{};
    float rad = 240, dur = 3, time = 0, acc = 0;
};
