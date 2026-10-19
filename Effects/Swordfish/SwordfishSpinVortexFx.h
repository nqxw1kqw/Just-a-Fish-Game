#pragma once
#include "SwordfishFxCommon.h"

// SwordfishSpinVortex: spinning water ring/vortex around the body.
// Start(bodyPos, duration): duration <= 0 means "until Stop()". SetBody() follows the boss.
class SwordfishSpinVortexEffect : public SwordfishFx::FxType<512> {
public:
    void Start(VECTOR bodyPos, float duration = -1.0f) {
        body = bodyPos; dur = duration; elapsed = 0; acc = 0; running = true;
    }
    void SetBody(VECTOR pos) { body = pos; }
    void Stop() { running = false; }
    void Update(float dt) {
        using namespace SwordfishFx;
        if (running) {
            elapsed += dt;
            if (dur > 0 && elapsed >= dur) running = false;
            else {
                acc += dt * 70.0f;                                      // ~70 foam bits/s
                while (acc >= 1.0f) {
                    acc -= 1.0f;
                    float a = rng.Range(-Pi, Pi), r = rng.Range(42, 72);
                    bool bub = rng.Next() < 0.15f;
                    SwordfishFx::Particle* p = Add(VAdd(body, VGet(std::sin(a) * r, rng.Range(-8, 14), std::cos(a) * r)),
                        VGet(0, 0, 0), rng.Range(0.5f, 0.7f), bub ? 3.0f : rng.Range(4.5f, 6.5f), bub ? 4.5f : 1.5f,
                        bub ? 0.7f : 0.75f, 208, 234, 246, !bub);
                    if (!p) break;
                    p->orbit = true; p->orbitCenter = body; p->orbitRadius = r; p->orbitAngle = a;
                    p->orbitSpeed = 5.5f; p->inwardSpeed = 6; p->riseSpeed = rng.Range(4, 16);
                    if (bub) p->tex = 1;
                }
            }
        }
        pool.Update(dt);
    }
    void Clear() { pool.Clear(); running = false; acc = 0; elapsed = 0; }
private:
    VECTOR body{};
    float dur = -1, elapsed = 0, acc = 0;
};
