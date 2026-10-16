#pragma once
#include "EnemyTrailFxCommon.h"

// ChargeDashTrail: call every frame per charging small shark. 1 streak + occasional bubble per call.
// One pool is meant to be shared by all sharks (dropped silently when full).
class ChargeDashTrailEffect : public EnemyTrailFx::FxType<1024> {
public:
    void Emit(VECTOR pos, VECTOR dashDir) {
        using namespace EnemyTrailFx;
        VECTOR d = Flat(dashDir, VGet(0, 0, 1));
        VECTOR side = VGet(d.z, 0, -d.x);
        EnemyTrailFx::Particle* s = Add(VAdd(pos, VScale(side, rng.Range(-4, 4))), VScale(d, -rng.Range(40, 80)),
            0.25f, 4.5f, 1.5f, 0.85f, 255, 200, 140, true);
        if (s) { s->stretch = 0.12f; s->tint = true; s->r1 = 255; s->g1 = 245; s->b1 = 230; }
        if (rng.Next() < 0.15f) {
            EnemyTrailFx::Particle* b = Add(VAdd(pos, VScale(side, rng.Range(-6, 6))), VAdd(VScale(d, -20), VGet(0, 25, 0)),
                0.3f, 2.5f, 3.5f, 0.7f, 210, 240, 255, false);
            if (b) { b->tex = 1; b->gravity = -60; b->drag = 2; }
        }
    }
};
