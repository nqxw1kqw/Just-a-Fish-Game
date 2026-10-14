#pragma once
#include "EnemyHitFxCommon.h"

// Damage on ENEMY fish (replaces HitBubblePuff): bright shards + flash, a water jet/splash
// along dir, knocked-off glittering scale flakes, and only a token couple of bubbles.
class EnemyHitEffect : public EnemyHitFx::FxType<1024> {
public:
    void Emit(VECTOR pos, VECTOR dir, bool crit = false) {
        using namespace EnemyHitFx;
        float k = crit ? 1.6f : 1.0f;
        VECTOR fwd = SafeNorm(dir, VGet(1, 0, 0));
        if (EnemyHitFx::Particle* f = pool.Spawn()) {               // flash
            f->pos = pos; f->life = crit ? 0.11f : 0.08f; f->size0 = 18 * k; f->size1 = 30 * k; f->alpha = crit ? 1.0f : 0.85f;
            f->r = 255; f->g = 250; f->b = crit ? 200 : 240;
        }
        int shards = crit ? 14 : 7;
        for (int i = 0; i < shards; ++i) {                           // bright shards
            EnemyHitFx::Particle* p = pool.Spawn();
            if (!p) return;
            p->pos = pos; p->vel = VScale(ConeDir(rng, fwd, 1.0f), rng.Range(200, 420) * k);
            p->life = rng.Range(0.08f, 0.15f); p->size0 = 2.4f * k; p->size1 = 1.0f; p->stretch = 0.04f; p->drag = 7;
            p->r = 255; p->g = crit ? 230 : 250; p->b = crit ? 150 : 255;
        }
        int jet = crit ? 16 : 10;
        for (int i = 0; i < jet; ++i) {                              // water jet / splash along dir
            EnemyHitFx::Particle* p = pool.Spawn();
            if (!p) return;
            p->pos = pos; p->vel = VScale(ConeDir(rng, fwd, 0.35f), rng.Range(150, 320) * k);
            p->life = rng.Range(0.2f, 0.35f); p->size0 = rng.Range(3, 5) * k; p->size1 = p->size0 * 2.4f;
            p->drag = 5; p->additive = false; p->alpha = 0.6f; p->r = 170; p->g = 225; p->b = 255;
            if (i % 3 == 0) { p->stretch = 0.035f; p->alpha = 0.75f; }
        }
        int flakes = crit ? 10 : 5;
        for (int i = 0; i < flakes; ++i) {                           // knocked-off scale flakes
            EnemyHitFx::Particle* p = pool.Spawn();
            if (!p) return;
            p->pos = pos; p->vel = VAdd(VScale(ConeDir(rng, fwd, 1.2f), rng.Range(70, 160) * k), VGet(0, rng.Range(20, 60), 0));
            p->life = rng.Range(0.7f, 1.2f); p->size0 = rng.Range(2.5f, 4.0f) * (crit ? 1.25f : 1.0f); p->size1 = p->size0 * 0.8f;
            p->gravity = 60; p->drag = 3.5f; p->wobble = 2; p->phase = rng.Range(0, 6.28f); p->spin = rng.Range(12, 24);
            p->r = 230; p->g = 245; p->b = 255;
            if (crit && i % 2) { p->r = 255; p->g = 225; p->b = 140; }
        }
        for (int i = 0; i < (crit ? 2 : 1); ++i) {                   // token bubble(s)
            EnemyHitFx::Particle* b = pool.Spawn();
            if (!b) return;
            b->pos = pos; b->vel = VAdd(VScale(fwd, 40), VGet(0, 30, 0)); b->life = 0.5f; b->size0 = 3; b->size1 = 4.5f;
            b->gravity = -90; b->drag = 3; b->tex = 1; b->additive = false; b->alpha = 0.7f; b->r = 210; b->g = 240; b->b = 255;
        }
    }
};
