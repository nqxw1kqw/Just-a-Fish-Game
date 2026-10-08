#pragma once
#include <string>
#include <vector>
#include <cmath>
#include "Consts.h"

enum class ModifierType
{
    Add,     // Cộng thẳng (+5)
    Percent, // Phần trăm cộng dồn (+0.1 = +10%)
    More     // Nhân độc lập (x1.5)
};

struct StatModifier
{
    std::string sourceId;
    ModifierType type = ModifierType::Add;
    float value = 0.0f;
};

class StatValue
{
public:
    explicit StatValue(float base = 0.0f) : base_(base), final_(base), dirty_(true) {}

    void SetBase(float base)
    {
        base_ = base;
        dirty_ = true;
    }

    float GetBase() const { return base_; }

    void AddModifier(const std::string& sourceId, ModifierType type, float value)
    {
        modifiers_.push_back({ sourceId, type, value });
        dirty_ = true;
    }

    void ClearModifiers()
    {
        modifiers_.clear();
        dirty_ = true;
    }

    float GetValue() const
    {
        if (dirty_)
        {
            Calculate();
        }
        return final_;
    }

private:
    void Calculate() const
    {
        float sumAdd = 0.0f;
        float sumPercent = 0.0f;
        float prodMore = 1.0f;

        for (const auto& mod : modifiers_)
        {
            switch (mod.type)
            {
            case ModifierType::Add:
                sumAdd += mod.value;
                break;
            case ModifierType::Percent:
                sumPercent += mod.value;
                break;
            case ModifierType::More:
                prodMore *= mod.value;
                break;
            }
        }

        // final = (base + sum(add)) * (1 + sum(percent)) * product(more)
        final_ = (base_ + sumAdd) * (1.0f + sumPercent) * prodMore;
        dirty_ = false;
    }

    float base_ = 0.0f;
    mutable float final_ = 0.0f;
    mutable bool dirty_ = true;
    std::vector<StatModifier> modifiers_;
};

struct PlayerStats
{
    StatValue maxHp{ 100.0f };
    StatValue moveSpeed{ GameConsts::FISH_SPEED };
    StatValue bounceDuration{ GameConsts::JUMP_DURATION };
    StatValue bounceHeight{ GameConsts::JUMP_HEIGHT };
    StatValue apexThreshold{ GameConsts::APEX_THRESHOLD };

    // Chỉ số Lõi nhảy & Tấn công tiếp đất (J9)
    StatValue slamDamage{ GameConsts::SLAM_DAMAGE };
    StatValue slamRadius{ GameConsts::SLAM_RADIUS };
    StatValue jumpCooldown{ GameConsts::JUMP_COOLDOWN };
    StatValue jumpHeight{ GameConsts::JUMP_HEIGHT };
    StatValue airSpeed{ GameConsts::FISH_SPEED * GameConsts::AIR_SPEED_MUL }; // hiện chưa có thẻ nào dùng
    StatValue slamKnockback{ GameConsts::SLAM_KNOCKBACK };
    StatValue extraJumps{ 0.0f }; // hiện chưa có thẻ nào dùng

    // Tương thích ngược với hệ cũ
    StatValue shockwaveDamage{ GameConsts::SLAM_DAMAGE };
    StatValue shockwaveRadius{ GameConsts::SLAM_RADIUS };

    StatValue bulletDamage{ 22.0f };
    StatValue fireCooldown{ 0.16f };
    StatValue bulletSpeed{ 850.0f };
    StatValue bulletCount{ 1.0f }; // Số tia đạn

    void ResetToBase()
    {
        maxHp.ClearModifiers();
        moveSpeed.ClearModifiers();
        bounceDuration.ClearModifiers();
        bounceHeight.ClearModifiers();
        apexThreshold.ClearModifiers();

        slamDamage.ClearModifiers();
        slamRadius.ClearModifiers();
        jumpCooldown.ClearModifiers();
        jumpHeight.ClearModifiers();
        airSpeed.ClearModifiers();
        slamKnockback.ClearModifiers();
        extraJumps.ClearModifiers();

        shockwaveDamage.ClearModifiers();
        shockwaveRadius.ClearModifiers();
        bulletDamage.ClearModifiers();
        fireCooldown.ClearModifiers();
        bulletSpeed.ClearModifiers();
        bulletCount.ClearModifiers();
    }
};
