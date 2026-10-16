#include "ResultScreen.h"
#include "Input.h"
#include "FontManager.h"
#include "Localization.h"
#include "Consts.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    float Ease(float t) { t = std::clamp(t, 0.0f, 1.0f); return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t); }

    int TextWidth(const wchar_t* s, FontSize size)
    {
        int h = FontManager::GetHandle(size);
        return (h >= 0) ? GetDrawStringWidthToHandle(s, static_cast<int>(wcslen(s)), h) : static_cast<int>(wcslen(s)) * 10;
    }

    void DrawCentered(int cx, int y, const wchar_t* s, unsigned int color, FontSize size)
    {
        FontManager::Draw(cx - TextWidth(s, size) / 2, y, s, color, size);
    }
}

void ResultScreen::Enter(bool victory, bool diedAtBoss)
{
    timer_ = 0.0f;
    selected_ = 0;
    victory_ = victory;
    diedAtBoss_ = diedAtBoss;
    confirmPrev_ = true;
}

int ResultScreen::ComputeScore(const ResultStats& s)
{
    // Điểm: 100/quái + 250/level + 1000/wave đã qua + 5000 khi thắng
    int wavesCleared = (std::max)(0, s.wave - 1);
    int score = s.kills * 100 + (std::max)(0, s.level - 1) * 250 + wavesCleared * 1000;
    if (s.victory) score += 5000;
    return score;
}

ResultScreen::Action ResultScreen::Update(float dt, const Input& input)
{
    timer_ += dt;
    bool confirmNow = input.IsDown(KEY_INPUT_RETURN) || input.IsDown(KEY_INPUT_SPACE);
    bool confirmReleased = confirmPrev_ && !confirmNow;
    confirmPrev_ = confirmNow;
    if (timer_ < INPUT_DELAY) return Action::None;

    if (input.IsPressed(KEY_INPUT_UP) || input.IsPressed(KEY_INPUT_W)) selected_ = (selected_ + 1) % 2;
    if (input.IsPressed(KEY_INPUT_DOWN) || input.IsPressed(KEY_INPUT_S)) selected_ = (selected_ + 1) % 2;

    // Phím tắt cũ vẫn dùng được
    if (input.IsPressed(KEY_INPUT_R)) return Action::Retry;
    if (input.IsPressed(KEY_INPUT_T)) return Action::Title;

    if (confirmReleased) return (selected_ == 0) ? Action::Retry : Action::Title;
    return Action::None;
}

void ResultScreen::Draw(const ResultStats& s) const
{
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

    // 2. Tiêu đề: rơi xuống + nhún, từng chữ lượn sóng
    const wchar_t* title = s.victory ? TR(L"VICTORY!") : TR(L"GAME OVER");
    unsigned int titleCol = s.victory ? GetColor(255, 225, 70) : GetColor(255, 70, 70);
    float drop = Ease((t - 0.25f) / 0.6f);
    int baseY = static_cast<int>(150.0f - 90.0f * (1.0f - drop));
    int hHeader = FontManager::GetHandle(FontSize::Header);
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
    const wchar_t* sub = s.victory ? TR(L"You defeated the Giant Whale!")
                       : (s.diedAtBoss ? TR(L"Fell before the Giant Whale...") : TR(L"Your fish was stranded..."));
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
    swprintf_s(rows[2], L"Lv.%d", s.level);
    if (s.endless) swprintf_s(rows[3], s.victory ? TR(L"%d / %d  CLEAR") : TR(L"Stage %d"), s.wave, GameConsts::ENDLESS_MAX_STAGE);
    else swprintf_s(rows[3], TR(L"Tutorial %d / %d"), s.wave, GameConsts::TUTORIAL_WAVES);
    swprintf_s(rows[4], L"%d", static_cast<int>(score * countT + 0.5f));
    const wchar_t* labels[5] = { TR(L"TIME"), TR(L"KILLS"), TR(L"LEVEL"), TR(L"WAVE"), TR(L"SCORE") };
    for (int i = 0; i < 5; ++i)
    {
        float ra = Ease((t - 0.7f - i * 0.1f) / 0.3f);
        if (ra <= 0.0f) break;
        int ry = py + 26 + i * 40;
        int off = static_cast<int>(40.0f * (1.0f - ra));
        bool isScore = (i == 4);
        if (isScore) DrawLine(px + 30, ry - 8, px + panelW - 30, ry - 8, GetColor(70, 100, 140));
        FontManager::Draw(px + 50 - off, ry, labels[i], GetColor(150, 190, 225), FontSize::Medium);
        unsigned int vc = isScore ? GetColor(255, 225, 80) : GetColor(255, 255, 255);
        FontManager::Draw(px + panelW - 50 - TextWidth(rows[i], FontSize::Medium) + off, ry, rows[i], vc, FontSize::Medium);
    }

    int extraY = py + panelH + 14;
    if (s.endless)
    {
        int bm = static_cast<int>(s.bestTime) / 60, bs = static_cast<int>(s.bestTime) % 60;
        wchar_t best[96];
        swprintf_s(best, TR(L"Best: Wave %d  |  Kills %d  |  %02d:%02d  |  Score %d"), s.bestWave, s.bestKills, bm, bs, s.bestScore);
        DrawCentered(cx, extraY, best, GetColor(255, 215, 80), FontSize::Body);
        extraY += 22;
    }
    if (s.debugUsed)
    {
        DrawCentered(cx, extraY, s.endless ? TR(L"[DEBUG MODE USED - RECORD NOT SAVED]") : TR(L"[DEBUG MODE USED]"), GetColor(255, 90, 90), FontSize::Body);
    }

    // 4. Menu Retry / Title
    float ma = Ease((t - 1.2f) / 0.4f);
    if (ma <= 0.0f) return;
    const wchar_t* items[2] = { s.victory ? TR(L"Play Again") : TR(L"Retry"), TR(L"Return to Title") };
    int my = 650;
    for (int i = 0; i < 2; ++i)
    {
        bool sel = (i == selected_);
        int bw = 300, bh = 44, bx = cx - bw / 2, by = my + i * 56;
        float pulse = sel ? 0.5f + 0.5f * std::sin(t * 6.0f) : 0.0f;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>((sel ? 210.0f : 140.0f) * ma));
        DrawRoundRect(bx, by, bx + bw, by + bh, 10, 10, sel ? GetColor(30, 70, 120) : GetColor(12, 26, 44), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        if (sel)
        {
            DrawRoundRect(bx - 2, by - 2, bx + bw + 2, by + bh + 2, 12, 12, GetColor(255, 200 + static_cast<int>(55 * pulse), 80), FALSE);
            FontManager::Draw(bx + 18 + static_cast<int>(4.0f * pulse), by + 11, L">", GetColor(255, 225, 60), FontSize::Medium);
        }
        DrawCentered(cx, by + 11, items[i], sel ? GetColor(255, 225, 60) : GetColor(200, 215, 235), FontSize::Medium);
    }
    DrawCentered(cx, my + 122, TR(L"[W/S]: Select   [Enter/Space]: OK   [R]: Retry   [T]: Title"), GetColor(140, 170, 200), FontSize::Small);
}
