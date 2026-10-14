#pragma once
#include "SharkFxCommon.h"

// Rage: red aura + boiling bubbles around the shark while enraged. SetShark() every frame; Stop() fades out.
class SharkRageFx : public SharkFx::FxType<1024> {
public:
    void Start(float bodyRadius = 50.0f) { body = (std::max)(10.0f, bodyRadius); running = true; acc = 0; accAura = 0; }
    void SetShark(VECTOR pos) { shark = pos; }
    void Stop() { running = false; }
    void Update(float dt) {
        using namespace SharkFx;
        time += dt;
        if (running) {
            accAura += dt * 30;
            while (accAura >= 1) {                                          // pulsing red aura shells
                accAura -= 1;
                float pulse = 0.75f + 0.25f * std::sin(time * 9);
                SharkFx::Particle* a = Add(VAdd(shark, VGet(rng.Range(-8, 8), body * 0.4f + rng.Range(-6, 6), rng.Range(-8, 8))), VGet(0, 0, 0),
                                           0.22f, body * 2.2f * pulse, body * 2.8f * pulse, 0.22f, 255, 40, 30, true);
                (void)a;
                SharkFx::Particle* e = Add(VAdd(shark, VScale(ConeDir(rng, VGet(0, 1, 0), 1.5f), body * rng.Range(0.6f, 1.0f))), VGet(0, rng.Range(20, 60), 0),
                                           0.35f, 6, 2, 0.8f, 255, 90, 60, true);                                  // embers
                if (e) e->drag = 2;
            }
            acc += dt * 110;
            while (acc >= 1) {                                              // boiling bubbles
                acc -= 1;
                float ang = rng.Range(-Pi, Pi), r = body * rng.Range(0.5f, 1.1f);
                SharkFx::Particle* b = Add(VAdd(shark, VGet(std::sin(ang) * r, rng.Range(0, body * 0.5f), std::cos(ang) * r)), VGet(rng.Range(-20, 20), rng.Range(70, 160), rng.Range(-20, 20)),
                                           rng.Range(0.4f, 0.8f), rng.Range(3, 7), rng.Range(7, 12), 0.85f, 255, 200, 200, false);
                if (b) { b->tex = 1; b->gravity = -140; b->drag = 1.5f; b->wobble = 2.5f; b->phase = rng.Range(0, 6.28f); b->tint = true; b->r1 = 255; b->g1 = 120; b->b1 = 120; }
            }
        }
        pool.Update(dt);
    }
private:
    VECTOR shark{};
    float body = 50, time = 0, acc = 0, accAura = 0;
};
