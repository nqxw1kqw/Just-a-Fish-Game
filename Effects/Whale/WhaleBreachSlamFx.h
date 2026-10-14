#pragma once
#include "WhaleFxBase.h"

// 1. Breach Slam: bubbles fizz ever thicker over the landing point, then water column + outward foam.
class WhaleBreachSlamFx : public WhaleFx::FxBase<3072> {
public:
    // pos: landing point on the floor. telegraph: fizz time before impact (Impact() can also be called manually).
    void Start(VECTOR pos, float telegraph = 1.0f, float radius = 200.0f) {
        center = pos; teleTime = telegraph; areaRadius = radius; time = 0; acc = 0; impacted = false; running = true;
    }
    void SetCenter(VECTOR pos) { center = pos; }   // Just-a-Fish: điểm đáp bám theo người chơi
    void Impact() {
        if (impacted) return;
        impacted = true;
        using namespace WhaleFx;
        for (int i = 0; i < 420; ++i) {   // water column
            float a = rng.Range(-Pi, Pi), r = rng.Range(0, 1) * rng.Range(0, 70);
            VECTOR p = VAdd(center, VGet(std::cos(a) * r, rng.Range(0, 20), std::sin(a) * r));
            float out = rng.Range(20, 140);
            auto* d = Drop(p, VGet(std::cos(a) * out, rng.Range(450, 950), std::sin(a) * out), rng.Range(0.8f, 1.5f), rng.Range(10, 22), rng.Range(16, 34), 0.85f, 980);
            if (d && i % 3 == 0) { d->r = FoamR; d->g = FoamG; d->b = FoamB; }
        }
        for (int i = 0; i < 360; ++i) {   // foam ring bursting outward
            float a = 2 * Pi * (i + rng.Next()) / 360;
            float sp = rng.Range(220, 520);
            VECTOR dir = Dir(a);
            if (auto* f = Foam(VAdd(center, VScale(dir, rng.Range(40, 80))), VGet(dir.x * sp, rng.Range(60, 260), dir.z * sp), rng.Range(0.5f, 0.95f), rng.Range(12, 22), rng.Range(26, 40), 0.9f, 900)) f->drag = 1.2f;
        }
        for (int i = 0; i < 40; ++i) {    // mist
            float a = rng.Range(-Pi, Pi), sp = rng.Range(40, 160);
            auto* m = Foam(VAdd(center, VGet(0, rng.Range(40, 220), 0)), VGet(std::cos(a) * sp, rng.Range(20, 120), std::sin(a) * sp), rng.Range(0.9f, 1.6f), rng.Range(60, 90), rng.Range(140, 200), 0.25f, 0);
            if (m) m->drag = 1.5f;
        }
        running = false;
    }
    void Update(float dt) {
        time += dt;
        if (running && !impacted) {
            using namespace WhaleFx;
            float k = std::clamp(time / teleTime, 0.0f, 1.0f);
            acc += dt * (40 + 900 * k * k);              // fizz rate ramps up
            while (acc >= 1) {
                acc -= 1;
                float a = rng.Range(-Pi, Pi), r = areaRadius * std::sqrt(rng.Next()) * (1 - 0.35f * k * rng.Next());
                VECTOR p = VAdd(center, VGet(std::cos(a) * r, rng.Range(0, 8), std::sin(a) * r));
                Bubble(p, VGet(0, rng.Range(40, 120) * (1 + k), 0), rng.Range(0.35f, 0.8f), rng.Range(5, 9) * (1 + k), rng.Range(9, 16) * (1 + k), 0.55f + 0.4f * k);
                if (rng.Next() < 0.25f * k)               // surface fizz droplets
                    Foam(VAdd(p, VGet(0, 10, 0)), VGet(rng.Range(-40, 40), rng.Range(80, 200), rng.Range(-40, 40)), rng.Range(0.25f, 0.45f), 5, 8, 0.7f, 900);
            }
            if (time >= teleTime) Impact();
        }
        pool.Update(dt, floorY);
    }
private:
    VECTOR center{};
    float teleTime = 1, areaRadius = 200, acc = 0;
    bool impacted = false;
};
