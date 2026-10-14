#pragma once
#include "SharkFxCommon.h"

// Bite Lunge: telegraph = red streak + line of bubbles along the lunge direction (particles only),
// water wake behind the shark while lunging, Bite() = bubble burst + bright shards.
class SharkBiteLungeFx : public SharkFx::FxType<2048> {
public:
    // fwd: lunge direction (flattened; zero -> +Z). telegraph: seconds. length: marked lunge distance.
    void Start(VECTOR pos, VECTOR fwd, float telegraph = 0.7f, float length = 420.0f) {
        origin = shark = lastWake = pos; dir = SharkFx::Flat(fwd); tele = (std::max)(0.05f, telegraph); len = length;
        time = 0; acc = 0; running = true;
    }
    void SetShark(VECTOR pos, VECTOR fwd) { shark = pos; dir = SharkFx::Flat(fwd, dir); }
    void Bite(VECTOR pos) {
        using namespace SharkFx;
        running = false;
        Add(pos, VGet(0, 0, 0), 0.1f, 40, 70, 1.0f, 255, 250, 235, true);                 // flash
        for (int i = 0; i < 18; ++i) {                                                    // bright shards
            SharkFx::Particle* p = Add(pos, VScale(ConeDir(rng, VAdd(dir, VGet(0, 0.3f, 0)), 1.2f), rng.Range(250, 520)), rng.Range(0.1f, 0.18f), 3.5f, 1.2f, 1.0f, 255, 245, 220, true);
            if (p) { p->stretch = 0.045f; p->drag = 6; }
        }
        for (int i = 0; i < 34; ++i) {                                                    // bubble burst
            SharkFx::Particle* p = Add(VAdd(pos, VScale(ConeDir(rng, VGet(0, 1, 0), 1.5f), rng.Range(0, 20))), VScale(ConeDir(rng, VGet(0, 1, 0), 1.6f), rng.Range(60, 200)),
                                       rng.Range(0.5f, 1.0f), rng.Range(4, 9), rng.Range(7, 14), 0.85f, 210, 240, 255, false);
            if (p) { p->tex = 1; p->gravity = -110; p->drag = 3; p->wobble = 2; p->phase = rng.Range(0, 6.28f); }
        }
    }
    void Update(float dt) {
        using namespace SharkFx;
        time += dt;
        if (running && time < tele) {
            float k = time / tele;
            acc += dt * (60 + 260 * k);
            while (acc >= 1) {
                acc -= 1;
                float u = rng.Next();
                VECTOR p = VAdd(VAdd(origin, VScale(dir, 40 + u * len)), VGet(0, 6, 0));
                // red streak core pointing along the lunge
                SharkFx::Particle* s = Add(p, VScale(dir, 120 + 200 * k), rng.Range(0.18f, 0.3f), 4 + 3 * k, 2, 0.5f + 0.5f * k, 255, 40, 40, true);
                if (s) { s->stretch = 0.12f; s->drag = 2; }
                if (rng.Next() < 0.5f) {                                                     // bubble line
                    SharkFx::Particle* b = Add(VAdd(p, VGet(rng.Range(-6, 6), 0, rng.Range(-6, 6))), VGet(0, rng.Range(30, 70), 0), rng.Range(0.35f, 0.6f), 3, 5, 0.7f, 255, 190, 190, false);
                    if (b) { b->tex = 1; b->gravity = -60; b->drag = 2; }
                }
            }
        } else if (running) {                                                                // lunging: wake behind
            VECTOR delta = VSub(shark, lastWake);
            float moved = std::sqrt(VSquareSize(delta));
            VECTOR step = SafeNorm(delta, dir);
            VECTOR side = VGet(step.z, 0, -step.x);
            while (moved >= 9) {
                lastWake = VAdd(lastWake, VScale(step, 9)); moved -= 9;
                for (int s = -1; s <= 1; s += 2) {
                    SharkFx::Particle* f = Add(VAdd(VGet(lastWake.x, 5, lastWake.z), VScale(side, s * 12.0f)), VAdd(VScale(side, s * rng.Range(50, 110)), VGet(0, rng.Range(10, 60), 0)),
                                               rng.Range(0.6f, 1.0f), rng.Range(9, 14), rng.Range(22, 34), 0.75f, 225, 242, 255, false);
                    if (f) f->drag = 2.5f;
                }
                SharkFx::Particle* b = Add(VGet(lastWake.x, 10, lastWake.z), VAdd(VScale(step, -40), VGet(0, 40, 0)), 0.6f, 4, 7, 0.7f, 210, 240, 255, false);
                if (b) { b->tex = 1; b->gravity = -80; b->drag = 2; }
            }
        }
        pool.Update(dt);
    }
private:
    VECTOR origin{}, shark{}, lastWake{}, dir{0, 0, 1};
    float tele = 0.7f, len = 420, time = 0, acc = 0;
};
