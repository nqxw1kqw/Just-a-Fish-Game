#include "UIRenderer.h"
#include "GamePrototype.h"
#include "Boss.h"
#include "FontManager.h"
#include "Consts.h"
#include "DxLib.h"
#include <algorithm>

void UIRenderer::DrawBossHealthBar(const WhaleBoss& boss) const
{
    if (!boss.IsActive()) return;

    int sw = GameConsts::SCREEN_WIDTH;
    int barW = 520;
    int barH = 24;
    int barX = (sw - barW) / 2;
    int barY = 28;

    float ratio = std::clamp(boss.GetHp() / boss.GetMaxHp(), 0.0f, 1.0f);

    // Khung nền
    DrawBox(barX - 3, barY - 3, barX + barW + 3, barY + barH + 3, GetColor(15, 15, 25), TRUE);
    DrawBox(barX, barY, barX + barW, barY + barH, GetColor(50, 15, 20), TRUE);

    // Thanh máu
    int fillColor = (boss.GetPhase() == BossPhase::Phase1) ? GetColor(230, 45, 45) : GetColor(255, 90, 30);
    DrawBox(barX, barY, barX + static_cast<int>(barW * ratio), barY + barH, fillColor, TRUE);
    DrawBox(barX - 3, barY - 3, barX + barW + 3, barY + barH + 3, GetColor(255, 200, 80), FALSE);

    // Chữ tên Boss & Giai đoạn
    const wchar_t* phaseText = (boss.GetPhase() == BossPhase::Phase1) ? L"PHASE 1" : L"PHASE 2: ENRAGED";
    FontManager::DrawFormat(barX + 12, barY - 22, GetColor(255, 230, 100), FontSize::Body, L"FINAL BOSS: GIANT WHALE (%s)", phaseText);
    FontManager::DrawFormat(barX + barW / 2 - 45, barY + 4, GetColor(255, 255, 255), FontSize::Body, L"%.0f / %.0f", boss.GetHp(), boss.GetMaxHp());
}

void UIRenderer::DrawHUD(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    // 1. Thanh máu HP của cá (Góc trên bên trái)
    int hpX = 40;
    int hpY = 30;
    int barW = 260;
    int barH = 22;

    float hpRatio = std::clamp(game.player_.GetHp() / game.player_.GetMaxHp(), 0.0f, 1.0f);
    DrawBox(hpX - 2, hpY - 2, hpX + barW + 2, hpY + barH + 2, GetColor(20, 20, 30), TRUE);
    DrawBox(hpX, hpY, hpX + barW, hpY + barH, GetColor(80, 20, 20), TRUE);
    DrawBox(hpX, hpY, hpX + static_cast<int>(barW * hpRatio), hpY + barH, GetColor(240, 60, 60), TRUE);
    FontManager::DrawFormat(hpX + 8, hpY + 3, GetColor(255, 255, 255), FontSize::Body, L"HP: %.0f / %.0f", game.player_.GetHp(), game.player_.GetMaxHp());

    // 2. Thanh hồi chiêu Nhảy & Tiếp đất Slam (J2, J4, J5)
    int jumpY = hpY + 28;
    int jumpBarH = 18;
    DrawBox(hpX - 2, jumpY - 2, hpX + barW + 2, jumpY + jumpBarH + 2, GetColor(20, 20, 30), TRUE);
    DrawBox(hpX, jumpY, hpX + barW, jumpY + jumpBarH, GetColor(25, 30, 45), TRUE);

    if (game.player_.IsAirborne())
    {
        // Đang bay trên không: Bất tử!
        DrawBox(hpX, jumpY, hpX + barW, jumpY + jumpBarH, GetColor(255, 215, 60), TRUE);
        FontManager::Draw(hpX + 8, jumpY + 1, L"* AIRBORNE (INVINCIBLE) *", GetColor(20, 20, 20), FontSize::Body);
    }
    else if (game.player_.GetLandRecoveryTimer() > 0.0f)
    {
        // Khựng hồi phục sau tiếp đất 0.2s
        float recRatio = game.player_.GetLandRecoveryTimer() / GameConsts::LAND_RECOVERY;
        DrawBox(hpX, jumpY, hpX + static_cast<int>(barW * recRatio), jumpY + jumpBarH, GetColor(230, 90, 40), TRUE);
        FontManager::Draw(hpX + 8, jumpY + 1, L"LANDING RECOVERY (0.2s)", GetColor(255, 255, 255), FontSize::Body);
    }
    else if (game.player_.CanJump())
    {
        // Sẵn sàng nhảy!
        DrawBox(hpX, jumpY, hpX + barW, jumpY + jumpBarH, GetColor(50, 220, 140), TRUE);
        FontManager::Draw(hpX + 8, jumpY + 1, L"[SPACE] READY TO SLAM!", GetColor(15, 30, 20), FontSize::Body);
    }
    else
    {
        // Đang hồi chiêu
        float cdRatio = 1.0f - (game.player_.GetJumpCooldownTimer() / game.player_.GetJumpCooldownMax());
        DrawBox(hpX, jumpY, hpX + static_cast<int>(barW * cdRatio), jumpY + jumpBarH, GetColor(70, 150, 220), TRUE);
        FontManager::DrawFormat(hpX + 8, jumpY + 1, GetColor(255, 255, 255), FontSize::Body, L"COOLDOWN: %.1fs", game.player_.GetJumpCooldownTimer());
    }

    // 3. Trạng thái Wave / Boss ở giữa trên
    if (game.state_ == GameState::Wave)
    {
        int waveSecLeft = (std::max)(0, static_cast<int>(game.waveDuration_ - game.waveTimer_));
        if (game.mode_ == GameMode::Endless)
        {
            FontManager::DrawFormat(sw / 2 - 100, 24, GetColor(100, 220, 255), FontSize::Header, L"ENDLESS WAVE %d", game.currentWave_);
        }
        else
        {
            FontManager::DrawFormat(sw / 2 - 80, 24, GetColor(100, 220, 255), FontSize::Header, L"WAVE %d / 5", game.currentWave_);
        }
        FontManager::DrawFormat(sw / 2 - 95, 52, GetColor(255, 220, 100), FontSize::Body, L"TIME LEFT: %02ds", waveSecLeft);
        FontManager::DrawFormat(sw / 2 - 110, 74, GetColor(255, 180, 80), FontSize::Body, L"ENEMIES: %d | KILLS: %d", game.enemies_.GetActiveCount(), game.enemies_.GetTotalKills());
    }

    // 4. Danh sách các Thẻ đã sở hữu ở góc trái dưới
    int cardListY = jumpY + 28;
    const auto& cards = game.cardManager_.GetActiveCards();
    if (!cards.empty())
    {
        FontManager::Draw(hpX, cardListY, L"[ACTIVE CARDS]:", GetColor(220, 230, 245), FontSize::Body);
        cardListY += 18;
        for (size_t i = 0; i < cards.size() && i < 6; ++i)
        {
            int col = GetColor(200, 220, 255);
            if (cards[i].rarity == CardRarity::Rare) col = GetColor(255, 215, 0);
            else if (cards[i].rarity == CardRarity::Epic) col = GetColor(210, 120, 255);
            FontManager::DrawFormat(hpX + 6, cardListY, col, FontSize::Small, L"- %s", cards[i].name.c_str());
            cardListY += 17;
        }
    }

    // 5. Debug Panel & Debug Badge góc phải (F1)
    game.debugController_.DrawHUD(game);

    // 6. Hướng dẫn điều khiển góc dưới
    int guideY = sh - 45;
    FontManager::Draw(40, guideY, L"[WASD]: Swim  |  [SPACE]: Jump Slam  |  [A/D+Enter / 1,2,3]: Pick Cards  |  [R]: Reset  |  [F1]: Debug  |  [F2]: Shake Cam", GetColor(200, 215, 235), FontSize::Body);
}

void UIRenderer::DrawTitleScreen(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    // Lớp phủ nền tối đại dương
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 215);
    DrawBox(0, 0, sw, sh, GetColor(8, 18, 32), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int midY = sh / 2;

    // Khung viền phong cách Pixel Box biển sâu
    int boxW = 820;
    int boxH = 480;
    int boxX = midX - boxW / 2;
    int boxY = midY - boxH / 2 - 10;
    DrawBox(boxX - 2, boxY - 2, boxX + boxW + 2, boxY + boxH + 2, GetColor(45, 95, 155), FALSE);
    DrawBox(boxX - 4, boxY - 4, boxX + boxW + 4, boxY + boxH + 4, GetColor(20, 50, 90), FALSE);

    // Tiêu đề game
    FontManager::Draw(midX - 250, boxY + 28, L"=== ODORU : JUST A FISH GAME ===", GetColor(100, 220, 255), FontSize::Header);
    FontManager::Draw(midX - 180, boxY + 65, L"CORE MECHANIC: JUMP-TO-ATTACK", GetColor(255, 215, 60), FontSize::Medium);

    // Mô tả cách chơi
    FontManager::Draw(boxX + 50, boxY + 105, L"- Controls: [WASD] or Arrow Keys to swim smoothly in deep waters.", GetColor(220, 235, 255), FontSize::Body);
    FontManager::Draw(boxX + 50, boxY + 130, L"- Attack: Press [SPACE] to launch into the air and slam down!", GetColor(220, 235, 255), FontSize::Body);
    FontManager::Draw(boxX + 50, boxY + 155, L"- Aerial Invincibility: While airborne, completely immune to damage.", GetColor(140, 240, 180), FontSize::Body);
    FontManager::Draw(boxX + 50, boxY + 180, L"- Landing Shockwave: Deals 40 damage, knocks back and breaks armor.", GetColor(255, 180, 120), FontSize::Body);

    // Lựa chọn chế độ chơi (Task 10 T4a)
    int mode1Y = boxY + 225;
    int mode2Y = boxY + 285;
    bool isClassicSelected = (game.titleSelectedMode_ == 0);
    bool isEndlessSelected = (game.titleSelectedMode_ == 1);

    // Box Mode 1: Classic
    int bgCol1 = isClassicSelected ? GetColor(25, 55, 95) : GetColor(15, 25, 40);
    int edgeCol1 = isClassicSelected ? GetColor(255, 220, 80) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode1Y, boxX + boxW - 50, mode1Y + 48, bgCol1, TRUE);
    DrawBox(boxX + 50, mode1Y, boxX + boxW - 50, mode1Y + 48, edgeCol1, FALSE);
    const wchar_t* pfx1 = isClassicSelected ? L"> [1] CLASSIC MODE :" : L"  [1] CLASSIC MODE :";
    FontManager::Draw(boxX + 65, mode1Y + 12, pfx1, isClassicSelected ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode1Y + 14, L"5 Waves + Giant Whale Final Boss", GetColor(200, 220, 255), FontSize::Body);

    // Box Mode 2: Endless
    int bgCol2 = isEndlessSelected ? GetColor(25, 55, 95) : GetColor(15, 25, 40);
    int edgeCol2 = isEndlessSelected ? GetColor(255, 220, 80) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode2Y, boxX + boxW - 50, mode2Y + 48, bgCol2, TRUE);
    DrawBox(boxX + 50, mode2Y, boxX + boxW - 50, mode2Y + 48, edgeCol2, FALSE);
    const wchar_t* pfx2 = isEndlessSelected ? L"> [2] ENDLESS MODE :" : L"  [2] ENDLESS MODE :";
    FontManager::Draw(boxX + 65, mode2Y + 12, pfx2, isEndlessSelected ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode2Y + 14, L"Infinite Waves, Stacking Cards, Periodic Bosses", GetColor(255, 180, 120), FontSize::Body);

    // Kỷ lục Endless Highscore (Task 10 T4e)
    const auto& hs = game.GetHighscore();
    if (hs.maxWave > 0)
    {
        int hsMin = static_cast<int>(hs.maxSurvivalTime) / 60;
        int hsSec = static_cast<int>(hs.maxSurvivalTime) % 60;
        FontManager::DrawFormat(boxX + 55, boxY + 348, GetColor(255, 215, 80), FontSize::Body,
            L"[ENDLESS RECORD]: Max Wave %d  |  Kills: %d  |  Time: %02d:%02d",
            hs.maxWave, hs.maxKills, hsMin, hsSec);
    }

    // Nút Bắt đầu
    FontManager::Draw(midX - 225, boxY + 388, L"Press [1] or [2], or [UP/DOWN] + [ENTER/SPACE] to START!", GetColor(255, 255, 255), FontSize::Medium);
    FontManager::Draw(midX - 180, boxY + 428, L"[F1]: Toggle Debug HUD  |  [F2]: Toggle Camera Shake", GetColor(140, 170, 200), FontSize::Body);
}

void UIRenderer::DrawBossIntroScreen() const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(0, 0, sw, sh, GetColor(35, 10, 15), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int midY = sh / 2;

    FontManager::Draw(midX - 240, midY - 60, L"! WARNING: DEEP SEA ENERGY ANOMALY !", GetColor(255, 60, 60), FontSize::Header);
    FontManager::Draw(midX - 210, midY - 15, L"FINAL BOSS: GIANT WHALE HAS APPEARED!", GetColor(255, 215, 60), FontSize::Medium);
    FontManager::Draw(midX - 160, midY + 40, L"The battle for survival begins now...", GetColor(240, 240, 240), FontSize::Body);
}

void UIRenderer::DrawVictoryScreen(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
    DrawBox(0, 0, sw, sh, GetColor(10, 25, 45), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    FontManager::Draw(midX - 100, sh / 2 - 90, L"* VICTORY! *", GetColor(255, 225, 60), FontSize::Header);
    FontManager::Draw(midX - 220, sh / 2 - 40, L"You survived 5 waves and defeated the Giant Whale!", GetColor(220, 240, 255), FontSize::Body);

    int min = static_cast<int>(game.gameTime_) / 60;
    int sec = static_cast<int>(game.gameTime_) % 60;
    FontManager::DrawFormat(midX - 120, sh / 2 + 10, GetColor(160, 220, 255), FontSize::Body, L"Survival Time: %02d:%02d", min, sec);
    FontManager::DrawFormat(midX - 120, sh / 2 + 35, GetColor(160, 220, 255), FontSize::Body, L"Fish Level Achieved: Lv.%d", game.player_.GetLevel());

    if (game.debugController_.WasDebugUsed())
    {
        FontManager::Draw(midX - 90, sh / 2 + 65, L"[DEBUG MODE USED]", GetColor(255, 90, 90), FontSize::Body);
    }

    FontManager::Draw(midX - 160, sh / 2 + 100, L"Press [R] to start a new swim!", GetColor(255, 255, 255), FontSize::Body);
}

void UIRenderer::DrawGameOverScreen(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
    DrawBox(0, 0, sw, sh, GetColor(35, 10, 10), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    FontManager::Draw(midX - 160, sh / 2 - 90, L"FISH STRANDED! (GAME OVER)", GetColor(255, 60, 60), FontSize::Header);

    if (game.mode_ == GameMode::Endless)
    {
        FontManager::Draw(midX - 95, sh / 2 - 50, L"[ENDLESS MODE RUN]", GetColor(255, 180, 80), FontSize::Medium);
        FontManager::DrawFormat(midX - 120, sh / 2 - 20, GetColor(255, 230, 200), FontSize::Body,
            L"Wave Reached: Wave %d", game.currentWave_);
        FontManager::DrawFormat(midX - 120, sh / 2 + 5, GetColor(255, 230, 200), FontSize::Body,
            L"Total Enemies Defeated: %d", game.GetTotalKills());

        int min = static_cast<int>(game.gameTime_) / 60;
        int sec = static_cast<int>(game.gameTime_) % 60;
        FontManager::DrawFormat(midX - 120, sh / 2 + 30, GetColor(255, 230, 200), FontSize::Body,
            L"Survival Time: %02d:%02d", min, sec);

        const auto& hs = game.GetHighscore();
        int hsMin = static_cast<int>(hs.maxSurvivalTime) / 60;
        int hsSec = static_cast<int>(hs.maxSurvivalTime) % 60;
        FontManager::DrawFormat(midX - 150, sh / 2 + 60, GetColor(255, 215, 80), FontSize::Body,
            L"Best Record: Wave %d | Kills: %d | Time: %02d:%02d",
            hs.maxWave, hs.maxKills, hsMin, hsSec);

        if (game.debugController_.WasDebugUsed())
        {
            FontManager::Draw(midX - 150, sh / 2 + 90, L"[DEBUG MODE USED - RECORD NOT SAVED]", GetColor(255, 90, 90), FontSize::Body);
        }

        FontManager::Draw(midX - 160, sh / 2 + 125, L"Press [R] to retry  |  [T] to Title", GetColor(255, 255, 255), FontSize::Body);
    }
    else
    {
        if (game.state_ == GameState::Boss)
        {
            FontManager::Draw(midX - 140, sh / 2 - 25, L"Fell before: Giant Whale Boss", GetColor(255, 180, 180), FontSize::Body);
        }
        else
        {
            FontManager::DrawFormat(midX - 110, sh / 2 - 25, GetColor(255, 180, 180), FontSize::Body, L"Fell at: Wave %d / 5", game.currentWave_);
        }

        if (game.debugController_.WasDebugUsed())
        {
            FontManager::Draw(midX - 35, sh / 2 + 10, L"[DEBUG]", GetColor(255, 90, 90), FontSize::Body);
        }

        FontManager::Draw(midX - 160, sh / 2 + 50, L"Press [R] to revive and try again! | [T] Title", GetColor(255, 255, 255), FontSize::Body);
    }
}
