#pragma once
#include "HealFxCommon.h"

// Healing sparkles rising around the player. One light shared pool, typed emits.
namespace HealFx {
enum class HealType { WaveEnd, CardScale, PoolBonus, BossReward, OnKill };
}

class HealEffect : public HealFx::FxType<512> {
public:
    // radius: spawn ring radius around pos (game units). <= 0 uses the type default.
    void Emit(HealFx::HealType type, VECTOR pos, float radius = -1.0f) {
        using namespace HealFx;
        switch (type) {
        case HealType::WaveEnd:    Sparkles(pos, radius > 0 ? radius : 28.0f, 14, 0.8f, 5.0f, 120, 255, 150, 0); break;
        case HealType::CardScale:  Sparkles(pos, radius > 0 ? radius : 22.0f, 10, 0.6f, 5.5f, 150, 255, 215, 2); break;
        case HealType::PoolBonus:  Sparkles(pos, radius > 0 ? radius : 34.0f, 20, 1.0f, 6.5f, 255, 210, 80, 0); break;
        case HealType::BossReward: {
            float r = radius > 0 ? radius : 40.0f;
            Sparkles(pos, r, 18, 1.2f, 7.0f, 255, 205, 70, 0);
            Sparkles(pos, r * 0.7f, 10, 1.2f, 5.0f, 255, 255, 245, 0);
            for (int i = 0; i < 20; ++i) {                                  // expanding ground ring
                float a = 2 * Pi * i / 20;
                HealFx::Particle* p = Add(VAdd(pos, VGet(std::sin(a) * r * 0.4f, 3, std::cos(a) * r * 0.4f)),
                    VGet(std::sin(a) * 70, 0, std::cos(a) * 70), 0.7f, 6, 9, 0.85f, 255, 230, 140, true);
                if (p) p->drag = 2.5f;
            }
            break;
        }
        case HealType::OnKill:     Sparkles(pos, radius > 0 ? radius : 14.0f, 5, 0.4f, 3.5f, 110, 255, 140, 0); break;
        }
    }
    void HealWaveEnd(VECTOR pos, float radius = -1.0f)    { Emit(HealFx::HealType::WaveEnd, pos, radius); }
    void HealCardScale(VECTOR pos, float radius = -1.0f)  { Emit(HealFx::HealType::CardScale, pos, radius); }
    void HealPoolBonus(VECTOR pos, float radius = -1.0f)  { Emit(HealFx::HealType::PoolBonus, pos, radius); }
    void HealBossReward(VECTOR pos, float radius = -1.0f) { Emit(HealFx::HealType::BossReward, pos, radius); }
    void HealOnKill(VECTOR pos, float radius = -1.0f)     { Emit(HealFx::HealType::OnKill, pos, radius); }
private:
    void Sparkles(VECTOR pos, float radius, int count, float life, float size, int r, int g, int b, unsigned char tex) {
        using namespace HealFx;
        for (int i = 0; i < count; ++i) {
            float a = rng.Range(-Pi, Pi), rr = radius * std::sqrt(rng.Next());
            HealFx::Particle* p = Add(VAdd(pos, VGet(std::sin(a) * rr, rng.Range(0, 10), std::cos(a) * rr)),
                VGet(rng.Range(-6, 6), rng.Range(35, 75), rng.Range(-6, 6)),
                life * rng.Range(0.75f, 1.0f), size * rng.Range(0.7f, 1.1f), size * 0.4f, 0.95f, r, g, b, tex != 2);
            if (!p) return;
            p->drag = 1.2f; p->spin = rng.Range(8, 16); p->phase = rng.Range(0, 6.28f); p->tex = tex;
            if (tex == 2) { p->wobble = 2; p->alpha = 0.85f; }
        }
    }
};
