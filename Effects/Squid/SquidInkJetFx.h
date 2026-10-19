#pragma once
#include "SquidFxCommon.h"

// SquidInkJet: cheap ink projectile trail. Call EmitNode once per projectile node / frame.
// One dark droplet with a short fading streak per call (~0.3 s). dir: travel direction (any length).
class SquidInkJetEffect : public SquidFx::FxType<512> {
public:
    void EmitNode(VECTOR pos, VECTOR dir) {
        using namespace SquidFx;
        VECTOR d = SafeNorm(dir, VGet(0, 0, 1));
        SquidFx::Particle* p = Add(pos, VScale(d, 30), rng.Range(0.26f, 0.34f),
            rng.Range(5, 7), 1.5f, 0.55f, 22, 18, 42, false);
        if (p) { p->stretch = 0.08f; p->drag = 1.5f; }
    }
};
