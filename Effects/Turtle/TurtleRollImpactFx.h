#pragma once
#include "TurtleFxCommon.h"

// TurtleRollImpact: rolling shell hits a wall (~0.5 s): spray cone along the surface normal + dust.
class TurtleRollImpactEffect : public TurtleFx::FxType<256> {
public:
    // normal: wall surface normal (any length; zero/invalid -> straight up).
    void Emit(VECTOR pos, VECTOR normal = VGet(0, 0, 0)) {
        using namespace TurtleFx;
        VECTOR n = SafeNorm(normal, VGet(0, 1, 0));
        for (int i = 0; i < 14; ++i) {                                  // spray
            TurtleFx::Particle* p = Add(pos, VScale(ConeDir(rng, n, 0.7f), rng.Range(120, 260)),
                rng.Range(0.4f, 0.55f), rng.Range(4, 7), 2, 0.88f, 218, 238, 246, false);
            if (p) { p->gravity = 560; p->stretch = 0.04f; }
        }
        for (int i = 0; i < 6; ++i) {                                   // dust
            TurtleFx::Particle* p = Add(VAdd(pos, VGet(0, rng.Range(2, 10), 0)),
                VScale(ConeDir(rng, n, 1.2f), rng.Range(25, 60)),
                rng.Range(0.45f, 0.6f), rng.Range(11, 15), rng.Range(22, 30), 0.3f, 172, 156, 120, false);
            if (p) p->drag = 1.8f;
        }
    }
};
