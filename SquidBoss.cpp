#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "SquidBoss.h"
#include "PlayerFish.h"
#include "Effects.h"
#include "RenderInterp.h"
#include "Render3DUtil.h"
#include "CollisionUtil.h"
#include "DxConv.h"
#include "LowPolyModels.h"
#include "SoundManager.h"
#include "Effects/Hit/HitFeedback.h"
#include "Effects/Squid/SquidFxSet.h"
#include "DxLib.h"

namespace
{
    constexpr float PI = 3.14159265f;
    float Rand01() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }
    float YawOf(const Vec3& d) { return std::atan2(d.x, d.z); }
    float ArenaLimit() { return GameConsts::ARENA_HALF_SIZE - GameConsts::SQUID_HITBOX_RADIUS; }
}

SquidBoss::SquidBoss() : fx_(std::make_unique<SquidFxSet>()) {}
SquidBoss::~SquidBoss() = default;

void SquidBoss::Reset()
{
    if (fx_) fx_->ClearAll();
    active_ = false;
    action_ = SquidAction::Idle;
    enraged_ = false;
    inkTimer_ = 0.0f;
    for (auto& s : inkShots_) s.active = false;
}

void SquidBoss::Spawn(const Vec3& arenaCenter)
{
    Reset();
    active_ = true;
    pos_ = arenaCenter + Vec3{ 0.0f, GameConsts::SQUID_HOVER_HEIGHT, -GameConsts::SQUID_SPAWN_DIST };
    yaw_ = 0.0f;
    prevPos_ = pos_; prevYaw_ = yaw_;
    hp_ = maxHp_;
    fightTimer_ = 0.0f;
    hitFlashTimer_ = 0.0f;
    hoverPhase_ = 0.0f;
    strafeSign_ = (rand() % 2) ? 1.0f : -1.0f;
    strafeTimer_ = GameConsts::SQUID_STRAFE_SWITCH_TIME;
    idleTimer_ = 1.0f;
    burstCount_ = 0;
    lastSkill_ = prevSkill_ = SquidSkill::None;
    inkCooldown_ = 3.0f;
    grabCooldown_ = GameConsts::SQUID_GRAB_FIRST_DELAY;
    inkShotsLeft_ = 0;
}

void SquidBoss::SetBossDifficulty(int bossIndex)
{
    tier_ = (std::max)(1, bossIndex);
    maxHp_ = GameConsts::SQUID_MAX_HP * (1.0f + 0.5f * (tier_ - 1)) * stageHpMul_;
    hp_ = maxHp_;
}

void SquidBoss::SetHpMultiplier(float mul)
{
    stageHpMul_ = mul;
    maxHp_ = GameConsts::SQUID_MAX_HP * (1.0f + 0.5f * (tier_ - 1)) * stageHpMul_;
    hp_ = maxHp_;
}

void SquidBoss::BeginRenderInterp(float alpha)
{
    RenderInterp::Apply(pos_, yaw_, prevPos_, prevYaw_, alpha, savedPos_, savedYaw_);
}

Vec3 SquidBoss::Fwd() const { return Vec3{ std::sin(yaw_), 0.0f, std::cos(yaw_) }; }
Vec3 SquidBoss::GetMantlePos() const { return pos_ + Vec3{ 0.0f, GameConsts::SQUID_WEAK_LOCAL_Y, 0.0f } + Fwd() * GameConsts::SQUID_WEAK_LOCAL_Z; }

void SquidBoss::TurnToward(float targetYaw, float rate, float dt)
{
    float d = targetYaw - yaw_;
    while (d > PI) d -= 2.0f * PI;
    while (d < -PI) d += 2.0f * PI;
    yaw_ += std::clamp(d, -rate * dt, rate * dt);
}

const wchar_t* SquidBoss::SkillName(SquidSkill s)
{
    switch (s)
    {
    case SquidSkill::TentacleSlam: return L"TentacleSlam";
    case SquidSkill::InkSpray: return L"InkSpray";
    case SquidSkill::InkJet: return L"InkJet";
    case SquidSkill::Grab: return L"Grab";
    default: return L"-";
    }
}

void SquidBoss::NoteSkill(SquidSkill s) { prevSkill_ = lastSkill_; lastSkill_ = s; }

void SquidBoss::EndSkill()
{
    burstCount_++;
    int size = enraged_ ? GameConsts::SQUID_BURST_SIZE_ENRAGED : GameConsts::SQUID_BURST_SIZE;
    if (burstCount_ >= size)
    {
        burstCount_ = 0;
        restTimer_ = (enraged_ ? GameConsts::SQUID_REST_TIME_ENRAGED : GameConsts::SQUID_REST_TIME) * (hell_ ? GameConsts::HELL_BOSS_REST_MUL : 1.0f);
        action_ = SquidAction::Rest;
    }
    else
    {
        action_ = SquidAction::Idle;
        idleTimer_ = enraged_ ? GameConsts::SQUID_BURST_GAP_ENRAGED : GameConsts::SQUID_BURST_GAP;
    }
    actionTimer_ = 0.0f;
}

void SquidBoss::StartSlam(PlayerFish& player)
{
    NoteSkill(SquidSkill::TentacleSlam);
    action_ = SquidAction::SlamTelegraph;
    actionTimer_ = 0.0f;
    slamHitDone_ = false;
    slamPos_ = player.GetPosition(); slamPos_.y = 0.0f;
    slamSecond_ = enraged_;
    if (slamSecond_)
    {
        Vec3 side{ -Fwd().z, 0.0f, Fwd().x };
        slamPos2_ = slamPos_ + side * (GameConsts::SQUID_SLAM_COLUMN_RADIUS * 1.6f);
        float lim = ArenaLimit();
        slamPos2_.x = std::clamp(slamPos2_.x, -lim, lim);
        slamPos2_.z = std::clamp(slamPos2_.z, -lim, lim);
        slamPos2_.y = 0.0f;
    }
}

void SquidBoss::StartInkSpray()
{
    NoteSkill(SquidSkill::InkSpray);
    action_ = SquidAction::InkSprayCast;
    actionTimer_ = 0.0f;
    inkCooldown_ = GameConsts::SQUID_INK_COOLDOWN;
}

void SquidBoss::StartInkJet()
{
    NoteSkill(SquidSkill::InkJet);
    action_ = SquidAction::InkJetCast;
    actionTimer_ = 0.0f;
    inkShotTimer_ = 0.0f;
    inkShotsLeft_ = enraged_ ? GameConsts::SQUID_INKJET_COUNT_ENRAGED : GameConsts::SQUID_INKJET_COUNT;
}

void SquidBoss::StartGrab(PlayerFish& player)
{
    NoteSkill(SquidSkill::Grab);
    if (fx_) fx_->grab.Start(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 25.0f, 0.0f } + Fwd() * 80.0f),
                             DxConv::ToVECTOR(player.GetPosition() + Vec3{ 0.0f, 12.0f, 0.0f }),
                             GameConsts::SQUID_GRAB_TELEGRAPH + GameConsts::SQUID_GRAB_PULL_TIME);
    action_ = SquidAction::GrabTelegraph;
    actionTimer_ = 0.0f;
    grabHitDone_ = false;
    grabCooldown_ = GameConsts::SQUID_GRAB_COOLDOWN;
}

void SquidBoss::StartEnrage(EffectSystem& effects)
{
    if (enraged_) return;
    enraged_ = true;
    if (fx_) { fx_->enrageAura.Start(); fx_->enrageAura.SetBoss(DxConv::ToVECTOR(GetMantlePos())); }
    effects.AddTrauma(0.45f);
    effects.SpawnBubbles(pos_ + Vec3{ 0.0f, 35.0f, 0.0f }, 30, 70.0f, 3.0f, 6.0f);
    effects.SpawnShockwave(Vec3{ pos_.x, 0.0f, pos_.z }, 150.0f);
}

void SquidBoss::PickNext(PlayerFish& player)
{
    float dist = Collision::DistXZ(player.GetPosition(), pos_);
    float hpRatio = (player.GetMaxHp() > 0.0f) ? player.GetHp() / player.GetMaxHp() : 1.0f;
    bool grabReady = grabCooldown_ <= 0.0f && (dist <= GameConsts::SQUID_GRAB_RANGE || hpRatio <= GameConsts::SQUID_GRAB_LOW_HP_RATIO);
    const SquidSkill skills[4] = { SquidSkill::TentacleSlam, SquidSkill::InkSpray, SquidSkill::InkJet, SquidSkill::Grab };
    float w[4] = {
        enraged_ ? GameConsts::SQUID_WEIGHT_SLAM_ENRAGED : GameConsts::SQUID_WEIGHT_SLAM,
        (inkCooldown_ <= 0.0f) ? GameConsts::SQUID_WEIGHT_INK_SPRAY : 0.0f,
        GameConsts::SQUID_WEIGHT_INKJET,
        grabReady ? GameConsts::SQUID_WEIGHT_GRAB : 0.0f };
    float total = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        if (skills[i] == lastSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_LAST;
        if (skills[i] == prevSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_PREV;
        if (skills[i] == lastSkill_ && skills[i] == prevSkill_) w[i] = 0.0f;
        total += w[i];
    }
    SquidSkill pick = SquidSkill::TentacleSlam;
    if (total > 0.0f)
    {
        float r = Rand01() * total;
        for (int i = 0; i < 4; ++i) { if (w[i] <= 0.0f) continue; pick = skills[i]; if (r < w[i]) break; r -= w[i]; }
    }
    if (pick == SquidSkill::InkSpray) StartInkSpray();
    else if (pick == SquidSkill::InkJet) StartInkJet();
    else if (pick == SquidSkill::Grab) StartGrab(player);
    else StartSlam(player);
}

void SquidBoss::UpdateHover(float dt, const PlayerFish& player, float speedMul)
{
    hoverPhase_ += dt * GameConsts::SQUID_HOVER_BOB_SPEED;
    strafeTimer_ -= dt;
    if (strafeTimer_ <= 0.0f) { strafeTimer_ = GameConsts::SQUID_STRAFE_SWITCH_TIME; strafeSign_ = -strafeSign_; }
    Vec3 toPlayer = Collision::FlatDiff(player.GetPosition(), pos_);
    float dist = toPlayer.Length();
    Vec3 radial = (dist > 1.0f) ? toPlayer / dist : Fwd();
    TurnToward(YawOf(radial), GameConsts::SQUID_TURN_RATE, dt);
    Vec3 vel{ 0.0f, 0.0f, 0.0f };
    if (dist > GameConsts::SQUID_PREFERRED_RANGE + GameConsts::SQUID_RANGE_TOLERANCE) vel += radial;
    else if (dist < GameConsts::SQUID_PREFERRED_RANGE - GameConsts::SQUID_RANGE_TOLERANCE) vel -= radial;
    Vec3 side{ -radial.z, 0.0f, radial.x };
    vel += side * (strafeSign_ * GameConsts::SQUID_STRAFE_SPEED / GameConsts::SQUID_MOVE_SPEED);
    if (vel.LengthSq() > 1.0f) vel = vel.Normalized();
    pos_ += vel * (GameConsts::SQUID_MOVE_SPEED * speedMul * dt);
    float targetY = GameConsts::SQUID_HOVER_HEIGHT + std::sin(hoverPhase_) * GameConsts::SQUID_HOVER_BOB_AMP;
    pos_.y += (targetY - pos_.y) * (std::min)(1.0f, dt * 3.0f);
    float lim = ArenaLimit();
    pos_.x = std::clamp(pos_.x, -lim, lim);
    pos_.z = std::clamp(pos_.z, -lim, lim);
}

void SquidBoss::UpdateInkShots(float dt, PlayerFish& player, EffectSystem& effects)
{
    for (auto& s : inkShots_)
    {
        if (!s.active) continue;
        s.life += dt;
        if (s.life >= GameConsts::SQUID_INKJET_LIFE) { s.active = false; continue; }
        s.pos += s.vel * dt;
        if (fx_) fx_->inkJet.EmitNode(DxConv::ToVECTOR(s.pos), DxConv::ToVECTOR(s.vel.Normalized()));
        Vec3 target = player.GetPosition() + Vec3{ 0.0f, 12.0f, 0.0f };
        if ((s.pos - target).Length() <= GameConsts::SQUID_INKJET_RADIUS + player.GetHitboxRadius())
        {
            player.TakeDamage(GameConsts::SQUID_INKJET_DAMAGE, effects, &pos_);
            s.active = false;
        }
    }
}
void SquidBoss::Update(float dt, PlayerFish& player, EffectSystem& effects)
{
    if (!active_) return;
    if (hp_ <= 0.0f) return;
    if (fx_) fx_->Update(dt);
    if (enraged_ && fx_) fx_->enrageAura.SetBoss(DxConv::ToVECTOR(GetMantlePos()));
    fightTimer_ += dt;
    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;
    const float cdt = hell_ ? dt * GameConsts::HELL_BOSS_COOLDOWN_SPEED : dt;
    if (inkCooldown_ > 0.0f) inkCooldown_ -= cdt;
    if (grabCooldown_ > 0.0f) grabCooldown_ -= cdt;
    if (inkTimer_ > 0.0f) inkTimer_ -= dt;
    UpdateInkShots(dt, player, effects);

    switch (action_)
    {
    case SquidAction::Idle:
        UpdateHover(dt, player, 1.0f);
        idleTimer_ -= dt;
        if (idleTimer_ <= 0.0f) PickNext(player);
        break;
    case SquidAction::Rest:
        UpdateHover(dt, player, 0.45f);
        restTimer_ -= dt;
        if (restTimer_ <= 0.0f) { action_ = SquidAction::Idle; idleTimer_ = 0.15f; }
        break;
    case SquidAction::SlamTelegraph:
    {
        UpdateHover(dt, player, 0.15f);
        actionTimer_ += dt;
        float tele = enraged_ ? GameConsts::SQUID_SLAM_TELEGRAPH_ENRAGED : GameConsts::SQUID_SLAM_TELEGRAPH;
        // Lock-on follows the player until the final lock window, then the column is fixed.
        if (actionTimer_ < tele - GameConsts::SQUID_SLAM_LOCK_TIME)
        {
            slamPos_ = player.GetPosition(); slamPos_.y = 0.0f;
            if (slamSecond_)
            {
                Vec3 side{ -Fwd().z, 0.0f, Fwd().x };
                slamPos2_ = slamPos_ + side * (GameConsts::SQUID_SLAM_COLUMN_RADIUS * 1.6f);
                float lim = ArenaLimit();
                slamPos2_.x = std::clamp(slamPos2_.x, -lim, lim); slamPos2_.z = std::clamp(slamPos2_.z, -lim, lim); slamPos2_.y = 0.0f;
            }
        }
        if (actionTimer_ >= tele) { action_ = SquidAction::SlamStrike; actionTimer_ = 0.0f; }
        break;
    }
    case SquidAction::SlamStrike:
    {
        actionTimer_ += dt;
        if (!slamHitDone_ && actionTimer_ >= GameConsts::SQUID_SLAM_STRIKE_TIME * 0.5f)
        {
            slamHitDone_ = true;
            effects.AddTrauma(0.35f);
            effects.SpawnShockwave(slamPos_, GameConsts::SQUID_SLAM_COLUMN_RADIUS * 1.5f);
            effects.SpawnWaterSplash(slamPos_, GameConsts::SQUID_SLAM_COLUMN_RADIUS, 1.2f);
            if (fx_) fx_->slam.EmitImpact(DxConv::ToVECTOR(slamPos_), GameConsts::SQUID_SLAM_COLUMN_RADIUS);
            if (slamSecond_ && fx_) fx_->slam.EmitImpact(DxConv::ToVECTOR(slamPos2_), GameConsts::SQUID_SLAM_COLUMN_RADIUS);
            if (slamSecond_) effects.SpawnShockwave(slamPos2_, GameConsts::SQUID_SLAM_COLUMN_RADIUS * 1.5f);
            Vec3 pp = player.GetPosition();
            bool hit = Collision::DistXZ(pp, slamPos_) <= GameConsts::SQUID_SLAM_COLUMN_RADIUS + player.GetHitboxRadius();
            if (slamSecond_ && Collision::DistXZ(pp, slamPos2_) <= GameConsts::SQUID_SLAM_COLUMN_RADIUS + player.GetHitboxRadius()) hit = true;
            // Airborne fish can dodge the column (same rule as other ground slams).
            if (hit && !player.IsAirborne()) player.TakeDamage(GameConsts::SQUID_SLAM_DAMAGE, effects, &pos_);
        }
        if (actionTimer_ >= GameConsts::SQUID_SLAM_STRIKE_TIME) { action_ = SquidAction::SlamRecover; actionTimer_ = 0.0f; }
        break;
    }
    case SquidAction::SlamRecover:
        UpdateHover(dt, player, 0.3f);
        actionTimer_ += dt;
        if (actionTimer_ >= (enraged_ ? GameConsts::SQUID_SLAM_RECOVER_ENRAGED : GameConsts::SQUID_SLAM_RECOVER)) EndSkill();
        break;
    case SquidAction::InkSprayCast:
        UpdateHover(dt, player, 0.25f);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SQUID_INK_CAST_TIME)
        {
            inkTimer_ = GameConsts::SQUID_INK_DARK_TIME;
            player.ApplySlow(GameConsts::SQUID_INK_SLOW_FACTOR, GameConsts::SQUID_INK_SLOW_TIME);
            if (fx_) fx_->inkSpray.Emit(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 25.0f, 0.0f } + Fwd() * 70.0f), DxConv::ToVECTOR(Fwd()));
            EndSkill();
        }
        break;
    case SquidAction::InkJetCast:
    {
        UpdateHover(dt, player, 0.35f);
        inkShotTimer_ -= dt;
        if (inkShotsLeft_ > 0 && inkShotTimer_ <= 0.0f)
        {
            inkShotTimer_ = GameConsts::SQUID_INKJET_INTERVAL;
            inkShotsLeft_--;
            for (auto& s : inkShots_)
            {
                if (s.active) continue;
                Vec3 from = pos_ + Vec3{ 0.0f, 25.0f, 0.0f } + Fwd() * 80.0f;
                Vec3 target = player.GetPosition() + Vec3{ 0.0f, 12.0f, 0.0f };
                Vec3 dir = target - from;
                float len = dir.Length();
                s.pos = from;
                s.vel = (len > 1.0f) ? dir / len * GameConsts::SQUID_INKJET_SPEED : Fwd() * GameConsts::SQUID_INKJET_SPEED;
                s.life = 0.0f; s.active = true;
                if (fx_) fx_->inkJet.EmitNode(DxConv::ToVECTOR(from), DxConv::ToVECTOR(s.vel.Normalized()));
                break;
            }
        }
        if (inkShotsLeft_ <= 0) EndSkill();
        break;
    }
    case SquidAction::GrabTelegraph:
        UpdateHover(dt, player, 0.2f);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SQUID_GRAB_TELEGRAPH) { action_ = SquidAction::GrabPull; actionTimer_ = 0.0f; }
        break;
    case SquidAction::GrabPull:
    {
        UpdateHover(dt, player, 0.15f);
        actionTimer_ += dt;
        if (fx_) fx_->grab.SetTarget(DxConv::ToVECTOR(player.GetPosition() + Vec3{ 0.0f, 12.0f, 0.0f }));
        Vec3 toSquid = Collision::FlatDiff(pos_, player.GetPosition());
        float d = toSquid.Length();
        if (d > 1.0f && !player.IsGrabbed())
            player.AddExternalPull(toSquid / d * GameConsts::SQUID_GRAB_PULL_SPEED);
        if (!grabHitDone_ && d <= GameConsts::SQUID_GRAB_HIT_RANGE)
        {
            grabHitDone_ = true;
            if (!player.IsAirborne()) player.TakeDamage(GameConsts::SQUID_GRAB_DAMAGE, effects, &pos_);
        }
        if (actionTimer_ >= GameConsts::SQUID_GRAB_PULL_TIME) { if (fx_) fx_->grab.Stop(); action_ = SquidAction::GrabRecover; actionTimer_ = 0.0f; }
        break;
    }
    case SquidAction::GrabRecover:
        UpdateHover(dt, player, 0.3f);
        actionTimer_ += dt;
        if (actionTimer_ >= GameConsts::SQUID_GRAB_RECOVER) EndSkill();
        break;
    }
}

int SquidBoss::GetInkOverlayAlpha() const
{
    if (!active_ || inkTimer_ <= 0.0f) return 0;
    float k = (std::min)(1.0f, inkTimer_ / 0.35f);
    return static_cast<int>(GameConsts::SQUID_INK_DARK_ALPHA * k);
}

bool SquidBoss::TakeDamageInternal(float dmg, EffectSystem& effects, bool weak)
{
    if (!active_ || hp_ <= 0.0f) return false;
    dmg *= damageTakenMul_;
    if (weak) dmg *= GameConsts::SQUID_WEAK_DAMAGE_MUL;
    if (action_ == SquidAction::SlamRecover || action_ == SquidAction::GrabRecover) dmg *= GameConsts::SQUID_RECOVER_DAMAGE_MUL;
    hp_ -= dmg;
    hitFlashTimer_ = GameConsts::HIT_FLASH_TIME;
    Vec3 mantle = GetMantlePos();
    effects.SpawnHitFeedback(mantle, Vec3{ 0.0f, 1.0f, 0.0f }, weak || dmg >= GameConsts::HIT_CRIT_DAMAGE);
    effects.SpawnDamagePopup(mantle + Vec3{ 0.0f, 35.0f, 0.0f }, dmg, weak || dmg >= 50.0f);
    if (hp_ > 0.0f && hp_ <= maxHp_ * GameConsts::SQUID_ENRAGE_HP_RATIO) StartEnrage(effects);
    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
        for (auto& s : inkShots_) s.active = false;
        if (fx_) fx_->ClearAll();
        effects.AddTrauma(1.0f);
        effects.SpawnFishDeath(mantle, 120.0f);
    }
    return true;
}

bool SquidBoss::TakeDamage(float dmg, EffectSystem& effects) { return TakeDamageInternal(dmg, effects, false); }

bool SquidBoss::TakeDamageAt(const Vec3& hitPos, float dmg, EffectSystem& effects)
{
    bool weak = Collision::DistXZ(hitPos, GetMantlePos()) <= GameConsts::SQUID_WEAK_RADIUS;
    return TakeDamageInternal(dmg, effects, weak);
}

void SquidBoss::ForceSlam(PlayerFish& player) { if (IsAlive()) StartSlam(player); }
void SquidBoss::ForceInkSpray(PlayerFish& player) { (void)player; if (IsAlive()) StartInkSpray(); }
void SquidBoss::ForceInkJet(PlayerFish& player) { (void)player; if (IsAlive()) StartInkJet(); }
void SquidBoss::ForceGrab(PlayerFish& player) { if (IsAlive()) StartGrab(player); }
void SquidBoss::ForceEnrage(EffectSystem& effects)
{
    if (!IsAlive()) return;
    hp_ = (std::min)(hp_, maxHp_ * GameConsts::SQUID_ENRAGE_HP_RATIO);
    StartEnrage(effects);
}

void SquidBoss::DrawScreenOverlay() const
{
    int alpha = GetInkOverlayAlpha();
    if (alpha <= 0) return;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawBox(0, 0, GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, GetColor(5, 4, 12), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void SquidBoss::Draw() const
{
    if (!active_) return;
    if (hp_ > 0.0f)
    {
        SetUseLighting(FALSE);
        // Tentacle slam telegraph: filled column footprint + shrinking lock ring.
        if (action_ == SquidAction::SlamTelegraph || action_ == SquidAction::SlamStrike)
        {
            float tele = enraged_ ? GameConsts::SQUID_SLAM_TELEGRAPH_ENRAGED : GameConsts::SQUID_SLAM_TELEGRAPH;
            float k = (action_ == SquidAction::SlamStrike) ? 1.0f : std::clamp(actionTimer_ / (std::max)(0.01f, tele), 0.0f, 1.0f);
            const Vec3 targets[2] = { slamPos_, slamPos2_ };
            int count = (action_ == SquidAction::SlamTelegraph && !slamSecond_) ? 1 : (slamSecond_ ? 2 : 1);
            for (int i = 0; i < count; ++i)
            {
                Vec3 g = targets[i] + Vec3{ 0.0f, 1.5f, 0.0f };
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(60 + 120 * k));
                Render3D::DrawCircle3D(g, GameConsts::SQUID_SLAM_COLUMN_RADIUS, 36, GetColor(150, 40, 90), true);
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
                Render3D::DrawCircle3D(g + Vec3{ 0.0f, 0.6f, 0.0f }, GameConsts::SQUID_SLAM_COLUMN_RADIUS * (1.7f - 0.7f * k), 36, GetColor(255, 120, 170), false);
                // Vertical guide lines of the incoming column.
                for (int s = 0; s < 4; ++s)
                {
                    float a = s * 1.5707963f + 0.4f;
                    Vec3 side{ std::cos(a) * GameConsts::SQUID_SLAM_COLUMN_RADIUS, 0.0f, std::sin(a) * GameConsts::SQUID_SLAM_COLUMN_RADIUS };
                    DrawLine3D(DxConv::ToVECTOR(g + side), DxConv::ToVECTOR(g + side + Vec3{ 0.0f, 260.0f, 0.0f }), GetColor(255, 120, 170));
                }
                if (action_ == SquidAction::SlamStrike)
                {
                    float t = std::clamp(actionTimer_ / GameConsts::SQUID_SLAM_STRIKE_TIME, 0.0f, 1.0f);
                    float topY = 280.0f * (1.0f - t);
                    // Tentacle: thick tapered-looking capsule slamming down (fallback visual, no new FX).
                    DrawCapsule3D(DxConv::ToVECTOR(targets[i] + Vec3{ 0.0f, topY + 120.0f, 0.0f }),
                                  DxConv::ToVECTOR(targets[i] + Vec3{ 0.0f, 4.0f, 0.0f }),
                                  GameConsts::SQUID_SLAM_COLUMN_RADIUS * 0.42f, 10, GetColor(150, 55, 95), GetColor(190, 85, 125), TRUE);
                    DrawSphere3D(DxConv::ToVECTOR(targets[i] + Vec3{ 0.0f, 8.0f, 0.0f }), GameConsts::SQUID_SLAM_COLUMN_RADIUS * 0.5f, 12, GetColor(190, 85, 125), GetColor(190, 85, 125), TRUE);
                }
            }
        }
        // Grab telegraph / pull: tentacle line to the player area.
        if (action_ == SquidAction::GrabTelegraph || action_ == SquidAction::GrabPull)
        {
            // Tentacle reach is drawn by Fuyuu SquidGrabTentacleFx (Fx().grab).
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
            Render3D::DrawCircle3D(Vec3{ pos_.x, 1.5f, pos_.z }, GameConsts::SQUID_GRAB_RANGE, 40, GetColor(200, 90, 140), false);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        SetUseLighting(TRUE);

        Render3D::DrawFlatShadow(Vec3{ pos_.x, 0.0f, pos_.z }, 75.0f, 0.85f);
        int handle = LowPolyModels::GetHandle(LowPolyId::SquidBoss);
        if (handle >= 0)
        {
            // Model: origin at bottom, mantle at -Z, arms/tentacles at +Z (forward).
            float sway = std::sin(hoverPhase_ * 0.7f) * 0.05f;
            MATRIX m = MMult(MMult(MGetRotZ(sway), MGetRotY(yaw_)), MGetTranslate(DxConv::ToVECTOR(pos_)));
            MV1SetMatrix(handle, m);
            COLOR_F tint = enraged_ ? GetColorF(1.4f, 0.7f, 0.8f, 1.0f) : GetColorF(1.0f, 1.0f, 1.0f, 1.0f);
            MV1SetDifColorScale(handle, tint);
            float flash = hitFlashTimer_ / GameConsts::HIT_FLASH_TIME;
            HitFlash::DrawModel(handle, flash);
        }
        // Ink projectiles are drawn by Fuyuu SquidInkJetFx nodes (emitted per frame in UpdateInkShots).
        SetUseLighting(TRUE);
    }
    if (fx_) fx_->Draw();
}
