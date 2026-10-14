#pragma once
#include "WhaleFxBase.h"

// 4. Inhale: water streams + droplets sucked into the mouth along a forward cone.
class WhaleInhaleFx : public WhaleFx::FxBase<2048> {
public:
    void Start(VECTOR mouthPos, VECTOR forward, float duration = 2.0f, float range = 420.0f, float coneDeg = 60.0f) {
        mouth = mouthPos; fwd = VNorm(VGet(forward.x, 0, forward.z)); dur = duration; reach = range; cone = coneDeg * WhaleFx::Pi / 180;
        time = 0; acc = 0; running = true;
    }
    void SetMouth(VECTOR mouthPos, VECTOR forward) { mouth = mouthPos; fwd = VNorm(VGet(forward.x, 0, forward.z)); }
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running && time < dur) {
            float ramp = (std::min)(1.0f, time / 0.3f) * (std::min)(1.0f, (dur - time) / 0.25f);
            acc += dt * 420 * ramp;
            float baseYaw = Yaw(fwd);
            while (acc >= 1) {
                acc -= 1;
                bool stream = rng.Next() < 0.6f;
                // Streams use 7 fixed lanes so they read as lines; droplets fill the cone randomly.
                float a = stream ? baseYaw + (std::floor(rng.Range(0, 7)) / 6 - 0.5f) * cone + rng.Range(-0.03f, 0.03f)
                                 : baseYaw + rng.Range(-0.5f, 0.5f) * cone;
                float dist = reach * rng.Range(0.45f, 1.0f);
                VECTOR p = VAdd(VGet(mouth.x, floorY + rng.Range(4, 40), mouth.z), VScale(Dir(a), dist));
                VECTOR toMouth = VNorm(VSub(mouth, p));
                WhaleFx::Particle* q = stream ? Drop(p, VScale(toMouth, 120), 2.0f, 9, 4, 0.75f, 0)
                                     : Foam(p, VScale(toMouth, 60), 2.0f, rng.Range(5, 9), 3, 0.8f, 0);
                if (q) { q->target = mouth; q->pull = 1400; q->killRadius = 18; q->drag = 0.8f; }
            }
        } else running = false;
        pool.Update(dt, floorY);
    }
private:
    VECTOR mouth{}, fwd{1, 0, 0};
    float dur = 2, reach = 420, cone = 1.05f, acc = 0;
};
