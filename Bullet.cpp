#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include "Bullet.h"
#include "Enemy.h"
#include "PlayerFish.h"
#include "Boss.h"
#include "Effects.h"
#include "Render3DUtil.h"
#include "DxLib.h"

void WeaponSystem::Init()
{
    Reset();
}

void WeaponSystem::Reset()
{
    for (auto& b : bullets_) b.active = false;
    fireCooldownTimer_ = 0.0f;
    aimScanTimer_ = 0.0f;
    aimYaw_ = 0.0f;
    hasTarget_ = false;

    baseDamage_ = GameConsts::BULLET_DAMAGE;
    fireCooldown_ = GameConsts::WEAPON_FIRE_COOLDOWN;
    bulletSpeed_ = GameConsts::BULLET_SPEED;
    bulletCount_ = 1;

    hasLightningChainHook_ = false;
    hasWaterfallCarpHook_ = false;
}

void WeaponSystem::ApplyStats(const PlayerStats& stats)
{
    baseDamage_ = stats.bulletDamage.GetValue();
    fireCooldown_ = stats.fireCooldown.GetValue();
    bulletSpeed_ = stats.bulletSpeed.GetValue();
    bulletCount_ = (std::max)(1, static_cast<int>(stats.bulletCount.GetValue()));
}

void WeaponSystem::SetHooks(bool hasLightningChain, bool hasWaterfallCarp, bool hasShark)
{
    hasLightningChainHook_ = hasLightningChain;
    hasWaterfallCarpHook_ = hasWaterfallCarp;
    hasSharkHook_ = hasShark;
}

void WeaponSystem::Shoot(const Vec3& startPos, const Vec3& targetPos, float damage, float speed, int bulletCount)
{
    Vec3 baseDir = targetPos - startPos;
    baseDir.y = 0.0f; // Bắn ngang trên mặt phẳng XZ
    float len = baseDir.Length();
    if (len > 0.01f)
    {
        baseDir = baseDir / len;
    }
    else
    {
        baseDir = Vec3{ 0, 0, 1 };
    }

    float baseAngle = std::atan2(baseDir.x, baseDir.z);
    float spreadAngle = 0.14f; // ~8 độ

    for (int i = 0; i < bulletCount; ++i)
    {
        float angleOffset = 0.0f;
        if (bulletCount > 1)
        {
            angleOffset = (i - (bulletCount - 1) * 0.5f) * spreadAngle;
        }

        float angle = baseAngle + angleOffset;
        Vec3 dir{ std::sin(angle), 0.0f, std::cos(angle) };

        for (auto& b : bullets_)
        {
            if (!b.active)
            {
                b.pos = startPos;
                b.vel = dir * speed;
                b.life = 0.0f;
                b.damage = damage;
                b.radius = 6.0f;
                b.active = true;
                break;
            }
        }
    }
}

void WeaponSystem::ShootRadial(const Vec3& center, int count, float damage, float speed)
{
    if (count <= 0) return;
    float step = 6.2831853f / count;

    for (int i = 0; i < count; ++i)
    {
        float a = i * step;
        Vec3 dir{ std::sin(a), 0.0f, std::cos(a) };

        for (auto& b : bullets_)
        {
            if (!b.active)
            {
                b.pos = center + Vec3{ 0.0f, 6.0f, 0.0f };
                b.vel = dir * speed;
                b.life = 0.0f;
                b.damage = damage;
                b.radius = 6.0f;
                b.active = true;
                break;
            }
        }
    }
}

void WeaponSystem::Update(float dt, const PlayerFish& player, EnemyManager& enemies, EffectSystem& effects, WhaleBoss* boss)
{
    const Vec3 playerPos = player.GetPosition();
    const Vec3 turretPos = playerPos + Vec3{ 0.0f, GameConsts::FISH_SIZE_Y + 4.0f, 0.0f };

    if constexpr (GameConsts::ENABLE_AUTO_GUN)
    {
        aimScanTimer_ += dt;
        if (aimScanTimer_ >= 0.08f)
        {
            aimScanTimer_ = 0.0f;
            hasTarget_ = false;

            // 1. Quét Boss trước nếu Boss đang hoạt động
            if (boss && boss->IsAlive())
            {
                Vec3 toBoss = boss->GetPosition() - turretPos;
                if (toBoss.Length() <= GameConsts::WEAPON_RANGE + 120.0f)
                {
                    hasTarget_ = true;
                    currentTargetPos_ = boss->GetPosition() + Vec3{ 0.0f, 25.0f, 0.0f };
                    aimYaw_ = std::atan2(toBoss.x, toBoss.z);
                }
            }

            // 2. Nếu không nhắm boss, quét quái thường gần nhất
            if (!hasTarget_)
            {
                Enemy* closest = enemies.FindClosestEnemy(turretPos, GameConsts::WEAPON_RANGE);
                if (closest)
                {
                    hasTarget_ = true;
                    currentTargetPos_ = closest->pos + Vec3{ 0.0f, 10.0f, 0.0f };
                    Vec3 diff = currentTargetPos_ - turretPos;
                    aimYaw_ = std::atan2(diff.x, diff.z);
                }
                else
                {
                    aimYaw_ = player.GetYaw();
                }
            }
        }

        // Tự động bắn theo hồi chiêu
        fireCooldownTimer_ -= dt;
        if (fireCooldownTimer_ <= 0.0f && hasTarget_)
        {
            float cd = fireCooldown_;
            if (player.IsApexInvincible())
            {
                // Hook 10: Cá chép vượt thác bắn nhanh x2, bình thường x1.45
                cd *= (hasWaterfallCarpHook_ ? 0.50f : 0.68f);
            }
            fireCooldownTimer_ = cd;

            float dmg = baseDamage_ * (player.IsComboBuffActive() ? 1.5f : 1.0f);
            Shoot(turretPos, currentTargetPos_, dmg, bulletSpeed_, bulletCount_);
            effects.AddTrauma(0.04f);
        }
    }

    // Cập nhật các viên đạn đang bay
    for (auto& b : bullets_)
    {
        if (!b.active) continue;

        b.life += dt;
        if (b.life >= b.maxLife)
        {
            b.active = false;
            continue;
        }

        b.pos += b.vel * dt;

        // Giới hạn trong sàn đấu
        if (std::abs(b.pos.x) > GameConsts::ARENA_HALF_SIZE + 20.0f ||
            std::abs(b.pos.z) > GameConsts::ARENA_HALF_SIZE + 20.0f)
        {
            b.active = false;
            continue;
        }

        // 1. Kiểm tra va chạm với Boss
        if (boss && boss->IsAlive())
        {
            Vec3 toBoss = boss->GetPosition() - b.pos;
            toBoss.y = 0.0f;
            if (toBoss.Length() <= (b.radius + boss->GetHitboxRadius()))
            {
                boss->TakeDamage(b.damage, effects);
                b.active = false;
                effects.SpawnHitParticles(b.pos, 5, GetColor(255, 230, 80));
                continue;
            }
        }

        // 2. Kiểm tra va chạm với quái thường
        int gainedExp = 0;
        Vec3 bulletDir = b.vel.Normalized();
        if (enemies.CheckBulletHit(b.pos, b.radius, b.damage, bulletDir, effects, gainedExp))
        {
            b.active = false;

            // Hook 9: Vây điện - giật sét sang 1 quái lân cận
            if (hasLightningChainHook_)
            {
                Enemy* chained = enemies.FindClosestEnemy(b.pos, 160.0f);
                if (chained)
                {
                    int extraExp = 0;
                    float chainDmg = b.damage * 0.5f;
                    chained->hp -= chainDmg;
                    chained->hitFlashTimer = 0.14f;
                    effects.SpawnHitParticles(chained->pos, 6, GetColor(160, 240, 255));
                    effects.SpawnDamagePopup(chained->pos + Vec3{ 0, chained->radius + 6.0f, 0 }, chainDmg, false);
                    if (chained->hp <= 0.0f)
                    {
                        chained->active = false;
                        extraExp = 1;
                        effects.SpawnExpOrb(chained->pos, 1.0f);
                    }
                    gainedExp += extraExp;
                }
            }

            if (gainedExp > 0)
            {
                const_cast<PlayerFish&>(player).AddExp(gainedExp);

                // Hook 12: Cá mập đói - giết quái có 15% cơ hội hồi 5 máu
                if (hasSharkHook_ && (rand() % 100 < 15))
                {
                    const_cast<PlayerFish&>(player).Heal(5.0f);
                    effects.SpawnHitParticles(player.GetPosition() + Vec3{ 0, 15, 0 }, 6, GetColor(80, 255, 120));
                }
            }
        }
    }
}

void WeaponSystem::Draw(const PlayerFish& player) const
{
    const Vec3 playerPos = player.GetPosition();
    const Vec3 turretBase = playerPos + Vec3{ 0.0f, GameConsts::FISH_SIZE_Y + 2.0f, 0.0f };

    // 1. Đế súng gắn trên lưng cá
    Vec3 baseHalfSize{ 5.0f, 3.5f, 5.0f };
    int baseColor = GetColor(190, 205, 220);
    int edgeColor = GetColor(80, 100, 130);
    Render3D::DrawOrientedBox3D(turretBase, baseHalfSize, aimYaw_, baseColor, edgeColor, true);

    // 2. Nòng súng vươn ra theo hướng ngắm
    float barrelLen = 9.0f;
    Vec3 barrelCenter = turretBase + Vec3{
        std::sin(aimYaw_) * barrelLen,
        1.5f,
        std::cos(aimYaw_) * barrelLen
    };
    Vec3 barrelHalfSize{ 2.5f, 2.5f, 6.0f };
    int barrelColor = hasTarget_ ? GetColor(255, 80, 80) : GetColor(160, 180, 200);
    Render3D::DrawOrientedBox3D(barrelCenter, barrelHalfSize, aimYaw_, barrelColor, edgeColor, true);

    // 3. Vẽ các viên đạn 3D
    int bulletColor = player.IsComboBuffActive() ? GetColor(255, 120, 255) : GetColor(255, 230, 40);
    int bulletEdge = GetColor(255, 120, 20);
    Vec3 bulletHalfSize{ 4.0f, 4.0f, 7.0f };

    for (const auto& b : bullets_)
    {
        if (!b.active) continue;

        float bYaw = std::atan2(b.vel.x, b.vel.z);
        Render3D::DrawOrientedBox3D(b.pos, bulletHalfSize, bYaw, bulletColor, bulletEdge, true);
    }
}
