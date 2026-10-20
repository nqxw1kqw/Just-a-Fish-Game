#include "ResultScreen.h"
#include "Input.h"
#include "FontManager.h"
#include "Localization.h"
#include "Consts.h"
#include "DxLib.h"
#include "MenuStyle.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    float Ease(float t) { t = std::clamp(t, 0.0f, 1.0f); return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t); }

    int TextWidth(const wchar_t* s, FontSize size)
    {
        int h = MenuStyle::Font(size);
        return (h >= 0) ? GetDrawStringWidthToHandle(s, static_cast<int>(wcslen(s)), h) : static_cast<int>(wcslen(s)) * 10;
    }

    void DrawCentered(int cx, int y, const wchar_t* s, unsigned int color, FontSize size)
    {
        MenuStyle::Center(cx, y, s, color, size);
    }

    void DrawFitted(int x, int y, const wchar_t* text, unsigned int color,
                    FontSize size, int maxWidth, float scale = 1.0f)
    {
        MenuStyle::Text(x, y, text, color, size, scale, maxWidth);
    }

    // Mutable run information remains text; the main lettering and frames are PNG art.
    void DrawRunNote(int cx, int y, const wchar_t* text, unsigned int color)
    {
        const unsigned int outline = GetColor(255, 248, 221);
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (dx || dy) MenuStyle::Center(cx + dx, y + dy, text, outline, FontSize::Body, 1.0f, 740);
        MenuStyle::Center(cx, y, text, color, FontSize::Body, 1.0f, 740);
    }
}

void ResultScreen::Init()
{
    Shutdown();
    layeredScreen_.Init();
    selectionArtwork_ = LoadGraph(L"Data/UI/Results/Illustrated-v1/selection-arrow.png");
    defeatArtwork_ = LoadGraph(L"Data/UI/Results/defeat-v2-simple.png");
    if (defeatArtwork_ == -1)
        AppLogAdd(L"[RESULT_UI] Missing defeat artwork; using the panel background.\n");
}

void ResultScreen::Shutdown()
{
    layeredScreen_.Shutdown();
    if (selectionArtwork_ != -1) DeleteGraph(selectionArtwork_);
    selectionArtwork_ = -1;
    if (defeatArtwork_ != -1) DeleteGraph(defeatArtwork_);
    defeatArtwork_ = -1;
}

void ResultScreen::Enter(bool victory, bool diedAtBoss)
{
    layeredScreen_.Enter();
    timer_ = 0.0f;
    selected_ = 0;
    victory_ = victory;
    diedAtBoss_ = diedAtBoss;
    confirmPrev_ = true;
    confirmReady_ = false;
    GetMousePoint(&mouseX_, &mouseY_);
    mousePrev_ = true;
    mouseReady_ = false;
    mousePressedItem_ = -1;
}

int ResultScreen::ComputeScore(const ResultStats& s)
{
    // 撃破100点、クリア済みウェーブ1000点、勝利5000点。
    int wavesCleared = (std::max)(0, s.wavesCleared);
    int score = s.kills * 100 + wavesCleared * 1000;
    if (s.victory) score += 5000;
    return score;
}

ResultScreen::Action ResultScreen::Update(float dt, const Input& input)
{
    timer_ += dt;
    bool confirmNow = input.IsDown(KEY_INPUT_RETURN) || input.IsDown(KEY_INPUT_SPACE);
    bool confirmReleased = confirmPrev_ && !confirmNow;
    confirmPrev_ = confirmNow;
    int mouseX = mouseX_, mouseY = mouseY_;
    GetMousePoint(&mouseX, &mouseY);
    const bool mouseNow = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    const bool mousePressed = mouseNow && !mousePrev_;
    const bool mouseReleased = !mouseNow && mousePrev_;
    const bool mouseMoved = mouseX != mouseX_ || mouseY != mouseY_;
    mouseX_ = mouseX; mouseY_ = mouseY; mousePrev_ = mouseNow;
    if (timer_ < INPUT_DELAY) return Action::None;

    const int language = 1;
    const bool layered = layeredScreen_.Ready(victory_, language == 1);
    if (layered)
    {
        const int hovered = layeredScreen_.ButtonAt(mouseX, mouseY, selected_, mouseNow, mousePressedItem_, true);
        // A stationary pointer must not undo selection made with the keyboard.
        if (mouseMoved && hovered >= 0) selected_ = hovered;
        if (!mouseReady_)
        {
            if (!mouseNow) mouseReady_ = true;
        }
        else if (mousePressed)
        {
            mousePressedItem_ = hovered;
            if (hovered >= 0) selected_ = hovered;
        }
        else if (mouseReleased)
        {
            const int pressedItem = mousePressedItem_;
            mousePressedItem_ = -1;
            if (pressedItem >= 0 && hovered == pressedItem)
                return pressedItem == 0 ? Action::Retry : Action::Title;
        }
    }

    if (input.IsPressed(KEY_INPUT_UP) || input.IsPressed(KEY_INPUT_W)) selected_ = (selected_ + 1) % 2;
    if (input.IsPressed(KEY_INPUT_DOWN) || input.IsPressed(KEY_INPUT_S)) selected_ = (selected_ + 1) % 2;

    // Phím tắt cũ vẫn dùng được
    if (input.IsPressed(KEY_INPUT_R)) return Action::Retry;
    if (input.IsPressed(KEY_INPUT_T)) return Action::Title;

    // Require a neutral key state after the entrance delay. A key held while
    // dying must be released before a new confirmation can be accepted.
    if (!confirmReady_)
    {
        if (!confirmNow) confirmReady_ = true;
        return Action::None;
    }
    if (confirmReleased) return (selected_ == 0) ? Action::Retry : Action::Title;
    return Action::None;
}

void ResultScreen::Draw(const ResultStats& s) const
{
    const MenuStyle::DrawState drawState;
    if (layeredScreen_.Draw(s, timer_, selected_, mousePrev_, mousePressedItem_, selectionArtwork_)) return;
    if (!s.victory)
    {
        DrawDefeat(s);
        return;
    }

    const int sw = GameConsts::SCREEN_WIDTH, sh = GameConsts::SCREEN_HEIGHT;
    const int cx = sw / 2;
    const float t = timer_;

    // 1. Fade-in nền
    float fade = Ease(t / 0.8f);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(215.0f * fade));
    DrawBox(0, 0, sw, sh, s.victory ? GetColor(8, 24, 46) : GetColor(30, 8, 12), TRUE);
    // Dải sáng chéo nhẹ di chuyển (cảm giác dưới nước)
    for (int i = 0; i < 6; ++i)
    {
        float x = std::fmod(t * 60.0f + i * 300.0f, static_cast<float>(sw + 400)) - 200.0f;
        SetDrawBlendMode(DX_BLENDMODE_ADD, static_cast<int>(18.0f * fade));
        DrawTriangleAA(x, 0.0f, x + 120.0f, 0.0f, x - 160.0f, static_cast<float>(sh), s.victory ? GetColor(80, 160, 255) : GetColor(255, 80, 80), TRUE);
        DrawTriangleAA(x + 120.0f, 0.0f, x - 40.0f, static_cast<float>(sh), x - 160.0f, static_cast<float>(sh), s.victory ? GetColor(80, 160, 255) : GetColor(255, 80, 80), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    if (t < 0.25f) return;

    const wchar_t* modeName = s.campaign ? TR(L"冒険") : TR(L"チュートリアル");
    DrawCentered(cx, 96, modeName, MenuStyle::Muted(), FontSize::Body);

    // 2. Tiêu đề: rơi xuống + nhún, từng chữ lượn sóng
    const wchar_t* title = s.victory ? TR(L"勝利！") : TR(L"ゲームオーバー");
    unsigned int titleCol = s.victory ? GetColor(255, 225, 70) : GetColor(255, 70, 70);
    float drop = Ease((t - 0.25f) / 0.6f);
    int baseY = static_cast<int>(150.0f - 90.0f * (1.0f - drop));
    int hHeader = MenuStyle::Font(FontSize::Header);
    int len = static_cast<int>(wcslen(title));
    int scale = 2;
    int totalW = TextWidth(title, FontSize::Header) * scale;
    int x = cx - totalW / 2;
    for (int i = 0; i < len; ++i)
    {
        wchar_t ch[2] = { title[i], 0 };
        int cw = TextWidth(ch, FontSize::Header) * scale;
        int wy = baseY + static_cast<int>(std::sin(t * 4.0f + i * 0.6f) * 6.0f);
        if (hHeader >= 0)
        {
            // Bóng + chữ phóng to x2
            DrawExtendStringToHandle(x + 4, wy + 4, scale, scale, ch, GetColor(0, 0, 0), hHeader);
            DrawExtendStringToHandle(x, wy, scale, scale, ch, titleCol, hHeader);
        }
        else
        {
            FontManager::Draw(x, wy, ch, titleCol, FontSize::Header);
        }
        x += cw;
    }
    const wchar_t* sub = s.victory ? TR(L"5体のボスを倒した！")
                       : (s.diedAtBoss ? TR(L"ボスとの戦いで力尽きた…") : TR(L"お魚は打ち上げられてしまった…"));
    DrawCentered(cx, baseY + 82, sub, GetColor(220, 235, 255), FontSize::Medium);

    // 3. Bảng thống kê (trượt vào lần lượt)
    int panelW = 520, panelH = 250, px = cx - panelW / 2, py = 300;
    float pa = Ease((t - 0.6f) / 0.4f);
    if (pa <= 0.0f) return;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(200.0f * pa));
    DrawRoundRect(px, py, px + panelW, py + panelH, 16, 16, GetColor(6, 18, 34), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawRoundRect(px, py, px + panelW, py + panelH, 16, 16, s.victory ? GetColor(255, 210, 90) : GetColor(200, 80, 80), FALSE);

    int score = ComputeScore(s);
    float countT = Ease((t - 0.9f) / 1.0f);   // số đếm tăng dần
    int min = static_cast<int>(s.time) / 60, sec = static_cast<int>(s.time) % 60;
    wchar_t rows[5][64] = {};
    swprintf_s(rows[0], L"%02d:%02d", min, sec);
    swprintf_s(rows[1], L"%d", static_cast<int>(s.kills * countT + 0.5f));
    if (s.campaign) swprintf_s(rows[2], L"%d / 5", s.stage);
    else swprintf_s(rows[2], L"練習");
    if (s.campaign) swprintf_s(rows[3], s.victory ? TR(L"%d / %d  クリア") : TR(L"ステージ %d"), s.wave, GameConsts::CAMPAIGN_WAVES);
    else swprintf_s(rows[3], TR(L"チュートリアル %d / %d"), s.wave, GameConsts::TUTORIAL_WAVES);
    swprintf_s(rows[4], L"%d", static_cast<int>(score * countT + 0.5f));
    const wchar_t* labels[5] = { TR(L"時間"), TR(L"撃破"), TR(L"区間"), s.campaign ? TR(L"ステージ") : TR(L"チュートリアル"), TR(L"スコア") };
    for (int i = 0; i < 5; ++i)
    {
        float ra = Ease((t - 0.7f - i * 0.1f) / 0.3f);
        if (ra <= 0.0f) break;
        int ry = py + 26 + i * 40;
        int off = static_cast<int>(40.0f * (1.0f - ra));
        bool isScore = (i == 4);
        if (isScore) DrawLine(px + 30, ry - 8, px + panelW - 30, ry - 8, GetColor(70, 100, 140));
        MenuStyle::Text(px + 50 - off, ry, labels[i], MenuStyle::Muted(), FontSize::Medium);
        unsigned int vc = isScore ? GetColor(255, 225, 80) : GetColor(255, 255, 255);
        MenuStyle::Text(px + panelW - 50 - TextWidth(rows[i], FontSize::Medium) + off, ry, rows[i], vc, FontSize::Medium);
    }

    int extraY = py + panelH + 14;
    if (s.campaign)
    {
        int bm = static_cast<int>(s.bestTime) / 60, bs = static_cast<int>(s.bestTime) % 60;
        wchar_t best[96];
        swprintf_s(best, TR(L"ベスト: ステージ %d  |  撃破 %d  |  %02d:%02d  |  スコア %d"), s.bestWave, s.bestKills, bm, bs, s.bestScore);
        DrawCentered(cx, extraY, best, GetColor(255, 215, 80), FontSize::Body);
        extraY += 22;
    }
    if (s.debugUsed)
    {
        DrawCentered(cx, extraY, s.campaign ? TR(L"[デバッグ使用 ― 記録は保存されません]") : TR(L"[デバッグ使用]"), GetColor(255, 90, 90), FontSize::Body);
    }

    // 4. Menu Retry / Title
    float ma = Ease((t - 1.2f) / 0.4f);
    if (ma <= 0.0f) return;
    const wchar_t* items[2] = { s.retryTutorial ? TR(L"チュートリアルから再挑戦") : TR(L"もう一度遊ぶ"), TR(L"タイトルへ戻る") };
    for (int i = 0; i < 2; ++i)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * ma));
        MenuStyle::Button(cx - 280, 650 + i * 66, 560, items[i], selected_ == i, MenuStyle::Accent());
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawCentered(cx, 802, TR(L"W/S・上下キーで選択   ENTER/SPACEで決定"), MenuStyle::Muted(), FontSize::Body);
    DrawCentered(cx, 833, TR(L"R: リトライ   T: タイトルへ"), MenuStyle::Muted(), FontSize::Body);
}

void ResultScreen::DrawDefeat(const ResultStats& s) const
{
    const int sw = GameConsts::SCREEN_WIDTH, sh = GameConsts::SCREEN_HEIGHT;
    const int artX = 112, artY = 112, artSize = 600;
    const int panelX = 800, panelW = 656;
    const float fade = Ease(timer_ / 0.45f);
    const unsigned int background = GetColor(7, 19, 31);
    const unsigned int paper = GetColor(247, 238, 217);
    const unsigned int muted = GetColor(155, 180, 192);
    const unsigned int accent = GetColor(245, 205, 137);
    const int previousDrawMode = GetDrawMode();
    int previousBlend = DX_BLENDMODE_NOBLEND, previousAlpha = 255;
    GetDrawBlendMode(&previousBlend, &previousAlpha);
    SetDrawMode(DX_DRAWMODE_BILINEAR);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawBox(0, 0, sw, sh, background, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * fade));
    DrawBox(artX, artY, artX + artSize, artY + artSize, GetColor(11, 30, 45), TRUE);
    if (defeatArtwork_ != -1)
        DrawExtendGraph(artX, artY, artX + artSize, artY + artSize, defeatArtwork_, TRUE);
    // Keep the simple drawing on a plain paper-like card, with no image effects.
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * fade));
    const wchar_t* caption = TR(L"ひと休みしたら、また泳ごう。");
    const float captionScale = (std::min)(1.0f, static_cast<float>(artSize - 36) / (std::max)(1, TextWidth(caption, FontSize::Medium)));
    DrawFitted(artX + (artSize - static_cast<int>(TextWidth(caption, FontSize::Medium) * captionScale)) / 2,
               731, caption, muted, FontSize::Medium, artSize - 36);

    const wchar_t* modeName = s.campaign ? TR(L"冒険") : TR(L"チュートリアル");
    wchar_t summary[96] = {};
    swprintf_s(summary, L"%s / %s", TR(L"今回の記録"), modeName);
    DrawFitted(panelX, 100, summary, muted, FontSize::Body, panelW);
    DrawFitted(panelX, 132, TR(L"ゲームオーバー"), paper, FontSize::Title, panelW, 1.55f);
    DrawFitted(panelX, 198, s.diedAtBoss ? TR(L"ボスとの戦いで力尽きた。") : TR(L"今回の冒険はここまで。"),
               muted, FontSize::Medium, panelW);

    // The existing score formula is shared with highscore persistence.
    const float count = Ease((timer_ - 0.45f) / 0.85f);
    wchar_t score[32] = {};
    swprintf_s(score, L"%d", static_cast<int>(ComputeScore(s) * count + 0.5f));
    DrawFitted(panelX, 258, TR(L"スコア"), accent, FontSize::Body, panelW);
    DrawFitted(panelX - 2, 282, score, paper, FontSize::Title, panelW, 2.05f);
    DrawLine(panelX, 365, panelX + panelW, 365, GetColor(52, 75, 91));

    const int minutes = static_cast<int>(s.time) / 60, seconds = static_cast<int>(s.time) % 60;
    wchar_t values[4][64] = {};
    swprintf_s(values[0], L"%02d:%02d", minutes, seconds);
    swprintf_s(values[1], L"%d", static_cast<int>(s.kills * count + 0.5f));
    if (s.campaign) swprintf_s(values[2], L"%d / 5", s.stage);
    else swprintf_s(values[2], L"練習");
    if (s.campaign) swprintf_s(values[3], L"%d / %d", s.wave, GameConsts::CAMPAIGN_WAVES);
    else swprintf_s(values[3], L"%d / %d", s.wave, GameConsts::TUTORIAL_WAVES);
    const wchar_t* labels[4] = { TR(L"時間"), TR(L"撃破"), TR(L"区間"), s.campaign ? TR(L"ステージ") : TR(L"チュートリアル") };
    for (int i = 0; i < 4; ++i)
    {
        const int x = panelX + (i % 2) * 340;
        const int y = 389 + (i / 2) * 96;
        DrawFitted(x, y, labels[i], muted, FontSize::Body, 300);
        DrawFitted(x, y + 27, values[i], paper, FontSize::Header, 300, 1.16f);
    }

    if (s.campaign)
    {
        wchar_t best[128] = {};
        const int bestMinutes = static_cast<int>(s.bestTime) / 60, bestSeconds = static_cast<int>(s.bestTime) % 60;
        swprintf_s(best, TR(L"ベスト: ステージ %d  |  撃破 %d  |  %02d:%02d  |  スコア %d"),
                   s.bestWave, s.bestKills, bestMinutes, bestSeconds, s.bestScore);
        DrawFitted(panelX, 568, best, accent, FontSize::Body, panelW);
    }
    if (s.debugUsed)
        DrawFitted(panelX, 600, s.campaign ? TR(L"[デバッグ使用 ― 記録は保存されません]") : TR(L"[デバッグ使用]"),
                   GetColor(239, 151, 142), FontSize::Body, panelW);

    const float menuFade = Ease((timer_ - INPUT_DELAY) / 0.3f);
    const wchar_t* items[2] = { s.retryTutorial ? TR(L"チュートリアルから再挑戦") : TR(L"リトライ"), TR(L"タイトルへ戻る") };
    for (int i = 0; i < 2; ++i)
    {
        const bool active = selected_ == i;
        const int y = 650 + i * 66;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * menuFade));
        MenuStyle::Button(panelX, y, panelW, items[i], active, accent);
    }
    DrawCentered(panelX + panelW / 2, 802, TR(L"W/S・上下キーで選択   ENTER/SPACEで決定"), muted, FontSize::Body);
    DrawCentered(panelX + panelW / 2, 833, TR(L"R: リトライ   T: タイトルへ"), muted, FontSize::Body);
    SetDrawMode(previousDrawMode);
    SetDrawBlendMode(previousBlend, previousAlpha);
}
