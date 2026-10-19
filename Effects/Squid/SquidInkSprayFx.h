#pragma once
#include "SquidFxCommon.h"

// SquidInkSpray: dark ink cloud burst (~0.8 s). Particles expand, drift and fade.
// dir: spray direction (any length; zero = omnidirectional cloud).
class SquidInkSprayEffect : public SquidFx::FxType<384> {
public:
    void Emit(VECTOR pos, VECTOR dir = VGet(0, 0, 0)) {
        using namespace SquidFx;
        bool hasDir = VSquareSize(dir) > 1e-6f;
        VECTOR axis = hasDir ? SafeNorm(dir) : VGet(0, 1, 0);
        for (int i = 0; i < 22; ++i) {
            int shade = static_cast<int>(rng.Range(0, 3));
            int R = shade == 0 ? 16 : shade == 1 ? 32 : 48;
            int G = shade == 0 ? 14 : shade == 1 ? 24 : 34;
            int B = shade == 0 ? 34 : shade == 1 ? 54 : 72;
            VECTOR vel = hasDir ? VScale(ConeDir(rng, axis, 0.55f), rng.Range(90, 210))
                                : VScale(ConeDir(rng, axis, Pi), rng.Range(25, 80));
            SquidFx::Particle* p = Add(VAdd(pos, VScale(ConeDir(rng, axis, 1.2f), rng.Range(0, 8))), vel,
                rng.Range(0.65f, 0.85f), rng.Range(10, 16), rng.Range(30, 44), 0.5f, R, G, B, false);
            if (p) { p->drag = 2.6f; p->wobble = 2.0f; p->phase = rng.Range(0, 6.28f); }
        }
        for (int i = 0; i < 4; ++i) {                                   // faint grey-violet edge wisps so the cloud reads on dark water
            SquidFx::Particle* p = Add(pos, VScale(ConeDir(rng, axis, hasDir ? 0.8f : Pi), rng.Range(40, 90)),
                rng.Range(0.5f, 0.7f), rng.Range(8, 12), rng.Range(22, 30), 0.22f, 105, 95, 135, false);
            if (p) p->drag = 2.2f;
        }
    }
};
