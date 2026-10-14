#pragma once
#include "PlayerHurtFxCommon.h"

// Shown only when the PLAYER takes damage: blood-red drops (arc under gravity) and
// streaks (stretched along velocity). Colors darken and fade as they drift underwater. No bubbles.
// amount: 0.5 = light hit, 1 = normal, 2+ = heavy (scales count and size, clamped to 0.25..3).
class PlayerHurtEffect : public PlayerHurtFx::FxType<1024> {
public:
    void Emit(VECTOR pos, VECTOR dir, float amount = 1.0f) {
        using namespace PlayerHurtFx;
        float k = std::clamp(amount, 0.25f, 3.0f);
        float sizeK = 0.75f + 0.25f * k;
        int drops = static_cast<int>(8 * k + 2), streaks = static_cast<int>(6 * k + 2);
        for (int i = 0; i < drops; ++i) {
            PlayerHurtFx::Particle* p = pool.Spawn();
            if (!p) return;
            VECTOR d = ConeDir(rng, VAdd(SafeNorm(dir), VGet(0, 0.5f, 0)), 0.9f);
            p->pos = pos; p->vel = VScale(d, rng.Range(90, 230) * (0.8f + 0.2f * k));
            p->life = rng.Range(0.45f, 0.8f); p->size0 = rng.Range(3.0f, 5.5f) * sizeK; p->size1 = p->size0 * 1.35f;
            p->gravity = 520; p->drag = 1.6f; p->additive = false; p->alpha = 0.95f;
            p->r = static_cast<unsigned char>(rng.Range(185, 225)); p->g = static_cast<unsigned char>(rng.Range(10, 28)); p->b = 22;
            p->tint = true; p->r1 = 85; p->g1 = 8; p->b1 = 18;           // darkens/diffuses in water
        }
        for (int i = 0; i < streaks; ++i) {
            PlayerHurtFx::Particle* p = pool.Spawn();
            if (!p) return;
            VECTOR d = ConeDir(rng, dir, 0.6f);
            p->pos = pos; p->vel = VScale(d, rng.Range(220, 420) * (0.8f + 0.2f * k));
            p->life = rng.Range(0.16f, 0.28f); p->size0 = rng.Range(2.0f, 3.2f) * sizeK; p->size1 = p->size0 * 0.5f;
            p->stretch = rng.Range(0.04f, 0.07f); p->gravity = 380; p->drag = 4.5f; p->additive = false; p->alpha = 0.95f;
            p->r = 220; p->g = 24; p->b = 30; p->tint = true; p->r1 = 120; p->g1 = 10; p->b1 = 20;
        }
        for (int i = 0; i < 1 + static_cast<int>(k); ++i) {          // diffusing blood cloud
            PlayerHurtFx::Particle* c = pool.Spawn();
            if (!c) return;
            c->pos = pos; c->vel = VAdd(VScale(SafeNorm(dir), rng.Range(20, 50)), VGet(0, 8, 0));
            c->life = rng.Range(0.7f, 1.1f); c->size0 = 10 * sizeK; c->size1 = 28 * sizeK; c->drag = 2.5f;
            c->additive = false; c->alpha = 0.35f; c->r = 150; c->g = 14; c->b = 24; c->tint = true; c->r1 = 60; c->g1 = 8; c->b1 = 16;
        }
    }
};
