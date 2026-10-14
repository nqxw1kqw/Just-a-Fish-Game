#pragma once
#include "HitFxCommon.h"

// Fish death: big bubble cluster rising + glittering scale flakes that slowly sink.
// size: fish body size in game units (e.g. FISH_SIZE_Z = 38; small enemies ~20-40).
class FishDeathFx : public HitFx::FxType<4096> {   // Just-a-Fish: 4096 (wipe cuối wave nổ nhiều cá cùng lúc)
public:
    void Emit(VECTOR pos, float size = 38.0f) {
        using namespace HitFx;
        float k = (std::max)(0.3f, size / 38.0f);
        int bubbles = static_cast<int>(18 * k + 6), flakes = static_cast<int>(14 * k + 6);
        for (int i = 0; i < bubbles; ++i) {
            HitFx::Particle* p = pool.Spawn();
            if (!p) return;
            VECTOR d = ConeDir(rng, VGet(0, 1, 0), 1.4f);
            p->pos = VAdd(pos, VScale(d, rng.Range(0, size * 0.4f)));
            p->vel = VAdd(VScale(d, rng.Range(30, 110) * k), VGet(0, rng.Range(20, 60), 0));
            p->life = rng.Range(0.8f, 1.5f); p->size0 = rng.Range(4, 11) * k; p->size1 = p->size0 * 1.5f;
            p->gravity = -120; p->drag = 2.5f; p->wobble = 2.5f * k; p->phase = rng.Range(0, 6.28f);
            p->tex = 1; p->additive = false; p->alpha = 0.9f; p->r = 215; p->g = 242; p->b = 255;
        }
        for (int i = 0; i < flakes; ++i) {
            HitFx::Particle* p = pool.Spawn();
            if (!p) return;
            VECTOR d = ConeDir(rng, VGet(0, 1, 0), 1.5f);
            p->pos = VAdd(pos, VScale(d, rng.Range(0, size * 0.3f)));
            p->vel = VScale(d, rng.Range(50, 140) * k);
            p->life = rng.Range(1.4f, 2.2f); p->size0 = rng.Range(2.5f, 4.5f) * k; p->size1 = p->size0 * 0.7f;
            p->gravity = 25; p->drag = 3.0f; p->wobble = 3.0f; p->phase = rng.Range(0, 6.28f);
            p->spin = rng.Range(10, 22);   // glitter twinkle
            p->additive = true; p->r = 255; p->g = static_cast<unsigned char>(rng.Range(215, 250)); p->b = static_cast<unsigned char>(rng.Range(150, 230));
        }
    }
};
