#pragma once
#include "Vector3.h"
#include "Consts.h"
#include <array>

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
    LaunchRadialTsunami     // Phần A: Phóng Sóng thần tỏa tròn
};

enum class BossSkill
{
    None,
    Slam,
    DoubleSlam,
    Minions,
    Tsunami
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
    void Init();
    void Reset();
    void Update(float dt, PlayerFish& player, EnemyManager& enemies, EffectSystem& effects);
    void Draw() const;

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
    void StartTsunamiTelegraph(PlayerFish& player, float gapDeg = 0.0f);

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
    void SetBossDifficulty(int bossIndex)
    {
        bossTier_ = (std::max)(1, bossIndex);
        float mul = 1.0f + 0.5f * (bossTier_ - 1);
        SetHpMultiplier(mul);
    }
    int GetBossTier() const { return bossTier_; }

    float GetBossFightTimer() const { return bossFightTimer_; }
    int GetActionHistoryCount() const { return actionHistoryCount_; }
    const wchar_t* GetRecentAction(int indexFromNewest) const;
    void RecordActionHistory(const wchar_t* actionName);

private:
    void PickNextAction(PlayerFish& player, EnemyManager& enemies);
    void SpawnRing(float gapDeg, const Vec3& targetPos);

    bool active_ = false;
    Vec3 pos_{ 0, 0, 0 };
    Vec3 targetPos_{ 0, 0, 0 };
    float yaw_ = 0.0f;
    int bossTier_ = 1;
    float hpMultiplier_ = 1.0f;
    float hp_ = GameConsts::BOSS_MAX_HP;
    float maxHp_ = GameConsts::BOSS_MAX_HP;
    BossPhase phase_ = BossPhase::Phase1;

    BossAction action_ = BossAction::Idle;
    float actionTimer_ = 0.0f;
    float idleDuration_ = 1.8f;
    int actionCycle_ = 0;
    BossSkill lastSkill_ = BossSkill::None;
    float timeSinceLastTsunami_ = 10.0f;
    float currentTsunamiGapDeg_ = 0.0f;

    // Đòn 1: Nhảy đập (Slam)
    Vec3 slamTarget_{ 0, 0, 0 };
    float slamRadius_ = 200.0f;
    float slamProgress_ = 0.0f; // Pha nhảy lên đập xuống
    bool isSecondSlam_ = false;

    // Đòn 2: Sóng thần tỏa tròn (Radial Tsunami - Phần A)
    std::array<TsunamiRing, GameConsts::TSUNAMI_MAX_ACTIVE> tsunamiRings_{};
    float timeSinceLastJumpAttack_ = 5.0f; // Theo dõi giãn cách đòn bắt buộc nhảy (A5 >= 2.0s)
    float timeSinceLastRingLaunch_ = 5.0f; // Giãn cách giữa 2 lần phóng vòng sóng (A5 >= 2.3s)
    bool needSecondRing_ = false;          // Hỗ trợ gọi 2 vòng liên tiếp ở Phase 2

    // Hiệu ứng & Hit flash
    float hitFlashTimer_ = 0.0f;
    float phaseTransitionTimer_ = 0.0f;

    // Đo đạc & Lịch sử chiêu (Task 10 T1)
    float bossFightTimer_ = 0.0f;
    std::array<const wchar_t*, ACTION_HISTORY_CAPACITY> actionHistory_{};
    int actionHistoryCount_ = 0;
    int actionHistoryHead_ = 0;
};
