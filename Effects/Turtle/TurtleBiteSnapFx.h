#pragma once
#include "TurtleFxCommon.h"

// TurtleBiteSnap: lunge bite splash (~0.4 s): forward droplet cone + mist in the bite direction.
class TurtleBiteSnapEffect : public TurtleFx::FxType<256> {
public:
    // dir: bite/lunge direction (any length; zero -> forward +Z per game convention fallback).
    void Emit(VECTOR pos, VECTOR dir) {
        using namespace TurtleFx;
        VECTOR d = Flat(dir, VGet(0, 0, 1));
        VECTOR axis = SafeNorm(VAdd(VScale(d, 1.0f), VGet(0, 0.35f, 0)));
        for (int i = 0; i < 12; ++i) {
            TurtleFx::Particle* p = Add(pos, VScale(ConeDir(rng, axis, 0.55f), rng.Range(150, 300)),
                rng.Range(0.32f, 0.42f), rng.Range(4, 6.5f), 1.5f, 0.9f, 220, 240, 248, false);
            if (p) { p->gravity = 520; p->stretch = 0.05f; }
        }
        for (int i = 0; i < 4; ++i) {
            TurtleFx::Particle* p = Add(VAdd(pos, VScale(d, 14)), VScale(ConeDir(rng, axis, 0.9f), rng.Range(40, 80)),
                0.4f, rng.Range(9, 12), rng.Range(18, 24), 0.26f, 195, 220, 232, false);
            if (p) p->drag = 2.0f;
        }
    }
};
