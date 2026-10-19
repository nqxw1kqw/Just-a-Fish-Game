#pragma once
#include "SquidFxCommon.h"

// SquidTentacleSlam: tentacle slam splash (~0.3 s): central jet up + ring spray + mist.
class SquidTentacleSlamEffect : public SquidFx::FxType<256> {
public:
    // radius: impact ring radius in game units; <= 0 uses the default (60).
    void EmitImpact(VECTOR pos, float radius = -1.0f) {
        using namespace SquidFx;
        float r = radius > 0 ? radius : 60.0f;
        for (int i = 0; i < 16; ++i) {                                  // jet up
            SquidFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-r, r) * 0.25f, 2, rng.Range(-r, r) * 0.25f)),
                VScale(ConeDir(rng, VGet(0, 1, 0), 0.35f), rng.Range(240, 420)),
                rng.Range(0.26f, 0.34f), rng.Range(5, 8), 2, 0.9f, 225, 240, 250, false);
            if (p) { p->gravity = 700; p->stretch = 0.04f; }
        }
        for (int i = 0; i < 12; ++i) {                                  // ring spray
            float a = 2 * Pi * i / 12 + rng.Range(-0.15f, 0.15f);
            VECTOR out = VGet(std::sin(a), 0, std::cos(a));
            SquidFx::Particle* p = Add(VAdd(pos, VScale(out, r * 0.8f)),
                VAdd(VScale(out, rng.Range(140, 230)), VGet(0, rng.Range(60, 130), 0)),
                rng.Range(0.3f, 0.4f), rng.Range(4, 6), 2, 0.85f, 210, 232, 245, false);
            if (p) p->gravity = 620;
        }
        for (int i = 0; i < 6; ++i) {                                   // mist
            SquidFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-r, r) * 0.5f, rng.Range(4, 14), rng.Range(-r, r) * 0.5f)),
                VGet(rng.Range(-20, 20), rng.Range(10, 30), rng.Range(-20, 20)),
                rng.Range(0.35f, 0.45f), rng.Range(12, 18), rng.Range(26, 36), 0.28f, 190, 215, 230, false);
            if (p) p->drag = 2.0f;
        }
    }
};
