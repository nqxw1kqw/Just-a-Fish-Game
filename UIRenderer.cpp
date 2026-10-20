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
    pauseMenuScreen_.Init();
}

void UIRenderer::Shutdown()
{
    if (titleArtwork_ != -1) DeleteGraph(titleArtwork_);
    titleArtwork_ = -1;
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

    // 1-2. 戦闘HUD: HP (máu trễ), 区間, vòng hồi chiêu Slam, combo
    battleHud_.DrawPlayerPanel(game.player_, game.GetProgressionStage());
    int jumpY = BattleHud::PanelBottomY() - 28;

    // 3. Trạng thái Wave / Boss ở giữa trên
    if (game.state_ == GameState::Wave)
    {
        const int remaining = (std::max)(0, static_cast<int>(std::ceil(game.waveDuration_ - game.waveTimer_)));
        MenuStyle::Center(sw / 2, 24, game.IsCampaign() ? TR(L"冒険") : TR(L"チュートリアル"), GetColor(100,220,255), FontSize::Header);
        wchar_t progress[80];
        swprintf_s(progress, game.IsCampaign() ? L"ウェーブ %d / %d　残り %02d秒" : L"練習 %d / %d　残り %02d秒",
                   game.currentWave_, game.IsCampaign() ? GameConsts::CAMPAIGN_WAVES : GameConsts::TUTORIAL_WAVES, remaining);
        MenuStyle::Center(sw / 2, 52, progress, GetColor(255,220,100), FontSize::Body);
        FontManager::DrawFormat(sw / 2 - 110, 74, GetColor(255,180,80), FontSize::Body,
            TR(L"敵: %d | 撃破: %d"), game.enemies_.GetActiveCount(), game.enemies_.GetTotalKills());
        if (game.IsCampaign())
            FontManager::DrawFormat(sw / 2 - 90, 94, GetColor(180,220,245), FontSize::Small,
                TR(L"ボス撃破 %d / %d"), game.bossesDefeated_, GameConsts::CAMPAIGN_BOSSES);
    }
    else if (game.state_ == GameState::Boss || game.state_ == GameState::BossIntro)
    {
        FontManager::DrawFormat(sw / 2 - 110, 88, GetColor(255,220,100), FontSize::Body,
            TR(L"ボス戦 %d / %d"), game.bossesDefeated_ + 1, GameConsts::CAMPAIGN_BOSSES);
    }

    // 4. Danh sách các Thẻ đã sở hữu ở góc trái dưới
    int cardListY = jumpY + 28;

    // F11: ẩn/hiện toàn bộ bảng thông tin (stats, thẻ, debug, hướng dẫn phím)
    if (!game.panelsVisible_)
    {
        FontManager::Draw(hpX, sh - 30, TR(L"[F11] パネル表示"), GetColor(140, 170, 200), FontSize::Small);
        return;
    }

    if (game.IsCampaign())
    {
        const PlayerStats& st = game.playerStats_;
        const int stage = game.GetProgressionStage();
        const int defense = static_cast<int>(GameConsts::STAGE_DEFENSE_PCT * (stage - 1) * 100.0f + 0.5f);
        int y = cardListY;
        DrawBox(hpX - 4, y - 2, hpX + 290, y + 6 * 16 + 4, GetColor(10, 18, 30), TRUE);
        FontManager::DrawFormat(hpX, y, GetColor(120, 210, 255), FontSize::Small, L"[ステータス]  第%d区間 / 5", stage); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"攻撃  スラム %.0f  弾 %.0f", game.player_.GetSlamDamage(), st.bulletDamage.GetValue()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"被ダメ -%d%%   最大体力 %.0f", defense, game.player_.GetMaxHp()); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"速度 %.0f", (std::min)(GameConsts::MAX_MOVE_SPEED, st.moveSpeed.GetValue())); y += 16;
        FontManager::DrawFormat(hpX, y, GetColor(220, 220, 230), FontSize::Small, L"ジャンプ待ち時間 %.2f秒", game.player_.GetJumpCooldownMax()); y += 16;
        FontManager::Draw(hpX, y, L"3ウェーブごとに自動強化", GetColor(150, 220, 185), FontSize::Small); y += 16;
        cardListY = y + 8;
    }
    const auto& cards = game.cardManager_.GetActiveCards();
    if (!cards.empty())
    {
        FontManager::Draw(hpX, cardListY, TR(L"[所持カード]:"), GetColor(220, 230, 245), FontSize::Body);
        cardListY += 18;

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
    FontManager::Draw(40, guideY, TR(L"[WASD]: 泳ぐ  |  [SPACE]: ジャンプスラム  |  [A/D+Enter / 1,2,3]: カード選択  |  [R]: リセット  |  [F1]: デバッグ  |  [F2]: 画面揺れ  |  [F11]: パネル"), GetColor(200, 215, 235), FontSize::Body);
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

    MenuStyle::Center(midX, sh - 168, TR(L"オドル : ジャスト・ア・フィッシュ・ゲーム"), MenuStyle::Paper(), FontSize::Header);
    MenuStyle::Button(midX - 280, sh - 121, 560, TR(L"SPACE / ENTER   スタート"), true, MenuStyle::Accent());
    SetDrawMode(previousDrawMode);
    SetDrawBlendMode(previousBlend, previousAlpha);
}


void UIRenderer::DrawFlowOverlay(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;
    if (game.activeTip_ < 0 || game.paused_) return;

    static const wchar_t* kTips[GamePrototype::Tip_Count] = {
        L"移動: [WASD] / 矢印キーで泳ぐ",
        L"ジャンプ/スラム: [SPACE] ― 空中は無敵、着地で攻撃",
        L"攻撃: 敵の上にスラムして倒そう",
        L"カード: 強化を1枚選ぶ ― [A/D]+[ENTER] または [1][2][3]",
        L"ボス: 空中で攻撃をかわし、スキを見てスラム！" };
    int y = (game.state_ == GameState::CardPick) ? 150 : 104;
    DrawBox(sw / 2 - 340, y, sw / 2 + 340, y + 52, GetColor(15, 40, 70), TRUE);
    DrawBox(sw / 2 - 340, y, sw / 2 + 340, y + 52, GetColor(255, 220, 80), FALSE);
    FontManager::Draw(sw / 2 - 325, y + 6, TR(kTips[game.activeTip_]), GetColor(255, 235, 120), FontSize::Medium);
    FontManager::Draw(sw / 2 - 325, y + 32, TR(L"[TAB] ヒントをスキップ"), GetColor(170, 190, 215), FontSize::Small);
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

    DrawCX(midX, midY - 60, TR(L"！ 警告: 深海エネルギー異常 ！"), GetColor(255, 60, 60), FontSize::Header);
    DrawCX(midX, midY - 15, bossKind == 4 ? TR(L"ボス: カジキが斬り込んでくる！") : bossKind == 3 ? TR(L"ボス: 古代ガメが岩礁を守る！") : bossKind == 2 ? TR(L"ボス: 巨大イカが深海を塞ぐ！") : bossKind == 1 ? TR(L"ボス: 巨大ザメが狙っている！") : TR(L"ボス: 巨大クジラが現れた！"), GetColor(255, 215, 60), FontSize::Medium);
    DrawCX(midX, midY + 40, TR(L"生き残りをかけた戦いが今始まる…"), GetColor(240, 240, 240), FontSize::Body);
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

    MenuStyle::Center(midX, boxY + 28, TR(L"一時停止"), MenuStyle::Paper(), FontSize::Title, 1.3f);

    const SoundManager& sound = SoundManager::Instance();
    wchar_t labels[GamePrototype::PauseItem_Count][64] = {};
    swprintf_s(labels[GamePrototype::PauseItem_Resume], TR(L"再開"));
    swprintf_s(labels[GamePrototype::PauseItem_BgmVolume], TR(L"音楽音量:  < %3d >"), sound.GetVolume(SoundChannel::BGM));
    swprintf_s(labels[GamePrototype::PauseItem_SfxVolume], TR(L"効果音量:  < %3d >"), sound.GetVolume(SoundChannel::SFX));
    swprintf_s(labels[GamePrototype::PauseItem_WindowMode], TR(L"画面:  < %s >"), GetWindowModeFlag() == TRUE ? TR(L"ウィンドウ") : TR(L"フルスクリーン"));
    swprintf_s(labels[GamePrototype::PauseItem_Title], TR(L"タイトルへ戻る"));

    int itemY = boxY + 106;
    for (int i = 0; i < GamePrototype::PauseItem_Count; ++i)
    {
        bool selected = (i == game.pauseSelected_);
        MenuStyle::Button(boxX + 40, itemY, boxW - 80, labels[i], selected, MenuStyle::Accent());
        itemY += 64;
    }

    MenuStyle::Center(midX, boxY + 445, TR(L"W/S・上下キーで選択   ENTER/SPACEで決定"), MenuStyle::Muted(), FontSize::Body, 1.0f, boxW - 60);
    MenuStyle::Center(midX, boxY + 480, TR(L"A/D: 調整   ESC/P: 再開"), MenuStyle::Muted(), FontSize::Body);
}
