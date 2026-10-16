#pragma once
#include "Vector3.h"
#include "Consts.h"
#include "TsunamiWaterEffect.h"
#include <array>
#include <vector>
#include <memory>

struct WhaleFxSet; // Effects/Whale/WhaleFxSet.h (cấp phát trên heap)

class PlayerFish;
class EnemyManager;
class EffectSystem;

enum class BossPhase
{
    Phase1, // 100% -> 50% HP
    Phase2  // < 50% HP (Cuồng nộ: nhanh hơn, thêm đòn đập kép)
};

enum class BossAction
{
    Idle,
    TelegraphSlam,
    PerformSlam,
    TelegraphMinions,
    PerformMinions,
    TelegraphDoubleSlam,
    PerformDoubleSlam,
    TelegraphDoubleSlam2,   // Báo hiệu cú đập thứ 2 (Task 10 T3b)
    TelegraphRadialTsunami, // Phần A: Báo hiệu Sóng thần tỏa tròn
    LaunchRadialTsunami,    // Phần A: Phóng Sóng thần tỏa tròn
    // Boss rework (PR A)
    TelegraphTail,          // Quật đuôi: giơ đuôi
    PerformTail,            // Quật đuôi: hồi sau đòn
    Stunned,                // Choáng (nhận thêm sát thương)
    // PR C
    TelegraphBubbleNet,     // Lặn xuống, bọt sủi quanh người chơi
    BubbleNetSwim,          // Bơi vòng tròn thả cột bọt (lưới khép dần)
    TelegraphInhale,        // Há miệng, nước bắt đầu xoáy vào
    PerformInhale,          // Hút: kéo người chơi về miệng
    // PR D
    TelegraphDive,          // Lặn sâu, vệt bọt chỉ hướng lao
    PerformDive             // Lao thẳng dưới nước
};

enum class BossSkill
{
    None,
    Slam,
    DoubleSlam,
    Minions,
    Tsunami,
    Tail,
    BubbleNet,
    Inhale,
    Dive
};

// Cấu trúc Vành Sóng thần tỏa tròn (Radial Tsunami - Phần A3)
struct TsunamiRing
{
    bool  active = false;
    Vec3  center{ 0.0f, 0.0f, 0.0f }; // Tâm vòng (cố định lúc phóng, y = 0)
    float radius = GameConsts::TSUNAMI_START_RADIUS; // Bán kính tâm vành sóng
    float speed = GameConsts::TSUNAMI_SPEED_P1;
    float thickness = GameConsts::TSUNAMI_THICKNESS;
    float gapCenterRad = 0.0f;        // Hướng tâm khe hở (radian, nếu có)
    float gapHalfRad = 0.0f;          // Nửa độ rộng góc khe hở (radian), 0 = không có khe
    bool  hitPlayer = false;          // Mỗi vòng chỉ gây sát thương tối đa 1 lần
    bool  dodgedNotice = false;       // Đánh dấu đã né thành công khi nhảy qua
};

class WhaleBoss
{
public:
    WhaleBoss();
    ~WhaleBoss();
    WhaleBoss(const WhaleBoss&) = delete;
    WhaleBoss& operator=(const WhaleBoss&) = delete;
    void Init();
    void Reset();
    void Update(float dt, PlayerFish& player, EnemyManager& enemies, EffectSystem& effects);
    void Draw() const;

    // Render interpolation (#26)
    void SavePrevTransform() { prevPos_ = pos_; prevYaw_ = yaw_; }
    void BeginRenderInterp(float alpha);
    void EndRenderInterp() { pos_ = savedPos_; yaw_ = savedYaw_; }

    bool TakeDamage(float dmg, EffectSystem& effects);

    bool IsAlive() const { return active_ && hp_ > 0.0f; }
    bool IsDead() const { return active_ && hp_ <= 0.0f; }
    bool IsActive() const { return active_; }
    void Kill() { hp_ = 0.0f; }
    void Spawn(const Vec3& arenaCenter);

    // Hỗ trợ Debug & Kỹ năng Sóng thần tỏa tròn (Phần A & D, Task 10 T3)
    void ForceTsunami(PlayerFish& player);
    void ForceTsunamiWithGap(PlayerFish& player);
    void ForceDoubleSlam(PlayerFish& player);
    void ForceTsunamiDouble(PlayerFish& player); // PR B: sóng đôi (không khe -> có khe)
    void ForceBubbleNet(PlayerFish& player);     // PR C
    void ForceInhale(PlayerFish& player);        // PR C
    void ForcePack(PlayerFish& player);          // PR D
    void ForceDive(PlayerFish& player);          // PR D
    void ForcePhaseStep(EffectSystem& effects);  // PR D: P1 -> P2 (kèm sóng), P2 -> Cuồng nộ
    bool IsEnraged() const { return enraged_; }
    void StartTsunamiTelegraph(PlayerFish& player, float gapDeg = 0.0f);

    // Boss rework (PR A): Breach Slam, Tail Slap, Stun
    void ForceBreach(PlayerFish& player);
    void ForceTail(PlayerFish& player);
    void Stun(float duration, float damageMul = GameConsts::BOSS_STUN_DAMAGE_MUL);
    bool IsStunned() const { return action_ == BossAction::Stunned; }

    // Truy vấn thông số cho Debug HUD (A8)
    int GetActiveTsunamiCount() const;
    float GetTsunamiJumpWindow() const;
    float GetTsunamiThicknessOverSpeed() const;
    float GetTimeToHitPlayer(const Vec3& playerPos) const;

    static constexpr int ACTION_HISTORY_CAPACITY = 5;

    float GetHp() const { return hp_; }
    float GetMaxHp() const { return maxHp_; }
    BossPhase GetPhase() const { return phase_; }
    const Vec3& GetPosition() const { return pos_; }
    float GetHitboxRadius() const { return 55.0f; }

    void SetHpMultiplier(float mul) { hpMultiplier_ = mul; maxHp_ = GameConsts::BOSS_MAX_HP * mul; hp_ = maxHp_; }
    float GetHpMultiplier() const { return hpMultiplier_; }
    void SetDamageTakenMul(float m) { damageTakenMul_ = m; }
    void SetHellMode(bool h) { hell_ = h; }
    void SetBossDifficulty(int bossIndex)
    {
        bossTier_ = (std::max)(1, bossIndex);
        float mul = 1.0f + 0.5f * (bossTier_ - 1);
        SetHpMultiplier(mul);
    }
    int GetBossTier() const { return bossTier_; }

    float GetBossFightTimer() const { return bossFightTimer_; }
    int GetActionHistoryCount() const { return actionHistoryCount_; }
    // AI pacing (debug overlay)
    int GetBurstCount() const { return burstCount_; }
    int GetBurstSize() const { return burstSize_; }
    float GetRestTimer() const { return restTimer_; }
    static const wchar_t* SkillName(BossSkill s);
    BossSkill GetLastSkill() const { return lastSkill_; }
    BossSkill GetPrevSkill() const { return prevSkill_; }
    const wchar_t* GetRecentAction(int indexFromNewest) const;
    void RecordActionHistory(const wchar_t* actionName);

private:
    void PickNextAction(PlayerFish& player, EnemyManager& enemies);
    void SpawnRing(float gapDeg, const Vec3& targetPos);

    // Boss rework (BossSkills.cpp): xử lý các trạng thái mới; trả về true nếu đã xử lý
    bool UpdateExtendedAction(float dt, PlayerFish& player, EnemyManager& enemies, EffectSystem& effects);
    void StartBreach(PlayerFish& player);
    void StartTail();
    void UpdateBreachTelegraph(float dt, float duration, PlayerFish& player, EffectSystem& effects, bool trackPlayer);
    void LandBreach(PlayerFish& player, EffectSystem& effects, float innerDamage, float outerDamage,
                    float innerRadius = GameConsts::BREACH_INNER_RADIUS, float outerRadius = GameConsts::BREACH_OUTER_RADIUS);
    bool IsPlayerBehind(const PlayerFish& player) const;
    void EnsureActiveForDebug();

    bool active_ = false;
    Vec3 pos_{ 0, 0, 0 };
    Vec3 targetPos_{ 0, 0, 0 };
    float yaw_ = 0.0f;
    Vec3 prevPos_{ 0, 0, 0 };
    float prevYaw_ = 0.0f;
    Vec3 savedPos_{ 0, 0, 0 };
    float savedYaw_ = 0.0f;
    int bossTier_ = 1;
    float hpMultiplier_ = 1.0f;
    float damageTakenMul_ = 1.0f;
    bool hell_ = false; // Tutorial: x3 (đặt mỗi lần StartBossFight)
    float hp_ = GameConsts::BOSS_MAX_HP;
    float maxHp_ = GameConsts::BOSS_MAX_HP;
    BossPhase phase_ = BossPhase::Phase1;

    BossAction action_ = BossAction::Idle;
    float actionTimer_ = 0.0f;
    float idleDuration_ = 1.8f;
    int actionCycle_ = 0;
    BossSkill lastSkill_ = BossSkill::None;
    // AI pacing
    BossSkill prevSkill_ = BossSkill::None;     // chiêu trước lastSkill_
    void NoteSkill(BossSkill s);                // ghi lịch sử 2 chiêu gần nhất (1 lần / chiêu)
    int burstCount_ = 0;
    int burstSize_ = 3;
    float restTimer_ = 0.0f;
    Vec3 lastPlayerPos_{ 0.0f, 0.0f, 0.0f };
    Vec3 playerVel_{ 0.0f, 0.0f, 0.0f };
    bool hasLastPlayerPos_ = false;
    int RollBurstSize() const;
    float CurrentBurstGap() const;
    float CurrentRestTime() const;
    Vec3 PredictPlayerPos(const PlayerFish& player, float leadTime) const;
    float timeSinceLastTsunami_ = 10.0f;
    float currentTsunamiGapDeg_ = 0.0f;

    // Đòn 1: Nhảy đập (Slam)
    Vec3 slamTarget_{ 0, 0, 0 };
    float slamRadius_ = 200.0f;
    float slamProgress_ = 0.0f; // Pha nhảy lên đập xuống
    bool isSecondSlam_ = false;
    struct BubblePillar { Vec3 pos; float life = 0.0f; float fxTimer = 0.0f; };
    std::vector<BubblePillar> bubblePillars_;
    Vec3 netCenter_{ 0.0f, 0.0f, 0.0f };
    float netStartAngle_ = 0.0f;
    int netPillarsDropped_ = 0;
    float netHitCooldown_ = 0.0f;
    float bubbleNetCooldown_ = 0.0f;
    float inhaleCooldown_ = 0.0f;
    void StartBubbleNet(PlayerFish& player);
    void StartInhale(PlayerFish& player);
    void UpdateBossHazards(float dt, PlayerFish& player, EffectSystem& effects);
    void DrawBossHazards() const;
    // PR D
    Vec3 diveDir_{ 0.0f, 0.0f, 1.0f };
    float diveCooldown_ = 0.0f;
    bool diveHit_ = false;
    BossSkill comboNext_ = BossSkill::None;
    bool pendingPhaseWave_ = false;
    bool enraged_ = false;
    void StartDive(PlayerFish& player);
    void StartPackTelegraph(PlayerFish& player);
    void SpawnPack(EnemyManager& enemies);
    std::array<Vec3, 12> packSpots_{};
    int packSpotCount_ = 0;
    // PR E: hiệu ứng hạt Cá voi (Fuyuu)
    std::unique_ptr<WhaleFxSet> fx_;
    WhaleFxSet& Fx();
    bool breachFxArmed_ = false;
    bool netBreachPending_ = false;
    Vec3 HeadPos() const;
    Vec3 TailPos() const;
    Vec3 MouthPos() const;
    int slamCount_ = 0;              // PR B: số cú đã đáp trong Quật ba nhịp
    float secondRingDelay_ = 0.0f;   // PR B: độ trễ vòng sóng thứ 2
    float secondRingGapDeg_ = 0.0f;  // PR B: khe của vòng sóng thứ 2

    // Đòn 2: Sóng thần tỏa tròn (Radial Tsunami - Phần A)
    std::array<TsunamiRing, GameConsts::TSUNAMI_MAX_ACTIVE> tsunamiRings_{};
    TsunamiWaterEffect tsunamiWater_; // Hiệu ứng nước sóng thần (pool hạt riêng)
    float timeSinceLastJumpAttack_ = 5.0f; // Theo dõi giãn cách đòn bắt buộc nhảy (A5 >= 2.0s)
    float timeSinceLastRingLaunch_ = 5.0f; // Giãn cách giữa 2 lần phóng vòng sóng (A5 >= 2.3s)
    bool needSecondRing_ = false;          // Hỗ trợ gọi 2 vòng liên tiếp ở Phase 2

    // Boss rework (PR A)
    float tailCooldown_ = 0.0f;
    float hugTimer_ = 0.0f;
    float stunTimer_ = 0.0f;
    float stunDamageMul_ = 1.0f;
    float telegraphFxTimer_ = 0.0f;

    // Hiệu ứng & Hit flash
    float hitFlashTimer_ = 0.0f;
    float phaseTransitionTimer_ = 0.0f;
    float waterFxTimer_ = 0.0f;

    // Đo đạc & Lịch sử chiêu (Task 10 T1)
    float bossFightTimer_ = 0.0f;
    std::array<const wchar_t*, ACTION_HISTORY_CAPACITY> actionHistory_{};
    int actionHistoryCount_ = 0;
    int actionHistoryHead_ = 0;
};
