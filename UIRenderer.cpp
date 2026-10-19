#include "UIRenderer.h"
#include "GamePrototype.h"
#include "Boss.h"
#include "FontManager.h"
#include "Localization.h"
namespace
{
    // Vẽ chuỗi căn giữa theo trục X quanh cx (độ dài chữ EN/JP khác nhau)
    void DrawCX(int cx, int y, const wchar_t* text, unsigned int color, FontSize size)
    {
        FontManager::Draw(cx - FontManager::GetWidth(text, size) / 2, y, text, color, size);
    }
}

#include "Consts.h"
#include "SoundManager.h"
#include "DxLib.h"
#include "CardFrame.h"
#include <algorithm>
#include <cwchar>

void UIRenderer::DrawBossHealthBar(const WhaleBoss& boss) const
{
    battleHud_.DrawBossBar(boss);
}

void UIRenderer::DrawBossHealthBar(const SharkBoss& shark) const
{
    battleHud_.DrawBossBar(shark);
}

void UIRenderer::DrawBossHealthBar(const SquidBoss& squid) const
{
    squid.DrawScreenOverlay();
    battleHud_.DrawBossBar(squid);
}

void UIRenderer::DrawBossHealthBar(const TurtleBoss& turtle) const
{
    battleHud_.DrawBossBar(turtle);
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
            FontManager::DrawFormat(sw / 2 - 150, 24, GetColor(100, 220, 255), FontSize::Header, TR(L"NORMAL STAGE %d / %d"), game.currentWave_, GameConsts::ENDLESS_MAX_STAGE);
        }
        else if (game.mode_ == GameMode::Hell)
        {
            FontManager::DrawFormat(sw / 2 - 140, 24, GetColor(255, 90, 70), FontSize::Header, TR(L"HELL STAGE %d / %d"), game.currentWave_, GameConsts::ENDLESS_MAX_STAGE);
        }
        else
        {
            FontManager::DrawFormat(sw / 2 - 120, 24, GetColor(100, 220, 255), FontSize::Header, TR(L"TUTORIAL %d / %d"), game.currentWave_, GameConsts::TUTORIAL_WAVES);
        }
        FontManager::DrawFormat(sw / 2 - 95, 52, GetColor(255, 220, 100), FontSize::Body, TR(L"TIME LEFT: %02ds"), waveSecLeft);
        FontManager::DrawFormat(sw / 2 - 110, 74, GetColor(255, 180, 80), FontSize::Body, TR(L"ENEMIES: %d | KILLS: %d"), game.enemies_.GetActiveCount(), game.enemies_.GetTotalKills());
    }

    // 4. Danh sách các Thẻ đã sở hữu ở góc trái dưới
    int cardListY = jumpY + 28;

    // F11: ẩn/hiện toàn bộ bảng thông tin (stats, thẻ, debug, hướng dẫn phím)
    if (!game.panelsVisible_)
    {
        FontManager::Draw(hpX, sh - 30, TR(L"[F11] Show panels"), GetColor(140, 170, 200), FontSize::Small);
        return;
    }

    // Normal/Hell: bảng chỉ số nhỏ (góc trái, dưới bảng HP) - điểm từ lên cấp
    if (game.IsEndlessLike())
    {
        const bool hell = (game.mode_ == GameMode::Hell);
        const PlayerStats& st = game.playerStats_;
        const int* lv = game.lvStat_;
        const float def = (std::min)(GameConsts::LV_DEF_MAX, GameConsts::LV_DEF_PCT * lv[GamePrototype::LS_Def]);
        int y = cardListY;
        DrawBox(hpX - 4, y - 2, hpX + 270, y + 6 * 16 + 4, hell ? GetColor(20, 10, 14) : GetColor(10, 18, 30), TRUE);
        FontManager::DrawFormat(hpX, y, hell ? GetColor(255, 140, 110) : GetColor(120, 210, 255), FontSize::Small, TR(L"[STATS]  Lv %d  EXP %d/%d"), game.player_.GetLevel(), game.player_.GetExp(), game.player_.GetExpNext()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, TR(L"ATK +%d   SLAM %.0f  BULLET %.0f"), lv[0], st.slamDamage.GetValue(), st.bulletDamage.GetValue()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, TR(L"DEF +%d   -%d%% dmg taken   HP %.0f"), lv[1], (int)(def * 100.0f + 0.5f), st.maxHp.GetValue()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, TR(L"SPD +%d   %.0f"), lv[2], st.moveSpeed.GetValue()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, TR(L"CD  +%d   jump %.2fs"), lv[3], st.jumpCooldown.GetValue()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, TR(L"LUCK +%d  rarity +%d"), lv[4], lv[4] * GameConsts::LV_LUCK_RARITY_SHIFT); y += 16;
        cardListY = y + 8;
    }
    const auto& cards = game.cardManager_.GetActiveCards();
    if (!cards.empty())
    {
        FontManager::Draw(hpX, cardListY, TR(L"[ACTIVE CARDS]:"), GetColor(220, 230, 245), FontSize::Body);
        cardListY += 18;
        if (game.mode_ == GameMode::Hell)
        {
            // Hell: gộp thẻ trùng, hiện số stack như "Lv" sau tên thẻ (hiệu lực thẻ x0.1)
            int shown = 0;
            for (size_t i = 0; i < cards.size() && shown < 8; ++i)
            {
                bool dup = false;
                for (size_t j = 0; j < i; ++j) if (cards[j].id == cards[i].id) { dup = true; break; }
                if (dup) continue;
                int col = static_cast<int>(CardFrame::GetStyle(cards[i].rarity).edge);
                FontManager::DrawFormat(hpX + 6, cardListY, col, FontSize::Small, TR(L"- %s  Lv %d"), TR(cards[i].name.c_str()), game.cardManager_.GetCardCount(cards[i].id));
                cardListY += 17;
                ++shown;
            }
        }
        else
        for (size_t i = 0; i < cards.size() && i < 6; ++i)
        {
            int col = GetColor(200, 220, 255);
            if (cards[i].rarity == CardRarity::Rare) col = GetColor(255, 215, 0);
            else if (cards[i].rarity == CardRarity::Epic) col = GetColor(210, 120, 255);
            FontManager::DrawFormat(hpX + 6, cardListY, col, FontSize::Small, L"- %s", TR(cards[i].name.c_str()));
            cardListY += 17;
        }
    }

    // 5. Debug Panel & Debug Badge góc phải (F1)
    game.debugController_.DrawHUD(game);

    // 6. Hướng dẫn điều khiển góc dưới
    int guideY = sh - 45;
    FontManager::Draw(40, guideY, TR(L"[WASD]: Swim  |  [SPACE]: Jump Slam  |  [A/D+Enter / 1,2,3]: Pick Cards  |  [R]: Reset  |  [F1]: Debug  |  [F2]: Shake Cam  |  [F11]: Panels"), GetColor(200, 215, 235), FontSize::Body);
}

void UIRenderer::DrawLanguageSelectScreen(const GamePrototype& game) const
{
    // Màn đầu tiên: chọn ngôn ngữ (EN / 日本語). Chữ Nhật luôn vẽ bằng font JP.
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 225);
    DrawBox(0, 0, sw, sh, GetColor(8, 18, 32), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int midY = sh / 2;
    int boxW = 560, boxH = 330;
    int boxX = midX - boxW / 2, boxY = midY - boxH / 2;
    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(12, 26, 46), TRUE);
    DrawBox(boxX - 2, boxY - 2, boxX + boxW + 2, boxY + boxH + 2, GetColor(45, 95, 155), FALSE);
    DrawBox(boxX - 4, boxY - 4, boxX + boxW + 4, boxY + boxH + 4, GetColor(20, 50, 90), FALSE);

    auto drawJpCentered = [midX](int y, const wchar_t* text, unsigned int color, FontSize size)
    {
        int w = GetDrawStringWidthToHandle(text, static_cast<int>(wcslen(text)), FontManager::GetHandleJP(size));
        FontManager::DrawJP(midX - w / 2, y, text, color, size);
    };
    drawJpCentered(boxY + 30, L"SELECT LANGUAGE / 言語を選択", GetColor(100, 220, 255), FontSize::Header);

    const wchar_t* items[2] = { L"[1]  English", L"[2]  日本語 (Japanese)" };
    for (int i = 0; i < 2; ++i)
    {
        bool sel = (game.langSelectIdx_ == i);
        int y = boxY + 95 + i * 72;
        DrawBox(boxX + 60, y, boxX + boxW - 60, y + 52, sel ? GetColor(25, 55, 95) : GetColor(15, 25, 40), TRUE);
        DrawBox(boxX + 60, y, boxX + boxW - 60, y + 52, sel ? GetColor(255, 220, 80) : GetColor(60, 90, 130), FALSE);
        if (sel) FontManager::DrawJP(boxX + 76, y + 14, L">", GetColor(255, 235, 100), FontSize::Medium);
        FontManager::DrawJP(boxX + 104, y + 14, items[i], sel ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    }

    drawJpCentered(boxY + boxH - 46, L"[W/S] [UP/DOWN] + [ENTER/SPACE]  /  [1] [2]", GetColor(140, 170, 200), FontSize::Small);
    drawJpCentered(boxY + boxH - 26, L"Press [L] on the title to change  /  タイトルで [L] キーで変更", GetColor(120, 150, 180), FontSize::Small);
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
    DrawCX(midX, boxY + 28, TR(L"=== ODORU : JUST A FISH GAME ==="), GetColor(100, 220, 255), FontSize::Header);
    DrawCX(midX, boxY + 65, TR(L"CORE MECHANIC: JUMP-TO-ATTACK"), GetColor(255, 215, 60), FontSize::Medium);

    // Mô tả cách chơi
    FontManager::Draw(boxX + 50, boxY + 105, TR(L"- Controls: [WASD] or Arrow Keys to swim smoothly in deep waters."), GetColor(220, 235, 255), FontSize::Body);
    FontManager::Draw(boxX + 50, boxY + 130, TR(L"- Attack: Press [SPACE] to launch into the air and slam down!"), GetColor(220, 235, 255), FontSize::Body);
    FontManager::Draw(boxX + 50, boxY + 155, TR(L"- Aerial Invincibility: While airborne, completely immune to damage."), GetColor(140, 240, 180), FontSize::Body);
    FontManager::Draw(boxX + 50, boxY + 180, TR(L"- Landing Shockwave: Deals 40 damage, knocks back and breaks armor."), GetColor(255, 180, 120), FontSize::Body);

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
    const wchar_t* pfx1 = isClassicSelected ? TR(L"> [1] STORY MODE :") : TR(L"  [1] STORY MODE :");
    FontManager::Draw(boxX + 65, mode1Y + 8, pfx1, isClassicSelected ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode1Y + 10, TR(L"Tutorial (3x15s) + Whale, then Normal / Hell"), GetColor(200, 220, 255), FontSize::Body);

    // Box Mode 2: Endless
    int bgCol2 = isEndlessSelected ? GetColor(25, 55, 95) : GetColor(15, 25, 40);
    int edgeCol2 = isEndlessSelected ? GetColor(255, 220, 80) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode2Y, boxX + boxW - 50, mode2Y + modeBoxH, bgCol2, TRUE);
    DrawBox(boxX + 50, mode2Y, boxX + boxW - 50, mode2Y + modeBoxH, edgeCol2, FALSE);
    const wchar_t* pfx2 = isEndlessSelected ? TR(L"> [2] NORMAL MODE :") : TR(L"  [2] NORMAL MODE :");
    FontManager::Draw(boxX + 65, mode2Y + 8, pfx2, isEndlessSelected ? GetColor(255, 235, 100) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode2Y + 10, TR(L"30 Stages (20s), Stacking Cards, Boss every 5"), GetColor(255, 180, 120), FontSize::Body);

    // Box Mode 3: Hell
    int bgCol3 = isHellSelected ? GetColor(70, 20, 20) : GetColor(15, 25, 40);
    int edgeCol3 = isHellSelected ? GetColor(255, 90, 70) : GetColor(60, 90, 130);
    DrawBox(boxX + 50, mode3Y, boxX + boxW - 50, mode3Y + modeBoxH, bgCol3, TRUE);
    DrawBox(boxX + 50, mode3Y, boxX + boxW - 50, mode3Y + modeBoxH, edgeCol3, FALSE);
    const wchar_t* pfx3 = isHellSelected ? TR(L"> [3] HELL MODE :") : TR(L"  [3] HELL MODE :");
    FontManager::Draw(boxX + 65, mode3Y + 8, pfx3, isHellSelected ? GetColor(255, 140, 110) : GetColor(200, 220, 240), FontSize::Medium);
    FontManager::Draw(boxX + 270, mode3Y + 10, TR(L"Dmg taken x1.5, Random Cards, No Wipe"), GetColor(255, 120, 100), FontSize::Body);

    // Kỷ lục Endless Highscore (Task 10 T4e)
    const auto& hs = game.GetHighscore();
    if (hs.maxWave > 0)
    {
        int hsMin = static_cast<int>(hs.maxSurvivalTime) / 60;
        int hsSec = static_cast<int>(hs.maxSurvivalTime) % 60;
        FontManager::DrawFormat(boxX + 55, boxY + 352, GetColor(255, 215, 80), FontSize::Body,
            TR(L"[NORMAL RECORD]: Max Stage %d  |  Kills: %d  |  Time: %02d:%02d  |  Score: %d"),
            hs.maxWave, hs.maxKills, hsMin, hsSec, hs.maxScore);
    }
    const auto& hh = game.GetHellHighscore();
    if (hh.maxWave > 0)
    {
        FontManager::DrawFormat(boxX + 55, boxY + 374, GetColor(255, 120, 100), FontSize::Body,
            TR(L"[HELL RECORD]: Max Stage %d  |  Kills: %d  |  Score: %d"), hh.maxWave, hh.maxKills, hh.maxScore);
    }

    // Nút Bắt đầu
    DrawCX(midX, boxY + 400, TR(L"Press [1] [2] [3], or [UP/DOWN] + [ENTER/SPACE] to START!"), GetColor(255, 255, 255), FontSize::Medium);
    DrawCX(midX, boxY + 438, TR(L"[F1]: Toggle Debug HUD  |  [F2]: Toggle Camera Shake  |  [F11]: Panels  |  [L]: Language"), GetColor(140, 170, 200), FontSize::Body);
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
    DrawCX(midX, midY - 150, TR(L"TUTORIAL CLEAR!  CHOOSE YOUR MODE"), GetColor(100, 220, 255), FontSize::Header);
    DrawCX(midX, midY - 110, TR(L"Your cards and the whale-clear heal carry over."), GetColor(200, 220, 240), FontSize::Body);

    const wchar_t* names[2] = { TR(L"[1] NORMAL"), TR(L"[2] HELL") };
    const wchar_t* descs[2] = {
        TR(L"30 stages (20s), pick 1 of 3 cards, field cleared each stage, boss every 5"),
        TR(L"Damage taken x1.5, random card (no pause), enemies stay between stages") };
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
    DrawCX(midX, midY + 120, TR(L"[UP/DOWN] + [ENTER/SPACE], or [1] / [2]"), GetColor(255, 255, 255), FontSize::Body);
}

void UIRenderer::DrawFlowOverlay(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    if (game.levelUpBannerTimer_ > 0.0f && game.IsEndlessLike())
    {
        static const wchar_t* kStat[GamePrototype::LS_Count] = { L"+1 ATTACK", L"+1 DEFENSE", L"+1 SPEED", L"+1 COOLDOWN", L"+1 LUCK" };
        float k = (std::min)(1.0f, game.levelUpBannerTimer_ / 0.4f); // mờ dần 0.4s cuối
        int by = GameConsts::SCREEN_HEIGHT / 2 - 150;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(200 * k));
        DrawBox(sw / 2 - 170, by, sw / 2 + 170, by + 70, GetColor(10, 30, 50), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * k));
        DrawBox(sw / 2 - 170, by, sw / 2 + 170, by + 70, GetColor(120, 230, 255), FALSE);
        FontManager::DrawFormat(sw / 2 - 120, by + 8, GetColor(255, 255, 255), FontSize::Header, TR(L"LEVEL UP!  Lv %d"), game.player_.GetLevel());
        FontManager::Draw(sw / 2 - 70, by + 44, TR(kStat[game.levelUpStat_]), GetColor(120, 255, 180), FontSize::Medium);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    if (game.rouletteActive_ && game.mode_ == GameMode::Hell)
    {
        // Vòng quay thẻ Hell: 3 thẻ dùng khung CardFrame theo độ hiếm, ô sáng chạy -> dừng ở thẻ trúng -> mờ dần
        const int n = game.rouletteCount_;
        const int cw = 150, ch = 96, gap = 14;
        int total = n * cw + (n - 1) * gap;
        int x0 = sw / 2 - total / 2, y0 = 104;
        const float ft = game.rouletteTimer_;
        const bool fading = game.rouletteLanded_ && ft > GameConsts::HELL_ROULETTE_HOLD_TIME;
        if (!fading)
        {
            FontManager::Draw(x0, y0 - 22, game.rouletteLanded_ ? TR(L"HELL CARD GET!") : TR(L"HELL ROULETTE..."), GetColor(255, 140, 110), FontSize::Body);
            for (int i = 0; i < n; ++i)
            {
                const Card& c = game.rouletteCards_[i];
                int x = x0 + i * (cw + gap);
                bool on = (i == game.rouletteIdx_);
                bool win = on && game.rouletteLanded_;
                int lift = on ? -8 : 0;
                CardFrame::DrawBack(x, y0 + lift, cw, ch, c.rarity, on, ft + i * 0.7f);
                FontManager::Draw(x + 10, y0 + lift + 30, TR(c.name.c_str()), on ? GetColor(255, 240, 160) : GetColor(190, 190, 205), FontSize::Small);
                FontManager::Draw(x + 10, y0 + lift + 52, TR(CardFrame::GetStyle(c.rarity).label), GetColor(170, 180, 200), FontSize::Small);
                CardFrame::DrawFront(x, y0 + lift, cw, ch, c.rarity, on, ft + i * 0.7f);
                if (!on)
                {
                    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120); // làm tối thẻ không được chọn
                    DrawBox(x, y0, x + cw, y0 + ch, GetColor(0, 0, 0), TRUE);
                    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
                }
                if (win)
                {
                    DrawBox(x - 4, y0 + lift - 4, x + cw + 4, y0 + lift + ch + 4, GetColor(255, 230, 90), FALSE);
                    DrawBox(x - 5, y0 + lift - 5, x + cw + 5, y0 + lift + ch + 5, GetColor(255, 230, 90), FALSE);
                }
            }
        }
        else
        {
            // Mờ dần: chỉ còn thẻ trúng (vẽ đơn giản để alpha hoạt động)
            float k = 1.0f - (ft - GameConsts::HELL_ROULETTE_HOLD_TIME) / GameConsts::HELL_ROULETTE_FADE_TIME;
            int alpha = (std::max)(0, (std::min)(255, static_cast<int>(255.0f * k)));
            const Card& c = game.rouletteCards_[game.rouletteTarget_];
            const CardFrame::Style& s = CardFrame::GetStyle(c.rarity);
            int x = x0 + game.rouletteTarget_ * (cw + gap);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawBox(x, y0 - 8, x + cw, y0 - 8 + ch, GetColor(s.bgTop[0], s.bgTop[1], s.bgTop[2]), TRUE);
            DrawBox(x, y0 - 8, x + cw, y0 - 8 + ch, s.edge, FALSE);
            FontManager::Draw(x + 10, y0 + 22, TR(c.name.c_str()), GetColor(255, 240, 160), FontSize::Small);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
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
    FontManager::Draw(sw / 2 - 325, y + 6, TR(kTips[game.activeTip_]), GetColor(255, 235, 120), FontSize::Medium);
    FontManager::Draw(sw / 2 - 325, y + 32, TR(L"[TAB] Skip tips"), GetColor(170, 190, 215), FontSize::Small);
}

void UIRenderer::DrawBossIntroScreen(int bossKind) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(0, 0, sw, sh, GetColor(35, 10, 15), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int midY = sh / 2;

    DrawCX(midX, midY - 60, TR(L"! WARNING: DEEP SEA ENERGY ANOMALY !"), GetColor(255, 60, 60), FontSize::Header);
    DrawCX(midX, midY - 15, bossKind == 3 ? TR(L"BOSS: ANCIENT TURTLE GUARDS THE REEF!") : bossKind == 2 ? TR(L"BOSS: GIANT SQUID BLOCKS THE DEEP!") : bossKind == 1 ? TR(L"BOSS: GREAT SHARK IS HUNTING YOU!") : TR(L"FINAL BOSS: GIANT WHALE HAS APPEARED!"), GetColor(255, 215, 60), FontSize::Medium);
    DrawCX(midX, midY + 40, TR(L"The battle for survival begins now..."), GetColor(240, 240, 240), FontSize::Body);
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

    DrawCX(midX, boxY + 22, TR(L"PAUSED"), GetColor(255, 225, 60), FontSize::Header);

    const SoundManager& sound = SoundManager::Instance();
    wchar_t labels[GamePrototype::PauseItem_Count][64] = {};
    swprintf_s(labels[GamePrototype::PauseItem_Resume], TR(L"Resume"));
    swprintf_s(labels[GamePrototype::PauseItem_BgmVolume], TR(L"BGM Volume:  < %3d >"), sound.GetVolume(SoundChannel::BGM));
    swprintf_s(labels[GamePrototype::PauseItem_SfxVolume], TR(L"SFX Volume:  < %3d >"), sound.GetVolume(SoundChannel::SFX));
    swprintf_s(labels[GamePrototype::PauseItem_WindowMode], TR(L"Screen:  < %s >"), GetWindowModeFlag() == TRUE ? TR(L"Window") : TR(L"Fullscreen"));
    swprintf_s(labels[GamePrototype::PauseItem_Title], TR(L"Return to Title"));

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

    FontManager::Draw(boxX + 30, boxY + boxH - 40, TR(L"[W/S]: Select  [A/D]: Adjust  [Enter]: OK  [ESC/P]: Resume"), GetColor(140, 170, 200), FontSize::Small);
}
