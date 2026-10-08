#pragma once
#include <string>
#include <vector>
#include "Stats.h"

enum class CardRarity
{
    Common, // Trắng / Xanh lam nhạt
    Rare,   // Vàng kim
    Epic    // Tím huyền bí
};

enum class HookType
{
    None,
    OnLand,      // Khi tiếp đất (Mưa đạn)
    OnBounce,    // Khi nảy (Đếm chuỗi nảy liên hoàn)
    OnHitTaken,  // Khi bị trúng đòn (Cá nóc bắn gai)
    OnHitDealt,  // Khi đạn bắn trúng quái (Vây điện giật sét)
    OnApex,      // Khi ở đỉnh nảy (Cá chép vượt thác)
    OnKill,      // Khi tiêu diệt quái (Cá mập đói hồi máu)
    OnWaveEnd    // Khi kết thúc wave (hiện chưa có thẻ nào dùng)
};

struct CardModifierEntry
{
    std::string statName; // maxHp, moveSpeed, bounceDuration, bounceHeight, shockwaveDamage, shockwaveRadius, bulletDamage, fireCooldown, bulletSpeed, bulletCount, apexThreshold
    ModifierType type = ModifierType::Percent;
    float value = 0.0f;
};

struct Card
{
    std::string id;
    std::wstring name;
    std::wstring benefit;     // Dòng lợi (màu xanh lá)
    std::wstring drawback;    // Dòng hại (màu đỏ cam)
    CardRarity rarity = CardRarity::Common;
    bool stackable = false;
    int maxStack = 1; // Số lần cộng dồn tối đa trong Endless (Task 10 T4d)
    std::vector<CardModifierEntry> modifiers;
    HookType hook = HookType::None;
    int imageHandle = -1; // Handle đồ họa thẻ (T2 - nạp từ Data/UI/Card/)
};
