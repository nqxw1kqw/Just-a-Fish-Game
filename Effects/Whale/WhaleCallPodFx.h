#pragma once
#include "WhaleFxBase.h"

// 6. Call Pod: a bubble plume spouting upward (default 0.8s) with spout droplets at the top.
class WhaleCallPodFx : public WhaleFx::FxBase<2048> {
public:
    void Start(VECTOR pos, float duration = 0.8f, float height = 320.0f) { base = pos; dur = duration; h = height; time = 0; acc = 0; running = true; }
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running && time < dur) {
            acc += dt * 700;
            while (acc >= 1) {
                acc -= 1;
                float a = rng.Range(-Pi, Pi), r = rng.Range(0, 24);
                VECTOR p = VAdd(base, VGet(std::cos(a) * r, rng.Range(0, 20), std::sin(a) * r));
                float up = h * rng.Range(1.1f, 1.8f);
                if (rng.Next() < 0.6f) Bubble(p, VGet(std::cos(a) * rng.Range(10, 60), up, std::sin(a) * rng.Range(10, 60)), rng.Range(0.5f, 0.9f), rng.Range(7, 12), rng.Range(14, 24), 0.8f);
                else Drop(p, VGet(std::cos(a) * rng.Range(40, 140), up * 1.2f, std::sin(a) * rng.Range(40, 140)), rng.Range(0.7f, 1.1f), rng.Range(8, 13), rng.Range(10, 18), 0.85f, 900);
            }
        } else running = false;
        pool.Update(dt, floorY);
    }
private:
    VECTOR base{};
    float dur = 0.8f, h = 320, acc = 0;
};
