#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "TurtleBoss.h"
#include "PlayerFish.h"
#include "Effects.h"
#include "RenderInterp.h"
#include "Render3DUtil.h"
#include "CollisionUtil.h"
#include "MapObstacles.h"
#include "DxConv.h"
#include "LowPolyModels.h"
#include "Effects/Hit/HitFeedback.h"
#include "Effects/Turtle/TurtleFxSet.h"
#include "DxLib.h"

namespace
{
    constexpr float PI = 3.14159265f;
    float Rand01() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }
    float YawOf(const Vec3& d) { return std::atan2(d.x, d.z); }
    float ArenaLimit() { return GameConsts::ARENA_HALF_SIZE - GameConsts::TURTLE_HITBOX_RADIUS; }
}

TurtleBoss::TurtleBoss() : fx_(std::make_unique<TurtleFxSet>()) {}
TurtleBoss::~TurtleBoss() = default;

void TurtleBoss::Reset()
{
    if (fx_) fx_->ClearAll();
    active_ = false;
    action_ = TurtleAction::Idle;
    phase2_ = false;
    shellBroken_ = false;
}

void TurtleBoss::Spawn(const Vec3& arenaCenter)
{
    Reset();
    active_ = true;
    pos_ = arenaCenter + Vec3{ 0.0f, 0.0f, -GameConsts::TURTLE_SPAWN_DIST };
    yaw_ = 0.0f;
    prevPos_ = pos_; prevYaw_ = yaw_;
    hp_ = maxHp_;
    shellHp_ = shellMaxHp_;
    shellBroken_ = false;
    phase2_ = false;
    fightTimer_ = 0.0f;
    hitFlashTimer_ = 0.0f;
    animTime_ = 0.0f;
    bobPhase_ = 0.0f;
    rollAngle_ = 0.0f;
    idleTimer_ = 1.2f;
    lastSkill_ = prevSkill_ = TurtleSkill::None;
}

void TurtleBoss::SetBossDifficulty(int bossIndex)
{
    tier_ = (std::max)(1, bossIndex);
    maxHp_ = GameConsts::TURTLE_MAX_HP * (1.0f + 0.5f * (tier_ - 1)) * stageHpMul_;
    shellMaxHp_ = GameConsts::TURTLE_SHELL_HP * (1.0f + 0.25f * (tier_ - 1)) * stageHpMul_;
    hp_ = maxHp_;
    shellHp_ = shellMaxHp_;
}

void TurtleBoss::SetHpMultiplier(float mul)
{
    stageHpMul_ = mul;
    maxHp_ = GameConsts::TURTLE_MAX_HP * (1.0f + 0.5f * (tier_ - 1)) * stageHpMul_;
    shellMaxHp_ = GameConsts::TURTLE_SHELL_HP * (1.0f + 0.25f * (tier_ - 1)) * stageHpMul_;
    hp_ = maxHp_;
    shellHp_ = shellMaxHp_;
}

void TurtleBoss::BeginRenderInterp(float alpha)
{
    RenderInterp::Apply(pos_, yaw_, prevPos_, prevYaw_, alpha, savedPos_, savedYaw_);
}

Vec3 TurtleBoss::Fwd() const { return Vec3{ std::sin(yaw_), 0.0f, std::cos(yaw_) }; }
float TurtleBoss::RollSpeed() const { return phase2_ ? GameConsts::TURTLE_ROLL_SPEED_P2 : GameConsts::TURTLE_ROLL_SPEED; }

void TurtleBoss::TurnToward(float targetYaw, float rate, float dt)
{
    float d = targetYaw - yaw_;
    while (d > PI) d -= 2.0f * PI;
    while (d < -PI) d += 2.0f * PI;
    yaw_ += std::clamp(d, -rate * dt, rate * dt);
}

const wchar_t* TurtleBoss::SkillName(TurtleSkill s)
{
    switch (s)
    {
    case TurtleSkill::Roll: return L"RollCharge";
    case TurtleSkill::Bite: return L"Bite";
    case TurtleSkill::Tail: return L"TailSweep";
    default: return L"-";
    }
}

void TurtleBoss::NoteSkill(TurtleSkill s) { prevSkill_ = lastSkill_; lastSkill_ = s; }

void TurtleBoss::EndSkill()
{
    restTimer_ = (phase2_ ? GameConsts::TURTLE_REST_TIME_P2 : GameConsts::TURTLE_REST_TIME) * (hell_ ? GameConsts::HELL_BOSS_REST_MUL : 1.0f);
    action_ = TurtleAction::Rest;
    actionTimer_ = 0.0f;
}

void TurtleBoss::StartWithdraw(PlayerFish& player)
{
    (void)player;
    if (fx_) Fx().withdrawDust.Emit(DxConv::ToVECTOR(Vec3{ pos_.x, 4.0f, pos_.z }));
    NoteSkill(TurtleSkill::Roll);
    action_ = TurtleAction::Withdraw;
    actionTimer_ = 0.0f;
    withdrawDuration_ = GameConsts::TURTLE_WITHDRAW_TIME;
    rollsLeft_ = phase2_ ? GameConsts::TURTLE_ROLL_COUNT_P2 : GameConsts::TURTLE_ROLL_COUNT;
}

void TurtleBoss::StartRollCharge()
{
    action_ = TurtleAction::RollTelegraph;
    actionTimer_ = 0.0f;
}

void TurtleBoss::StartRollChargeLocked()
{
    // Lock the roll line: from here to the arena wall along the facing at lock time.
    rollDir_ = Fwd();
    float lim = ArenaLimit();
    float t = GameConsts::TURTLE_ROLL_MAX_DIST;
    if (rollDir_.x > 0.001f) t = (std::min)(t, (lim - pos_.x) / rollDir_.x);
    else if (rollDir_.x < -0.001f) t = (std::min)(t, (-lim - pos_.x) / rollDir_.x);
    if (rollDir_.z > 0.001f) t = (std::min)(t, (lim - pos_.z) / rollDir_.z);
    else if (rollDir_.z < -0.001f) t = (std::min)(t, (-lim - pos_.z) / rollDir_.z);
    rollDist_ = (std::max)(80.0f, t);
    rollEnd_ = pos_ + rollDir_ * rollDist_;
    rollTravel_ = 0.0f;
    rollHitDone_ = false;
    action_ = TurtleAction::Rolling;
    actionTimer_ = 0.0f;
}

void TurtleBoss::StartBite(PlayerFish& player)
{
    NoteSkill(TurtleSkill::Bite);
    biteDir_ = Collision::FlatDiff(player.GetPosition(), pos_);
    float len = biteDir_.Length();
    biteDir_ = (len > 1.0f) ? biteDir_ / len : Fwd();
    biteHitDone_ = false;
    action_ = TurtleAction::BiteTelegraph;
    actionTimer_ = 0.0f;
}

void TurtleBoss::StartTail()
{
    NoteSkill(TurtleSkill::Tail);
    tailHitDone_ = false;
    action_ = TurtleAction::TailTelegraph;
    actionTimer_ = 0.0f;
}

void TurtleBoss::EnterPhase2(EffectSystem& effects)
{
    if (phase2_) return;
    phase2_ = true;
    effects.AddTrauma(0.5f);
    effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 180.0f);
    effects.SpawnBubbles(pos_ + Vec3{ 0.0f, 50.0f, 0.0f }, 30, 80.0f, 3.0f, 6.0f);
    AppLogAdd(L"[TURTLE] Phase 2 (double roll)\n");
}

void TurtleBoss::BreakShell(EffectSystem& effects)
{
    shellBroken_ = true;
    shellHp_ = 0.0f;
    action_ = TurtleAction::Stagger;
    actionTimer_ = 0.0f;
    effects.AddTrauma(0.7f);
    effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 200.0f);
    effects.SpawnWaterSplash(pos_ + Vec3{ 0.0f, 30.0f, 0.0f }, 110.0f, 1.6f);
    if (fx_)
    {
        Fx().shellBreak.Emit(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 45.0f, 0.0f }), 95.0f); // Fuyuu shell shards
        Fx().staggerStars.Start(DxConv::ToVECTOR(pos_ + Fwd() * 100.0f + Vec3{ 0.0f, 55.0f, 0.0f }), GameConsts::TURTLE_STAGGER_TIME);
    }
    effects.SpawnBubbles(pos_ + Vec3{ 0.0f, 50.0f, 0.0f }, 40, 90.0f, 3.0f, 7.0f);
    AppLogAdd(L"[TURTLE] Shell broken -> stagger\n");
}

void TurtleBoss::PickNext(PlayerFish& player)
{
    float dist = Collision::DistXZ(player.GetPosition(), pos_);
    const TurtleSkill skills[3] = { TurtleSkill::Roll, TurtleSkill::Bite, TurtleSkill::Tail };
    float w[3] = {
        GameConsts::TURTLE_WEIGHT_ROLL,
        (dist <= GameConsts::TURTLE_BITE_TRIGGER_RANGE) ? GameConsts::TURTLE_WEIGHT_BITE : 0.0f,
        (dist <= GameConsts::TURTLE_TAIL_TRIGGER_RANGE) ? GameConsts::TURTLE_WEIGHT_TAIL : 0.0f };
    float total = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        if (skills[i] == lastSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_LAST;
        if (skills[i] == prevSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_PREV;
        if (skills[i] == lastSkill_ && skills[i] == prevSkill_) w[i] = 0.0f;
        total += w[i];
    }
    TurtleSkill pick = TurtleSkill::Roll;
    if (total > 0.0f)
    {
        float r = Rand01() * total;
        for (int i = 0; i < 3; ++i) { if (w[i] <= 0.0f) continue; pick = skills[i]; if (r < w[i]) break; r -= w[i]; }
    }
    if (pick == TurtleSkill::Bite) StartBite(player);
    else if (pick == TurtleSkill::Tail) StartTail();
    else StartWithdraw(player);
}

void TurtleBoss::UpdateAdvance(float dt, const PlayerFish& player, float speedMul)
{
    Vec3 toPlayer = Collision::FlatDiff(player.GetPosition(), pos_);
    float dist = toPlayer.Length();
    Vec3 radial = (dist > 1.0f) ? MapObstacles::ChaseDirection(pos_, player.GetPosition(), GameConsts::TURTLE_HITBOX_RADIUS) : Fwd();
    TurnToward(YawOf(radial), GameConsts::TURTLE_TURN_RATE, dt);
    float speed = GameConsts::TURTLE_MOVE_SPEED * (phase2_ ? GameConsts::TURTLE_PHASE2_SPEED_MUL : 1.0f) * speedMul;
    if (dist > GameConsts::TURTLE_PREFERRED_RANGE + GameConsts::TURTLE_RANGE_TOLERANCE)
        pos_ = MapObstacles::Move(pos_, radial * (speed * dt), GameConsts::TURTLE_HITBOX_RADIUS);
    else if (dist < GameConsts::TURTLE_PREFERRED_RANGE - GameConsts::TURTLE_RANGE_TOLERANCE)
        pos_ = MapObstacles::Move(pos_, -(radial * (speed * 0.7f * dt)), GameConsts::TURTLE_HITBOX_RADIUS);
    float lim = ArenaLimit();
    pos_.x = std::clamp(pos_.x, -lim, lim);
    pos_.z = std::clamp(pos_.z, -lim, lim);
}
void TurtleBoss::Update(float dt, PlayerFish& player, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return;
    if (fx_) fx_->Update(dt);
    animTime_ += dt;
    bobPhase_ += dt * GameConsts::TURTLE_ANIM_BOB_SPEED;
    fightTimer_ += dt;
    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;
    const float cdt = hell_ ? dt * GameConsts::HELL_BOSS_COOLDOWN_SPEED : dt;
    (void)cdt;

    switch (action_)
    {
    case TurtleAction::Idle:
        UpdateAdvance(dt, player, 1.0f);
        idleTimer_ -= dt;
        if (idleTimer_ <= 0.0f) PickNext(player);
        break;
    case TurtleAction::Rest:
        UpdateAdvance(dt, player, 0.4f);
        restTimer_ -= dt;
        if (restTimer_ <= 0.0f) { action_ = TurtleAction::Idle; idleTimer_ = GameConsts::TURTLE_IDLE_GAP; }
        break;
    case TurtleAction::Withdraw:
        // Tucked in: no advance, shell squash is drawn in Draw().
        actionTimer_ += dt;
        if (actionTimer_ >= withdrawDuration_) StartRollCharge();
        break;
    case TurtleAction::RollTelegraph:
        // Still aiming during the telegraph; the line locks when the roll starts.
        TurnToward(YawOf(Collision::FlatDiff(player.GetPosition(), pos_)), GameConsts::TURTLE_TURN_RATE * 1.6f, dt);
        actionTimer_ += dt;
        if (actionTimer_ >= (phase2_ ? GameConsts::TURTLE_ROLL_TELEGRAPH_P2 : GameConsts::TURTLE_ROLL_TELEGRAPH))
            StartRollChargeLocked();
        break;
    case TurtleAction::Rolling:
    {
        float speed = RollSpeed();
        const Vec3 requestedRoll = pos_ + rollDir_ * (speed * dt);
        pos_ = MapObstacles::Move(pos_, rollDir_ * (speed * dt), GameConsts::TURTLE_HITBOX_RADIUS);
        const bool hitReef = Collision::DistSqXZ(pos_, requestedRoll) > 0.25f;
        rollTravel_ += speed * dt;
        // Fuyuu roll trail (crowd-lightened by particleScale like Shark sand)
        if (fx_ && (effects.particleScale_ >= 1.0f || Rand01() <= effects.particleScale_))
            Fx().rollTrail.EmitNode(DxConv::ToVECTOR(Vec3{ pos_.x, 6.0f, pos_.z }), DxConv::ToVECTOR(rollDir_ * speed));
        rollAngle_ += speed * dt / GameConsts::TURTLE_ROLL_SPIN_RADIUS;
        yaw_ = YawOf(rollDir_);
        float lim = ArenaLimit();
        bool hitWall = (std::fabs(pos_.x) >= lim - 1.0f || std::fabs(pos_.z) >= lim - 1.0f);
        pos_.x = std::clamp(pos_.x, -lim, lim);
        pos_.z = std::clamp(pos_.z, -lim, lim);
        if (!rollHitDone_ && !player.IsAirborne() &&
            Collision::DistXZ(player.GetPosition(), pos_) <= GameConsts::TURTLE_HITBOX_RADIUS + player.GetHitboxRadius())
        {
            rollHitDone_ = true;
            player.TakeDamage(GameConsts::TURTLE_ROLL_DAMAGE, effects, &pos_);
            effects.AddTrauma(0.3f);
        }
        if ((static_cast<int>(rollTravel_ / 90.0f) != static_cast<int>((rollTravel_ - speed * dt) / 90.0f)))
            effects.SpawnWaterSplash(Vec3{ pos_.x, 0.0f, pos_.z }, 60.0f, 0.7f);
        if (rollTravel_ >= rollDist_ || hitWall || hitReef)
        {
            effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 130.0f);
            effects.SpawnWaterSplash(Vec3{ pos_.x, 0.0f, pos_.z }, 90.0f, 1.0f);
            if (fx_) Fx().rollImpact.Emit(DxConv::ToVECTOR(Vec3{ pos_.x, 10.0f, pos_.z }), DxConv::ToVECTOR(rollDir_ * -1.0f)); // wall normal = back into arena
            rollsLeft_--;
            if (rollsLeft_ > 0)
            {
                // Phase 2: tuck in briefly and roll a second time at the player.
                action_ = TurtleAction::Withdraw;
                actionTimer_ = 0.0f;
                withdrawDuration_ = GameConsts::TURTLE_WITHDRAW_TIME_P2;
                if (fx_) Fx().withdrawDust.Emit(DxConv::ToVECTOR(Vec3{ pos_.x, 4.0f, pos_.z }));
                rollAngle_ = 0.0f;
                Vec3 toP = Collision::FlatDiff(player.GetPosition(), pos_);
                if (toP.LengthSq() > 1.0f) yaw_ = YawOf(toP);
            }
            else
            {
                action_ = TurtleAction::RollRecover;
                actionTimer_ = 0.0f;
            }
        }
        break;
    }
    case TurtleAction::RollRecover:
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::TURTLE_ROLL_RECOVER) { rollAngle_ = 0.0f; EndSkill(); }
        break;
    case TurtleAction::Stagger:
        actionTimer_ += dt;
        if (fx_) Fx().staggerStars.SetHead(DxConv::ToVECTOR(pos_ + Fwd() * 100.0f + Vec3{ 0.0f, 55.0f, 0.0f }));
        if (actionTimer_ >= GameConsts::TURTLE_STAGGER_TIME) { if (fx_) Fx().staggerStars.Stop(); EndSkill(); }
        break;
    case TurtleAction::BiteTelegraph:
        TurnToward(YawOf(Collision::FlatDiff(player.GetPosition(), pos_)), GameConsts::TURTLE_TURN_RATE * 1.4f, dt);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::TURTLE_BITE_TELEGRAPH)
        {
            biteDir_ = Fwd();
            if (fx_) Fx().biteSnap.Emit(DxConv::ToVECTOR(pos_ + biteDir_ * 95.0f + Vec3{ 0.0f, 32.0f, 0.0f }), DxConv::ToVECTOR(biteDir_));
            action_ = TurtleAction::BiteLunge;
            actionTimer_ = 0.0f;
        }
        break;
    case TurtleAction::BiteLunge:
    {
        pos_ = MapObstacles::Move(pos_, biteDir_ * (GameConsts::TURTLE_BITE_LUNGE_SPEED * dt), GameConsts::TURTLE_HITBOX_RADIUS);
        float lim = ArenaLimit();
        pos_.x = std::clamp(pos_.x, -lim, lim);
        pos_.z = std::clamp(pos_.z, -lim, lim);
        actionTimer_ += dt;
        if (!biteHitDone_ && !player.IsAirborne() &&
            Collision::DistXZ(player.GetPosition(), pos_) <= GameConsts::TURTLE_BITE_HIT_RANGE + player.GetHitboxRadius())
        {
            biteHitDone_ = true;
            player.TakeDamage(GameConsts::TURTLE_BITE_DAMAGE, effects, &pos_);
            effects.SpawnWaterSplash(player.GetPosition(), 50.0f, 0.8f);
        }
        if (actionTimer_ >= GameConsts::TURTLE_BITE_LUNGE_TIME) { action_ = TurtleAction::BiteRecover; actionTimer_ = 0.0f; }
        break;
    }
    case TurtleAction::BiteRecover:
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::TURTLE_BITE_RECOVER) EndSkill();
        break;
    case TurtleAction::TailTelegraph:
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::TURTLE_TAIL_TELEGRAPH) { action_ = TurtleAction::TailSweep; actionTimer_ = 0.0f; }
        break;
    case TurtleAction::TailSweep:
    {
        if (!tailHitDone_)
        {
            tailHitDone_ = true;
            effects.AddTrauma(0.25f);
            effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, GameConsts::TURTLE_TAIL_RADIUS * 1.3f);
            effects.SpawnWaterSplash(Vec3{ pos_.x, 0.0f, pos_.z } - Fwd() * 70.0f, 70.0f, 0.9f);
            if (!player.IsAirborne() &&
                Collision::DistXZ(player.GetPosition(), pos_) <= GameConsts::TURTLE_TAIL_RADIUS + player.GetHitboxRadius())
                player.TakeDamage(GameConsts::TURTLE_TAIL_DAMAGE, effects, &pos_);
        }
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::TURTLE_TAIL_SWEEP_TIME) { action_ = TurtleAction::TailRecover; actionTimer_ = 0.0f; }
        break;
    }
    case TurtleAction::TailRecover:
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::TURTLE_TAIL_RECOVER) EndSkill();
        break;
    }
}

bool TurtleBoss::TakeDamageInternal(float dmg, EffectSystem& effects, bool fromSlam)
{
    if (!active_ || hp_ <= 0.0f) return false;
    dmg *= damageTakenMul_;
    bool brokeNow = false;
    if (!shellBroken_)
    {
        shellHp_ -= dmg * (fromSlam ? 1.0f : GameConsts::TURTLE_SHELL_CHIP_MUL);
        if (shellHp_ <= 0.0f) brokeNow = true;
    }
    float bodyMul = shellBroken_ ? 1.0f : GameConsts::TURTLE_SHELL_DMG_MUL;
    if (action_ == TurtleAction::Stagger) bodyMul *= GameConsts::TURTLE_STAGGER_DMG_MUL;
    if (action_ == TurtleAction::RollRecover) bodyMul *= GameConsts::TURTLE_RECOVER_DMG_MUL;
    float bodyDmg = dmg * bodyMul;
    hp_ -= bodyDmg;
    hitFlashTimer_ = GameConsts::HIT_FLASH_TIME;
    Vec3 hitPos = pos_ + Vec3{ 0.0f, 62.0f, 0.0f };
    effects.SpawnHitFeedback(hitPos, Vec3{ 0.0f, 1.0f, 0.0f }, fromSlam || bodyDmg >= GameConsts::HIT_CRIT_DAMAGE);
    effects.SpawnDamagePopup(hitPos + Vec3{ 0.0f, 30.0f, 0.0f }, bodyDmg, fromSlam);
    if (brokeNow && hp_ > 0.0f) BreakShell(effects);
    if (hp_ > 0.0f && hp_ <= maxHp_ * GameConsts::TURTLE_PHASE2_HP_RATIO) EnterPhase2(effects);
    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
        if (fx_) fx_->ClearAll();
        effects.AddTrauma(1.0f);
        effects.SpawnFishDeath(pos_ + Vec3{ 0.0f, 40.0f, 0.0f }, 130.0f);
        effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 200.0f);
    }
    return true;
}

bool TurtleBoss::TakeDamage(float dmg, EffectSystem& effects) { return TakeDamageInternal(dmg, effects, false); }
bool TurtleBoss::TakeDamageSlam(float dmg, EffectSystem& effects) { return TakeDamageInternal(dmg, effects, true); }

void TurtleBoss::ForceRoll(PlayerFish& player) { if (IsAlive()) StartWithdraw(player); }
void TurtleBoss::ForceBite(PlayerFish& player) { if (IsAlive()) StartBite(player); }
void TurtleBoss::ForceTail(PlayerFish& player) { (void)player; if (IsAlive()) StartTail(); }
void TurtleBoss::ForcePhase2(EffectSystem& effects)
{
    if (!IsAlive()) return;
    hp_ = (std::min)(hp_, maxHp_ * GameConsts::TURTLE_PHASE2_HP_RATIO);
    EnterPhase2(effects);
}

void TurtleBoss::Draw() const
{
    if (!active_) return;
    if (hp_ > 0.0f)
    {
        SetUseLighting(FALSE);
        // Rolling charge telegraph: red lane from the turtle to the arena wall.
        if (action_ == TurtleAction::RollTelegraph || action_ == TurtleAction::Withdraw)
        {
            Vec3 fw = Fwd();
            float lim = ArenaLimit();
            float t = GameConsts::TURTLE_ROLL_MAX_DIST;
            if (fw.x > 0.001f) t = (std::min)(t, (lim - pos_.x) / fw.x);
            else if (fw.x < -0.001f) t = (std::min)(t, (-lim - pos_.x) / fw.x);
            if (fw.z > 0.001f) t = (std::min)(t, (lim - pos_.z) / fw.z);
            else if (fw.z < -0.001f) t = (std::min)(t, (-lim - pos_.z) / fw.z);
            Vec3 end = pos_ + fw * (std::max)(80.0f, t);
            Vec3 side{ -fw.z, 0.0f, fw.x };
            float halfW = GameConsts::TURTLE_HITBOX_RADIUS * 0.9f;
            Vec3 a = pos_ + side * halfW + Vec3{ 0.0f, 2.0f, 0.0f };
            Vec3 b = pos_ - side * halfW + Vec3{ 0.0f, 2.0f, 0.0f };
            Vec3 c = end - side * halfW + Vec3{ 0.0f, 2.0f, 0.0f };
            Vec3 d = end + side * halfW + Vec3{ 0.0f, 2.0f, 0.0f };
            int alpha = (action_ == TurtleAction::Withdraw) ? 70 : 130;
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawTriangle3D(DxConv::ToVECTOR(a), DxConv::ToVECTOR(b), DxConv::ToVECTOR(c), GetColor(220, 60, 50), TRUE);
            DrawTriangle3D(DxConv::ToVECTOR(a), DxConv::ToVECTOR(c), DxConv::ToVECTOR(d), GetColor(220, 60, 50), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            Render3D::DrawCircle3D(end + Vec3{ 0.0f, 2.5f, 0.0f }, GameConsts::TURTLE_HITBOX_RADIUS * 0.9f, 28, GetColor(255, 110, 90), false);
        }
        // Bite telegraph: orange ring where the head will lunge.
        if (action_ == TurtleAction::BiteTelegraph)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
            Render3D::DrawCircle3D(pos_ + Fwd() * 110.0f + Vec3{ 0.0f, 2.0f, 0.0f }, GameConsts::TURTLE_BITE_HIT_RANGE, 28, GetColor(255, 150, 60), true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
        // Tail sweep telegraph: ring around the whole shell.
        if (action_ == TurtleAction::TailTelegraph || action_ == TurtleAction::TailSweep)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, action_ == TurtleAction::TailSweep ? 200 : 120);
            Render3D::DrawCircle3D(pos_ + Vec3{ 0.0f, 2.0f, 0.0f }, GameConsts::TURTLE_TAIL_RADIUS, 36, GetColor(255, 190, 70), action_ == TurtleAction::TailSweep);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
        SetUseLighting(TRUE);

        Render3D::DrawFlatShadow(Vec3{ pos_.x, 0.0f, pos_.z }, GameConsts::TURTLE_HITBOX_RADIUS * 1.05f, 0.85f);
        int handle = LowPolyModels::GetHandle(LowPolyId::TurtleBoss);
        if (handle >= 0)
        {
            // Model: origin at bottom centre, head +Z, shell top ~y66.
            // Procedural animation: whole-body bob/pitch (flipper paddling read),
            // withdrawn squash, and a full forward roll while Rolling.
            float sc = GameConsts::TURTLE_SCALE;
            bool withdrawn = (action_ == TurtleAction::Withdraw || action_ == TurtleAction::RollTelegraph ||
                              action_ == TurtleAction::Rolling || action_ == TurtleAction::RollRecover);
            float squash = withdrawn ? GameConsts::TURTLE_WITHDRAW_SQUASH : 1.0f;
            float bobY = std::sin(bobPhase_) * GameConsts::TURTLE_ANIM_BOB_AMP;
            if (action_ == TurtleAction::Rolling) bobY = 0.0f;
            Vec3 drawPos = pos_ + Vec3{ 0.0f, bobY, 0.0f };
            float idleRoll = std::sin(animTime_ * 0.9f) * GameConsts::TURTLE_ANIM_ROLL_IDLE;
            float idlePitch = std::sin(bobPhase_ * 0.5f) * GameConsts::TURTLE_ANIM_PITCH_AMP;
            float cy = GameConsts::TURTLE_CENTER_Y * sc;
            MATRIX m;
            if (action_ == TurtleAction::Rolling || (action_ == TurtleAction::RollRecover && rollAngle_ > 0.01f))
            {
                // Roll about the shell centre: T(0,-cy,0) -> RotX -> T(0,cy,0) -> yaw -> place.
                m = MMult(MMult(MMult(MMult(MMult(MGetScale(VGet(sc, sc * squash, sc)), MGetTranslate(VGet(0.0f, -cy, 0.0f))), MGetRotX(rollAngle_)), MGetTranslate(VGet(0.0f, cy, 0.0f))), MGetRotY(yaw_)), MGetTranslate(DxConv::ToVECTOR(drawPos)));
            }
            else
            {
                m = MMult(MMult(MMult(MGetScale(VGet(sc, sc * squash, sc)), MGetRotZ(idleRoll)), MGetRotX(idlePitch)), MGetRotY(yaw_));
                m = MMult(m, MGetTranslate(DxConv::ToVECTOR(drawPos)));
            }
            MV1SetMatrix(handle, m);
            COLOR_F tint = phase2_ ? GetColorF(1.25f, 0.85f, 0.75f, 1.0f) : GetColorF(1.0f, 1.0f, 1.0f, 1.0f);
            if (shellBroken_) tint = GetColorF(tint.r * 0.92f, tint.g * 0.88f, tint.b * 0.85f, 1.0f);
            MV1SetDifColorScale(handle, tint);
            float flash = hitFlashTimer_ / GameConsts::HIT_FLASH_TIME;
            HitFlash::DrawModel(handle, flash);
        }
        if (fx_) fx_->Draw();
    }
}
