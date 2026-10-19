#pragma once
#include "TurtleFxCommon.h"

// TurtleShellBreak: shell crack burst (~0.8 s): hard shell shards + dust + a few bubbles.
class TurtleShellBreakEffect : public TurtleFx::FxType<384> {
public:
    // radius: debris spread radius in game units; <= 0 uses the default (55).
    void Emit(VECTOR pos, float radius = -1.0f) {
        using namespace TurtleFx;
        float r = radius > 0 ? radius : 55.0f;
        for (int i = 0; i < 18; ++i) {                                  // shell shards
            bool rim = (i % 3) == 0;
            TurtleFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-r, r) * 0.5f, rng.Range(0, 16), rng.Range(-r, r) * 0.5f)),
                VScale(ConeDir(rng, VGet(0, 1, 0), 1.0f), rng.Range(90, 240)),
                rng.Range(0.65f, 0.85f), rng.Range(5, 8), 2.5f, 1.0f,
                rim ? 122 : 74, rim ? 138 : 96, rim ? 86 : 62, false);
            if (p) { p->gravity = 520; p->spin = rng.Range(6, 12); p->phase = rng.Range(0, 6.28f); }
        }
        for (int i = 0; i < 10; ++i) {                                  // dust
            TurtleFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-r, r) * 0.7f, rng.Range(2, 12), rng.Range(-r, r) * 0.7f)),
                VScale(ConeDir(rng, VGet(0, 1, 0), 1.3f), rng.Range(25, 70)),
                rng.Range(0.6f, 0.85f), rng.Range(12, 17), rng.Range(26, 34), 0.3f, 168, 152, 118, false);
            if (p) p->drag = 1.8f;
        }
        for (int i = 0; i < 6; ++i) {                                   // bubbles knocked loose
            TurtleFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-r, r) * 0.4f, 8, rng.Range(-r, r) * 0.4f)),
                VGet(0, rng.Range(50, 95), 0), rng.Range(0.5f, 0.7f), 3, 5, 0.75f, 210, 240, 255, false);
            if (p) { p->tex = 1; p->gravity = -80; p->drag = 1.5f; }
        }
    }
};
