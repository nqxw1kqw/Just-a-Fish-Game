#include "UIRenderer.h"
#include "GamePrototype.h"
#include "Boss.h"
#include "FontManager.h"
#include "Consts.h"
#include "SoundManager.h"
#include "DxLib.h"
#include <algorithm>

void UIRenderer::DrawBossHealthBar(const WhaleBoss& boss) const
{
    battleHud_.DrawBossBar(boss);
}

void UIRenderer::DrawBossHealthBar(const SharkBoss& shark) const
{
    battleHud_.DrawBossBar(shark);
}

void UIRenderer::DrawHUD(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;
    int hpX = 40;

    // 0. Chớp đỏ khi bị trúng đòn (vẽ dưới các bảng HUD)
    battleHud_.DrawDamageFlash();

    // 1-2. 戦闘HUD: HP (máu trễ), EXP/Lv, vòng hồi chiêu Slam, combo
    battleHud_.DrawPlayerPanel(game.player_);
    int jumpY = BattleHud::PanelBottomY() - 28;

    // 3. Trạng thái Wave / Boss ở giữa trên
    if (game.state_ == GameState::Wave)
    {
        int waveSecLeft = (std::max)(0, static_cast<int>(game.waveDuration_ - game.waveTimer_));
        if (game.mode_ == GameMode::Endless)
        {
            FontManager::DrawFormat(sw / 2 - 150, 24, GetColor(100, 220, 255), FontSize::Header, L"NORMAL STAGE %d / %d", game.currentWave_, GameConsts::ENDLESS_MAX_STAGE);
        }
        else if (game.mode_ == GameMode::Hell)
        {
            FontManager::DrawFormat(sw / 2 - 140, 24, GetColor(255, 90, 70), FontSize::Header, L"HELL STAGE %d / %d", game.currentWave_, GameConsts::ENDLESS_MAX_STAGE);
        }
        else
        {
            FontManager::DrawFormat(sw / 2 - 120, 24, GetColor(100, 220, 255), FontSize::Header, L"TUTORIAL %d / %d", game.currentWave_, GameConsts::TUTORIAL_WAVES);
        }
        FontManager::DrawFormat(sw / 2 - 95, 52, GetColor(255, 220, 100), FontSize::Body, L"TIME LEFT: %02ds", waveSecLeft);
        FontManager::DrawFormat(sw / 2 - 110, 74, GetColor(255, 180, 80), FontSize::Body, L"ENEMIES: %d | KILLS: %d", game.enemies_.GetActiveCount(), game.enemies_.GetTotalKills());
    }

    // 4. Danh sách các Thẻ đã sở hữu ở góc trái dưới
    int cardListY = jumpY + 28;

    // Hell: bảng chỉ số nhỏ (góc trái, dưới bảng HP)
    if (game.mode_ == GameMode::Hell)
    {
        const PlayerStats& st = game.playerStats_;
        const auto& lv = game.hellStatLv_;
        int y = cardListY;
        DrawBox(hpX - 4, y - 2, hpX + 250, y + 5 * 16 + 4, GetColor(20, 10, 14), TRUE);
        FontManager::DrawFormat(hpX, y, GetColor(255, 140, 110), FontSize::Small, L"[HELL STATS]  Lv %d  Cards %d", game.player_.GetLevel(), (int)game.cardManager_.GetActiveCards().size()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"HP %.0f (+%d)   SPD %.0f (+%d)", st.maxHp.GetValue(), lv[0], st.moveSpeed.GetValue(), lv[1]); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"SLAM %.0f (+%d)  R %.0f (+%d)", st.slamDamage.GetValue(), lv[2], st.slamRadius.GetValue(), lv[3]); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"JUMP CD %.2fs (+%d)", st.jumpCooldown.GetValue(), lv[4]); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"BULLET %.0f (+%d)  DMG x%.1f", st.bulletDamage.GetValue(), lv[5], GameConsts::HELL_PLAYER_DMG_MUL); y += 16;
        cardListY = y + 8;
    }
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
    int mode1Y = boxY + 215;
    int mode2Y = boxY + 262;
    int mode3Y = boxY + 309;
    bool isClassicSelected = (game.titleSelectedMode_ == 0);
    bool isEndlessSelected = (game.titleSelectedMode_ == 1);
    bool isHellSelected = (game.titleSelectedMode_ == 2);
    const int modeBoxH = 40;

    // Box Mode 1: Classic
    int bgCol1 = isClassicSelected ? GetColor(25, 55, 95) : GetColor(15, 25, 40);
    int edgeCol1 = isClassicSelected ? GetColor(255, 220, 80) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode1Y, boxX + boxW - 50, mode1Y + modeBoxH, bgCol1, TRUE);
    DrawBox(boxX + 50, mode1Y, boxX + boxW - 50, mode1Y + modeBoxH, edgeCol1, FALSE);
    const wchar_t* pfx1 = isClassicSelected ? L"> [1] STORY MODE :" : L"  [1] STORY MODE :";
    FontManager::Draw(boxX + 65, mode1Y + 8, pfx1, isClassicSelected ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode1Y + 10, L"Tutorial (3x15s) + Whale, then Normal / Hell", GetColor(200, 220, 255), FontSize::Body);

    // Box Mode 2: Endless
    int bgCol2 = isEndlessSelected ? GetColor(25, 55, 95) : GetColor(15, 25, 40);
    int edgeCol2 = isEndlessSelected ? GetColor(255, 220, 80) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode2Y, boxX + boxW - 50, mode2Y + modeBoxH, bgCol2, TRUE);
    DrawBox(boxX + 50, mode2Y, boxX + boxW - 50, mode2Y + modeBoxH, edgeCol2, FALSE);
    const wchar_t* pfx2 = isEndlessSelected ? L"> [2] NORMAL MODE :" : L"  [2] NORMAL MODE :";
    FontManager::Draw(boxX + 65, mode2Y + 8, pfx2, isEndlessSelected ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode2Y + 10, L"30 Stages (20s), Stacking Cards, Boss every 5", GetColor(255, 180, 120), FontSize::Body);

    // Box Mode 3: Hell
    int bgCol3 = isHellSelected ? GetColor(70, 20, 20) : GetColor(15, 25, 40);
    int edgeCol3 = isHellSelected ? GetColor(255, 90, 70) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode3Y, boxX + boxW - 50, mode3Y + modeBoxH, bgCol3, TRUE);
    DrawBox(boxX + 50, mode3Y, boxX + boxW - 50, mode3Y + modeBoxH, edgeCol3, FALSE);
    const wchar_t* pfx3 = isHellSelected ? L"> [3] HELL MODE :" : L"  [3] HELL MODE :";
    FontManager::Draw(boxX + 65, mode3Y + 8, pfx3, isHellSelected ? GetColor(255, 140, 110) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode3Y + 10, L"Dmg taken x1.5, Random Cards, No Wipe", GetColor(255, 120, 100), FontSize::Body);

    // Kỷ lục Endless Highscore (Task 10 T4e)
    const auto& hs = game.GetHighscore();
    if (hs.maxWave > 0)
    {
        int hsMin = static_cast<int>(hs.maxSurvivalTime) / 60;
        int hsSec = static_cast<int>(hs.maxSurvivalTime) % 60;
        FontManager::DrawFormat(boxX + 55, boxY + 352, GetColor(255, 215, 80), FontSize::Body,
            L"[NORMAL RECORD]: Max Stage %d  |  Kills: %d  |  Time: %02d:%02d  |  Score: %d",
            hs.maxWave, hs.maxKills, hsMin, hsSec, hs.maxScore);
    }
    const auto& hh = game.GetHellHighscore();
    if (hh.maxWave > 0)
    {
        FontManager::DrawFormat(boxX + 55, boxY + 374, GetColor(255, 120, 100), FontSize::Body,
            L"[HELL RECORD]: Max Stage %d  |  Kills: %d  |  Score: %d", hh.maxWave, hh.maxKills, hh.maxScore);
    }

    // Nút Bắt đầu
    FontManager::Draw(midX - 225, boxY + 400, L"Press [1] [2] [3], or [UP/DOWN] + [ENTER/SPACE] to START!", GetColor(255, 255, 255), FontSize::Medium);
    FontManager::Draw(midX - 180, boxY + 438, L"[F1]: Toggle Debug HUD  |  [F2]: Toggle Camera Shake", GetColor(140, 170, 200), FontSize::Body);
}

void UIRenderer::DrawModeSelectScreen(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
    DrawBox(0, 0, sw, sh, GetColor(8, 18, 32), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int midY = sh / 2;
    FontManager::Draw(midX - 200, midY - 150, L"TUTORIAL CLEAR!  CHOOSE YOUR MODE", GetColor(100, 220, 255), FontSize::Header);
    FontManager::Draw(midX - 230, midY - 110, L"Your cards and the whale-clear heal carry over.", GetColor(200, 220, 240), FontSize::Body);

    const wchar_t* names[2] = { L"[1] NORMAL", L"[2] HELL" };
    const wchar_t* descs[2] = {
        L"30 stages (20s), pick 1 of 3 cards, field cleared each stage, boss every 5",
        L"Damage taken x1.5, random card (no pause), enemies stay between stages" };
    for (int i = 0; i < 2; ++i)
    {
        bool sel = (game.modeSelectIdx_ == i);
        int y = midY - 60 + i * 80;
        int edge = sel ? (i == 1 ? GetColor(255, 90, 70) : GetColor(255, 220, 80)) : GetColor(60, 90, 130);
        int bg = sel ? (i == 1 ? GetColor(70, 20, 20) : GetColor(25, 55, 95)) : GetColor(15, 25, 40);
        DrawBox(midX - 380, y, midX + 380, y + 62, bg, TRUE);
        DrawBox(midX - 380, y, midX + 380, y + 62, edge, FALSE);
        FontManager::Draw(midX - 360, y + 8, names[i], sel ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
        FontManager::Draw(midX - 360, y + 36, descs[i], GetColor(200, 215, 235), FontSize::Small);
    }
    FontManager::Draw(midX - 200, midY + 120, L"[UP/DOWN] + [ENTER/SPACE], or [1] / [2]", GetColor(255, 255, 255), FontSize::Body);
}

void UIRenderer::DrawFlowOverlay(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    if (game.rouletteActive_ && game.mode_ == GameMode::Hell)
    {
        // Phase 2: vòng quay thẻ Hell (3 ô, ô sáng chạy rồi dừng ở thẻ trúng)
        const int n = game.rouletteCount_;
        const int cw = 200, ch = 54, gap = 12;
        int total = n * cw + (n - 1) * gap;
        int x0 = sw / 2 - total / 2, y0 = 100;
        DrawBox(x0 - 10, y0 - 26, x0 + total + 10, y0 + ch + 8, GetColor(30, 8, 8), TRUE);
        DrawBox(x0 - 10, y0 - 26, x0 + total + 10, y0 + ch + 8, GetColor(255, 90, 70), FALSE);
        FontManager::Draw(x0, y0 - 22, game.rouletteLanded_ ? L"HELL CARD GET!" : L"HELL ROULETTE...", GetColor(255, 140, 110), FontSize::Body);
        for (int i = 0; i < n; ++i)
        {
            int x = x0 + i * (cw + gap);
            bool on = (i == game.rouletteIdx_);
            bool win = on && game.rouletteLanded_;
            int bg = win ? GetColor(120, 40, 20) : on ? GetColor(80, 30, 30) : GetColor(25, 15, 20);
            int edge = win ? GetColor(255, 220, 80) : on ? GetColor(255, 120, 90) : GetColor(90, 60, 70);
            DrawBox(x, y0, x + cw, y0 + ch, bg, TRUE);
            DrawBox(x, y0, x + cw, y0 + ch, edge, FALSE);
            if (win) DrawBox(x - 2, y0 - 2, x + cw + 2, y0 + ch + 2, edge, FALSE);
            FontManager::Draw(x + 8, y0 + 18, game.rouletteCards_[i].name.c_str(), on ? GetColor(255, 235, 150) : GetColor(170, 160, 170), FontSize::Small);
        }
    }
    else if (game.hellCardToastTimer_ > 0.0f && game.mode_ == GameMode::Hell)
    {
        DrawBox(sw / 2 - 220, 104, sw / 2 + 220, 136, GetColor(60, 15, 15), TRUE);
        DrawBox(sw / 2 - 220, 104, sw / 2 + 220, 136, GetColor(255, 90, 70), FALSE);
        FontManager::Draw(sw / 2 - 205, 110, game.hellCardToast_, GetColor(255, 200, 160), FontSize::Body);
    }
    if (game.activeTip_ < 0 || game.paused_) return;

    static const wchar_t* kTips[GamePrototype::Tip_Count] = {
        L"MOVE: Swim with [WASD] / Arrow keys",
        L"JUMP / SLAM: Press [SPACE] - airborne = invincible, landing hits",
        L"ATTACK: Land a slam on an enemy to defeat it",
        L"CARDS: Pick 1 upgrade - [A/D] + [ENTER] or [1] [2] [3]",
        L"BOSS: Dodge its attacks in the air, slam it when it is open!" };
    int y = (game.state_ == GameState::CardPick) ? 150 : 104;
    DrawBox(sw / 2 - 340, y, sw / 2 + 340, y + 52, GetColor(15, 40, 70), TRUE);
    DrawBox(sw / 2 - 340, y, sw / 2 + 340, y + 52, GetColor(255, 220, 80), FALSE);
    FontManager::Draw(sw / 2 - 325, y + 6, kTips[game.activeTip_], GetColor(255, 235, 120), FontSize::Medium);
    FontManager::Draw(sw / 2 - 325, y + 32, L"[TAB] Skip tips", GetColor(170, 190, 215), FontSize::Small);
}

void UIRenderer::DrawBossIntroScreen(bool shark) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(0, 0, sw, sh, GetColor(35, 10, 15), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int midY = sh / 2;

    FontManager::Draw(midX - 240, midY - 60, L"! WARNING: DEEP SEA ENERGY ANOMALY !", GetColor(255, 60, 60), FontSize::Header);
    FontManager::Draw(midX - 210, midY - 15, shark ? L"BOSS: GREAT SHARK IS HUNTING YOU!" : L"FINAL BOSS: GIANT WHALE HAS APPEARED!", GetColor(255, 215, 60), FontSize::Medium);
    FontManager::Draw(midX - 160, midY + 40, L"The battle for survival begins now...", GetColor(240, 240, 240), FontSize::Body);
}

void UIRenderer::DrawVictoryScreen(const GamePrototype& game) const
{
    game.resultScreen_.Draw(game.BuildResultStats());
}

void UIRenderer::DrawGameOverScreen(const GamePrototype& game) const
{
    game.resultScreen_.Draw(game.BuildResultStats());
}

void UIRenderer::DrawPauseScreen(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    // Lớp phủ làm tối khung hình game đang đóng băng phía sau
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
    DrawBox(0, 0, sw, sh, GetColor(6, 14, 26), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int boxW = 460;
    int boxH = 330;
    int boxX = midX - boxW / 2;
    int boxY = (sh - boxH) / 2;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(12, 28, 48), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(80, 180, 255), FALSE);

    FontManager::Draw(midX - 60, boxY + 22, L"PAUSED", GetColor(255, 225, 60), FontSize::Header);

    const SoundManager& sound = SoundManager::Instance();
    wchar_t labels[GamePrototype::PauseItem_Count][64] = {};
    swprintf_s(labels[GamePrototype::PauseItem_Resume], L"Resume");
    swprintf_s(labels[GamePrototype::PauseItem_BgmVolume], L"BGM Volume:  < %3d >", sound.GetVolume(SoundChannel::BGM));
    swprintf_s(labels[GamePrototype::PauseItem_SfxVolume], L"SFX Volume:  < %3d >", sound.GetVolume(SoundChannel::SFX));
    swprintf_s(labels[GamePrototype::PauseItem_WindowMode], L"Screen:  < %s >", GetWindowModeFlag() == TRUE ? L"Window" : L"Fullscreen");
    swprintf_s(labels[GamePrototype::PauseItem_Title], L"Return to Title");

    int itemY = boxY + 80;
    for (int i = 0; i < GamePrototype::PauseItem_Count; ++i)
    {
        bool selected = (i == game.pauseSelected_);
        unsigned int color = selected ? GetColor(255, 225, 60) : GetColor(200, 215, 235);
        if (selected)
        {
            FontManager::Draw(boxX + 40, itemY, L">", color, FontSize::Medium);
        }
        FontManager::Draw(boxX + 70, itemY, labels[i], color, FontSize::Medium);
        itemY += 40;
    }

    FontManager::Draw(boxX + 30, boxY + boxH - 40, L"[W/S]: Select  [A/D]: Adjust  [Enter]: OK  [ESC/P]: Resume", GetColor(140, 170, 200), FontSize::Small);
}
