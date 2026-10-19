#pragma once
#include "SquidFxCommon.h"

// SquidEnrageAura (optional): red ink wisps orbiting the boss while enraged.
// Start() + SetBoss(pos) each frame; Stop() stops emitting (wisps fade out in < 1 s).
class SquidEnrageAuraEffect : public SquidFx::FxType<256> {
public:
    void Start() { running = true; }
    void SetBoss(VECTOR pos) { boss = pos; }
    void Stop() { running = false; }
    void Update(float dt) {
        using namespace SquidFx;
        if (running) {
            acc += dt * 36.0f;                                          // ~36 wisps/s
            while (acc >= 1.0f) {
                acc -= 1.0f;
                float a = rng.Range(-Pi, Pi), r = rng.Range(38, 62);
                bool ember = (rng.Next() < 0.25f);
                SquidFx::Particle* p = Add(VAdd(boss, VGet(std::sin(a) * r, rng.Range(0, 18), std::cos(a) * r)),
                    VGet(0, rng.Range(22, 42), 0), rng.Range(0.7f, 0.95f),
                    ember ? 4.0f : rng.Range(9, 13), ember ? 1.5f : rng.Range(18, 26), ember ? 0.9f : 0.42f,
                    ember ? 255 : 128, ember ? 92 : 22, ember ? 48 : 34, ember);
                if (!p) break;
                p->orbit = true; p->orbitCenter = boss; p->orbitRadius = r; p->orbitAngle = a;
                p->orbitSpeed = rng.Range(1.6f, 2.4f); p->inwardSpeed = 4; p->riseSpeed = rng.Range(22, 42);
                p->drag = 0;
            }
        }
        pool.Update(dt);
    }
    void Clear() { pool.Clear(); running = false; acc = 0; }
private:
    VECTOR boss{};
    float acc = 0;
};
