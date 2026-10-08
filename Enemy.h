#pragma once
#include "Vector3.h"
#include "Consts.h"
#include <array>

class EffectSystem;
class PlayerFish;

enum class EnemyType
{
    Minion, // Quái lính đỏ
    Hopper, // Cua nảy cam
    Tanker, // Rùa xanh trâu bò
    Float,  // Sứa bay lơ lửng, gây tê liệt (G4)
    Charge  // Cá mập khựng báo hiệu rồi lao thẳng (G4)
};

struct Enemy
{
    EnemyType type = EnemyType::Minion;
    Vec3 pos{ 0, 0, 0 };
    Vec3 knockback{ 0, 0, 0 };
    float yaw = 0.0f;

    float hp = 25.0f;
    float maxHp = 25.0f;
    float speed = 100.0f;
    float radius = 14.0f;
    float damage = 12.0f;

    // Hiệu ứng nhảy cho quái Hopper
    float bouncePhase = 0.0f;
    float bounceHeight = 28.0f;
    float bounceDuration = 0.35f;

    // Trạng thái cho quái Float (Sứa bay)
    float floatPhase = 0.0f;

    // Trạng thái cho quái Charge (Cá mập lao)
    int chargeState = 0; // 0: rình rập, 1: dừng nạp lực báo hiệu, 2: lao vút, 3: hồi phục
    float chargeTimer = 0.0f;
    Vec3 chargeDir{ 0, 0, 1 };

    float hitFlashTimer = 0.0f;
    float stunTimer = 0.0f; // Thời gian bị choáng (J6)
    float animTime = 0.0f;  // Thời gian hoạt ảnh 3D
    bool active = false;
};

struct EnemyModelConfig
{
    int handle = -1;
    int animAttachIndex = -1;
    float animTotalTime = 0.0f;
    float scale = 0.22f;
    float rotYDeg = 180.0f;
    float offsetY = 0.0f;
    bool hasTexture = false;
};

class EnemyManager
{
public:
    void Init();
    void Reset();
    void Update(float dt, PlayerFish& player, EffectSystem& effects);
    void Draw() const;

    // Sinh quái (Task 10 T2: bool return, spawn có khoảng cách với Player)
    bool SpawnEnemy(EnemyType type, const Vec3& pos);
    bool SpawnRandomEdge(EnemyType type, const Vec3& playerPos = Vec3{ 0, 0, 0 });
    float GetWaveCostCap(int wave) const;
    static float GetEnemyCost(EnemyType type);

    // Xử lý sóng nước tiếp đất từ cá (J6: Trả về số quái trúng đòn cho Hit stop & Trauma)
    int ApplyShockwave(const Vec3& center, float radius, float damage, float knockbackStrength, EffectSystem& effects);

    // Lấy danh sách quái cho súng ngắm
    Enemy* FindClosestEnemy(const Vec3& fromPos, float maxRange);

    // Va chạm với đạn
    bool CheckBulletHit(const Vec3& bulletPos, float bulletRadius, float damage, Vec3 bulletDir, EffectSystem& effects, int& outExp);

    int GetActiveCount() const;
    float GetAliveCost() const;
    int GetSpawnedThisWave() const { return spawnedThisWave_; }
    int GetKilledThisWave() const { return killedThisWave_; }
    int GetTotalKills() const { return totalKills_; }
    void RecordKill() { killedThisWave_++; totalKills_++; }
    void KillAll(EffectSystem& effects);

    void SetWave(int wave, bool isEndless = false);
    void SetEndless(bool enable) { isEndless_ = enable; }
    bool IsEndless() const { return isEndless_; }
    void SetAutoSpawn(bool enable) { autoSpawn_ = enable; }

    // Quản lý mô hình 3D (Đa quái: Minion, Hopper, Tanker, Float, Charge)
    void Release();
    void Shutdown() { Release(); }
    ~EnemyManager() { Release(); }

    int GetModelHandle(EnemyType type) const;
    float GetModelScale(EnemyType type) const;
    void SetModelScale(EnemyType type, float s);
    float GetModelRotYDeg(EnemyType type) const;
    void SetModelRotYDeg(EnemyType type, float deg);
    float GetModelOffsetY(EnemyType type) const;
    void SetModelOffsetY(EnemyType type, float y);

    // Tương thích ngược với điều khiển Cua (debug keys)
    float GetCrabModelScale() const { return GetModelScale(EnemyType::Hopper); }
    void SetCrabModelScale(float s) { SetModelScale(EnemyType::Hopper, s); }
    float GetCrabModelRotYDeg() const { return GetModelRotYDeg(EnemyType::Hopper); }
    void SetCrabModelRotYDeg(float deg) { SetModelRotYDeg(EnemyType::Hopper, deg); }
    float GetCrabModelOffsetY() const { return GetModelOffsetY(EnemyType::Hopper); }
    void SetCrabModelOffsetY(float y) { SetModelOffsetY(EnemyType::Hopper, y); }

private:
    static constexpr size_t ENEMY_TYPE_COUNT = 5;

    std::array<Enemy, GameConsts::MAX_ENEMIES> enemies_{};
    float spawnTimer_ = 0.0f;
    float waveTimer_ = 0.0f;
    int currentWave_ = 1;
    bool autoSpawn_ = true;
    bool isEndless_ = false;
    float hpMultiplier_ = 1.0f;
    float dmgMultiplier_ = 1.0f;
    float spdMultiplier_ = 1.0f;
    int spawnedThisWave_ = 0;
    int killedThisWave_ = 0;
    int totalKills_ = 0;

    std::array<EnemyModelConfig, ENEMY_TYPE_COUNT> modelConfigs_{};
    int sharedTextureHandle_ = -1;
    int fallbackCrabHandle_ = -1;
    int fallbackCrabTexHandle_ = -1;
};
