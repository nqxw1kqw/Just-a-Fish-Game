#pragma once
#include "SwordfishFxCommon.h"

// SwordfishDashStreak: cheap speed trail. Call EmitNode every frame while dashing.
// 1 streak per call plus a bill glint every 4th call. vel: travel velocity/direction (any length, may be zero).
class SwordfishDashStreakEffect : public SwordfishFx::FxType<512> {
public:
    void EmitNode(VECTOR pos, VECTOR vel) {
        using namespace SwordfishFx;
        float speed = VSize(vel);
        VECTOR d = SafeNorm(vel, VGet(0, 0, 1));
        SwordfishFx::Particle* p = Add(pos, VScale(d, (std::max)(60.0f, (std::min)(speed, 600.0f)) * 0.35f),
            rng.Range(0.24f, 0.32f), rng.Range(4, 5.5f), 1.2f, 0.8f, 205, 232, 244, true);
        if (p) { p->stretch = 0.10f; p->tint = true; p->r1 = 240; p->g1 = 248; p->b1 = 252; }
        if ((++calls % 4) == 0) {                                       // bill glint
            SwordfishFx::Particle* g = Add(VAdd(pos, VGet(0, rng.Range(2, 6), 0)), VScale(d, 40),
                0.22f, rng.Range(6, 8), 1.5f, 1.0f, 255, 252, 235, true);
            if (g) { g->spin = 16; g->phase = rng.Range(0, 6.28f); }
        }
    }
private:
    int calls = 0;
};
