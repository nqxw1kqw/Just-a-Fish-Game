#pragma once
#include "Vector3.h"
#include "Consts.h"
#include <memory>
#include "DxLib.h"

// =======================================================================
//  BOSS CÁ MẬP (chế độ Endless) - bộ chiêu gọn hơn cá voi:
//   1. Bite Lunge   2. Circle Strafe   3. Tail Vortex   4. Frenzy (< 30% HP)
//  Dùng lại ý tưởng của cá voi: chọn chiêu có trọng số chống lặp, nhịp burst/rest, ngắm đón đầu.
// =======================================================================

struct SharkFxSet; // Effects/Shark/SharkFxSet.h (heap)
class PlayerFish;
class EffectSystem;

enum class SharkAction
{
    Idle,
    Rest,
    BiteTelegraph,
    BiteLunge,
    BiteRecover,
    Strafe,
    VortexCast,
    Stunned,
    HuntLeap,       // Nhảy lên rồi lao xuống cát
    HuntTrack,      // Dưới cát, chỉ thấy vây, bám theo người chơi
    HuntLock,       // Vây dừng + vòng cảnh báo
    HuntDash,       // Vây lao ngầm cực nhanh tới vòng đỏ
    HuntBurst,      // Phóng thẳng lên (gần như tức thì) -> ngoạm mọi người chơi trong vòng
    HuntDrag,       // Ngoạm trúng: quay vòng kéo người chơi (camera cận cảnh)
    HuntRecover     // Cắn hụt: đứng phơi (cơ hội phản công)
};

enum class SharkSkill
{
    None,
    Bite,
    Strafe,
    Vortex,
    Hunt
};

class SharkBoss
{
public:
    SharkBoss();
    ~SharkBoss();
    SharkBoss(const SharkBoss&) = delete;
    SharkBoss& operator=(const SharkBoss&) = delete;

    void Reset();
    void Spawn(const Vec3& arenaCenter);
    void Update(float dt, PlayerFish& player, EffectSystem& effects);
    void Draw() const;

    void SavePrevTransform() { prevPos_ = pos_; prevYaw_ = yaw_; }
    void BeginRenderInterp(float alpha);
    void EndRenderInterp() { pos_ = savedPos_; yaw_ = savedYaw_; }

    bool TakeDamage(float dmg, EffectSystem& effects);
    void Kill() { hp_ = 0.0f; }

    bool IsActive() const { return active_; }
    bool IsAlive() const { return active_ && hp_ > 0.0f; }
    bool IsDead() const { return active_ && hp_ <= 0.0f; }
    bool IsFrenzy() const { return frenzy_; }
    float GetHp() const { return hp_; }
    float GetMaxHp() const { return maxHp_; }
    void SetHpMultiplier(float mul) { maxHp_ = GameConsts::SHARK_MAX_HP * mul; hp_ = maxHp_; }
    const Vec3& GetPosition() const { return pos_; }
    float GetHitboxRadius() const { return GameConsts::SHARK_HITBOX_RADIUS; }
    float GetBossFightTimer() const { return fightTimer_; }
    int GetBossTier() const { return tier_; }
    void SetBossDifficulty(int bossIndex);

    // Debug
    void ForceBite(PlayerFish& player);
    void ForceStrafe(PlayerFish& player);
    void ForceVortex(PlayerFish& player);
    void ForceFrenzy(EffectSystem& effects);
    void ForceHunt(PlayerFish& player);
    bool IsCinematic() const { return grabbedPlayer_ != nullptr; }
    bool IsBurrowed() const { return action_ == SharkAction::HuntTrack || action_ == SharkAction::HuntLock || action_ == SharkAction::HuntDash || action_ == SharkAction::HuntBurst; }
    float GetHuntCooldown() const { return huntCooldown_; }
    void SetHellMode(bool h) { hell_ = h; huntCooldown_ = h ? GameConsts::HELL_SHARK_HUNT_FIRST_DELAY : GameConsts::SHARK_HUNT_FIRST_DELAY; }
    SharkAction GetAction() const { return action_; }
    SharkSkill GetLastSkill() const { return lastSkill_; }
    SharkSkill GetPrevSkill() const { return prevSkill_; }
    int GetBurstCount() const { return burstCount_; }
    float GetRestTimer() const { return restTimer_; }
    float GetVortexTimer() const { return vortexTimer_; }
    static const wchar_t* SkillName(SharkSkill s);

private:
    void PickNext(PlayerFish& player);
    void NoteSkill(SharkSkill s);
    void EndSkill();
    void StartBite(PlayerFish& player, float telegraph);
    void StartStrafe(PlayerFish& player);
    void StartVortex();
    void StartHunt();
    void UpdateHunt(float dt, PlayerFish& player, EffectSystem& effects);
    void Stun(float duration);
    void StartFrenzy(EffectSystem& effects);
    void UpdateVortex(float dt, PlayerFish& player, EffectSystem& effects);
    void TurnToward(float targetYaw, float rate, float dt);
    Vec3 PredictPlayerPos(const PlayerFish& player, float leadTime) const;
    Vec3 MouthPos() const;
    MATRIX BodyMatrix(const Vec3& at) const;
    Vec3 JawPos() const;
    void EndHunt(PlayerFish* player);
    Vec3 Fwd() const;
    SharkFxSet& Fx() const { return *fx_; }

    bool active_ = false;
    Vec3 pos_{ 0.0f, 0.0f, 0.0f };
    float yaw_ = 0.0f;
    float hp_ = GameConsts::SHARK_MAX_HP;
    float maxHp_ = GameConsts::SHARK_MAX_HP;
    int tier_ = 1;
    float fightTimer_ = 0.0f;
    float hitFlashTimer_ = 0.0f;
    float tailPhase_ = 0.0f;
    float tailSpeedMul_ = 1.0f;
    float bank_ = 0.0f;

    SharkAction action_ = SharkAction::Idle;
    float actionTimer_ = 0.0f;
    float idleTimer_ = 1.0f;
    SharkSkill lastSkill_ = SharkSkill::None;
    SharkSkill prevSkill_ = SharkSkill::None;
    int burstCount_ = 0;
    float restTimer_ = 0.0f;
    bool frenzy_ = false;
    int chainBitesLeft_ = 0;

    Vec3 lungeDir_{ 0.0f, 0.0f, 1.0f };
    float lungeTraveled_ = 0.0f;
    float biteTelegraph_ = GameConsts::SHARK_BITE_TELEGRAPH;
    bool biteHit_ = false;

    float strafeAngle_ = 0.0f;
    float strafeSwept_ = 0.0f;
    float strafeSign_ = 1.0f;
    float finHitCd_ = 0.0f;

    Vec3 vortexCenter_{ 0.0f, 0.0f, 0.0f };
    float vortexTimer_ = 0.0f;
    float vortexTickCd_ = 0.0f;
    float vortexCooldown_ = 0.0f;

    float stunTimer_ = 0.0f;

    float huntCooldown_ = GameConsts::SHARK_HUNT_FIRST_DELAY;
    bool hell_ = false;
public:
    void SetDamageTakenMul(float m) { damageTakenMul_ = m; }
private:
    float damageTakenMul_ = 1.0f;
    Vec3 huntLockPos_{ 0.0f, 0.0f, 0.0f };
    float huntPose_ = 0.0f;    // pitch: < 0 ngóc đầu, > 0 chúi xuống
    float huntTurnSign_ = 1.0f;
    float huntElapsed_ = 0.0f;
    float swingAngle_ = 0.0f;
    float swingSwept_ = 0.0f;
    float swingRadius_ = 0.0f;
    float huntDashSpeed_ = 0.0f;
    PlayerFish* grabbedPlayer_ = nullptr;

    Vec3 playerVel_{ 0.0f, 0.0f, 0.0f };
    Vec3 lastPlayerPos_{ 0.0f, 0.0f, 0.0f };
    bool hasLastPlayerPos_ = false;

    Vec3 prevPos_{ 0.0f, 0.0f, 0.0f }, savedPos_{ 0.0f, 0.0f, 0.0f };
    float prevYaw_ = 0.0f, savedYaw_ = 0.0f;

    std::unique_ptr<SharkFxSet> fx_;
};
