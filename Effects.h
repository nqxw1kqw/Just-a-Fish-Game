#pragma once
#include "Vector3.h"
#include "DxLib.h"
#include <vector>
#include <array>

// Vòng sóng nước nở trên mặt sàn
struct Shockwave
{
    Vec3 center;
    float currentRadius = 0.0f;
    float maxRadius = 130.0f;
    float life = 0.0f;
    float maxLife = 0.35f;
    bool active = false;
};

// Hạt vụn 3D
struct Particle3D
{
    Vec3 pos;
    Vec3 vel;
    Vec3 size;
    int color = 0;
    float life = 0.0f;
    float maxLife = 0.5f;
    bool active = false;
};

// Hạt ngọc kinh nghiệm / tiền
struct ExpOrb
{
    Vec3 pos;
    Vec3 vel;
    float value = 1.0f;
    float life = 0.0f;
    bool active = false;
};

// Số sát thương nổi 3D chiếu lên màn hình 2D
struct DamagePopup
{
    Vec3 pos;
    Vec3 vel;
    float damage = 0.0f;
    float life = 0.0f;
    float maxLife = 0.70f;
    bool isCrit = false;
    bool active = false;
};

#include "WaterSplash.h"
#include "BubbleSystem.h"

class EffectSystem
{
public:
    void Init();
    void Reset();
    void Shutdown();
    void Update(float dt, const Vec3& playerPos);
    void Draw() const;
    void DrawDamagePopups() const;

    void SpawnShockwave(const Vec3& pos, float maxRadius = 130.0f);
    void SpawnWaterSplash(const Vec3& pos, float radius, float strength = 1.0f);
    void SpawnBubbles(const Vec3& center, int count, float spread, float sizeMin = 2.5f, float sizeMax = 6.0f);
    void SpawnSwimWake(const Vec3& fishPos, const Vec3& moveDir);
    void SpawnFishSwim(const Vec3& pos, const Vec3& dir, float speed, float sizeMul = 1.0f, bool isDashing = false);
    void SpawnTurtlePaddle(const Vec3& pos, const Vec3& dir, float radius);
    void SpawnFrogHop(const Vec3& pos, const Vec3& dir, float radius);
    void SpawnFrogLand(const Vec3& pos, float radius);
    void SpawnCrabScuttle(const Vec3& pos, const Vec3& moveDir, const Vec3& facingDir, float radius);
    void DrawShallowOcean(float arenaHalfSize) const;
    void SpawnHitParticles(const Vec3& pos, int count, int color);
    void SpawnExpOrb(const Vec3& pos, float val = 1.0f);
    void SpawnDamagePopup(const Vec3& pos, float damage, bool isCrit = false);
    void AddTrauma(float amount);

    Vec3 GetCameraShakeOffset(float shakeScale = 1.0f) const;
    int CollectExpOrbs(const Vec3& playerPos, float pickupRadius);

private:
    static constexpr size_t MAX_DAMAGE_POPUPS = 128;

    std::array<Shockwave, 32> shockwaves_{};
    std::array<Particle3D, 500> particles_{};
    std::array<ExpOrb, 300> expOrbs_{};
    std::array<DamagePopup, MAX_DAMAGE_POPUPS> damagePopups_{};

    WaterSplashSystem waterSplash_;
    BubbleSystem bubbles_; // #17 泡パーティクル: bọt nổi từ sàn

    float trauma_ = 0.0f; // Chấn động camera [0, 1]
    float shakeTime_ = 0.0f;
};
