#pragma once
#include "HitFxCommon.h"

// Small puff of bubbles squirting out from the hit point, then floating up.
class HitBubblePuffFx : public HitFx::FxType<1024> {
public:
    void Emit(VECTOR pos, VECTOR dir) {
        using namespace HitFx;
        for (int i = 0; i < 9; ++i) {
            HitFx::Particle* p = pool.Spawn();
            if (!p) return;
            VECTOR d = ConeDir(rng, dir, 0.7f);
            p->pos = pos; p->vel = VAdd(VScale(d, rng.Range(70, 170)), VGet(0, 20, 0));
            p->life = rng.Range(0.45f, 0.8f); p->size0 = rng.Range(3, 6); p->size1 = p->size0 * rng.Range(1.3f, 1.8f);
            p->gravity = -90; p->drag = 4.5f; p->wobble = 1.5f; p->phase = rng.Range(0, 6.28f);
            p->tex = 1; p->additive = false; p->alpha = 0.85f; p->r = 210; p->g = 240; p->b = 255;
        }
        if (HitFx::Particle* m = pool.Spawn()) {   // soft water mist
            m->pos = pos; m->vel = VScale(SafeNorm(dir), 40); m->life = 0.25f; m->size0 = 10; m->size1 = 22;
            m->alpha = 0.35f; m->drag = 5; m->additive = false; m->r = 170; m->g = 220; m->b = 255;
        }
    }
};
