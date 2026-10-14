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
#include "Render3DUtil.h"

namespace
{
    constexpr float DEG2RAD = 3.14159265f / 180.0f;

    Vec3 Forward(float yaw) { return Vec3{ std::sin(yaw), 0.0f, std::cos(yaw) }; }
    float Rand01() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }
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
            // PR D combo P2: Quật đuôi -> Breach
            if (phase_ == BossPhase::Phase2 && Rand01() < GameConsts::COMBO_TAIL_BREACH_CHANCE)
                comboNext_ = BossSkill::Slam;
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
    case BossAction::TelegraphBubbleNet:
    {
        // Lặn xuống, bọt sủi vòng ngoài quanh người chơi
        pos_.y = -GameConsts::BREACH_SINK_DEPTH * std::clamp(actionTimer_ / GameConsts::BUBBLENET_TELEGRAPH, 0.0f, 1.0f);
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.05f)
        {
            telegraphFxTimer_ -= 0.05f;
            float a = static_cast<float>(rand() % 628) * 0.01f;
            Vec3 p = netCenter_ + Vec3{ std::sin(a) * GameConsts::BUBBLENET_START_RADIUS, 0.0f, std::cos(a) * GameConsts::BUBBLENET_START_RADIUS };
            effects.SpawnBubbles(p, 2, 12.0f, 2.0f, 4.0f);
        }
        if (actionTimer_ >= GameConsts::BUBBLENET_TELEGRAPH)
        {
            action_ = BossAction::BubbleNetSwim;
            actionTimer_ = 0.0f;
            netPillarsDropped_ = 0;
        }
        return true;
    }
    case BossAction::BubbleNetSwim:
    {
        float t = std::clamp(actionTimer_ / GameConsts::BUBBLENET_SWIM_TIME, 0.0f, 1.0f);
        float radius = GameConsts::BUBBLENET_START_RADIUS + (GameConsts::BUBBLENET_END_RADIUS - GameConsts::BUBBLENET_START_RADIUS) * t;
        float ang = netStartAngle_ + t * 6.2831853f;
        pos_ = netCenter_ + Vec3{ std::sin(ang) * radius, -GameConsts::BREACH_SINK_DEPTH * 0.5f, std::cos(ang) * radius };
        yaw_ = ang + 1.5707963f; // hướng tiếp tuyến

        int want = static_cast<int>(t * GameConsts::BUBBLENET_PILLARS);
        while (netPillarsDropped_ < want && netPillarsDropped_ < GameConsts::BUBBLENET_PILLARS)
        {
            float pa = netStartAngle_ + (static_cast<float>(netPillarsDropped_) + 0.5f) / GameConsts::BUBBLENET_PILLARS * 6.2831853f;
            float pt = (static_cast<float>(netPillarsDropped_) + 0.5f) / GameConsts::BUBBLENET_PILLARS;
            float pr = GameConsts::BUBBLENET_START_RADIUS + (GameConsts::BUBBLENET_END_RADIUS - GameConsts::BUBBLENET_START_RADIUS) * pt;
            BubblePillar bp;
            bp.pos = netCenter_ + Vec3{ std::sin(pa) * pr, 0.0f, std::cos(pa) * pr };
            bp.life = GameConsts::BUBBLENET_PILLAR_LIFE;
            bubblePillars_.push_back(bp);
            effects.SpawnBubbles(bp.pos, 6, 10.0f, 3.0f, 6.0f);
            netPillarsDropped_++;
        }

        if (actionTimer_ >= GameConsts::BUBBLENET_SWIM_TIME)
        {
            // Kết thúc: Breach từ dưới lên ngay giữa lưới
            pos_.y = 0.0f;
            StartBreach(player);
            slamTarget_ = netCenter_;
            actionTimer_ = GameConsts::BREACH_TELEGRAPH_P1 - GameConsts::BUBBLENET_FINISH_TELEGRAPH;
            if (actionTimer_ < 0.0f) actionTimer_ = 0.0f;
        }
        return true;
    }
    case BossAction::TelegraphInhale:
    {
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.04f)
        {
            telegraphFxTimer_ -= 0.04f;
            Vec3 f = Forward(yaw_);
            float side = (static_cast<float>(rand() % 200) - 100.0f);
            Vec3 p = pos_ + f * 200.0f + Vec3{ f.z * side, 0.0f, -f.x * side };
            effects.SpawnBubbles(p, 1, 8.0f, 2.0f, 3.5f);
        }
        if (actionTimer_ >= GameConsts::INHALE_TELEGRAPH)
        {
            action_ = BossAction::PerformInhale;
            actionTimer_ = 0.0f;
            SoundManager::Instance().PlaySFX(SfxId::TsunamiCharge);
        }
        return true;
    }
    case BossAction::PerformInhale:
    {
        Vec3 f = Forward(yaw_);
        Vec3 mouth = pos_ + f * 60.0f;
        mouth.y = 0.0f;
        const Vec3& pp = player.GetPosition();
        bool inCone = Collision::InConeXZ(pos_, f, GameConsts::INHALE_CONE_HALF_DEG * DEG2RAD, GameConsts::INHALE_RANGE, pp, player.GetHitboxRadius());
        if (inCone)
        {
            Vec3 toMouth = Collision::FlatDiff(mouth, pp);
            float len = std::sqrt(toMouth.x * toMouth.x + toMouth.z * toMouth.z);
            if (len > 1.0f)
            {
                float spd = GameConsts::INHALE_PULL_SPEED * (player.IsAirborne() ? 0.5f : 1.0f);
                player.AddExternalPull(Vec3{ toMouth.x / len * spd, 0.0f, toMouth.z / len * spd });
            }
            if (len <= GameConsts::INHALE_MOUTH_RADIUS && !player.IsAirborne())
            {
                // Bị nuốt -> đớp + phun ra
                player.TakeDamage(GameConsts::INHALE_BITE_DAMAGE, effects);
                player.ApplyKnockback(Vec3{ f.x * GameConsts::INHALE_BITE_KNOCKBACK, 0.0f, f.z * GameConsts::INHALE_BITE_KNOCKBACK });
                effects.SpawnWaterSplash(mouth, 80.0f, 1.5f);
                effects.AddTrauma(0.4f);
                Stun(GameConsts::INHALE_RECOVERY_STUN, GameConsts::BOSS_STUN_DAMAGE_MUL);
                return true;
            }
        }
        // Dòng bọt chảy vào miệng
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.03f)
        {
            telegraphFxTimer_ -= 0.03f;
            float side = (static_cast<float>(rand() % 300) - 150.0f);
            float dist = 80.0f + static_cast<float>(rand() % 300);
            Vec3 p = pos_ + f * dist + Vec3{ f.z * side, 0.0f, -f.x * side };
            effects.SpawnBubbles(p, 1, 6.0f, 2.0f, 4.0f);
        }
        if (actionTimer_ >= GameConsts::INHALE_DURATION)
        {
            // Hút hụt -> boss hụt hơi, choáng ngắn
            Stun(GameConsts::INHALE_RECOVERY_STUN, GameConsts::BOSS_STUN_DAMAGE_MUL);
        }
        return true;
    }
    case BossAction::TelegraphDive:
    {
        float dur = (phase_ == BossPhase::Phase1) ? GameConsts::DIVE_TELEGRAPH_P1 : GameConsts::DIVE_TELEGRAPH_P2;
        if (enraged_) dur *= 0.8f;
        pos_.y = -GameConsts::BREACH_SINK_DEPTH * 1.5f * std::clamp(actionTimer_ / dur, 0.0f, 1.0f);
        // Hướng lao khoá theo người chơi tới 70% telegraph
        if (actionTimer_ < dur * 0.7f)
        {
            Vec3 to = Collision::FlatDiff(player.GetPosition(), pos_);
            float len = std::sqrt(to.x * to.x + to.z * to.z);
            if (len > 1.0f) diveDir_ = Vec3{ to.x / len, 0.0f, to.z / len };
            yaw_ = std::atan2(diveDir_.x, diveDir_.z);
        }
        // Vệt bọt chỉ đường lao
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.05f)
        {
            telegraphFxTimer_ -= 0.05f;
            float d = 60.0f + static_cast<float>(rand() % 500);
            Vec3 p = pos_ + diveDir_ * d;
            p.y = 0.0f;
            effects.SpawnBubbles(p, 1, 10.0f, 2.0f, 4.0f);
        }
        if (actionTimer_ >= dur)
        {
            action_ = BossAction::PerformDive;
            actionTimer_ = 0.0f;
            diveHit_ = false;
            SoundManager::Instance().PlaySFX(SfxId::BossHeavySlam);
        }
        return true;
    }
    case BossAction::PerformDive:
    {
        pos_.x += diveDir_.x * GameConsts::DIVE_SPEED * dt;
        pos_.z += diveDir_.z * GameConsts::DIVE_SPEED * dt;
        effects.SpawnBubbles(Vec3{ pos_.x, 0.0f, pos_.z }, 2, 30.0f, 3.0f, 6.0f);

        if (!diveHit_ && !player.IsAirborne() &&
            Collision::DistXZ(player.GetPosition(), pos_) <= GameConsts::DIVE_HIT_RADIUS + player.GetHitboxRadius())
        {
            diveHit_ = true;
            player.TakeDamage(GameConsts::DIVE_DAMAGE, effects);
            player.ApplyKnockback(diveDir_ * GameConsts::DIVE_KNOCKBACK);
            effects.AddTrauma(0.45f);
        }

        float lim = GameConsts::ARENA_HALF_SIZE - 60.0f;
        bool hitWall = std::fabs(pos_.x) > lim || std::fabs(pos_.z) > lim;
        if (hitWall)
        {
            pos_.x = std::clamp(pos_.x, -lim, lim);
            pos_.z = std::clamp(pos_.z, -lim, lim);
            effects.SpawnWaterSplash(Vec3{ pos_.x, 0.0f, pos_.z }, 120.0f, 2.0f);
            effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 120.0f);
            effects.AddTrauma(0.6f);
            SoundManager::Instance().PlaySFX(SfxId::BossHeavySlam);
            Stun(GameConsts::DIVE_WALL_STUN, GameConsts::BOSS_STUN_DAMAGE_MUL);
            return true;
        }
        if (actionTimer_ >= GameConsts::DIVE_MAX_TIME)
        {
            pos_.y = 0.0f;
            effects.SpawnWaterSplash(pos_, 100.0f, 1.6f);
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
            // PR D combo P2: Lao ngầm -> Sóng thần
            if (phase_ == BossPhase::Phase2 && Rand01() < GameConsts::COMBO_DIVE_TSUNAMI_CHANCE)
                comboNext_ = BossSkill::Tsunami;
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

// ---------------- PR C: Bubble Net / Inhale ----------------
void WhaleBoss::StartBubbleNet(PlayerFish& player)
{
    netCenter_ = player.GetPosition();
    netCenter_.y = 0.0f;
    Vec3 rel = Collision::FlatDiff(pos_, netCenter_);
    netStartAngle_ = (rel.LengthSq() > 1.0f) ? std::atan2(rel.x, rel.z) : 0.0f;
    netPillarsDropped_ = 0;
    action_ = BossAction::TelegraphBubbleNet;
    actionTimer_ = 0.0f;
    telegraphFxTimer_ = 0.0f;
    bubbleNetCooldown_ = GameConsts::BUBBLENET_COOLDOWN;
    lastSkill_ = BossSkill::BubbleNet;
    RecordActionHistory(L"Bubble Net");
}

void WhaleBoss::ForceBubbleNet(PlayerFish& player)
{
    EnsureActiveForDebug();
    StartBubbleNet(player);
}

void WhaleBoss::StartInhale(PlayerFish& player)
{
    Vec3 to = Collision::FlatDiff(player.GetPosition(), pos_);
    if (to.LengthSq() > 1.0f) yaw_ = std::atan2(to.x, to.z);
    pos_.y = 0.0f;
    action_ = BossAction::TelegraphInhale;
    actionTimer_ = 0.0f;
    telegraphFxTimer_ = 0.0f;
    inhaleCooldown_ = GameConsts::INHALE_COOLDOWN;
    lastSkill_ = BossSkill::Inhale;
    RecordActionHistory(L"Inhale");
}

void WhaleBoss::ForceInhale(PlayerFish& player)
{
    EnsureActiveForDebug();
    StartInhale(player);
}

void WhaleBoss::UpdateBossHazards(float dt, PlayerFish& player, EffectSystem& effects)
{
    if (netHitCooldown_ > 0.0f) netHitCooldown_ -= dt;
    const Vec3& pp = player.GetPosition();
    for (auto& bp : bubblePillars_)
    {
        bp.life -= dt;
        bp.fxTimer += dt;
        while (bp.fxTimer >= 0.12f)
        {
            bp.fxTimer -= 0.12f;
            effects.SpawnBubbles(bp.pos, 2, 8.0f, 2.5f, 5.0f);
        }
        // Nhảy qua tường bọt thì an toàn
        if (bp.life > 0.0f && !player.IsAirborne() &&
            Collision::DistXZ(pp, bp.pos) <= GameConsts::BUBBLENET_PILLAR_RADIUS + player.GetHitboxRadius())
        {
            player.ApplySlow(GameConsts::BUBBLENET_SLOW_FACTOR, GameConsts::BUBBLENET_SLOW_TIME);
            if (netHitCooldown_ <= 0.0f)
            {
                player.TakeDamage(GameConsts::BUBBLENET_PILLAR_DAMAGE, effects);
                netHitCooldown_ = GameConsts::BUBBLENET_HIT_INTERVAL;
            }
        }
    }
    bubblePillars_.erase(std::remove_if(bubblePillars_.begin(), bubblePillars_.end(),
        [](const BubblePillar& b) { return b.life <= 0.0f; }), bubblePillars_.end());
}

void WhaleBoss::DrawBossHazards() const
{
    for (const auto& bp : bubblePillars_)
    {
        float a = std::clamp(bp.life / 0.6f, 0.0f, 1.0f);
        Render3D::DrawFlatShadow(bp.pos, GameConsts::BUBBLENET_PILLAR_RADIUS, 0.45f * a);
    }
}

// ---------------- PR D: Call the Pack / Dive Charge / Phase ----------------
void WhaleBoss::StartPackTelegraph(PlayerFish& player)
{
    action_ = BossAction::TelegraphMinions;
    actionTimer_ = 0.0f;
    telegraphFxTimer_ = 0.0f;
    int total = (phase_ == BossPhase::Phase1) ? GameConsts::PACK_P1_MINIONS : (GameConsts::PACK_P2_MINIONS + GameConsts::PACK_P2_HOPPERS);
    total = (std::min)(total, static_cast<int>(packSpots_.size()));
    float lim = GameConsts::ARENA_HALF_SIZE - 40.0f;
    float base = static_cast<float>(rand() % 628) * 0.01f;
    Vec3 c = player.GetPosition();
    packSpotCount_ = total;
    for (int i = 0; i < total; ++i)
    {
        float a = base + 6.2831853f * static_cast<float>(i) / static_cast<float>(total);
        Vec3 p{ c.x + std::sin(a) * GameConsts::PACK_SPAWN_RADIUS, 0.0f, c.z + std::cos(a) * GameConsts::PACK_SPAWN_RADIUS };
        p.x = std::clamp(p.x, -lim, lim);
        p.z = std::clamp(p.z, -lim, lim);
        packSpots_[i] = p;
    }
}

void WhaleBoss::SpawnPack(EnemyManager& enemies)
{
    int maxCanSpawn = (std::max)(0, GameConsts::BOSS_MINION_ALIVE_CAP - enemies.GetActiveCount());
    int minions = (phase_ == BossPhase::Phase1) ? GameConsts::PACK_P1_MINIONS : GameConsts::PACK_P2_MINIONS;
    int spawned = 0;
    for (int i = 0; i < packSpotCount_ && spawned < maxCanSpawn; ++i)
    {
        EnemyType type = (i < minions) ? EnemyType::Minion : EnemyType::Hopper;
        if (enemies.SpawnEnemy(type, packSpots_[i])) spawned++;
    }
    packSpotCount_ = 0;
}

void WhaleBoss::ForcePack(PlayerFish& player)
{
    EnsureActiveForDebug();
    pos_.y = 0.0f;
    lastSkill_ = BossSkill::Minions;
    StartPackTelegraph(player);
    RecordActionHistory(L"Call the Pack (debug)");
}

void WhaleBoss::StartDive(PlayerFish& player)
{
    Vec3 to = Collision::FlatDiff(player.GetPosition(), pos_);
    float len = std::sqrt(to.x * to.x + to.z * to.z);
    diveDir_ = (len > 1.0f) ? Vec3{ to.x / len, 0.0f, to.z / len } : Forward(yaw_);
    action_ = BossAction::TelegraphDive;
    actionTimer_ = 0.0f;
    telegraphFxTimer_ = 0.0f;
    diveCooldown_ = GameConsts::DIVE_COOLDOWN;
    lastSkill_ = BossSkill::Dive;
    RecordActionHistory(L"Dive Charge");
}

void WhaleBoss::ForceDive(PlayerFish& player)
{
    EnsureActiveForDebug();
    pos_.y = 0.0f;
    StartDive(player);
}

void WhaleBoss::ForcePhaseStep(EffectSystem& effects)
{
    EnsureActiveForDebug();
    if (phase_ == BossPhase::Phase1)
    {
        float dmg = hp_ - maxHp_ * 0.5f + 1.0f;
        if (dmg > 0.0f) TakeDamage(dmg, effects);
    }
    else if (!enraged_)
    {
        float dmg = hp_ - maxHp_ * GameConsts::RAGE_HP_RATIO + 1.0f;
        if (dmg > 0.0f) TakeDamage(dmg, effects);
    }
}
