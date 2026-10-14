#pragma once
#include "WhaleFxBase.h"

// 2. Tail Slap: drips from the tail during the raise, then a water fan sprayed in a cone behind the boss.
class WhaleTailSlapFx : public WhaleFx::FxBase<2048> {
public:
    // tailPos: tail tip; bossForward: boss facing (fan goes the opposite way).
    void Start(VECTOR tailPos, VECTOR bossForward, float raiseTime = 0.6f, float coneDeg = 140.0f, float length = 220.0f) {
        tail = tailPos; back = VNorm(VScale(VGet(bossForward.x, 0, bossForward.z), -1));
        raise = raiseTime; cone = coneDeg * WhaleFx::Pi / 180; len = length; time = 0; acc = 0; slapped = false; running = true;
    }
    void SetTail(VECTOR tailPos) { tail = tailPos; }   // follow the animated tail while raising
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running && time < raise) {
            acc += dt * 70;
            while (acc >= 1) {
                acc -= 1;
                VECTOR p = VAdd(tail, VGet(rng.Range(-14, 14), rng.Range(-6, 6), rng.Range(-14, 14)));
                auto* d = Drop(p, VGet(rng.Range(-15, 15), rng.Range(-30, 10), rng.Range(-15, 15)), rng.Range(0.5f, 0.9f), rng.Range(5, 8), rng.Range(4, 6), 0.85f, 980);
                if (d) { d->size1 = d->size0 * 0.7f; }
            }
        } else if (running && !slapped) {
            slapped = true; running = false;
            float baseYaw = Yaw(back);
            for (int i = 0; i < 520; ++i) {
                float a = baseYaw + rng.Range(-0.5f, 0.5f) * cone;
                float edge = std::abs(WrapAngle(a - baseYaw)) / (cone * 0.5f);   // 0 center .. 1 cone edge
                float life = rng.Range(0.45f, 0.7f);
                float reach = len * rng.Range(0.35f, 1.0f) * (1 - 0.25f * edge);
                float sp = reach / life;
                VECTOR d = Dir(a);
                VECTOR origin = VGet(tail.x, floorY + 6, tail.z);
                bool foam = rng.Next() < 0.4f;
                auto* p = foam ? Foam(origin, VGet(d.x * sp, rng.Range(120, 320), d.z * sp), life, rng.Range(8, 14), rng.Range(18, 30), 0.9f, 800)
                               : Drop(origin, VGet(d.x * sp, rng.Range(150, 380), d.z * sp), life, rng.Range(7, 13), rng.Range(10, 16), 0.85f, 900);
                (void)p;
            }
        }
        pool.Update(dt, floorY);
    }
private:
    VECTOR tail{}, back{};
    float raise = 0.6f, cone = 2.44f, len = 220, acc = 0;
    bool slapped = false;
};
