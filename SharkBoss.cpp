#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "SharkBoss.h"
#include "PlayerFish.h"
#include "Effects.h"
#include "RenderInterp.h"
#include "Render3DUtil.h"
#include "CollisionUtil.h"
#include "DxConv.h"
#include "LowPolyModels.h"
#include "SoundManager.h"
#include "Effects/Hit/HitFeedback.h"
#include "Effects/Shark/SharkFxSet.h"
#include "DxLib.h"

namespace
{
    constexpr float PI = 3.14159265f;
    Vec3 YawDir(float yaw) { return Vec3{ std::sin(yaw), 0.0f, std::cos(yaw) }; }
    float YawOf(const Vec3& d) { return std::atan2(d.x, d.z); }
    float Rand01() { return static_cast<float>(rand()) / static_cast<float>(RAND_MAX); }
    float ArenaLimit() { return GameConsts::ARENA_HALF_SIZE - GameConsts::SHARK_HITBOX_RADIUS; }
}

SharkBoss::SharkBoss() : fx_(std::make_unique<SharkFxSet>()) {}
SharkBoss::~SharkBoss() = default;

void SharkBoss::SetBossDifficulty(int bossIndex)
{
    tier_ = (std::max)(1, bossIndex);
    maxHp_ = GameConsts::SHARK_MAX_HP * (1.0f + 0.5f * (tier_ - 1)); // cùng công thức với cá voi
    hp_ = maxHp_;
}

void SharkBoss::Reset()
{
    active_ = false;
    hp_ = maxHp_;
    action_ = SharkAction::Idle;
    vortexTimer_ = 0.0f;
    frenzy_ = false;
    if (fx_) fx_->ClearAll();
    SoundManager::Instance().StopSFX(SfxId::BossInhaleLoop);
}

void SharkBoss::Spawn(const Vec3& arenaCenter)
{
    Reset();
    active_ = true;
    pos_ = arenaCenter + Vec3{ 0.0f, 0.0f, 380.0f };
    yaw_ = PI; // nhìn về tâm
    prevPos_ = pos_; prevYaw_ = yaw_;
    hp_ = maxHp_;
    fightTimer_ = 0.0f;
    hitFlashTimer_ = 0.0f;
    action_ = SharkAction::Idle;
    idleTimer_ = 1.2f;
    lastSkill_ = prevSkill_ = SharkSkill::None;
    burstCount_ = 0;
    restTimer_ = 0.0f;
    chainBitesLeft_ = 0;
    vortexCooldown_ = 2.0f;
    vortexTimer_ = 0.0f;
    stunTimer_ = 0.0f;
    finHitCd_ = 0.0f;
    hasLastPlayerPos_ = false;
    playerVel_ = Vec3{ 0.0f, 0.0f, 0.0f };
}

void SharkBoss::BeginRenderInterp(float alpha)
{
    RenderInterp::Apply(pos_, yaw_, prevPos_, prevYaw_, alpha, savedPos_, savedYaw_);
}

Vec3 SharkBoss::Fwd() const { return YawDir(yaw_); }
Vec3 SharkBoss::MouthPos() const { return pos_ + Fwd() * GameConsts::SHARK_MOUTH_FWD + Vec3{ 0.0f, 25.0f, 0.0f }; }

Vec3 SharkBoss::PredictPlayerPos(const PlayerFish& player, float leadTime) const
{
    Vec3 p = player.GetPosition() + playerVel_ * (leadTime * GameConsts::AIM_LEAD_SCALE);
    float lim = GameConsts::ARENA_HALF_SIZE - GameConsts::AIM_ARENA_MARGIN;
    p.x = std::clamp(p.x, -lim, lim);
    p.z = std::clamp(p.z, -lim, lim);
    p.y = 0.0f;
    return p;
}

void SharkBoss::TurnToward(float targetYaw, float rate, float dt)
{
    float d = targetYaw - yaw_;
    while (d > PI) d -= 2.0f * PI;
    while (d < -PI) d += 2.0f * PI;
    float step = rate * dt;
    yaw_ += std::clamp(d, -step, step);
}

const wchar_t* SharkBoss::SkillName(SharkSkill s)
{
    switch (s)
    {
    case SharkSkill::Bite: return L"Bite";
    case SharkSkill::Strafe: return L"Strafe";
    case SharkSkill::Vortex: return L"Vortex";
    default: return L"-";
    }
}

void SharkBoss::NoteSkill(SharkSkill s)
{
    prevSkill_ = lastSkill_;
    lastSkill_ = s;
}

void SharkBoss::EndSkill()
{
    burstCount_++;
    int size = frenzy_ ? GameConsts::SHARK_BURST_SIZE_FRENZY : GameConsts::SHARK_BURST_SIZE;
    if (burstCount_ >= size)
    {
        burstCount_ = 0;
        restTimer_ = frenzy_ ? GameConsts::SHARK_REST_TIME_FRENZY : GameConsts::SHARK_REST_TIME;
        action_ = SharkAction::Rest;
    }
    else
    {
        action_ = SharkAction::Idle;
        idleTimer_ = frenzy_ ? GameConsts::SHARK_BURST_GAP_FRENZY : GameConsts::SHARK_BURST_GAP;
    }
}

void SharkBoss::PickNext(PlayerFish& player)
{
    const SharkSkill skills[3] = { SharkSkill::Bite, SharkSkill::Strafe, SharkSkill::Vortex };
    float w[3] = {
        frenzy_ ? GameConsts::SHARK_WEIGHT_BITE_FRENZY : GameConsts::SHARK_WEIGHT_BITE,
        GameConsts::SHARK_WEIGHT_STRAFE,
        (vortexTimer_ <= 0.0f && vortexCooldown_ <= 0.0f) ? GameConsts::SHARK_WEIGHT_VORTEX : 0.0f
    };
    float total = 0.0f;
    for (int i = 0; i < 3; ++i)
    {
        if (skills[i] == lastSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_LAST;
        if (skills[i] == prevSkill_) w[i] *= GameConsts::AI_REPEAT_WEIGHT_PREV;
        if (skills[i] == lastSkill_ && skills[i] == prevSkill_) w[i] = 0.0f; // không 3 lần liên tiếp
        total += w[i];
    }
    SharkSkill pick = SharkSkill::Bite;
    if (total > 0.0f)
    {
        float r = Rand01() * total;
        for (int i = 0; i < 3; ++i)
        {
            if (w[i] <= 0.0f) continue;
            pick = skills[i];
            if (r < w[i]) break;
            r -= w[i];
        }
    }
    if (pick == SharkSkill::Bite)
    {
        NoteSkill(SharkSkill::Bite);
        chainBitesLeft_ = frenzy_ ? (GameConsts::SHARK_FRENZY_BITES_MIN - 1 +
            rand() % (GameConsts::SHARK_FRENZY_BITES_MAX - GameConsts::SHARK_FRENZY_BITES_MIN + 1)) : 0;
        StartBite(player, frenzy_ ? GameConsts::SHARK_BITE_TELEGRAPH_FRENZY : GameConsts::SHARK_BITE_TELEGRAPH);
    }
    else if (pick == SharkSkill::Strafe) StartStrafe(player);
    else StartVortex();
}

void SharkBoss::StartBite(PlayerFish& player, float telegraph)
{
    action_ = SharkAction::BiteTelegraph;
    actionTimer_ = 0.0f;
    biteTelegraph_ = telegraph;
    biteHit_ = false;
    lungeTraveled_ = 0.0f;
    float travel = GameConsts::SHARK_BITE_RANGE / GameConsts::SHARK_BITE_SPEED;
    Vec3 to = Collision::FlatDiff(PredictPlayerPos(player, telegraph + travel), pos_);
    if (to.LengthSq() > 1.0f) lungeDir_ = to * (1.0f / to.Length());
    Fx().bite.Start(DxConv::ToVECTOR(pos_), DxConv::ToVECTOR(lungeDir_), telegraph, GameConsts::SHARK_BITE_RANGE);
    SoundManager::Instance().PlaySFX(SfxId::BossDiveCharge);
}

void SharkBoss::StartStrafe(PlayerFish& player)
{
    NoteSkill(SharkSkill::Strafe);
    action_ = SharkAction::Strafe;
    Vec3 from = Collision::FlatDiff(pos_, player.GetPosition());
    strafeAngle_ = std::atan2(from.x, from.z);
    strafeSwept_ = 0.0f;
    strafeSign_ = (rand() % 2) ? 1.0f : -1.0f;
    finHitCd_ = 0.0f;
    Fx().fin.Start();
    SoundManager::Instance().PlaySFX(SfxId::BossTailSlap); // vẫy đuôi tăng tốc
}

void SharkBoss::StartVortex()
{
    NoteSkill(SharkSkill::Vortex);
    action_ = SharkAction::VortexCast;
    actionTimer_ = 0.0f;
}

void SharkBoss::Stun(float duration)
{
    action_ = SharkAction::Stunned;
    stunTimer_ = duration;
    chainBitesLeft_ = 0;
    SoundManager::Instance().PlaySFX(SfxId::BossStun);
}

void SharkBoss::StartFrenzy(EffectSystem& effects)
{
    if (frenzy_) return;
    frenzy_ = true;
    effects.AddTrauma(0.7f);
    effects.SpawnShockwave(pos_, 240.0f);
    Fx().rage.Start(GameConsts::SHARK_HITBOX_RADIUS);
    SoundManager::Instance().PlaySFX(SfxId::BossRoar);
}

void SharkBoss::UpdateVortex(float dt, PlayerFish& player, EffectSystem& effects)
{
    if (vortexTimer_ <= 0.0f) return;
    vortexTimer_ -= dt;
    vortexTickCd_ -= dt;
    const Vec3& pp = player.GetPosition();
    Vec3 to = Collision::FlatDiff(vortexCenter_, pp);
    float d = to.Length();
    if (!player.IsAirborne() && d < GameConsts::SHARK_VORTEX_RADIUS && d > 1.0f)
    {
        // Kéo nhẹ: mạnh ở tâm, yếu dần ra mép -> vẫn bơi ra được
        float k = 0.35f + 0.65f * (1.0f - d / GameConsts::SHARK_VORTEX_RADIUS);
        player.AddExternalPull(to * (GameConsts::SHARK_VORTEX_PULL * k / d));
    }
    if (d < GameConsts::SHARK_VORTEX_CORE_RADIUS && vortexTickCd_ <= 0.0f)
    {
        if (player.TakeDamage(GameConsts::SHARK_VORTEX_CORE_DAMAGE, effects, &vortexCenter_))
            vortexTickCd_ = GameConsts::SHARK_VORTEX_TICK;
    }
    if (vortexTimer_ <= 0.0f)
    {
        vortexTimer_ = 0.0f;
        SoundManager::Instance().StopSFX(SfxId::BossInhaleLoop);
    }
}

void SharkBoss::Update(float dt, PlayerFish& player, EffectSystem& effects)
{
    if (!active_) return;
    if (fx_) fx_->Update(dt);
    if (hp_ <= 0.0f) return;

    fightTimer_ += dt;
    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;
    if (vortexCooldown_ > 0.0f) vortexCooldown_ -= dt;
    if (finHitCd_ > 0.0f) finHitCd_ -= dt;

    // Vận tốc người chơi từ chênh lệch khung (EMA, như cá voi)
    const Vec3& pp = player.GetPosition();
    if (dt > 0.0001f)
    {
        if (hasLastPlayerPos_)
        {
            Vec3 v{ (pp.x - lastPlayerPos_.x) / dt, 0.0f, (pp.z - lastPlayerPos_.z) / dt };
            if (v.Length() <= GameConsts::AIM_MAX_PLAYER_SPEED)
                playerVel_ = playerVel_ * (1.0f - GameConsts::AIM_VEL_SMOOTHING) + v * GameConsts::AIM_VEL_SMOOTHING;
        }
        lastPlayerPos_ = pp;
        hasLastPlayerPos_ = true;
    }

    UpdateVortex(dt, player, effects);
    tailSpeedMul_ = 1.0f;
    float targetBank = 0.0f;

    Vec3 toPlayer = Collision::FlatDiff(pp, pos_);
    float distPlayer = toPlayer.Length();

    switch (action_)
    {
    case SharkAction::Idle:
    case SharkAction::Rest:
    {
        // Lượn vòng quanh người chơi ở khoảng cách ưa thích; khi nghỉ thì chậm và đuôi lười
        bool rest = (action_ == SharkAction::Rest);
        float speed = rest ? GameConsts::SHARK_REST_DRIFT_SPEED : GameConsts::SHARK_SWIM_SPEED;
        Vec3 radial = (distPlayer > 1.0f) ? toPlayer * (1.0f / distPlayer) : Fwd();
        Vec3 tangent{ radial.z, 0.0f, -radial.x };
        float pull = std::clamp((distPlayer - GameConsts::SHARK_PREFERRED_DIST) / 150.0f, -1.0f, 1.0f);
        Vec3 desire = tangent + radial * pull;
        TurnToward(YawOf(desire), GameConsts::SHARK_TURN_RATE * (rest ? 0.5f : 1.0f), dt);
        pos_ += Fwd() * (speed * dt);
        tailSpeedMul_ = rest ? 0.45f : 1.0f;
        if (rest)
        {
            restTimer_ -= dt;
            if (restTimer_ <= 0.0f) { restTimer_ = 0.0f; action_ = SharkAction::Idle; idleTimer_ = 0.3f; }
        }
        else
        {
            idleTimer_ -= dt;
            if (idleTimer_ <= 0.0f) PickNext(player);
        }
        break;
    }
    case SharkAction::BiteTelegraph:
    {
        actionTimer_ += dt;
        // Bám mục tiêu (đón đầu) tới SHARK_BITE_LOCK_TIME trước khi lao
        if (actionTimer_ < biteTelegraph_ - GameConsts::SHARK_BITE_LOCK_TIME)
        {
            float travel = GameConsts::SHARK_BITE_RANGE / GameConsts::SHARK_BITE_SPEED;
            Vec3 to = Collision::FlatDiff(PredictPlayerPos(player, (biteTelegraph_ - actionTimer_) + travel), pos_);
            if (to.LengthSq() > 1.0f) lungeDir_ = to * (1.0f / to.Length());
        }
        TurnToward(YawOf(lungeDir_), GameConsts::SHARK_TURN_RATE * 3.0f, dt);
        pos_ -= lungeDir_ * (25.0f * dt); // lùi nhẹ lấy đà
        tailSpeedMul_ = 2.5f;
        Fx().bite.SetShark(DxConv::ToVECTOR(pos_), DxConv::ToVECTOR(lungeDir_));
        if (actionTimer_ >= biteTelegraph_)
        {
            action_ = SharkAction::BiteLunge;
            yaw_ = YawOf(lungeDir_);
            SoundManager::Instance().PlaySFX(SfxId::BossTailSlap); // whoosh
        }
        break;
    }
    case SharkAction::BiteLunge:
    {
        float step = GameConsts::SHARK_BITE_SPEED * dt;
        pos_ += lungeDir_ * step;
        lungeTraveled_ += step;
        tailSpeedMul_ = 3.0f;
        Fx().bite.SetShark(DxConv::ToVECTOR(pos_), DxConv::ToVECTOR(lungeDir_));
        Vec3 mouth = MouthPos();
        if (!biteHit_ && Collision::DistXZ(mouth, pp) <= GameConsts::SHARK_BITE_HIT_RADIUS + player.GetHitboxRadius())
        {
            biteHit_ = true;
            if (player.TakeDamage(GameConsts::SHARK_BITE_DAMAGE, effects, &pos_))
            {
                Fx().bite.Bite(DxConv::ToVECTOR(mouth));
                effects.AddTrauma(0.35f);
            }
        }
        float lim = ArenaLimit();
        if (std::fabs(pos_.x) > lim || std::fabs(pos_.z) > lim)
        {
            pos_.x = std::clamp(pos_.x, -lim, lim);
            pos_.z = std::clamp(pos_.z, -lim, lim);
            effects.AddTrauma(0.4f);
            effects.SpawnShockwave(mouth, 90.0f);
            SoundManager::Instance().PlaySFX(SfxId::BossHeavySlam);
            Stun(GameConsts::SHARK_WALL_STUN); // đâm tường -> choáng (kết thúc chuỗi)
            break;
        }
        if (lungeTraveled_ >= GameConsts::SHARK_BITE_RANGE)
        {
            if (chainBitesLeft_ > 0)
            {
                chainBitesLeft_--; // Frenzy: nối cú cắn tiếp theo (cả chuỗi tính 1 bước burst)
                StartBite(player, GameConsts::SHARK_FRENZY_CHAIN_TELEGRAPH);
            }
            else
            {
                action_ = SharkAction::BiteRecover;
                actionTimer_ = 0.0f;
            }
        }
        break;
    }
    case SharkAction::BiteRecover:
        actionTimer_ += dt;
        pos_ += Fwd() * (GameConsts::SHARK_SWIM_SPEED * 0.4f * dt);
        if (actionTimer_ >= GameConsts::SHARK_BITE_RECOVER) EndSkill();
        break;
    case SharkAction::Strafe:
    {
        float ang = frenzy_ ? GameConsts::SHARK_STRAFE_ANG_SPEED_FRENZY : GameConsts::SHARK_STRAFE_ANG_SPEED;
        strafeAngle_ += strafeSign_ * ang * dt;
        strafeSwept_ += ang * dt;
        Vec3 target = pp + Vec3{ std::sin(strafeAngle_), 0.0f, std::cos(strafeAngle_) } * GameConsts::SHARK_STRAFE_RADIUS;
        Vec3 delta = Collision::FlatDiff(target, pos_);
        Vec3 oldPos = pos_;
        float maxStep = (GameConsts::SHARK_STRAFE_RADIUS * ang + 200.0f) * dt;
        float dl = delta.Length();
        pos_ += (dl > maxStep) ? delta * (maxStep / dl) : delta;
        Vec3 vel = (pos_ - oldPos) * (1.0f / (std::max)(dt, 0.0001f));
        if (vel.LengthSq() > 1.0f) TurnToward(YawOf(vel), GameConsts::SHARK_TURN_RATE * 2.0f, dt);
        targetBank = -0.3f * strafeSign_;
        tailSpeedMul_ = 1.8f;
        Vec3 finPos = pos_ + Vec3{ 0.0f, 60.0f, 0.0f } - Fwd() * 20.0f;
        Fx().fin.SetFin(DxConv::ToVECTOR(finPos), DxConv::ToVECTOR(vel));
        // Chạm thân/vây khi đang lượn
        if (finHitCd_ <= 0.0f && distPlayer <= GameConsts::SHARK_HITBOX_RADIUS + player.GetHitboxRadius())
        {
            if (player.TakeDamage(GameConsts::SHARK_FIN_DAMAGE, effects, &pos_)) finHitCd_ = GameConsts::SHARK_FIN_HIT_COOLDOWN;
        }
        if (strafeSwept_ >= GameConsts::SHARK_STRAFE_LAPS * 2.0f * PI)
        {
            Fx().fin.Stop();
            chainBitesLeft_ = 0;
            StartBite(player, GameConsts::SHARK_STRAFE_BITE_TELEGRAPH); // ngoặt vào cắn
        }
        break;
    }
    case SharkAction::VortexCast:
    {
        actionTimer_ += dt;
        yaw_ += 9.0f * dt; // xoay tròn quất đuôi
        tailSpeedMul_ = 4.0f;
        if (actionTimer_ >= GameConsts::SHARK_VORTEX_CAST_TIME)
        {
            Vec3 c = pos_ - Fwd() * GameConsts::SHARK_VORTEX_BACK;
            // Ưu tiên đặt xoáy về phía người chơi nếu ở gần
            Vec3 toP = Collision::FlatDiff(pp, pos_);
            if (toP.Length() < GameConsts::SHARK_VORTEX_RADIUS * 1.5f) c = pos_ + toP * 0.6f;
            float lim = GameConsts::ARENA_HALF_SIZE - GameConsts::SHARK_VORTEX_RADIUS * 0.5f;
            c.x = std::clamp(c.x, -lim, lim);
            c.z = std::clamp(c.z, -lim, lim);
            c.y = 0.0f;
            vortexCenter_ = c;
            vortexTimer_ = GameConsts::SHARK_VORTEX_DURATION;
            vortexTickCd_ = 0.0f;
            vortexCooldown_ = GameConsts::SHARK_VORTEX_COOLDOWN;
            Fx().vortex.Start(DxConv::ToVECTOR(c), GameConsts::SHARK_VORTEX_RADIUS, GameConsts::SHARK_VORTEX_DURATION);
            SoundManager::Instance().PlaySFXLoop(SfxId::BossInhaleLoop); // tiếng nước xoáy
            effects.SpawnWaterSplash(c, GameConsts::SHARK_VORTEX_RADIUS * 0.5f, 1.2f);
            EndSkill();
        }
        break;
    }
    case SharkAction::Stunned:
        stunTimer_ -= dt;
        tailSpeedMul_ = 0.3f;
        if (stunTimer_ <= 0.0f) EndSkill();
        break;
    }

    bank_ += (targetBank - bank_) * (std::min)(1.0f, dt * 6.0f);
    tailPhase_ += dt * GameConsts::SHARK_TAIL_FREQ * 2.0f * PI * tailSpeedMul_;

    float lim = ArenaLimit();
    pos_.x = std::clamp(pos_.x, -lim, lim);
    pos_.z = std::clamp(pos_.z, -lim, lim);
    if (frenzy_) Fx().rage.SetShark(DxConv::ToVECTOR(pos_ + Vec3{ 0.0f, 30.0f, 0.0f }));
}

bool SharkBoss::TakeDamage(float dmg, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return false;
    if (action_ == SharkAction::Stunned) dmg *= GameConsts::SHARK_STUN_DAMAGE_MUL;
    hp_ -= dmg;
    hitFlashTimer_ = GameConsts::HIT_FLASH_TIME;
    effects.SpawnHitFeedback(pos_ + Vec3{ 0.0f, 30.0f, 0.0f }, Vec3{ 0.0f, 1.0f, 0.0f }, dmg >= GameConsts::HIT_CRIT_DAMAGE);
    effects.SpawnDamagePopup(pos_ + Vec3{ 0.0f, 45.0f, 0.0f }, dmg, dmg >= 50.0f);
    if (hp_ > 0.0f && hp_ <= maxHp_ * GameConsts::SHARK_FRENZY_HP_RATIO) StartFrenzy(effects);
    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
        effects.AddTrauma(1.0f);
        effects.SpawnFishDeath(pos_ + Vec3{ 0.0f, 30.0f, 0.0f }, 120.0f);
        if (fx_) { fx_->rage.Stop(); fx_->fin.Stop(); }
        vortexTimer_ = 0.0f;
        SoundManager::Instance().StopSFX(SfxId::BossInhaleLoop);
    }
    return true;
}

void SharkBoss::ForceBite(PlayerFish& player) { if (!IsAlive()) return; NoteSkill(SharkSkill::Bite); chainBitesLeft_ = 0; StartBite(player, GameConsts::SHARK_BITE_TELEGRAPH); }
void SharkBoss::ForceStrafe(PlayerFish& player) { if (IsAlive()) StartStrafe(player); }
void SharkBoss::ForceVortex(PlayerFish& player) { (void)player; if (IsAlive()) { vortexCooldown_ = 0.0f; StartVortex(); } }
void SharkBoss::ForceFrenzy(EffectSystem& effects)
{
    if (!IsAlive()) return;
    hp_ = (std::min)(hp_, maxHp_ * GameConsts::SHARK_FRENZY_HP_RATIO);
    StartFrenzy(effects);
}

void SharkBoss::Draw() const
{
    if (!active_) return;
    if (hp_ > 0.0f)
    {
        // Vệt đỏ chỉ hướng lao (Bite Lunge telegraph)
        if (action_ == SharkAction::BiteTelegraph)
        {
            float k = std::clamp(actionTimer_ / (std::max)(0.01f, biteTelegraph_), 0.0f, 1.0f);
            Vec3 side{ lungeDir_.z, 0.0f, -lungeDir_.x };
            float hw = GameConsts::SHARK_BITE_HIT_RADIUS;
            Vec3 a = pos_ + Vec3{ 0.0f, 1.5f, 0.0f };
            Vec3 b = a + lungeDir_ * (GameConsts::SHARK_MOUTH_FWD + GameConsts::SHARK_BITE_RANGE * k);
            VECTOR p0 = DxConv::ToVECTOR(a + side * hw), p1 = DxConv::ToVECTOR(a - side * hw);
            VECTOR p2 = DxConv::ToVECTOR(b + side * hw), p3 = DxConv::ToVECTOR(b - side * hw);
            SetUseLighting(FALSE);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(70 + 110 * k));
            unsigned int red = GetColor(255, 40, 40);
            DrawTriangle3D(p0, p1, p2, red, TRUE);
            DrawTriangle3D(p1, p3, p2, red, TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            SetUseLighting(TRUE);
        }

        Render3D::DrawFlatShadow(pos_, 70.0f, 0.9f);

        Vec3 shake{ 0.0f, 0.0f, 0.0f };
        if (action_ == SharkAction::Stunned) shake.x = std::sin(stunTimer_ * 20.0f) * 3.0f;
        else if (action_ == SharkAction::BiteTelegraph) shake.x = (rand() % 5 - 2) * 0.6f;
        Vec3 drawPos = pos_ + shake;
        float flash = hitFlashTimer_ / GameConsts::HIT_FLASH_TIME;
        COLOR_F tint = frenzy_ ? GetColorF(1.35f, 0.75f, 0.75f, 1.0f) : GetColorF(1.0f, 1.0f, 1.0f, 1.0f);
        float sway = std::sin(tailPhase_);

        int body = LowPolyModels::GetHandle(LowPolyId::SharkBoss);
        if (body >= 0)
        {
            MV1SetPosition(body, DxConv::ToVECTOR(drawPos));
            MV1SetRotationXYZ(body, VGet(0.0f, yaw_ + sway * 0.05f, bank_));
            MV1SetScale(body, VGet(1.0f, 1.0f, 1.0f));
            MV1SetDifColorScale(body, tint);
            HitFlash::DrawModel(body, flash);
        }
        int tail = LowPolyModels::GetHandle(LowPolyId::SharkBossTail);
        if (tail >= 0)
        {
            // Khớp đuôi theo hướng thân, đuôi vẫy quanh trục Y
            Vec3 pivot = drawPos + Fwd() * GameConsts::SHARK_TAIL_PIVOT_Z + Vec3{ 0.0f, GameConsts::SHARK_TAIL_PIVOT_Y, 0.0f };
            MV1SetPosition(tail, DxConv::ToVECTOR(pivot));
            MV1SetRotationXYZ(tail, VGet(0.0f, yaw_ + sway * GameConsts::SHARK_TAIL_SWAY, bank_));
            MV1SetScale(tail, VGet(1.0f, 1.0f, 1.0f));
            MV1SetDifColorScale(tail, tint);
            HitFlash::DrawModel(tail, flash);
        }
    }
    if (fx_) fx_->Draw();
}
