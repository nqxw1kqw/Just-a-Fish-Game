#pragma once
#include "TurtleFxCommon.h"

// TurtleWithdrawDust: tuck-into-shell puff (~0.4 s): low dust ring squeezed out around the shell rim.
class TurtleWithdrawDustEffect : public TurtleFx::FxType<256> {
public:
    void Emit(VECTOR pos) {
        using namespace TurtleFx;
        for (int i = 0; i < 12; ++i) {
            float a = 2 * Pi * i / 12 + rng.Range(-0.2f, 0.2f);
            VECTOR out = VGet(std::sin(a), 0, std::cos(a));
            TurtleFx::Particle* p = Add(VAdd(pos, VAdd(VScale(out, rng.Range(28, 38)), VGet(0, rng.Range(2, 8), 0))),
                VAdd(VScale(out, rng.Range(45, 85)), VGet(0, rng.Range(8, 22), 0)),
                rng.Range(0.35f, 0.45f), rng.Range(10, 14), rng.Range(20, 28), 0.32f, 176, 162, 126, false);
            if (p) p->drag = 2.0f;
        }
        for (int i = 0; i < 4; ++i) {
            TurtleFx::Particle* b = Add(VAdd(pos, VGet(rng.Range(-16, 16), rng.Range(8, 18), rng.Range(-16, 16))),
                VGet(0, rng.Range(40, 70), 0), 0.4f, 2.5f, 4, 0.7f, 210, 240, 255, false);
            if (b) { b->tex = 1; b->gravity = -70; }
        }
    }
};
