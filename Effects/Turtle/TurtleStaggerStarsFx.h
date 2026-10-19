#pragma once
#include "TurtleFxCommon.h"

// TurtleStaggerStars: gold stars swirling over the head while staggered.
// Start(headPos, duration): duration <= 0 means "until Stop()" (controllable).
// Typical stagger is 1-2 s. SetHead() follows a moving head; Stop() ends emission (stars fade < 0.7 s).
class TurtleStaggerStarsEffect : public TurtleFx::FxType<256> {
public:
    void Start(VECTOR headPos, float duration = -1.0f) {
        head = headPos; dur = duration; elapsed = 0; acc = 0; running = true;
    }
    void SetHead(VECTOR pos) { head = pos; }
    void Stop() { running = false; }
    void Update(float dt) {
        using namespace TurtleFx;
        if (running) {
            elapsed += dt;
            if (dur > 0 && elapsed >= dur) running = false;
            else {
                acc += dt * 26.0f;                                      // ~26 stars/s
                while (acc >= 1.0f) {
                    acc -= 1.0f;
                    float a = rng.Range(-Pi, Pi), r = rng.Range(17, 30);
                    TurtleFx::Particle* p = Add(VAdd(head, VGet(std::sin(a) * r, rng.Range(16, 26), std::cos(a) * r)),
                        VGet(0, 0, 0), rng.Range(0.55f, 0.75f), rng.Range(5, 7), rng.Range(2, 3), 0.95f, 255, 208, 92, true);
                    if (!p) break;
                    p->orbit = true; p->orbitCenter = VAdd(head, VGet(0, 20, 0)); p->orbitRadius = r; p->orbitAngle = a;
                    p->orbitSpeed = rng.Range(4.5f, 6.0f); p->inwardSpeed = 0; p->riseSpeed = rng.Range(-2, 3);
                    p->spin = rng.Range(9, 14); p->phase = rng.Range(0, 6.28f);
                }
            }
        }
        pool.Update(dt);
    }
    void Clear() { pool.Clear(); running = false; acc = 0; elapsed = 0; }
private:
    VECTOR head{};
    float dur = -1, elapsed = 0, acc = 0;
};
