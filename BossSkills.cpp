// =======================================================================
// BossSkills.cpp
// Boss rework: các chiêu mới của WhaleBoss (tách khỏi Boss.cpp cho dễ đọc).
// PR A: Breach Slam (telegraph/land), Tail Slap, Stun.
// Tất cả số liệu nằm trong Consts.h (BREACH_*, TAIL_*, BOSS_*).
// =======================================================================
#include <algorithm>
#include <cmath>
#include "Boss.h"
#include "PlayerFish.h"
#include "Enemy.h"
#include "Effects.h"
#include "SoundManager.h"
#include "DxLib.h"
#include "CollisionUtil.h"

namespace
{
    constexpr float DEG2RAD = 3.14159265f / 180.0f;

    Vec3 Forward(float yaw) { return Vec3{ std::sin(yaw), 0.0f, std::cos(yaw) }; }
}

void WhaleBoss::EnsureActiveForDebug()
{
    if (!active_)
    {
        active_ = true;
        pos_ = Vec3{ 0.0f, 0.0f, 350.0f };
    }
    actionTimer_ = 0.0f;
}

bool WhaleBoss::IsPlayerBehind(const PlayerFish& player) const
{
    Vec3 to = Collision::FlatDiff(player.GetPosition(), pos_);
    float len = std::sqrt(to.x * to.x + to.z * to.z);
    if (len < 1.0f) return true;
    Vec3 f = Forward(yaw_);
    float cosA = (to.x * f.x + to.z * f.z) / len;
    return cosA <= std::cos(GameConsts::TAIL_TRIGGER_ANGLE_DEG * DEG2RAD);
}

// ---------------- Breach Slam ----------------
void WhaleBoss::StartBreach(PlayerFish& player)
{
    action_ = BossAction::TelegraphSlam;
    actionTimer_ = 0.0f;
    telegraphFxTimer_ = 0.0f;
    slamTarget_ = player.GetPosition();
    slamTarget_.y = 0.0f;
    RecordActionHistory(L"Breach Slam");
}

void WhaleBoss::UpdateBreachTelegraph(float dt, float duration, PlayerFish& player, EffectSystem& effects, bool trackPlayer)
{
    float ratio = std::clamp(actionTimer_ / (std::max)(0.01f, duration), 0.0f, 1.0f);
    if (trackPlayer)
    {
        slamTarget_ = player.GetPosition();
        slamTarget_.y = 0.0f;
    }
    // Lặn xuống dần
    pos_.y = -GameConsts::BREACH_SINK_DEPTH * ratio;

    // Bọt khí sủi lên dày dần ở điểm đáp
    telegraphFxTimer_ += dt;
    while (telegraphFxTimer_ >= 0.06f)
    {
        telegraphFxTimer_ -= 0.06f;
        int n = 1 + static_cast<int>(ratio * 4.0f);
        effects.SpawnBubbles(slamTarget_, n, 40.0f + ratio * (GameConsts::BREACH_OUTER_RADIUS - 40.0f), 2.5f, 5.0f + ratio * 3.0f);
        // Vệt bọt quanh thân boss khi lặn
        effects.SpawnBubbles(pos_, 1, 60.0f, 2.0f, 4.0f);
    }
}

void WhaleBoss::LandBreach(PlayerFish& player, EffectSystem& effects, float innerDamage, float outerDamage, float innerRadius, float outerRadius)
{
    pos_ = slamTarget_;
    pos_.y = 0.0f;
    effects.AddTrauma(0.55f);
    effects.SpawnShockwave(pos_, outerRadius);
    effects.SpawnWaterSplash(pos_, outerRadius, 2.2f);
    effects.SpawnBubbles(pos_, 24, outerRadius * 0.8f, 3.0f, 7.0f);
    SoundManager::Instance().PlaySFX(SfxId::BossHeavySlam);

    if (player.GetHeightY() >= GameConsts::BOSS_DODGE_HEIGHT || player.IsInvulnerable()) return;
    float d = Collision::DistXZ(player.GetPosition(), pos_);
    if (d <= innerRadius)
    {
        player.TakeDamage(innerDamage, effects);
    }
    else if (d <= outerRadius)
    {
        player.TakeDamage(outerDamage, effects);
    }
}

void WhaleBoss::ForceBreach(PlayerFish& player)
{
    EnsureActiveForDebug();
    StartBreach(player);
}

// ---------------- Tail Slap ----------------
void WhaleBoss::StartTail()
{
    action_ = BossAction::TelegraphTail;
    actionTimer_ = 0.0f;
    telegraphFxTimer_ = 0.0f;
    tailCooldown_ = GameConsts::TAIL_COOLDOWN;
    RecordActionHistory(L"Tail Slap");
}

void WhaleBoss::ForceTail(PlayerFish&)
{
    EnsureActiveForDebug();
    StartTail();
}

// ---------------- Stun ----------------
void WhaleBoss::Stun(float duration, float damageMul)
{
    if (!active_ || hp_ <= 0.0f) return;
    pos_.y = 0.0f;
    action_ = BossAction::Stunned;
    actionTimer_ = 0.0f;
    stunTimer_ = duration;
    stunDamageMul_ = damageMul;
    telegraphFxTimer_ = 0.0f;
    RecordActionHistory(L"Stunned");
}

// ---------------- State machine mở rộng ----------------
bool WhaleBoss::UpdateExtendedAction(float dt, PlayerFish& player, EnemyManager& enemies, EffectSystem& effects)
{
    (void)enemies;
    switch (action_)
    {
    case BossAction::TelegraphTail:
    {
        float dur = (phase_ == BossPhase::Phase1) ? GameConsts::TAIL_TELEGRAPH_P1 : GameConsts::TAIL_TELEGRAPH_P2;
        Vec3 back = Forward(yaw_) * -1.0f;
        Vec3 tailPos = pos_ + back * 110.0f + Vec3{ 0.0f, 30.0f, 0.0f };

        // Nước nhỏ giọt từ đuôi đang giơ
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.05f)
        {
            telegraphFxTimer_ -= 0.05f;
            effects.SpawnBubbles(tailPos, 2, 25.0f, 2.0f, 4.0f);
        }

        if (actionTimer_ >= dur)
        {
            // Quật: nón 140 độ phía sau
            Vec3 tailHit = pos_ + back * 90.0f;
            effects.SpawnWaterSplash(tailHit, 130.0f, 1.6f);
            effects.SpawnShockwave(tailHit, 110.0f);
            effects.AddTrauma(0.3f);
            SoundManager::Instance().PlaySFX(SfxId::BossHeavySlam);

            bool inCone = Collision::InConeXZ(pos_, back, GameConsts::TAIL_CONE_HALF_DEG * DEG2RAD,
                                               GameConsts::TAIL_RANGE, player.GetPosition(), player.GetHitboxRadius());
            if (inCone && player.GetHeightY() < GameConsts::BOSS_DODGE_HEIGHT && !player.IsInvulnerable())
            {
                Vec3 out = Collision::FlatDiff(player.GetPosition(), pos_);
                float l = std::sqrt(out.x * out.x + out.z * out.z);
                out = (l > 0.001f) ? out * (1.0f / l) : back;
                player.TakeDamage(GameConsts::TAIL_DAMAGE, effects);
                player.ApplyKnockback(out * GameConsts::TAIL_KNOCKBACK);
            }
            action_ = BossAction::PerformTail;
            actionTimer_ = 0.0f;
        }
        return true;
    }
    case BossAction::PerformTail:
    {
        if (actionTimer_ >= GameConsts::TAIL_RECOVERY)
        {
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
        }
        return true;
    }
    case BossAction::Stunned:
    {
        stunTimer_ -= dt;
        // Bọt khí xoay vòng trên đầu = đang choáng
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.12f)
        {
            telegraphFxTimer_ -= 0.12f;
            float a = stunTimer_ * 6.0f;
            effects.SpawnBubbles(pos_ + Vec3{ std::cos(a) * 40.0f, 70.0f, std::sin(a) * 40.0f }, 1, 6.0f, 3.0f, 5.0f);
        }
        if (stunTimer_ <= 0.0f)
        {
            stunTimer_ = 0.0f;
            stunDamageMul_ = 1.0f;
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
        }
        return true;
    }
    default:
        return false;
    }
}

// ---------------- PR B: Sóng đôi ----------------
void WhaleBoss::ForceTsunamiDouble(PlayerFish& player)
{
    EnsureActiveForDebug();
    phase_ = BossPhase::Phase2;
    needSecondRing_ = true;
    secondRingDelay_ = GameConsts::TSUNAMI_DOUBLE_DELAY;
    secondRingGapDeg_ = GameConsts::TSUNAMI_GAP_DEG_P2;
    StartTsunamiTelegraph(player, 0.0f);
    RecordActionHistory(L"Tsunami Double (debug)");
}
