#pragma once
#include "WhaleFxBase.h"

// 7a. Boss stunned: orbiting bubble ring + twinkling stars above the head. Call SetHead() each frame.
class WhaleStunFx : public WhaleFx::FxBase<1024> {
public:
    void Start(VECTOR headPos, float duration = 2.5f, float ringRadius = 45.0f, float heightAbove = 50.0f) {
        head = headPos; dur = duration; ring = ringRadius; above = heightAbove; time = 0; acc = 0; running = true;
    }
    void SetHead(VECTOR headPos) { head = headPos; }
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running && time < dur) {
            acc += dt * 60;
            while (acc >= 1) {
                acc -= 1;
                for (int i = 0; i < 6; ++i) {                  // 6 orbiting bubbles leave short trails
                    float a = time * 3.2f + i * 2 * Pi / 6;
                    VECTOR p = VAdd(head, VGet(std::cos(a) * ring, above + 6 * std::sin(a * 2), std::sin(a) * ring));
                    Bubble(p, VGet(0, 8, 0), 0.25f, 11, 9, 0.85f);
                }
                for (int i = 0; i < 3; ++i) {                  // 3 stars
                    float a = -time * 2.4f + i * 2 * Pi / 3;
                    VECTOR p = VAdd(head, VGet(std::cos(a) * ring * 0.7f, above + 18, std::sin(a) * ring * 0.7f));
                    auto* s = Drop(p, VGet(0, 0, 0), 0.18f, 18 + 6 * std::sin(time * 20 + i), 6, 1.0f, 0);
                    if (s) { s->additive = true; s->r = 255; s->g = 225; s->b = 90; }
                }
            }
        } else running = false;
        pool.Update(dt);
    }
private:
    VECTOR head{};
    float dur = 2.5f, ring = 45, above = 50, acc = 0;
};

// 7b. Player slowed: sticky bubble/water aura clinging around the fish. Call SetTarget() each frame.
class PlayerSlowFx : public WhaleFx::FxBase<1024> {
public:
    void Start(VECTOR targetPos, float duration = 2.0f, float radius = 26.0f) { target = targetPos; dur = duration; rad = radius; time = 0; acc = 0; running = true; }
    void SetTarget(VECTOR targetPos) { target = targetPos; }
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running && time < dur) {
            acc += dt * 90;
            while (acc >= 1) {
                acc -= 1;
                float a = rng.Range(-Pi, Pi), y = rng.Range(-6, 22);
                VECTOR p = VAdd(target, VGet(std::cos(a) * rad, y, std::sin(a) * rad));
                auto* b = Bubble(p, VGet(0, rng.Range(4, 18), 0), rng.Range(0.5f, 0.9f), rng.Range(5, 9), rng.Range(8, 12), 0.8f);
                if (b) { b->r = 150; b->g = 210; b->b = 255; b->wobble = 2; }
                if (rng.Next() < 0.25f)                       // goopy drips
                    Drop(VAdd(target, VGet(rng.Range(-rad, rad) * 0.6f, 8, rng.Range(-rad, rad) * 0.6f)), VGet(0, -10, 0), 0.6f, 6, 4, 0.7f, 300);
            }
            acc2 += dt * 20;                                   // soft glow shell
            while (acc2 >= 1) { acc2 -= 1; auto* g = Drop(VAdd(target, VGet(0, 8, 0)), VGet(0, 0, 0), 0.3f, rad * 2.6f, rad * 2.8f, 0.18f, 0); if (g) { g->r = 90; g->g = 170; g->b = 255; } }
        } else running = false;
        pool.Update(dt);
    }
private:
    VECTOR target{};
    float dur = 2, rad = 26, acc = 0, acc2 = 0;
};
