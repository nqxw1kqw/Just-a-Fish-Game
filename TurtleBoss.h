#pragma once
#include "Vector3.h"
#include "Consts.h"
#include "DxLib.h"

// Boss #4: Ancient Turtle. Slow tank. Its shell blocks most damage until
// player slams crack it (shell HP -> break -> long stagger).
// Skills: Withdraw + Rolling Charge (telegraphed line, rolls across the arena;
// phase 2 rolls twice, faster), Head Bite (close range lunge), Tail Sweep
// (close range all around). Phase 2 below 50% HP.
class PlayerFish;
class EffectSystem;

enum class TurtleAction
{
    Idle, Rest,
    Withdraw, RollTelegraph, Rolling, RollRecover,
    Stagger,
    BiteTelegraph, BiteLunge, BiteRecover,
    TailTelegraph, TailSweep, TailRecover
};

enum class TurtleSkill { None, Roll, Bite, Tail };

class TurtleBoss
{
public:
    TurtleBoss() = default;
    ~TurtleBoss() = default;
    TurtleBoss(const TurtleBoss&) = delete;
    TurtleBoss& operator=(const TurtleBoss&) = delete;

    void Reset();
    void Spawn(const Vec3& arenaCenter);
    void Update(float dt, PlayerFish& player, EffectSystem& effects);
    void Draw() const;

    void SavePrevTransform() { prevPos_ = pos_; prevYaw_ = yaw_; }
    void BeginRenderInterp(float alpha);
    void EndRenderInterp() { pos_ = savedPos_; yaw_ = savedYaw_; }

    bool TakeDamage(float dmg, EffectSystem& effects);       // bullets: chip the shell slowly
    bool TakeDamageSlam(float dmg, EffectSystem& effects);   // slams: crack the shell at full power
    void Kill() { hp_ = 0.0f; }

    bool IsActive() const { return active_; }
    bool IsAlive() const { return active_ && hp_ > 0.0f; }
    bool IsDead() const { return active_ && hp_ <= 0.0f; }
    bool IsPhase2() const { return phase2_; }
    bool IsShellBroken() const { return shellBroken_; }
    float GetHp() const { return hp_; }
    float GetMaxHp() const { return maxHp_; }
    float GetShellHp() const { return shellHp_; }
    float GetShellMaxHp() const { return shellMaxHp_; }
    void SetBossDifficulty(int bossIndex);
    void SetHpMultiplier(float mul);
    const Vec3& GetPosition() const { return pos_; }
    float GetHitboxRadius() const { return GameConsts::TURTLE_HITBOX_RADIUS; }
    float GetBossFightTimer() const { return fightTimer_; }
    int GetBossTier() const { return tier_; }
    void SetDamageTakenMul(float m) { damageTakenMul_ = m; }
    void SetHellMode(bool h) { hell_ = h; }

    void ForceRoll(PlayerFish& player);
    void ForceBite(PlayerFish& player);
    void ForceTail(PlayerFish& player);
    void ForcePhase2(EffectSystem& effects);
    static const wchar_t* SkillName(TurtleSkill s);

private:
    void PickNext(PlayerFish& player);
    void NoteSkill(TurtleSkill s);
    void EndSkill();
    void StartWithdraw(PlayerFish& player);
    void StartRollCharge();
    void StartRollChargeLocked();
    void StartBite(PlayerFish& player);
    void StartTail();
    void EnterPhase2(EffectSystem& effects);
    void BreakShell(EffectSystem& effects);
    bool TakeDamageInternal(float dmg, EffectSystem& effects, bool fromSlam);
    void UpdateAdvance(float dt, const PlayerFish& player, float speedMul);
    void TurnToward(float targetYaw, float rate, float dt);
    Vec3 Fwd() const;
    float RollSpeed() const;

    bool active_ = false;
    Vec3 pos_{ 0.0f, 0.0f, 0.0f };
    float yaw_ = 0.0f;
    float hp_ = GameConsts::TURTLE_MAX_HP;
    float maxHp_ = GameConsts::TURTLE_MAX_HP;
    float shellHp_ = GameConsts::TURTLE_SHELL_HP;
    float shellMaxHp_ = GameConsts::TURTLE_SHELL_HP;
    float stageHpMul_ = 1.0f;
    int tier_ = 1;
    float fightTimer_ = 0.0f;
    float hitFlashTimer_ = 0.0f;
    float animTime_ = 0.0f;
    float bobPhase_ = 0.0f;

    TurtleAction action_ = TurtleAction::Idle;
    float actionTimer_ = 0.0f;
    float idleTimer_ = 1.0f;
    float restTimer_ = 0.0f;
    TurtleSkill lastSkill_ = TurtleSkill::None;
    TurtleSkill prevSkill_ = TurtleSkill::None;
    bool phase2_ = false;
    bool shellBroken_ = false;
    bool hell_ = false;
    float damageTakenMul_ = 1.0f;

    float withdrawDuration_ = GameConsts::TURTLE_WITHDRAW_TIME;
    Vec3 rollDir_{ 0.0f, 0.0f, 1.0f };
    Vec3 rollEnd_{};
    float rollDist_ = 0.0f;
    float rollTravel_ = 0.0f;
    float rollAngle_ = 0.0f;
    int rollsLeft_ = 0;
    bool rollHitDone_ = false;
    Vec3 biteDir_{ 0.0f, 0.0f, 1.0f };
    bool biteHitDone_ = false;
    bool tailHitDone_ = false;

    Vec3 prevPos_{}; Vec3 savedPos_{};
    float prevYaw_ = 0.0f; float savedYaw_ = 0.0f;
};