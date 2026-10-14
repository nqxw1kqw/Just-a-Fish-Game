#pragma once
#include "HitFxCommon.h"

// Short streak sparks from a hit point (~0.15s). White/yellow; crit = bigger, more, orange.
class HitSparkFx : public HitFx::FxType<1024> {
public:
    // dir: hit direction (sparks fly mostly along it). Spawns are dropped when the pool is full.
    void Emit(VECTOR pos, VECTOR dir, bool crit = false) {
        using namespace HitFx;
        int n = crit ? 22 : 10;
        float speedMin = crit ? 260.0f : 180.0f, speedMax = crit ? 560.0f : 380.0f;
        for (int i = 0; i < n; ++i) {
            HitFx::Particle* p = pool.Spawn();
            if (!p) return;
            VECTOR d = ConeDir(rng, dir, crit ? 1.1f : 0.85f);
            float sp = rng.Range(speedMin, speedMax);
            p->pos = pos; p->vel = VScale(d, sp);
            p->life = rng.Range(0.10f, 0.17f) * (crit ? 1.25f : 1.0f);
            p->size0 = crit ? 3.2f : 2.2f; p->size1 = p->size0 * 0.4f;
            p->stretch = rng.Range(0.035f, 0.06f); p->drag = 6; p->gravity = 300;
            if (crit) { p->r = 255; p->g = static_cast<unsigned char>(rng.Range(120, 180)); p->b = 40; }
            else { bool white = rng.Next() < 0.5f; p->r = 255; p->g = white ? 250 : 225; p->b = white ? 235 : 110; }
        }
        // core flash
        if (HitFx::Particle* f = pool.Spawn()) {
            f->pos = pos; f->life = 0.08f; f->size0 = crit ? 34.0f : 20.0f; f->size1 = f->size0 * 1.4f; f->alpha = 0.9f;
            f->r = 255; f->g = crit ? 170 : 240; f->b = crit ? 70 : 200;
        }
    }
};
