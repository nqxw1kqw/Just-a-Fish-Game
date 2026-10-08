#pragma once
#include "Vector3.h"
#include "Consts.h"
#include "Stats.h"
#include <array>

class EnemyManager;
class EffectSystem;
class PlayerFish;
class WhaleBoss;

struct Bullet
{
    Vec3 pos{ 0, 0, 0 };
    Vec3 vel{ 0, 0, 0 };
    float life = 0.0f;
    float maxLife = GameConsts::BULLET_LIFETIME;
    float damage = GameConsts::BULLET_DAMAGE;
    float radius = 5.0f;
    bool active = false;
};

class WeaponSystem
{
public:
    void Init();
    void Reset();
    void Update(float dt, const PlayerFish& player, EnemyManager& enemies, EffectSystem& effects, WhaleBoss* boss = nullptr);
    void Draw(const PlayerFish& player) const;

    void ApplyStats(const PlayerStats& stats);
    void SetHooks(bool hasLightningChain, bool hasWaterfallCarp, bool hasShark = false);

    // Bắn đạn tỏa tròn theo hình nan quạt / vòng tròn (Mưa đạn, Cá nóc phản đòn)
    void ShootRadial(const Vec3& center, int count, float damage, float speed);

    float GetAimYaw() const { return aimYaw_; }
    bool HasTarget() const { return hasTarget_; }

private:
    void Shoot(const Vec3& startPos, const Vec3& targetPos, float damage, float speed, int bulletCount);

    std::array<Bullet, GameConsts::MAX_BULLETS> bullets_{};
    float fireCooldownTimer_ = 0.0f;
    float aimScanTimer_ = 0.0f;
    float aimYaw_ = 0.0f;
    bool hasTarget_ = false;
    Vec3 currentTargetPos_{ 0, 0, 0 };

    // Stats
    float baseDamage_ = GameConsts::BULLET_DAMAGE;
    float fireCooldown_ = GameConsts::WEAPON_FIRE_COOLDOWN;
    float bulletSpeed_ = GameConsts::BULLET_SPEED;
    int bulletCount_ = 1;

    // Hooks
    bool hasLightningChainHook_ = false;
    bool hasWaterfallCarpHook_ = false;
    bool hasSharkHook_ = false;
};
