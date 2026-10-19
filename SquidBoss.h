#pragma once
#include "Vector3.h"
#include "Consts.h"
#include "DxLib.h"
#include <array>
#include <memory>

// Boss #3: Giant Squid. Keeps mid-range, hovers and strafes.
// Skills: Tentacle Slam (telegraphed column, double when enraged), Ink Spray (darken + slow),
// Ink Jet (aimed shots), Grab Pull (close range or low-HP player). Enrage < 30% HP.
class PlayerFish;
class EffectSystem;
struct SquidFxSet; // Effects/Squid/SquidFxSet.h (heap)

enum class SquidAction
{
    Idle, Rest,
    SlamTelegraph, SlamStrike, SlamRecover,
    InkSprayCast, InkJetCast,
    GrabTelegraph, GrabPull, GrabRecover
};

enum class SquidSkill { None, TentacleSlam, InkSpray, InkJet, Grab };

class SquidBoss
{
public:
    SquidBoss();
    ~SquidBoss();
    SquidBoss(const SquidBoss&) = delete;
    SquidBoss& operator=(const SquidBoss&) = delete;

    void Reset();
    void Spawn(const Vec3& arenaCenter);
    void Update(float dt, PlayerFish& player, EffectSystem& effects);
    void Draw() const;
    void DrawScreenOverlay() const;

    void SavePrevTransform() { prevPos_ = pos_; prevYaw_ = yaw_; }
    void BeginRenderInterp(float alpha);
    void EndRenderInterp() { pos_ = savedPos_; yaw_ = savedYaw_; }

    bool TakeDamage(float dmg, EffectSystem& effects);
    bool TakeDamageAt(const Vec3& hitPos, float dmg, EffectSystem& effects);
    void Kill() { hp_ = 0.0f; }

    bool IsActive() const { return active_; }
    bool IsAlive() const { return active_ && hp_ > 0.0f; }
    bool IsDead() const { return active_ && hp_ <= 0.0f; }
    bool IsEnraged() const { return enraged_; }
    float GetHp() const { return hp_; }
    float GetMaxHp() const { return maxHp_; }
    void SetBossDifficulty(int bossIndex);
    void SetHpMultiplier(float mul);
    const Vec3& GetPosition() const { return pos_; }
    Vec3 GetMantlePos() const;
    void DrawTentacle(const Vec3& base, const Vec3& dir, float length, float thick = -1.0f) const;
    void DrawArmChain(const Vec3& base, const Vec3& dir, float length, float curl, float phase) const;
    void DrawArms() const;
    float GetHitboxRadius() const { return GameConsts::SQUID_HITBOX_RADIUS; }
    float GetBossFightTimer() const { return fightTimer_; }
    int GetBossTier() const { return tier_; }
    void SetDamageTakenMul(float m) { damageTakenMul_ = m; }
    void SetHellMode(bool h) { hell_ = h; }
    int GetInkOverlayAlpha() const;

    void ForceSlam(PlayerFish& player);
    void ForceInkSpray(PlayerFish& player);
    void ForceInkJet(PlayerFish& player);
    void ForceGrab(PlayerFish& player);
    void ForceEnrage(EffectSystem& effects);
    static const wchar_t* SkillName(SquidSkill s);

private:
    struct InkShot { Vec3 pos{}; Vec3 vel{}; float life = 0.0f; bool active = false; };
    void PickNext(PlayerFish& player);
    void NoteSkill(SquidSkill s);
    void EndSkill();
    void StartSlam(PlayerFish& player);
    void StartInkSpray();
    void StartInkJet();
    void StartGrab(PlayerFish& player);
    void StartEnrage(EffectSystem& effects);
    bool TakeDamageInternal(float dmg, EffectSystem& effects, bool weak);
    void UpdateHover(float dt, const PlayerFish& player, float speedMul);
    void UpdateInkShots(float dt, PlayerFish& player, EffectSystem& effects);
    void TurnToward(float targetYaw, float rate, float dt);
    Vec3 Fwd() const;

    bool active_ = false;
    Vec3 pos_{ 0.0f, 0.0f, 0.0f };
    float yaw_ = 0.0f;
    float hp_ = GameConsts::SQUID_MAX_HP;
    float maxHp_ = GameConsts::SQUID_MAX_HP;
    float stageHpMul_ = 1.0f;
    int tier_ = 1;
    float fightTimer_ = 0.0f;
    float hitFlashTimer_ = 0.0f;
    float hoverPhase_ = 0.0f;
    float animTime_ = 0.0f;
    float pulsePhase_ = 0.0f;
    float bankSmooth_ = 0.0f;
    float pitchSmooth_ = 0.0f;
    float moveSpeedSmooth_ = 0.0f;
    Vec3 lastPos_{};
    bool hasLastPos_ = false;
    float strafeSign_ = 1.0f;
    float strafeTimer_ = 0.0f;

    SquidAction action_ = SquidAction::Idle;
    float actionTimer_ = 0.0f;
    float idleTimer_ = 1.0f;
    float restTimer_ = 0.0f;
    SquidSkill lastSkill_ = SquidSkill::None;
    SquidSkill prevSkill_ = SquidSkill::None;
    int burstCount_ = 0;
    bool enraged_ = false;
    bool hell_ = false;
    float damageTakenMul_ = 1.0f;

    Vec3 slamPos_{}; Vec3 slamPos2_{}; bool slamSecond_ = false; bool slamHitDone_ = false;
    float inkTimer_ = 0.0f;
    float inkCooldown_ = 3.0f;
    float inkShotTimer_ = 0.0f;
    int inkShotsLeft_ = 0;
    float grabCooldown_ = GameConsts::SQUID_GRAB_FIRST_DELAY;
    bool grabHitDone_ = false;
    std::array<InkShot, GameConsts::SQUID_INK_MAX_SHOTS> inkShots_{};

    Vec3 prevPos_{}; Vec3 savedPos_{};
    Vec3 grabTargetPos_{};
    int inkVariant_ = 0;
    mutable int inkGraph_[3] = { -1, -1, -1 };
    mutable bool inkGraphTried_ = false;
    std::unique_ptr<SquidFxSet> fx_;
    SquidFxSet& Fx() const { return *fx_; }
    float prevYaw_ = 0.0f; float savedYaw_ = 0.0f;
};