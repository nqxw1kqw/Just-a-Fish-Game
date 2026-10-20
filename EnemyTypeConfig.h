#pragma once
// =============================
// EnemyTypeConfig.h
// 敵データ表 / Bảng thông số quái theo loại (Task #25)
// Muốn cân bằng game thì chỉ cần sửa số ở đây.
// =============================
#include <array>
#include <cstddef>
#include "Consts.h"

enum class EnemyType
{
    Minion, // Quái lính đỏ
    Hopper, // Cua nảy cam
    Tanker, // Rùa xanh trâu bò
    Float,  // Sứa bay lơ lửng, gây tê liệt (G4)
    Charge  // Cá mập khựng báo hiệu rồi lao thẳng (G4)
};

constexpr std::size_t ENEMY_TYPE_COUNT = 5;

struct EnemyTypeConfig
{
    const wchar_t* name;
    float baseHp;        // × hpMultiplier (Endless)
    float baseSpeed;     // × spdMultiplier (Endless)
    float radius;
    float baseDamage;    // × dmgMultiplier (Endless)
    float cost;          // Chi phí ngân sách wave
    bool  setSpawnY;     // true: đặt pos.y = spawnY khi sinh
    float spawnY;
    float bounceHeight;  // Chỉ dùng cho Hopper
    float bounceDuration;// Chỉ dùng cho Hopper
};

// Thứ tự phải khớp enum EnemyType
inline constexpr std::array<EnemyTypeConfig, ENEMY_TYPE_COUNT> kEnemyTypeConfigs = { {
    //  name        HP      Speed   Radius  Dmg    Cost                            setY   Y      bounceH bounceT
    { L"小魚",  26.0f,  110.0f, 13.0f,  5.0f,  GameConsts::ENEMY_COST_MINION, true,  0.0f,  0.0f,  0.0f  },
    { L"カニ",  38.0f,  135.0f, 15.0f,  7.0f,  GameConsts::ENEMY_COST_HOPPER, false, 0.0f,  28.0f, 0.36f },
    { L"カメ", 130.0f,   52.0f, 24.0f, 12.0f,  GameConsts::ENEMY_COST_TANKER, true,  0.0f,  0.0f,  0.0f  },
    { L"クラゲ",   32.0f,   60.0f, 16.0f,  7.0f,  GameConsts::ENEMY_COST_FLOAT,  true,  16.0f, 0.0f,  0.0f  },
    { L"突進ザメ",  45.0f,   92.0f, 17.0f, 12.0f,  GameConsts::ENEMY_COST_CHARGE, true,  0.0f,  0.0f,  0.0f  },
} };

inline const EnemyTypeConfig* GetEnemyTypeConfig(EnemyType type)
{
    std::size_t idx = static_cast<std::size_t>(type);
    return (idx < ENEMY_TYPE_COUNT) ? &kEnemyTypeConfigs[idx] : nullptr;
}

// 出現率表 / Tỉ lệ xuất hiện: ngưỡng cộng dồn (roll 0..99) theo thứ tự
// Minion, Hopper, Tanker, Float, Charge. Chọn loại đầu tiên có roll < ngưỡng.
struct EnemySpawnWeights
{
    int cumulative[ENEMY_TYPE_COUNT];
};

// Classic: index 0 = wave 1 (và <=1), 1 = wave 2, 2 = wave 3, 3 = wave 4, 4 = wave 5+
inline constexpr EnemySpawnWeights kClassicSpawnWeights[5] = {
    { { 75, 100, 100, 100, 100 } }, // W1: Minion 75 / Hopper 25
    { { 70, 100, 100, 100, 100 } }, // W2: Minion 70 / Hopper 30
    { { 45,  75,  90, 100, 100 } }, // W3
    { { 30,  55,  75,  88, 100 } }, // W4
    { { 20,  40,  60,  80, 100 } }, // W5: tổng lực
};

// Endless: index = tier ((wave-1)/5), tier >= 3 dùng index 3
inline constexpr EnemySpawnWeights kEndlessSpawnWeights[4] = {
    { { 45,  70,  85,  93, 100 } }, // Wave 1..5
    { { 30,  55,  75,  88, 100 } }, // Wave 6..10
    { { 20,  40,  65,  82, 100 } }, // Wave 11..15
    { { 15,  30,  55,  78, 100 } }, // Wave 16+
};

inline EnemyType PickEnemyTypeByRoll(const EnemySpawnWeights& w, int roll)
{
    for (std::size_t i = 0; i < ENEMY_TYPE_COUNT; ++i)
    {
        if (roll < w.cumulative[i]) return static_cast<EnemyType>(i);
    }
    return static_cast<EnemyType>(ENEMY_TYPE_COUNT - 1);
}
