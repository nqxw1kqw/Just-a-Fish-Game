#pragma once
#include "TurtleFxCommon.h"

// TurtleRollTrail: cheap churning water/sand behind the rolling shell. Call EmitNode every frame.
// 1 churn puff per call, plus an occasional sand grain / bubble. vel may be zero.
class TurtleRollTrailEffect : public TurtleFx::FxType<512> {
public:
    void EmitNode(VECTOR pos, VECTOR vel) {
        using namespace TurtleFx;
        VECTOR back = VScale(Flat(vel, VGet(0, 0, 1)), -1.0f);
        VECTOR side = VGet(back.z, 0, -back.x);
        VECTOR at = VAdd(pos, VAdd(VScale(side, rng.Range(-14, 14)), VGet(0, rng.Range(0, 6), 0)));
        bool sandy = rng.Next() < 0.45f;
        TurtleFx::Particle* p = Add(at, VAdd(VScale(back, rng.Range(15, 45)), VGet(0, rng.Range(12, 35), 0)),
            rng.Range(0.3f, 0.42f), rng.Range(6, 9), rng.Range(13, 18), 0.32f,
            sandy ? 178 : 215, sandy ? 160 : 232, sandy ? 122 : 240, false);
        if (p) { p->drag = 1.6f; p->wobble = 1.5f; p->phase = rng.Range(0, 6.28f); }
        if (rng.Next() < 0.25f) {
            TurtleFx::Particle* g = Add(at, VAdd(VScale(back, rng.Range(20, 50)), VGet(0, rng.Range(25, 55), 0)),
                0.35f, rng.Range(2.5f, 4), 1.5f, 0.95f, 196, 176, 126, false);
            if (g) g->gravity = 420;
        } else if (rng.Next() < 0.18f) {
            TurtleFx::Particle* b = Add(at, VGet(0, rng.Range(45, 80), 0), 0.4f, 2.5f, 4, 0.7f, 210, 240, 255, false);
            if (b) { b->tex = 1; b->gravity = -70; }
        }
    }
};
