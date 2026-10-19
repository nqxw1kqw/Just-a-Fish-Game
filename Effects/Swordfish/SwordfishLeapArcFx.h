#pragma once
#include "SwordfishFxCommon.h"

// SwordfishLeapArc: droplet arc when leaping out before the sand dive (~0.6 s).
// dir: leap velocity/direction (any length; a long vector is treated as velocity, capped; zero -> fountain up).
class SwordfishLeapArcEffect : public SwordfishFx::FxType<384> {
public:
    void Emit(VECTOR pos, VECTOR dir = VGet(0, 0, 0)) {
        using namespace SwordfishFx;
        float mag = VSize(dir);
        VECTOR vel = mag > 1e-4f ? VScale(SafeNorm(dir), (std::min)(mag, 520.0f) < 60.0f ? 380.0f : (std::min)(mag, 520.0f))
                                 : VGet(0, 340, 0);
        if (mag > 1e-4f && mag <= 520.0f && mag < 60.0f) vel = VAdd(vel, VGet(0, 140, 0));   // unit-ish dirs get arc lift
        for (int i = 0; i < 16; ++i) {
            SwordfishFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-6, 6), rng.Range(0, 8), rng.Range(-6, 6))),
                VAdd(VScale(vel, rng.Range(0.55f, 1.0f)), VGet(rng.Range(-30, 30), rng.Range(0, 50), rng.Range(-30, 30))),
                rng.Range(0.5f, 0.65f), rng.Range(3.5f, 5.5f), 1.2f, 0.9f, 216, 238, 248, false);
            if (p) { p->gravity = 680; p->stretch = 0.045f; }
        }
        for (int i = 0; i < 5; ++i) {                                   // mist at the breach point
            SwordfishFx::Particle* p = Add(VAdd(pos, VGet(rng.Range(-10, 10), rng.Range(2, 10), rng.Range(-10, 10))),
                VGet(rng.Range(-25, 25), rng.Range(15, 40), rng.Range(-25, 25)),
                rng.Range(0.4f, 0.55f), rng.Range(10, 14), rng.Range(20, 28), 0.26f, 196, 220, 234, false);
            if (p) p->drag = 2.0f;
        }
    }
};
