#include "BattleHud.h"
#include "SquidBoss.h"
#include "SharkBoss.h"
#include "PlayerFish.h"
#include "Boss.h"
#include "FontManager.h"
#include "Localization.h"
#include "Consts.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

namespace
{
    constexpr int PANEL_X = 28;
    constexpr int PANEL_Y = 20;
    constexpr int PANEL_W = 400;
    constexpr int PANEL_H = 112;
    constexpr float FLASH_DURATION = 0.35f;

    int Lerp(int a, int b, float t) { return a + static_cast<int>((b - a) * t); }

    // Thanh có viền + nền + phần đầy (dạng bo góc nhẹ)
    void DrawBar(int x, int y, int w, int h, float ratio, unsigned int back, unsigned int fill, unsigned int hilite)
    {
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        DrawBox(x - 2, y - 2, x + w + 2, y + h + 2, GetColor(8, 14, 24), TRUE);
        DrawBox(x, y, x + w, y + h, back, TRUE);
        int fw = static_cast<int>(w * ratio);
        if (fw > 0)
        {
            DrawBox(x, y, x + fw, y + h, fill, TRUE);
            DrawBox(x, y, x + fw, y + (std::max)(1, h / 3), hilite, TRUE); // ánh sáng trên
        }
    }

    // Vòng tròn tiến độ (hồi chiêu) vẽ bằng tam giác quạt
    void DrawRadial(float cx, float cy, float r, float ratio, unsigned int color)
    {
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        constexpr int SEG = 40;
        int n = static_cast<int>(SEG * ratio + 0.5f);
        const float start = -1.5707963f;
        for (int i = 0; i < n; ++i)
        {
            float a0 = start + 6.2831853f * i / SEG;
            float a1 = start + 6.2831853f * (i + 1) / SEG;
            DrawTriangleAA(cx, cy, cx + std::cos(a0) * r, cy + std::sin(a0) * r, cx + std::cos(a1) * r, cy + std::sin(a1) * r, color, TRUE);
        }
    }
}

float BattleHud::FrameDt() const
{
    long long now = GetNowHiPerformanceCount();
    float dt = (lastCount_ == 0) ? 0.0f : static_cast<float>(now - lastCount_) / 1000000.0f;
    lastCount_ = now;
    return std::clamp(dt, 0.0f, 0.1f);
}

void BattleHud::Reset() const
{
    lastPlayerHp_ = trailPlayerHp_ = -1.0f;
    lastBossHp_ = trailBossHp_ = -1.0f;
    flashTimer_ = 0.0f;
    bossShake_ = 0.0f;
}

int BattleHud::PanelBottomY()
{
    return PANEL_Y + PANEL_H + 10;
}

void BattleHud::DrawPlayerPanel(const PlayerFish& player) const
{
    dt_ = FrameDt();
    pulse_ += dt_;

    float hp = player.GetHp(), maxHp = (std::max)(1.0f, player.GetMaxHp());
    if (lastPlayerHp_ < 0.0f) trailPlayerHp_ = hp;
    if (lastPlayerHp_ >= 0.0f && hp < lastPlayerHp_ - 0.01f) flashTimer_ = FLASH_DURATION; // bị trúng đòn
    lastPlayerHp_ = hp;
    if (trailPlayerHp_ < hp) trailPlayerHp_ = hp;
    trailPlayerHp_ += (hp - trailPlayerHp_) * (std::min)(1.0f, dt_ * 3.0f);   // thanh máu trễ
    if (flashTimer_ > 0.0f) flashTimer_ -= dt_;

    // Khung nền bán trong suốt
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
    DrawRoundRect(PANEL_X, PANEL_Y, PANEL_X + PANEL_W, PANEL_Y + PANEL_H, 14, 14, GetColor(6, 20, 38), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawRoundRect(PANEL_X, PANEL_Y, PANEL_X + PANEL_W, PANEL_Y + PANEL_H, 14, 14, GetColor(70, 160, 220), FALSE);

    // Huy hiệu Level (vòng tròn bên trái)
    float bx = PANEL_X + 50.0f, by = PANEL_Y + PANEL_H * 0.5f;
    DrawCircleAA(bx, by, 36.0f, 48, GetColor(12, 40, 70), TRUE);
    float expRatio = (player.GetExpNext() > 0) ? static_cast<float>(player.GetExp()) / player.GetExpNext() : 0.0f;
    DrawRadial(bx, by, 36.0f, expRatio, GetColor(120, 230, 255));          // vòng EXP
    DrawCircleAA(bx, by, 29.0f, 48, GetColor(10, 30, 55), TRUE);
    DrawCircleAA(bx, by, 36.0f, 48, GetColor(140, 210, 255), FALSE, 2.0f);
    FontManager::Draw(static_cast<int>(bx) - 11, static_cast<int>(by) - 22, L"Lv", GetColor(150, 200, 235), FontSize::Small);
    FontManager::DrawFormat(static_cast<int>(bx) - (player.GetLevel() >= 10 ? 13 : 7), static_cast<int>(by) - 6, GetColor(255, 255, 255), FontSize::Medium, L"%d", player.GetLevel());

    // Thanh HP (+ phần máu trễ màu trắng)
    int hx = PANEL_X + 100, hy = PANEL_Y + 18, hw = 280, hh = 22;
    float ratio = hp / maxHp;
    bool low = ratio <= 0.3f;
    unsigned int hpFill = low ? GetColor(255, 70, 70) : GetColor(70, 220, 130);
    if (low && std::fmod(pulse_, 0.8f) < 0.4f) hpFill = GetColor(255, 130, 110);   // nhấp nháy khi máu thấp
    DrawBar(hx, hy, hw, hh, trailPlayerHp_ / maxHp, GetColor(40, 20, 28), GetColor(240, 240, 240), GetColor(255, 255, 255));
    DrawBox(hx + static_cast<int>(hw * std::clamp(ratio, 0.0f, 1.0f)), hy, hx + hw, hy + hh, GetColor(40, 20, 28), TRUE);
    if (ratio < trailPlayerHp_ / maxHp)
        DrawBox(hx + static_cast<int>(hw * std::clamp(ratio, 0.0f, 1.0f)), hy, hx + static_cast<int>(hw * std::clamp(trailPlayerHp_ / maxHp, 0.0f, 1.0f)), hy + hh, GetColor(255, 220, 200), TRUE);
    int fw = static_cast<int>(hw * std::clamp(ratio, 0.0f, 1.0f));
    if (fw > 0)
    {
        DrawBox(hx, hy, hx + fw, hy + hh, hpFill, TRUE);
        DrawBox(hx, hy, hx + fw, hy + 7, low ? GetColor(255, 160, 150) : GetColor(150, 255, 190), TRUE);
    }
    FontManager::Draw(hx + 6, hy + 3, L"HP", GetColor(15, 30, 20), FontSize::Body);
    FontManager::DrawFormat(hx + hw - 81, hy + 4, GetColor(0, 0, 0), FontSize::Body, L"%3.0f / %3.0f", hp, maxHp);   // bóng chữ
    FontManager::DrawFormat(hx + hw - 82, hy + 3, GetColor(255, 255, 255), FontSize::Body, L"%3.0f / %3.0f", hp, maxHp);

    // Thanh EXP mảnh
    int ey = hy + hh + 8;
    DrawBar(hx, ey, hw, 8, expRatio, GetColor(18, 30, 50), GetColor(90, 200, 255), GetColor(170, 235, 255));
    FontManager::DrawFormat(hx, ey + 11, GetColor(150, 200, 235), FontSize::Small, L"EXP %d / %d", player.GetExp(), player.GetExpNext());

    // Ô kỹ năng Slam (vòng hồi chiêu)
    float sx = static_cast<float>(hx + 22), sy = static_cast<float>(PANEL_Y + PANEL_H - 24);
    int textX = hx + 50, textY = PANEL_Y + PANEL_H - 33;
    DrawCircleAA(sx, sy, 17.0f, 36, GetColor(20, 30, 50), TRUE);
    if (player.IsAirborne())
    {
        DrawCircleAA(sx, sy, 17.0f, 36, GetColor(255, 215, 60), TRUE);
        FontManager::Draw(textX, textY, TR(L"AIRBORNE  - INVINCIBLE"), GetColor(255, 225, 90), FontSize::Body);
    }
    else if (player.GetLandRecoveryTimer() > 0.0f)
    {
        DrawRadial(sx, sy, 17.0f, 1.0f - player.GetLandRecoveryTimer() / GameConsts::LAND_RECOVERY, GetColor(230, 110, 50));
        FontManager::Draw(textX, textY, TR(L"RECOVERING..."), GetColor(255, 170, 120), FontSize::Body);
    }
    else if (player.CanJump())
    {
        float glow = 0.5f + 0.5f * std::sin(pulse_ * 6.0f);
        DrawCircleAA(sx, sy, 17.0f, 36, GetColor(Lerp(40, 90, glow), Lerp(190, 255, glow), Lerp(120, 170, glow)), TRUE);
        FontManager::Draw(textX, textY, TR(L"[SPACE] SLAM READY"), GetColor(140, 255, 190), FontSize::Body);
    }
    else
    {
        float cdMax = (std::max)(0.01f, player.GetJumpCooldownMax());
        DrawRadial(sx, sy, 17.0f, 1.0f - player.GetJumpCooldownTimer() / cdMax, GetColor(80, 160, 230));
        FontManager::DrawFormat(textX, textY, GetColor(190, 215, 240), FontSize::Body, TR(L"SLAM  %.1fs"), player.GetJumpCooldownTimer());
    }
    DrawCircleAA(sx, sy, 17.0f, 36, GetColor(200, 230, 255), FALSE, 2.0f);
    FontManager::Draw(static_cast<int>(sx) - 4, static_cast<int>(sy) - 8, L"S", GetColor(10, 20, 30), FontSize::Body);

    // Combo
    if (player.GetComboCount() >= 2)
    {
        unsigned int cc = player.IsComboBuffActive() ? GetColor(255, 200, 60) : GetColor(220, 230, 245);
        FontManager::DrawFormat(PANEL_X + PANEL_W - 110, textY, cc, FontSize::Body, player.IsComboBuffActive() ? TR(L"x%d COMBO!") : TR(L"x%d combo"), player.GetComboCount());
    }
}

void BattleHud::DrawBossBar(const WhaleBoss& boss) const
{
    const bool p2 = boss.GetPhase() == BossPhase::Phase2;
    DrawBossBarCore(boss.IsActive(), boss.GetHp(), boss.GetMaxHp(), p2, TR(L"GIANT WHALE"), p2 ? TR(L"PHASE 2 - ENRAGED") : TR(L"PHASE 1"), 0.5f);
}

void BattleHud::DrawBossBar(const SharkBoss& shark) const
{
    const bool fr = shark.IsFrenzy();
    DrawBossBarCore(shark.IsActive(), shark.GetHp(), shark.GetMaxHp(), fr, TR(L"GREAT SHARK"), fr ? TR(L"FRENZY") : TR(L"HUNTING"), GameConsts::SHARK_FRENZY_HP_RATIO);
}

void BattleHud::DrawBossBar(const SquidBoss& squid) const
{
    const bool en = squid.IsEnraged();
    DrawBossBarCore(squid.IsActive(), squid.GetHp(), squid.GetMaxHp(), en, TR(L"GIANT SQUID"), en ? TR(L"ENRAGED - DOUBLE TENTACLES") : TR(L"RANGED THREAT"), GameConsts::SQUID_ENRAGE_HP_RATIO);
}

void BattleHud::DrawBossBarCore(bool active, float hp, float maxHp, bool p2, const wchar_t* name, const wchar_t* phaseLabel, float markRatio) const
{
    if (!active) { lastBossHp_ = trailBossHp_ = -1.0f; return; }

    maxHp = (std::max)(1.0f, maxHp);
    if (lastBossHp_ < 0.0f || trailBossHp_ < hp) trailBossHp_ = hp;
    if (lastBossHp_ >= 0.0f && hp < lastBossHp_ - 0.01f) bossShake_ = 0.15f;
    lastBossHp_ = hp;
    trailBossHp_ += (hp - trailBossHp_) * (std::min)(1.0f, dt_ * 2.5f);
    if (bossShake_ > 0.0f) bossShake_ -= dt_;

    int sw = GameConsts::SCREEN_WIDTH;
    int w = 640, h = 22;
    int x = (sw - w) / 2, y = 40;
    if (bossShake_ > 0.0f) x += (GetRand(4) - 2);

    // Khung
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
    DrawRoundRect(x - 16, y - 30, x + w + 16, y + h + 14, 12, 12, GetColor(20, 8, 16), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawRoundRect(x - 16, y - 30, x + w + 16, y + h + 14, 12, 12, p2 ? GetColor(255, 90, 60) : GetColor(255, 200, 80), FALSE);

    // Tên + pha (2 chấm pha)
    FontManager::Draw(x, y - 25, name, GetColor(255, 230, 120), FontSize::Body);
    FontManager::Draw(x + 120, y - 25, phaseLabel, p2 ? GetColor(255, 110, 80) : GetColor(220, 220, 230), FontSize::Body);
    for (int i = 0; i < 2; ++i)
    {
        bool on = (i == 0) || p2;
        float cx = static_cast<float>(x + w - 30 + i * 18), cy = static_cast<float>(y - 16);
        DrawCircleAA(cx, cy, 6.0f, 20, on ? (p2 ? GetColor(255, 90, 50) : GetColor(255, 200, 80)) : GetColor(60, 50, 60), TRUE);
    }

    // Thanh máu: nền -> phần trễ -> phần thật
    DrawBar(x, y, w, h, trailBossHp_ / maxHp, GetColor(45, 15, 22), GetColor(255, 235, 200), GetColor(255, 250, 235));
    float ratio = std::clamp(hp / maxHp, 0.0f, 1.0f);
    int fw = static_cast<int>(w * ratio);
    if (fw > 0)
    {
        DrawBox(x, y, x + fw, y + h, p2 ? GetColor(255, 80, 30) : GetColor(225, 45, 55), TRUE);
        DrawBox(x, y, x + fw, y + 7, p2 ? GetColor(255, 160, 100) : GetColor(255, 120, 120), TRUE);
    }
    // Mốc 50% (chuyển pha)
    int mx = x + static_cast<int>(w * markRatio);
    DrawLine(mx, y - 3, mx, y + h + 3, GetColor(255, 255, 255), 2);
    FontManager::DrawFormat(x + w - 109, y + 4, GetColor(0, 0, 0), FontSize::Body, L"%.0f / %.0f", hp, maxHp);   // bóng chữ
    FontManager::DrawFormat(x + w - 110, y + 3, GetColor(255, 255, 255), FontSize::Body, L"%.0f / %.0f", hp, maxHp);
}

void BattleHud::DrawDamageFlash() const
{
    if (flashTimer_ <= 0.0f) return;
    float t = flashTimer_ / FLASH_DURATION;
    int sw = GameConsts::SCREEN_WIDTH, sh = GameConsts::SCREEN_HEIGHT;
    // Viền đỏ (vignette) quanh màn hình
    constexpr int STEPS = 10;
    for (int i = 0; i < STEPS; ++i)
    {
        int a = static_cast<int>(110.0f * t * (1.0f - static_cast<float>(i) / STEPS));
        int m = i * 9;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
        DrawBox(m, m, sw - m, m + 9, GetColor(255, 30, 30), TRUE);
        DrawBox(m, sh - m - 9, sw - m, sh - m, GetColor(255, 30, 30), TRUE);
        DrawBox(m, m + 9, m + 9, sh - m - 9, GetColor(255, 30, 30), TRUE);
        DrawBox(sw - m - 9, m + 9, sw - m, sh - m - 9, GetColor(255, 30, 30), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}
