#pragma once
#include "DodgeFxCommon.h"

// TsunamiDodge: mint/white water spray + short "perfect" flash (0.5s).
class TsunamiDodgeEffect : public DodgeFx::FxType<256> {
public:
    // dir: dodge direction (flattened; zero -> spray straight up/around).
    void Emit(VECTOR pos, VECTOR dir = VGet(0, 0, 0)) {
        using namespace DodgeFx;
        VECTOR flat = VGet(dir.x, 0, dir.z);
        bool hasDir = VSquareSize(flat) > 1e-6f;
        VECTOR axis = hasDir ? SafeNorm(VAdd(VScale(SafeNorm(flat), -0.7f), VGet(0, 0.7f, 0))) : VGet(0, 1, 0);
        if (DodgeFx::Particle* f = Add(VAdd(pos, VGet(0, 12, 0)), VGet(0, 0, 0), 0.14f, 26, 48, 0.9f, 230, 255, 245, true)) (void)f;
        for (int i = 0; i < 6; ++i) {                                          // "perfect" glints
            DodgeFx::Particle* s = Add(VAdd(pos, VGet(0, 12, 0)), VScale(ConeDir(rng, VGet(0, 1, 0), Pi), rng.Range(120, 200)),
                0.18f, 2.5f, 1, 1, 210, 255, 240, true);
            if (s) { s->stretch = 0.05f; s->drag = 6; }
        }
        for (int i = 0; i < 14; ++i) {                                         // water spray
            bool white = (i % 3) == 0;
            DodgeFx::Particle* p = Add(VAdd(pos, VGet(0, 5, 0)), VScale(ConeDir(rng, axis, hasDir ? 0.6f : 1.2f), rng.Range(90, 200)),
                rng.Range(0.35f, 0.5f), rng.Range(4, 7), rng.Range(7, 11), 0.85f, white ? 235 : 150, white ? 250 : 245, white ? 255 : 215, false);
            if (p) { p->gravity = 420; p->drag = 1.5f; }
        }
    }
};
