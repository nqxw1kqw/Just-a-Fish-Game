#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "SwordfishBoss.h"
#include "PlayerFish.h"
#include "Effects.h"
#include "RenderInterp.h"
#include "Render3DUtil.h"
#include "CollisionUtil.h"
#include "MapObstacles.h"
#include "DxConv.h"
#include "LowPolyModels.h"
#include "Effects/Hit/HitFeedback.h"
#include "Effects/SharkSand/SharkSandFx.h"
#include "Effects/Swordfish/SwordfishFxSet.h"
#include "DxLib.h"

namespace
{
    constexpr float PI = 3.14159265f;
    float Rand01() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }
    float YawOf(const Vec3& d) { return std::atan2(d.x, d.z); }
    float ArenaLimit() { return GameConsts::ARENA_HALF_SIZE - GameConsts::SWORDFISH_HITBOX_RADIUS; }
}

SwordfishBoss::SwordfishBoss() : sandFx_(std::make_unique<SharkSandEffect>()), swordFx_(std::make_unique<SwordfishFxSet>()) {}
SwordfishBoss::~SwordfishBoss() = default;

void SwordfishBoss::Reset()
{
    if (sandFx_) sandFx_->Clear();
    if (swordFx_) swordFx_->ClearAll();
    active_ = false;
    action_ = SwordfishAction::Idle;
    raged_ = false;
}

void SwordfishBoss::Spawn(const Vec3& arenaCenter)
{
    Reset();
    active_ = true;
    pos_ = arenaCenter + Vec3{ 0.0f, GameConsts::SWORDFISH_HOVER_HEIGHT, -GameConsts::SWORDFISH_SPAWN_DIST };
    yaw_ = 0.0f;
    prevPos_ = pos_; prevYaw_ = yaw_;
    hp_ = maxHp_;
    fightTimer_ = 0.0f;
    hitFlashTimer_ = 0.0f;
    animTime_ = 0.0f;
    idleTimer_ = 1.0f;
    lastSkill_ = prevSkill_ = SwordfishSkill::None;
    rageCooldown_ = GameConsts::SWORDFISH_RAGE_FIRST_DELAY;
    dashPass_ = 0;
    leapY_ = 0.0f;
    visualYaw_ = 0.0f;
}

void SwordfishBoss::SetBossDifficulty(int bossIndex)
{
    tier_ = (std::max)(1, bossIndex);
    maxHp_ = GameConsts::SWORDFISH_MAX_HP * (1.0f + 0.5f * (tier_ - 1)) * stageHpMul_;
    hp_ = maxHp_;
}

void SwordfishBoss::SetHpMultiplier(float mul)
{
    stageHpMul_ = mul;
    maxHp_ = GameConsts::SWORDFISH_MAX_HP * (1.0f + 0.5f * (tier_ - 1)) * stageHpMul_;
    hp_ = maxHp_;
}

void SwordfishBoss::BeginRenderInterp(float alpha)
{
    RenderInterp::Apply(pos_, yaw_, prevPos_, prevYaw_, alpha, savedPos_, savedYaw_);
}

Vec3 SwordfishBoss::Fwd() const { return Vec3{ std::sin(yaw_), 0.0f, std::cos(yaw_) }; }

bool SwordfishBoss::IsVulnerable() const
{
    return action_ == SwordfishAction::DashRecover || action_ == SwordfishAction::SpinStagger || action_ == SwordfishAction::Resurface;
}

void SwordfishBoss::TurnToward(float targetYaw, float rate, float dt)
{
    float d = targetYaw - yaw_;
    while (d > PI) d -= 2.0f * PI;
    while (d < -PI) d += 2.0f * PI;
    yaw_ += std::clamp(d, -rate * dt, rate * dt);
}

const wchar_t* SwordfishBoss::SkillName(SwordfishSkill s)
{
    switch (s)
    {
    case SwordfishSkill::Dash: return L"剣先突進";
    case SwordfishSkill::Spin: return L"回転斬り";
    case SwordfishSkill::Rage: return L"怒りの潜砂";
    default: return L"-";
    }
}

void SwordfishBoss::NoteSkill(SwordfishSkill s) { prevSkill_ = lastSkill_; lastSkill_ = s; }

void SwordfishBoss::EndSkill()
{
    restTimer_ = (raged_ ? GameConsts::SWORDFISH_REST_TIME_RAGE : GameConsts::SWORDFISH_REST_TIME) * (hell_ ? GameConsts::HELL_BOSS_REST_MUL : 1.0f);
    action_ = SwordfishAction::Rest;
    actionTimer_ = 0.0f;
}

void SwordfishBoss::LockDash(const PlayerFish& player, bool second)
{
    dashTarget_ = player.GetPosition();
    dashDir_ = Collision::FlatDiff(dashTarget_, pos_);
    float len = dashDir_.Length();
    dashDir_ = (len > 1.0f) ? dashDir_ / len : Fwd();
    yaw_ = YawOf(dashDir_);
    // Dash THROUGH the locked point plus overshoot, clamped to the arena.
    dashDist_ = len + GameConsts::SWORDFISH_DASH_OVERSHOOT;
    float lim = ArenaLimit();
    Vec3 end = pos_ + dashDir_ * dashDist_;
    if (end.x > lim || end.x < -lim || end.z > lim || end.z < -lim)
    {
        float t = dashDist_;
        if (dashDir_.x > 0.001f) t = (std::min)(t, (lim - pos_.x) / dashDir_.x);
        else if (dashDir_.x < -0.001f) t = (std::min)(t, (-lim - pos_.x) / dashDir_.x);
        if (dashDir_.z > 0.001f) t = (std::min)(t, (lim - pos_.z) / dashDir_.z);
        else if (dashDir_.z < -0.001f) t = (std::min)(t, (-lim - pos_.z) / dashDir_.z);
        dashDist_ = (std::max)(120.0f, t);
    }
    (void)second;
    dashTravel_ = 0.0f;
    dashHitDone_ = false;
    sprayAcc_ = 0.0f;
}

void SwordfishBoss::StartDash()
{
    NoteSkill(SwordfishSkill::Dash);
    dashPass_ = 1;
    action_ = SwordfishAction::DashTelegraph;
    actionTimer_ = 0.0f;
}

void SwordfishBoss::StartSpin()
{
    NoteSkill(SwordfishSkill::Spin);
    visualYaw_ = 0.0f;
    spinHitCd_ = 0.0f;
    if (swordFx_) swordFx_->spinVortex.Start(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 26.0f, 0.0f }), GameConsts::SWORDFISH_SPIN_TIME);
    action_ = SwordfishAction::Spin;
    actionTimer_ = 0.0f;
}

void SwordfishBoss::EnterRage(EffectSystem& effects)
{
    if (raged_) return;
    raged_ = true;
    effects.AddTrauma(0.5f);
    effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 170.0f);
    effects.SpawnBubbles(pos_ + Vec3{ 0.0f, 40.0f, 0.0f }, 30, 80.0f, 3.0f, 6.0f);
    AppLogAdd(L"[SWORDFISH] Rage (<30%% HP)\n");
}

void SwordfishBoss::StartRage(EffectSystem& effects)
{
    NoteSkill(SwordfishSkill::Rage);
    rageCooldown_ = GameConsts::SWORDFISH_RAGE_COOLDOWN;
    stabsLeft_ = GameConsts::SWORDFISH_RAGE_STABS;
    leapY_ = 0.0f;
    action_ = SwordfishAction::RageLeap;
    actionTimer_ = 0.0f;
    effects.SpawnWaterSplash(pos_, 70.0f, 1.0f);
    if (swordFx_) swordFx_->leapArc.Emit(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 20.0f, 0.0f }), DxConv::ToVECTOR(Fwd() * 320.0f + Vec3{ 0.0f, 180.0f, 0.0f })); // Fuyuu leap droplets
}

void SwordfishBoss::PickNext(PlayerFish& player, EffectSystem& effects)
{
    float dist = Collision::DistXZ(player.GetPosition(), pos_);
    const SwordfishSkill skills[3] = { SwordfishSkill::Dash, SwordfishSkill::Spin, SwordfishSkill::Rage };
    float w[3] = {
        GameConsts::SWORDFISH_WEIGHT_DASH,
        (dist <= GameConsts::SWORDFISH_SPIN_HIT_RADIUS + GameConsts::SWORDFISH_SPIN_ADVANCE * GameConsts::SWORDFISH_SPIN_TIME + player.GetHitboxRadius()) ? GameConsts::SWORDFISH_WEIGHT_SPIN : 0.0f,
        (raged_ && rageCooldown_ <= 0.0f) ? GameConsts::SWORDFISH_WEIGHT_RAGE : 0.0f };
    float total = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        if (skills[i] == lastSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_LAST;
        if (skills[i] == prevSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_PREV;
        if (skills[i] == lastSkill_ && skills[i] == prevSkill_) w[i] = 0.0f;
        total += w[i];
    }
    SwordfishSkill pick = SwordfishSkill::Dash;
    if (total > 0.0f)
    {
        float r = Rand01() * total;
        for (int i = 0; i < 3; ++i) { if (w[i] <= 0.0f) continue; pick = skills[i]; if (r < w[i]) break; r -= w[i]; }
    }
    if (pick == SwordfishSkill::Spin) StartSpin();
    else if (pick == SwordfishSkill::Rage) StartRage(effects);
    else StartDash();
}

void SwordfishBoss::UpdateSwim(float dt, const PlayerFish& player, float speedMul)
{
    Vec3 toPlayer = Collision::FlatDiff(player.GetPosition(), pos_);
    float dist = toPlayer.Length();
    Vec3 radial = (dist > 1.0f) ? MapObstacles::ChaseDirection(pos_, player.GetPosition(), GameConsts::SWORDFISH_HITBOX_RADIUS) : Fwd();
    TurnToward(YawOf(radial), GameConsts::SWORDFISH_TURN_RATE, dt);
    float speed = GameConsts::SWORDFISH_MOVE_SPEED * speedMul;
    if (dist > GameConsts::SWORDFISH_PREFERRED_RANGE + GameConsts::SWORDFISH_RANGE_TOLERANCE)
        pos_ = MapObstacles::Move(pos_, radial * (speed * dt), GameConsts::SWORDFISH_HITBOX_RADIUS);
    else if (dist < GameConsts::SWORDFISH_PREFERRED_RANGE - GameConsts::SWORDFISH_RANGE_TOLERANCE)
        pos_ = MapObstacles::Move(pos_, -(radial * (speed * 0.7f * dt)), GameConsts::SWORDFISH_HITBOX_RADIUS);
    else
    {
        Vec3 side{ -radial.z, 0.0f, radial.x };
        pos_ = MapObstacles::Move(pos_, side * (speed * 0.45f * dt), GameConsts::SWORDFISH_HITBOX_RADIUS); // fast circling at range
    }
    float targetY = GameConsts::SWORDFISH_HOVER_HEIGHT + std::sin(animTime_ * 1.7f) * 5.0f;
    pos_.y += (targetY - pos_.y) * (std::min)(1.0f, dt * 3.0f);
    float lim = ArenaLimit();
    pos_.x = std::clamp(pos_.x, -lim, lim);
    pos_.z = std::clamp(pos_.z, -lim, lim);
}

void SwordfishBoss::KnockPlayer(PlayerFish& player, const Vec3& from, float extraPower, EffectSystem& effects, float dmg)
{
    if (player.IsAirborne()) return;
    Vec3 src = from;
    player.TakeDamage(dmg, effects, &src);
    Vec3 away = Collision::FlatDiff(player.GetPosition(), from);
    float len = away.Length();
    away = (len > 1.0f) ? away / len : Vec3{ 0.0f, 0.0f, 1.0f };
    player.ApplyKnockback(away * extraPower); // on top of TakeDamage's own hurt knockback
}

void SwordfishBoss::Update(float dt, PlayerFish& player, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return;
    animTime_ += dt;
    if (hasLastPos_ && dt > 0.0001f)
    {
        Vec3 vel = (pos_ - lastPos_) / dt; vel.y = 0.0f;
        float speed = vel.Length();
        moveSpeedSmooth_ += (speed - moveSpeedSmooth_) * (std::min)(1.0f, dt * 4.0f);
        float wagRate = GameConsts::SWORDFISH_ANIM_WAG_SPEED + moveSpeedSmooth_ * GameConsts::SWORDFISH_ANIM_WAG_PER_SPEED;
        wagPhase_ += dt * wagRate;
        Vec3 fw = Fwd(); Vec3 side{ -fw.z, 0.0f, fw.x };
        float latSpeed = vel.x * side.x + vel.z * side.z;
        float fwdSpeed = vel.x * fw.x + vel.z * fw.z;
        float bankT = std::clamp(-latSpeed * GameConsts::SWORDFISH_ANIM_LEAN_PER_SPEED, -GameConsts::SWORDFISH_ANIM_BANK_MAX, GameConsts::SWORDFISH_ANIM_BANK_MAX);
        float pitchT = std::clamp(fwdSpeed * GameConsts::SWORDFISH_ANIM_LEAN_PER_SPEED * 0.6f, -GameConsts::SWORDFISH_ANIM_PITCH_MAX, GameConsts::SWORDFISH_ANIM_PITCH_MAX);
        float lk = (std::min)(1.0f, dt * GameConsts::SWORDFISH_ANIM_LEAN_SMOOTH);
        bankSmooth_ += (bankT - bankSmooth_) * lk;
        pitchSmooth_ += (pitchT - pitchSmooth_) * lk;
    }
    if (sandFx_) sandFx_->Update(dt);
    if (swordFx_) swordFx_->Update(dt);
    fightTimer_ += dt;
    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;
    if (spinHitCd_ > 0.0f) spinHitCd_ -= dt;
    const float cdt = hell_ ? dt * GameConsts::HELL_BOSS_COOLDOWN_SPEED : dt;
    if (rageCooldown_ > 0.0f) rageCooldown_ -= cdt;

    switch (action_)
    {
    case SwordfishAction::Idle:
        UpdateSwim(dt, player, 1.0f);
        idleTimer_ -= dt;
        if (idleTimer_ <= 0.0f) PickNext(player, effects);
        break;
    case SwordfishAction::Rest:
        UpdateSwim(dt, player, 0.5f);
        restTimer_ -= dt;
        if (restTimer_ <= 0.0f) { action_ = SwordfishAction::Idle; idleTimer_ = GameConsts::SWORDFISH_IDLE_GAP; }
        break;
    case SwordfishAction::DashTelegraph:
    {
        UpdateSwim(dt, player, 0.12f);
        actionTimer_ += dt;
        float tele = (dashPass_ >= 2) ? GameConsts::SWORDFISH_DASH_TELEGRAPH2 : GameConsts::SWORDFISH_DASH_TELEGRAPH;
        if (actionTimer_ < tele - GameConsts::SWORDFISH_DASH_LOCK_TIME) LockDash(player, dashPass_ >= 2);
        if (actionTimer_ >= tele) { LockDash(player, dashPass_ >= 2); action_ = SwordfishAction::Dashing; actionTimer_ = 0.0f; }
        break;
    }
    case SwordfishAction::Dashing:
    {
        float step = GameConsts::SWORDFISH_DASH_SPEED * dt;
        const Vec3 requestedDash = pos_ + dashDir_ * step;
        pos_ = MapObstacles::Move(pos_, dashDir_ * step, GameConsts::SWORDFISH_HITBOX_RADIUS);
        const bool hitReef = Collision::DistSqXZ(pos_, requestedDash) > 0.25f;
        dashTravel_ += step;
        yaw_ = YawOf(dashDir_);
        // Fuyuu dash streak (crowd-lightened by particleScale like Shark sand)
        if (swordFx_ && (effects.particleScale_ >= 1.0f || Rand01() <= effects.particleScale_))
            swordFx_->dashStreak.EmitNode(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 30.0f, 0.0f }), DxConv::ToVECTOR(dashDir_ * GameConsts::SWORDFISH_DASH_SPEED));
        float lim = ArenaLimit();
        pos_.x = std::clamp(pos_.x, -lim, lim);
        pos_.z = std::clamp(pos_.z, -lim, lim);
        // Water spray on BOTH sides of the dashing body.
        sprayAcc_ += step;
        if (sprayAcc_ >= GameConsts::SWORDFISH_DASH_SPRAY_EVERY)
        {
            sprayAcc_ = 0.0f;
            Vec3 side{ -dashDir_.z, 0.0f, dashDir_.x };
            effects.SpawnWaterSplash(pos_ + side * GameConsts::SWORDFISH_DASH_SPRAY_SIDE, 42.0f, 0.8f);
            effects.SpawnWaterSplash(pos_ - side * GameConsts::SWORDFISH_DASH_SPRAY_SIDE, 42.0f, 0.8f);
        }
        if (!dashHitDone_ && !player.IsAirborne() &&
            Collision::DistXZ(player.GetPosition(), pos_) <= GameConsts::SWORDFISH_DASH_HIT_RADIUS + player.GetHitboxRadius())
        {
            dashHitDone_ = true;
            player.TakeDamage(GameConsts::SWORDFISH_DASH_DAMAGE, effects, &pos_);
            effects.AddTrauma(0.25f);
        }
        if (dashTravel_ >= dashDist_ || hitReef)
        {
            if (dashPass_ < GameConsts::SWORDFISH_DASH_PASSES)
            {
                dashPass_++;
                action_ = SwordfishAction::DashPause;
                actionTimer_ = 0.0f;
            }
            else
            {
                action_ = SwordfishAction::DashRecover;
                actionTimer_ = 0.0f;
                effects.SpawnWaterSplash(pos_, 60.0f, 0.8f);
            }
        }
        break;
    }
    case SwordfishAction::DashPause:
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SWORDFISH_DASH_PAUSE)
        {
            action_ = SwordfishAction::DashTelegraph;
            actionTimer_ = 0.0f;
        }
        break;
    case SwordfishAction::DashRecover: // vulnerable window after the 2 stabs
        UpdateSwim(dt, player, 0.30f);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SWORDFISH_DASH_RECOVER) EndSkill();
        break;
    case SwordfishAction::Spin:
    {
        actionTimer_ += dt;
        visualYaw_ += dt * GameConsts::SWORDFISH_SPIN_SPEED;
        if (swordFx_) swordFx_->spinVortex.SetBody(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 26.0f, 0.0f }));
        Vec3 toP = Collision::FlatDiff(player.GetPosition(), pos_);
        float d = toP.Length();
        if (d > 1.0f) pos_ = MapObstacles::Move(pos_, (toP / d) * (GameConsts::SWORDFISH_SPIN_ADVANCE * dt), GameConsts::SWORDFISH_HITBOX_RADIUS);
        yaw_ = YawOf((d > 1.0f) ? toP : Fwd());
        float lim = ArenaLimit();
        pos_.x = std::clamp(pos_.x, -lim, lim);
        pos_.z = std::clamp(pos_.z, -lim, lim);
        if ((static_cast<int>(actionTimer_ / 0.22f) != static_cast<int>((actionTimer_ - dt) / 0.22f)))
        {
            effects.SpawnWaterSplash(pos_ + Fwd() * 40.0f, 46.0f, 0.7f);
            effects.SpawnWaterSplash(pos_ - Fwd() * 40.0f, 46.0f, 0.7f);
        }
        if (spinHitCd_ <= 0.0f && !player.IsAirborne() &&
            Collision::DistXZ(player.GetPosition(), pos_) <= GameConsts::SWORDFISH_SPIN_HIT_RADIUS + player.GetHitboxRadius())
        {
            spinHitCd_ = GameConsts::SWORDFISH_SPIN_HIT_CD;
            KnockPlayer(player, pos_, GameConsts::SWORDFISH_SPIN_KNOCKBACK, effects, GameConsts::SWORDFISH_SPIN_DAMAGE);
            effects.AddTrauma(0.45f);
            effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 110.0f);
        }
        if (actionTimer_ >= GameConsts::SWORDFISH_SPIN_TIME) { if (swordFx_) swordFx_->spinVortex.Stop(); action_ = SwordfishAction::SpinDrift; actionTimer_ = 0.0f; }
        break;
    }
    case SwordfishAction::SpinDrift:
        pos_ = MapObstacles::Move(pos_, Fwd() * (GameConsts::SWORDFISH_SPIN_ADVANCE * 0.8f * dt), GameConsts::SWORDFISH_HITBOX_RADIUS);
        visualYaw_ *= (std::max)(0.0f, 1.0f - dt * 4.0f);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SWORDFISH_SPIN_DRIFT_TIME) { action_ = SwordfishAction::SpinStagger; actionTimer_ = 0.0f; visualYaw_ = 0.0f; }
        break;
    case SwordfishAction::SpinStagger: // self-stun: vulnerable
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SWORDFISH_SPIN_STAGGER) EndSkill();
        break;
    case SwordfishAction::RageLeap:
    {
        actionTimer_ += dt;
        float k = std::clamp(actionTimer_ / GameConsts::SWORDFISH_RAGE_LEAP_TIME, 0.0f, 1.0f);
        leapY_ = std::sin(k * PI) * GameConsts::SWORDFISH_RAGE_LEAP_HEIGHT;
        Vec3 toP = Collision::FlatDiff(player.GetPosition(), pos_);
        if (toP.LengthSq() > 1.0f) TurnToward(YawOf(toP), GameConsts::SWORDFISH_TURN_RATE, dt);
        if (actionTimer_ >= GameConsts::SWORDFISH_RAGE_LEAP_TIME) { action_ = SwordfishAction::RageDive; actionTimer_ = 0.0f; }
        break;
    }
    case SwordfishAction::RageDive:
        actionTimer_ += dt;
        leapY_ -= dt * GameConsts::SWORDFISH_RAGE_DIVE_SPEED;
        if (leapY_ <= -GameConsts::SWORDFISH_SAND_SINK)
        {
            leapY_ = -GameConsts::SWORDFISH_SAND_SINK;
            effects.AddTrauma(0.35f);
            effects.SpawnWaterSplash(pos_, 95.0f, 1.3f);
            effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 130.0f);
            if (sandFx_) sandFx_->SandDiveBurst(DxConv::ToVECTOR(Vec3{ pos_.x, 0.0f, pos_.z })); // Fuyuu sand (SharkSand)
            action_ = SwordfishAction::UnderSand;
            actionTimer_ = 0.0f;
            sandTrailAcc_ = 0.0f;
        }
        break;
    case SwordfishAction::UnderSand:
    {
        actionTimer_ += dt;
        // Stalk: circle the player under the sand, fin tip + sand trail visible.
        Vec3 toP = Collision::FlatDiff(player.GetPosition(), pos_);
        float d = toP.Length();
        Vec3 radial = (d > 1.0f) ? toP / d : Fwd();
        Vec3 side{ -radial.z, 0.0f, radial.x };
        Vec3 vel = radial * ((d > GameConsts::SWORDFISH_SAND_ORBIT_RANGE) ? 1.0f : 0.15f) + side * 0.8f;
        if (vel.LengthSq() > 1.0f) vel = vel.Normalized();
        pos_ += vel * (GameConsts::SWORDFISH_SAND_SPEED * dt); // Burrowing passes below reefs.
        if (sandFx_ && (effects.particleScale_ >= 1.0f || Rand01() <= effects.particleScale_))
            sandFx_->SandFinTrail(DxConv::ToVECTOR(Vec3{ pos_.x, 2.0f, pos_.z }), DxConv::ToVECTOR(vel * GameConsts::SWORDFISH_SAND_SPEED));
        yaw_ = YawOf(vel);
        float lim = ArenaLimit();
        pos_.x = std::clamp(pos_.x, -lim, lim);
        pos_.z = std::clamp(pos_.z, -lim, lim);
        sandTrailAcc_ += dt;
        if (sandTrailAcc_ >= 0.10f)
        {
            sandTrailAcc_ = 0.0f;
            effects.SpawnWaterSplash(Vec3{ pos_.x, 0.0f, pos_.z }, 34.0f, 0.55f);
        }
        if (actionTimer_ >= GameConsts::SWORDFISH_SAND_TIME)
        {
            // Lock a random point near the player for the up-stab.
            float ang = Rand01() * 2.0f * PI;
            float rr = Rand01() * GameConsts::SWORDFISH_STAB_LOCK_RANGE;
            sandTarget_ = player.GetPosition() + Vec3{ std::cos(ang) * rr, 0.0f, std::sin(ang) * rr };
            sandTarget_.x = std::clamp(sandTarget_.x, -lim, lim);
            sandTarget_.z = std::clamp(sandTarget_.z, -lim, lim);
            sandTarget_.y = 0.0f;
            sandTarget_ = MapObstacles::Resolve(sandTarget_, sandTarget_, GameConsts::SWORDFISH_HITBOX_RADIUS);
            if (sandFx_) sandFx_->LockRingDust(DxConv::ToVECTOR(sandTarget_), GameConsts::SWORDFISH_STAB_HIT_RADIUS, GameConsts::SWORDFISH_STAB_TELEGRAPH);
            action_ = SwordfishAction::SandTelegraph;
            actionTimer_ = 0.0f;
        }
        break;
    }
    case SwordfishAction::SandTelegraph:
        actionTimer_ += dt;
        if ((static_cast<int>(actionTimer_ / 0.16f) != static_cast<int>((actionTimer_ - dt) / 0.16f)))
            effects.SpawnWaterSplash(Vec3{ sandTarget_.x, 0.0f, sandTarget_.z }, 40.0f, 0.6f);
        if (actionTimer_ >= GameConsts::SWORDFISH_STAB_TELEGRAPH)
        {
            pos_.x = sandTarget_.x; pos_.z = sandTarget_.z; // hidden by the stab splash
            yaw_ = YawOf(Collision::FlatDiff(player.GetPosition(), pos_));
            action_ = SwordfishAction::StabUp;
            actionTimer_ = 0.0f;
            effects.AddTrauma(0.3f);
            effects.SpawnWaterSplash(Vec3{ sandTarget_.x, 0.0f, sandTarget_.z }, 80.0f, 1.1f);
            if (sandFx_) sandFx_->SandEruptBurst(DxConv::ToVECTOR(sandTarget_), GameConsts::SWORDFISH_STAB_HIT_RADIUS); // Fuyuu sand eruption
            if (!player.IsAirborne() &&
                Collision::DistXZ(player.GetPosition(), sandTarget_) <= GameConsts::SWORDFISH_STAB_HIT_RADIUS + player.GetHitboxRadius())
            {
                KnockPlayer(player, sandTarget_, GameConsts::SWORDFISH_STAB_KNOCKBACK, effects, GameConsts::SWORDFISH_STAB_DAMAGE);
            }
        }
        break;
    case SwordfishAction::StabUp:
        actionTimer_ += dt;
        {
            float k = std::clamp(actionTimer_ / GameConsts::SWORDFISH_STAB_UP_TIME, 0.0f, 1.0f);
            leapY_ = -GameConsts::SWORDFISH_SAND_SINK + (GameConsts::SWORDFISH_STAB_RISE + GameConsts::SWORDFISH_SAND_SINK) * (1.0f - (1.0f - k) * (1.0f - k));
        }
        if (actionTimer_ >= GameConsts::SWORDFISH_STAB_UP_TIME)
        {
            stabsLeft_--;
            leapY_ = 0.0f;
            if (stabsLeft_ > 0)
            {
                // Dive again for the second up-stab.
                action_ = SwordfishAction::RageDive;
                actionTimer_ = 0.0f;
                leapY_ = GameConsts::SWORDFISH_SAND_SINK * 0.2f;
            }
            else
            {
                pos_.y = GameConsts::SWORDFISH_HOVER_HEIGHT;
                action_ = SwordfishAction::Resurface;
                actionTimer_ = 0.0f;
            }
        }
        break;
    case SwordfishAction::Resurface: // vulnerable after rage
        UpdateSwim(dt, player, 0.35f);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SWORDFISH_RAGE_RECOVER) EndSkill();
        break;
    }
    lastPos_ = pos_; hasLastPos_ = true;
}

bool SwordfishBoss::TakeDamageInternal(float dmg, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return false;
    dmg *= damageTakenMul_;
    if (action_ == SwordfishAction::DashRecover || action_ == SwordfishAction::Resurface) dmg *= GameConsts::SWORDFISH_VULN_DMG_MUL;
    if (action_ == SwordfishAction::SpinStagger) dmg *= GameConsts::SWORDFISH_STAGGER_DMG_MUL;
    if (action_ == SwordfishAction::UnderSand || action_ == SwordfishAction::SandTelegraph) dmg *= GameConsts::SWORDFISH_SAND_DMG_MUL;
    hp_ -= dmg;
    hitFlashTimer_ = GameConsts::HIT_FLASH_TIME;
    Vec3 hitPos = pos_ + Vec3{ 0.0f, 40.0f + leapY_, 0.0f };
    effects.SpawnHitFeedback(hitPos, Vec3{ 0.0f, 1.0f, 0.0f }, dmg >= GameConsts::HIT_CRIT_DAMAGE);
    effects.SpawnDamagePopup(hitPos + Vec3{ 0.0f, 30.0f, 0.0f }, dmg, IsVulnerable());
    if (hp_ > 0.0f && hp_ <= maxHp_ * GameConsts::SWORDFISH_RAGE_HP_RATIO) EnterRage(effects);
    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
        if (sandFx_) sandFx_->Clear();
        if (swordFx_) swordFx_->ClearAll();
        effects.AddTrauma(1.0f);
        effects.SpawnFishDeath(pos_ + Vec3{ 0.0f, 35.0f, 0.0f }, 120.0f);
        effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 180.0f);
    }
    return true;
}

bool SwordfishBoss::TakeDamage(float dmg, EffectSystem& effects) { return TakeDamageInternal(dmg, effects); }

bool SwordfishBoss::TakeDamageAt(const Vec3& hitPos, float dmg, EffectSystem& effects)
{
    (void)hitPos; // no weak point; slams just deal damage (bonus during vulnerable windows)
    return TakeDamageInternal(dmg, effects);
}

void SwordfishBoss::ForceDash(PlayerFish& player) { (void)player; if (IsAlive()) StartDash(); }
void SwordfishBoss::ForceSpin(PlayerFish& player) { (void)player; if (IsAlive()) StartSpin(); }
void SwordfishBoss::ForceRage(EffectSystem& effects)
{
    if (!IsAlive()) return;
    hp_ = (std::min)(hp_, maxHp_ * GameConsts::SWORDFISH_RAGE_HP_RATIO);
    EnterRage(effects);
    StartRage(effects);
}

void SwordfishBoss::Draw() const
{
    if (!active_) return;
    if (hp_ > 0.0f)
    {
        SetUseLighting(FALSE);
        // Dash telegraph: red lane through the locked target.
        if (action_ == SwordfishAction::DashTelegraph)
        {
            Vec3 fw = dashDir_;
            Vec3 end = pos_ + fw * dashDist_;
            Vec3 side{ -fw.z, 0.0f, fw.x };
            float halfW = GameConsts::SWORDFISH_DASH_HIT_RADIUS * 0.9f;
            Vec3 a = pos_ + side * halfW + Vec3{ 0.0f, 2.0f - pos_.y, 0.0f };
            Vec3 b = pos_ - side * halfW + Vec3{ 0.0f, 2.0f - pos_.y, 0.0f };
            Vec3 c = end - side * halfW + Vec3{ 0.0f, 2.0f - pos_.y, 0.0f };
            Vec3 d = end + side * halfW + Vec3{ 0.0f, 2.0f - pos_.y, 0.0f };
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);
            DrawTriangle3D(DxConv::ToVECTOR(a), DxConv::ToVECTOR(b), DxConv::ToVECTOR(c), GetColor(230, 70, 60), TRUE);
            DrawTriangle3D(DxConv::ToVECTOR(a), DxConv::ToVECTOR(c), DxConv::ToVECTOR(d), GetColor(230, 70, 60), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            Render3D::DrawCircle3D(Vec3{ dashTarget_.x, 2.5f, dashTarget_.z }, 46.0f, 24, GetColor(255, 120, 100), false);
        }
        // Spin danger ring.
        if (action_ == SwordfishAction::Spin)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);
            Render3D::DrawCircle3D(Vec3{ pos_.x, 2.0f, pos_.z }, GameConsts::SWORDFISH_SPIN_HIT_RADIUS, 32, GetColor(255, 170, 60), false);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
        // Under-sand stalk: bump ring + the locked stab point warning.
        if (action_ == SwordfishAction::UnderSand)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
            Render3D::DrawCircle3D(Vec3{ pos_.x, 2.0f, pos_.z }, 55.0f, 24, GetColor(215, 180, 110), true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
        if (action_ == SwordfishAction::SandTelegraph)
        {
            float k = std::clamp(actionTimer_ / GameConsts::SWORDFISH_STAB_TELEGRAPH, 0.0f, 1.0f);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(80 + 140 * k));
            Render3D::DrawCircle3D(Vec3{ sandTarget_.x, 2.5f, sandTarget_.z }, GameConsts::SWORDFISH_STAB_HIT_RADIUS * (1.4f - 0.4f * k), 28, GetColor(255, 90, 70), true);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
        SetUseLighting(TRUE);

        bool underSand = (action_ == SwordfishAction::UnderSand || action_ == SwordfishAction::SandTelegraph || action_ == SwordfishAction::RageDive);
        Render3D::DrawFlatShadow(Vec3{ pos_.x, 0.0f, pos_.z }, underSand ? 60.0f : 78.0f, 0.8f);
        int handle = LowPolyModels::GetHandle(LowPolyId::SwordfishBoss);
        if (handle >= 0)
        {
            // Model: origin bottom-centre, bill +Z, tail -Z, body centre ~y34.
            float sc = GameConsts::SWORDFISH_SCALE;
            float speedK = std::clamp(moveSpeedSmooth_ / GameConsts::SWORDFISH_ANIM_WAG_SPEED_REF, 0.0f, 1.0f);
            float wag = std::sin(wagPhase_) * GameConsts::SWORDFISH_ANIM_WAG_AMP * (0.35f + 0.65f * speedK);
            float drawYaw = yaw_ + wag + visualYaw_;
            float pitch = pitchSmooth_;
            if (action_ == SwordfishAction::RageLeap) pitch += -0.45f * std::sin(std::clamp(actionTimer_ / GameConsts::SWORDFISH_RAGE_LEAP_TIME, 0.0f, 1.0f) * PI);
            if (action_ == SwordfishAction::RageDive) pitch += 0.7f;
            if (action_ == SwordfishAction::StabUp) pitch += -0.9f; // bill points up out of the sand
            if (action_ == SwordfishAction::SpinStagger) pitch += 0.25f;
            Vec3 drawPos = pos_ + Vec3{ 0.0f, leapY_, 0.0f };
            if (underSand) drawPos.y = -GameConsts::SWORDFISH_FIN_SINK_DRAW; // only dorsal fin + back above the seabed
            MATRIX m = MMult(MMult(MMult(MGetScale(VGet(sc, sc, sc)), MGetRotZ(bankSmooth_)), MGetRotX(pitch)), MGetRotY(drawYaw));
            m = MMult(m, MGetTranslate(DxConv::ToVECTOR(drawPos)));
            MV1SetMatrix(handle, m);
            COLOR_F tint = raged_ ? GetColorF(1.35f, 0.75f, 0.7f, 1.0f) : GetColorF(1.0f, 1.0f, 1.0f, 1.0f);
            MV1SetDifColorScale(handle, tint);
            float flash = hitFlashTimer_ / GameConsts::HIT_FLASH_TIME;
            HitFlash::DrawModel(handle, flash);
        }
        if (sandFx_) sandFx_->Draw();
        if (swordFx_) swordFx_->Draw();
    }
}
