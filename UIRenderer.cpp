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
#include "MenuStyle.h"
#include <algorithm>
#include <cwchar>

void UIRenderer::Init()
{
    if (titleArtwork_ != -1) DeleteGraph(titleArtwork_);
    titleArtwork_ = LoadGraph(GameConsts::TITLE_COVER_PATH);
    modeSelectScreen_.Init();
    languageSelectScreen_.Init();
    pauseMenuScreen_.Init();
}

void UIRenderer::Shutdown()
{
    if (titleArtwork_ != -1) DeleteGraph(titleArtwork_);
    titleArtwork_ = -1;
    modeSelectScreen_.Shutdown();
    languageSelectScreen_.Shutdown();
    pauseMenuScreen_.Shutdown();
}

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

void UIRenderer::DrawBossHealthBar(const SwordfishBoss& swordfish) const
{
    battleHud_.DrawBossBar(swordfish);
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
            FontManager::DrawFormat(sw / 2 - 140, 24, GetColor(255, 90, 70), FontSize::Header, TR(L"HARD STAGE %d / %d"), game.currentWave_, GameConsts::ENDLESS_MAX_STAGE);
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
    languageSelectScreen_.Draw(game.langSelectIdx_);
}

void UIRenderer::DrawTitleScreen(const GamePrototype& game) const
{
    (void)game;
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    const int previousDrawMode = GetDrawMode();
    int previousBlend = DX_BLENDMODE_NOBLEND, previousAlpha = 255;
    GetDrawBlendMode(&previousBlend, &previousAlpha);
    SetDrawMode(DX_DRAWMODE_BILINEAR);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    if (titleArtwork_ >= 0)
        DrawExtendGraph(0, 0, sw, sh, titleArtwork_, TRUE);
    else
        DrawBox(0, 0, sw, sh, GetColor(8, 18, 32), TRUE);

    int midX = sw / 2;
    // Translucent band at the bottom so the text stays readable over the art.
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(0, sh - 190, sw, sh, GetColor(4, 10, 20), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    MenuStyle::Center(midX, sh - 168, TR(L"ODORU : JUST A FISH GAME"), MenuStyle::Paper(), FontSize::Header);
    MenuStyle::Button(midX - 280, sh - 121, 560, TR(L"SPACE / ENTER - START"), true, MenuStyle::Accent());
    MenuStyle::Center(midX, sh - 48, TR(L"[L]: Language"), MenuStyle::Muted(), FontSize::Body);
    SetDrawMode(previousDrawMode);
    SetDrawBlendMode(previousBlend, previousAlpha);
}
void UIRenderer::DrawModeSelectScreen(const GamePrototype& game) const
{
    modeSelectScreen_.Draw(game.modeSelectIdx_);
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
            FontManager::Draw(x0, y0 - 22, game.rouletteLanded_ ? TR(L"HARD CARD GET!") : TR(L"HARD ROULETTE..."), GetColor(255, 140, 110), FontSize::Body);
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
    DrawCX(midX, midY - 15, bossKind == 4 ? TR(L"BOSS: SWORDFISH SLASHES IN!") : bossKind == 3 ? TR(L"BOSS: ANCIENT TURTLE GUARDS THE REEF!") : bossKind == 2 ? TR(L"BOSS: GIANT SQUID BLOCKS THE DEEP!") : bossKind == 1 ? TR(L"BOSS: GREAT SHARK IS HUNTING YOU!") : TR(L"FINAL BOSS: GIANT WHALE HAS APPEARED!"), GetColor(255, 215, 60), FontSize::Medium);
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
    const SoundManager& pauseSound = SoundManager::Instance();
    if (pauseMenuScreen_.Draw(game.pauseSelected_, pauseSound.GetVolume(SoundChannel::BGM),
        pauseSound.GetVolume(SoundChannel::SFX), GetWindowModeFlag() == TRUE)) return;
    const MenuStyle::DrawState drawState;
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    // Lớp phủ làm tối khung hình game đang đóng băng phía sau
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
    DrawBox(0, 0, sw, sh, GetColor(6, 14, 26), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    int midX = sw / 2;
    int boxW = 680;
    int boxH = 530;
    int boxX = midX - boxW / 2;
    int boxY = (sh - boxH) / 2;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, MenuStyle::Background(), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawBox(boxX, boxY, boxX + boxW, boxY + boxH, GetColor(47, 69, 85), FALSE);

    MenuStyle::Center(midX, boxY + 28, TR(L"PAUSED"), MenuStyle::Paper(), FontSize::Title, 1.3f);

    const SoundManager& sound = SoundManager::Instance();
    wchar_t labels[GamePrototype::PauseItem_Count][64] = {};
    swprintf_s(labels[GamePrototype::PauseItem_Resume], TR(L"Resume"));
    swprintf_s(labels[GamePrototype::PauseItem_BgmVolume], TR(L"BGM Volume:  < %3d >"), sound.GetVolume(SoundChannel::BGM));
    swprintf_s(labels[GamePrototype::PauseItem_SfxVolume], TR(L"SFX Volume:  < %3d >"), sound.GetVolume(SoundChannel::SFX));
    swprintf_s(labels[GamePrototype::PauseItem_WindowMode], TR(L"Screen:  < %s >"), GetWindowModeFlag() == TRUE ? TR(L"Window") : TR(L"Fullscreen"));
    swprintf_s(labels[GamePrototype::PauseItem_Title], TR(L"Return to Title"));

    int itemY = boxY + 106;
    for (int i = 0; i < GamePrototype::PauseItem_Count; ++i)
    {
        bool selected = (i == game.pauseSelected_);
        MenuStyle::Button(boxX + 40, itemY, boxW - 80, labels[i], selected, MenuStyle::Accent());
        itemY += 64;
    }

    MenuStyle::Center(midX, boxY + 445, TR(L"W/S or UP/DOWN: Select   ENTER/SPACE: Confirm"), MenuStyle::Muted(), FontSize::Body, 1.0f, boxW - 60);
    MenuStyle::Center(midX, boxY + 480, TR(L"A/D: Adjust   ESC/P: Resume"), MenuStyle::Muted(), FontSize::Body);
}
