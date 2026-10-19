#pragma once
#include "Vector3.h"
#include "Consts.h"
#include "DxLib.h"
#include <memory>

// Boss #5: Swordfish. Fast glass-cannon duelist.
// Skills (exactly 3): Sword Dash (telegraphed lane, TWO stab passes, then a
// slow vulnerable window), Spin (fast yaw spin while advancing a little,
// huge knockback on touch, then self-stagger), Rage Dive (only < 30% HP:
// leaps, dives under the sand, stalks the player, stabs UP twice per rage).
class PlayerFish;
class EffectSystem;
class SharkSandEffect; // Effects/SharkSand/SharkSandFx.h (heap, sand FX shared with Shark hunt)
struct SwordfishFxSet; // Effects/Swordfish/SwordfishFxSet.h (heap)

enum class SwordfishAction
{
    Idle, Rest,
    DashTelegraph, Dashing, DashPause, DashRecover,
    Spin, SpinDrift, SpinStagger,
    RageLeap, RageDive, UnderSand, SandTelegraph, StabUp, Resurface
};

enum class SwordfishSkill { None, Dash, Spin, Rage };

class SwordfishBoss
{
public:
    SwordfishBoss();
    ~SwordfishBoss();
    SwordfishBoss(const SwordfishBoss&) = delete;
    SwordfishBoss& operator=(const SwordfishBoss&) = delete;

    void Reset();
    void Spawn(const Vec3& arenaCenter);
    void Update(float dt, PlayerFish& player, EffectSystem& effects);
    void Draw() const;

    void SavePrevTransform() { prevPos_ = pos_; prevYaw_ = yaw_; }
    void BeginRenderInterp(float alpha);
    void EndRenderInterp() { pos_ = savedPos_; yaw_ = savedYaw_; }

    bool TakeDamage(float dmg, EffectSystem& effects);
    bool TakeDamageAt(const Vec3& hitPos, float dmg, EffectSystem& effects);
    void Kill() { hp_ = 0.0f; }

    bool IsActive() const { return active_; }
    bool IsAlive() const { return active_ && hp_ > 0.0f; }
    bool IsDead() const { return active_ && hp_ <= 0.0f; }
    bool IsRaged() const { return raged_; }
    bool IsVulnerable() const;
    float GetHp() const { return hp_; }
    float GetMaxHp() const { return maxHp_; }
    void SetBossDifficulty(int bossIndex);
    void SetHpMultiplier(float mul);
    const Vec3& GetPosition() const { return pos_; }
    float GetHitboxRadius() const { return GameConsts::SWORDFISH_HITBOX_RADIUS; }
    float GetBossFightTimer() const { return fightTimer_; }
    int GetBossTier() const { return tier_; }
    void SetDamageTakenMul(float m) { damageTakenMul_ = m; }
    void SetHellMode(bool h) { hell_ = h; }

    void ForceDash(PlayerFish& player);
    void ForceSpin(PlayerFish& player);
    void ForceRage(EffectSystem& effects);
    static const wchar_t* SkillName(SwordfishSkill s);

private:
    void PickNext(PlayerFish& player, EffectSystem& effects);
    void NoteSkill(SwordfishSkill s);
    void EndSkill();
    void StartDash();
    void LockDash(const PlayerFish& player, bool second);
    void StartSpin();
    void StartRage(EffectSystem& effects);
    void EnterRage(EffectSystem& effects);
    bool TakeDamageInternal(float dmg, EffectSystem& effects);
    void UpdateSwim(float dt, const PlayerFish& player, float speedMul);
    void TurnToward(float targetYaw, float rate, float dt);
    Vec3 Fwd() const;
    void KnockPlayer(PlayerFish& player, const Vec3& from, float extraPower, EffectSystem& effects, float dmg);

    bool active_ = false;
    Vec3 pos_{ 0.0f, 0.0f, 0.0f };
    float yaw_ = 0.0f;
    float hp_ = GameConsts::SWORDFISH_MAX_HP;
    float maxHp_ = GameConsts::SWORDFISH_MAX_HP;
    float stageHpMul_ = 1.0f;
    int tier_ = 1;
    float fightTimer_ = 0.0f;
    float hitFlashTimer_ = 0.0f;
    float animTime_ = 0.0f;
    float wagPhase_ = 0.0f;
    float bankSmooth_ = 0.0f;
    float pitchSmooth_ = 0.0f;
    float moveSpeedSmooth_ = 0.0f;
    Vec3 lastPos_{};
    bool hasLastPos_ = false;

    SwordfishAction action_ = SwordfishAction::Idle;
    float actionTimer_ = 0.0f;
    float idleTimer_ = 1.0f;
    float restTimer_ = 0.0f;
    SwordfishSkill lastSkill_ = SwordfishSkill::None;
    SwordfishSkill prevSkill_ = SwordfishSkill::None;
    bool raged_ = false;
    bool hell_ = false;
    float damageTakenMul_ = 1.0f;
    float rageCooldown_ = 0.0f;

    Vec3 dashDir_{ 0.0f, 0.0f, 1.0f };
    Vec3 dashTarget_{};
    float dashDist_ = 0.0f;
    float dashTravel_ = 0.0f;
    int dashPass_ = 0;
    bool dashHitDone_ = false;
    float sprayAcc_ = 0.0f;

    float spinHitCd_ = 0.0f;
    float visualYaw_ = 0.0f;   // extra spin rotation while Spinning
    float leapY_ = 0.0f;       // height offset for rage leap / stab-up
    Vec3 sandTarget_{};
    int stabsLeft_ = 0;
    float sandTrailAcc_ = 0.0f;

    std::unique_ptr<SharkSandEffect> sandFx_;
    std::unique_ptr<SwordfishFxSet> swordFx_;
    Vec3 prevPos_{}; Vec3 savedPos_{};
    float prevYaw_ = 0.0f; float savedYaw_ = 0.0f;
};
