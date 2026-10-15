#pragma once
#include "SharkFxCommon.h"

// Circle Fin: wake wave + foam behind the dorsal fin while the shark circles the player.
// Call SetFin() every frame; Stop() lets remaining particles fade out.
class SharkCircleFinFx : public SharkFx::FxType<2048> {
public:
    void Start() { running = true; hasLast = false; }
    void SetFin(VECTOR pos, VECTOR vel) {
        using namespace SharkFx;
        if (!running) return;
        if (!hasLast) { last = pos; hasLast = true; return; }
        VECTOR fwd = Flat(vel, Flat(VSub(pos, last), VGet(0, 0, 1)));
        VECTOR side = VGet(fwd.z, 0, -fwd.x);
        float speed = std::sqrt(vel.x * vel.x + vel.z * vel.z);
        VECTOR delta = VSub(pos, last);
        float moved = std::sqrt(VSquareSize(delta));
        VECTOR step = SafeNorm(delta, fwd);
        while (moved >= 8) {
            last = VAdd(last, VScale(step, 8)); moved -= 8;
            for (int s = -1; s <= 1; s += 2) {                                   // V wake spreading sideways
                SharkFx::Particle* w = Add(VAdd(VGet(last.x, 3, last.z), VScale(side, s * 6.0f)), VAdd(VScale(side, s * rng.Range(40, 90)), VScale(fwd, -speed * 0.1f)),
                                           rng.Range(0.9f, 1.4f), rng.Range(8, 12), rng.Range(24, 36), 0.6f, 200, 232, 255, false);
                if (w) w->drag = 1.8f;
            }
            SharkFx::Particle* f = Add(VAdd(VGet(last.x, 4, last.z), VScale(side, rng.Range(-4, 4))), VGet(0, 0, 0), rng.Range(1.0f, 1.6f), 7, 18, 0.8f, 235, 248, 255, false);
            if (f) f->drag = 3;
        }
        // spray off the fin itself
        if (rng.Next() < 0.6f) {
            SharkFx::Particle* d = Add(VAdd(pos, VGet(0, rng.Range(5, 25), 0)), VAdd(VScale(fwd, speed * 0.3f), VAdd(VScale(side, rng.Range(-60, 60)), VGet(0, rng.Range(60, 140), 0))),
                                       rng.Range(0.3f, 0.5f), 3, 5, 0.85f, 190, 230, 255, false);
            if (d) d->gravity = 600;
        }
    }
    void Stop() { running = false; hasLast = false; }
    void Update(float dt) { pool.Update(dt); }
private:
    VECTOR last{};
    bool hasLast = false;
};
