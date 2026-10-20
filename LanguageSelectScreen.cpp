#include "LanguageSelectScreen.h"
#include "Consts.h"
#include "FontManager.h"
#include "MenuStyle.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    // Always use the bilingual font: the player has not chosen a language yet.
    void DrawLabel(int cx, int y, const wchar_t* text, unsigned int color,
                   FontSize size, float scale = 1.0f, int maxWidth = 1200)
    {
        MenuStyle::Center(cx, y, text, color, size, scale, maxWidth);
    }
}

void LanguageSelectScreen::Init()
{
    Shutdown();
    illustratedMenu_.Init(L"Data/UI/ChoiceMenus/Illustrated-v1/language.png", nullptr, IllustratedChoiceMenu::Layout::Language);
    artwork_[0] = LoadGraph(L"Data/UI/LanguageSelect/english-v1.png");
    artwork_[1] = LoadGraph(L"Data/UI/LanguageSelect/japanese-v1.png");
    for (int i = 0; i < 2; ++i)
    {
        if (artwork_[i] == -1)
            AppLogAdd(L"[LANGUAGE_UI] Missing %s artwork; using the panel background.\n", i == 0 ? L"English" : L"Japanese");
    }
}

void LanguageSelectScreen::Shutdown()
{
    illustratedMenu_.Shutdown();
    for (int& handle : artwork_)
    {
        if (handle != -1) DeleteGraph(handle);
        handle = -1;
    }
}

void LanguageSelectScreen::Draw(int selected) const
{
    if (illustratedMenu_.Draw(selected, false)) return;
    const int sw = GameConsts::SCREEN_WIDTH, sh = GameConsts::SCREEN_HEIGHT;
    const int width = 500, height = 560, gap = 48;
    const int startX = (sw - width * 2 - gap) / 2;
    selected = selected == 1 ? 1 : 0;
    const unsigned int paper = GetColor(248, 240, 222);
    const unsigned int muted = GetColor(162, 187, 196);
    const unsigned int base = GetColor(9, 28, 43);
    const int previousDrawMode = GetDrawMode();
    int previousBlend = DX_BLENDMODE_NOBLEND, previousAlpha = 255;
    GetDrawBlendMode(&previousBlend, &previousAlpha);
    SetDrawMode(DX_DRAWMODE_BILINEAR);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawBox(0, 0, sw, sh, GetColor(7, 19, 31), TRUE);

    DrawLabel(sw / 2, 58, L"言語を選ぼう", paper, FontSize::Title, 1.3f);
    DrawLabel(sw / 2, 116, L"CHOOSE YOUR LANGUAGE", muted, FontSize::Medium);

    const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetNowCount() % 10000) * 0.00314159f);
    const wchar_t* names[2] = { L"English", L"日本語" };
    const wchar_t* subtitles[2] = { L"英語", L"Japanese" };
    for (int i = 0; i < 2; ++i)
    {
        const bool active = i == selected;
        const int x = startX + i * (width + gap);
        const int y = 198 - (active ? 8 : 0);
        const int cx = x + width / 2;
        const unsigned int accent = i == 0 ? GetColor(249, 215, 148) : GetColor(244, 174, 192);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 90);
        DrawBox(x - 5, y + 12, x + width + 5, y + height + 12, GetColor(0, 0, 0), TRUE);
        if (active)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(25 + pulse * 15));
            DrawBox(x - 5, y - 5, x + width + 5, y + height + 5, accent, FALSE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(x, y, x + width, y + height, base, TRUE);
        if (artwork_[i] != -1)
            DrawExtendGraph(x, y, x + width, y + width, artwork_[i], TRUE);

        // Blend the lower part of each picture into a quiet live caption area.
        for (int fy = 340; fy < width; fy += 4)
        {
            const float k = static_cast<float>(fy - 340) / (width - 344);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * k));
            DrawBox(x, y + fy, x + width, y + fy + 4, base, TRUE);
        }
        if (!active)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 38);
            DrawBox(x, y, x + width, y + height, GetColor(7, 16, 28), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(x, y, x + width, y + height, active ? accent : GetColor(58, 79, 93), FALSE);
        if (active)
        {
            DrawLine(x, y, x + 44, y, accent, 3);
            DrawLine(x, y, x, y + 44, accent, 3);
            DrawLine(x + width - 44, y + height, x + width, y + height, accent, 3);
            DrawLine(x + width, y + height - 44, x + width, y + height, accent, 3);
        }

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
        DrawBox(x + 20, y + 20, x + 62, y + 62, base, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawLabel(x + 41, y + 28, i == 0 ? L"1" : L"2", paper, FontSize::Header);
        DrawLabel(cx, y + 390, names[i], paper, FontSize::Title, 1.55f, width - 60);
        DrawLabel(cx, y + 450, subtitles[i], muted, FontSize::Medium);
        DrawLine(x + 36, y + 490, x + width - 36, y + 490, GetColor(52, 77, 91));
        if (active)
            DrawLabel(cx, y + 511, L"選択中 / SELECTED", accent, FontSize::Body);
    }

    const unsigned int accent = selected == 0 ? GetColor(249, 215, 148) : GetColor(244, 174, 192);
    MenuStyle::Button(sw / 2 - 280, 783, 560, L"ENTER / SPACE  決定 / CONFIRM", true, accent);
    DrawLabel(sw / 2, 848, L"A / D ・ ← / →  選択 / CHOOSE     1 / 2  すぐに決定 / QUICK SELECT", muted, FontSize::Body);
    SetDrawMode(previousDrawMode);
    SetDrawBlendMode(previousBlend, previousAlpha);
}
