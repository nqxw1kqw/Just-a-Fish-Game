#include "ModeSelectScreen.h"
#include "Consts.h"
#include "FontManager.h"
#include "Localization.h"
#include "MenuStyle.h"
#include <algorithm>
#include <cmath>

namespace
{
    void DrawCentered(int cx, int y, const wchar_t* text, unsigned int color, FontSize size)
    {
        MenuStyle::Center(cx, y, text, color, size);
    }

    void DrawFitted(int x, int y, const wchar_t* text, unsigned int color,
                    FontSize size, float scale, int availableWidth)
    {
        MenuStyle::Text(x, y, text, color, size, scale, availableWidth);
    }
}

void ModeSelectScreen::Init()
{
    Shutdown();
    illustratedMenu_.Init(L"Data/UI/ChoiceMenus/Illustrated-v1/mode-ja.png");
}

void ModeSelectScreen::Shutdown()
{
    illustratedMenu_.Shutdown();
}

bool ModeSelectScreen::UpdatePointer(int& selected)
{
    return illustratedMenu_.UpdatePointer(selected);
}

void ModeSelectScreen::Draw(int selected) const
{
    if (illustratedMenu_.Draw(selected)) return;
    const int sw = GameConsts::SCREEN_WIDTH;
    const int sh = GameConsts::SCREEN_HEIGHT;
    const int cardWidth = 520, cardHeight = 570, gap = 48;
    const int startX = (sw - cardWidth * 2 - gap) / 2;
    const int top = 190;
    selected = selected == 1 ? 1 : 0;
    const float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetNowCount() % 10000) * 0.00314159f);
    const int previousDrawMode = GetDrawMode();
    int previousBlend = DX_BLENDMODE_NOBLEND, previousAlpha = 255;
    GetDrawBlendMode(&previousBlend, &previousAlpha);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    SetDrawMode(DX_DRAWMODE_BILINEAR);
    DrawBox(0, 0, sw, sh, MenuStyle::Background(), TRUE);

    const wchar_t* heading = TR(L"次の海を選ぼう");
    MenuStyle::Center(sw / 2, 65, heading, MenuStyle::Paper(), FontSize::Title, 1.3f, sw - 120);
    DrawCentered(sw / 2, 126, TR(L"チュートリアルクリア！ カードと回復はそのまま引き継ぎます。"), GetColor(158, 181, 187), FontSize::Medium);

    const wchar_t* names[2] = { TR(L"ノーマル"), TR(L"ハード") };
    const wchar_t* taglines[2] = { TR(L"自分のペースで挑もう"), TR(L"深海の試練に挑もう") };
    const wchar_t* rules[2][3] = {
        { TR(L"ステージごとに強化カードを選択"), TR(L"毎ステージ、敵をリセット"), TR(L"全30ステージ / 5ステージごとにボス") },
        { TR(L"受けるダメージが1.5倍"), TR(L"戦闘中にランダムで強化"), TR(L"ステージを越えて敵が残る") }
    };

    for (int i = 0; i < 2; ++i)
    {
        const bool isSelected = selected == i;
        const int x = startX + i * (cardWidth + gap);
        const int y = top - (isSelected ? 8 : 0);
        const unsigned int accent = i == 0 ? GetColor(139, 220, 202) : GetColor(239, 141, 113);
        const unsigned int base = i == 0 ? GetColor(9, 25, 33) : GetColor(23, 15, 26);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
        DrawBox(x - 7, y + 14, x + cardWidth + 7, y + cardHeight + 14, GetColor(0, 0, 0), TRUE);
        if (isSelected)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(20 + 12 * pulse));
            DrawBox(x - 5, y - 5, x + cardWidth + 5, y + cardHeight + 5, accent, FALSE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(x, y, x + cardWidth, y + cardHeight, base, TRUE);

        // Fade the illustration into its caption area without baking text into the image.
        const int fadeStart = y + 300;
        const int fadeEnd = y + cardWidth;
        for (int fy = fadeStart; fy < fadeEnd; fy += 4)
        {
            const float k = static_cast<float>(fy - fadeStart) / (fadeEnd - fadeStart - 4);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * k));
            DrawBox(x, fy, x + cardWidth, fy + 4, base, TRUE);
        }
        if (!isSelected)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 45);
            DrawBox(x, y, x + cardWidth, y + cardHeight, GetColor(4, 8, 16), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(x, y, x + cardWidth, y + cardHeight, isSelected ? accent : GetColor(55, 67, 76), FALSE);
        if (isSelected)
        {
            DrawLine(x, y, x + 48, y, accent, 3);
            DrawLine(x, y, x, y + 48, accent, 3);
            DrawLine(x + cardWidth - 48, y + cardHeight, x + cardWidth, y + cardHeight, accent, 3);
            DrawLine(x + cardWidth, y + cardHeight - 48, x + cardWidth, y + cardHeight, accent, 3);
        }

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 185);
        DrawBox(x + 22, y + 22, x + 64, y + 64, base, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawCentered(x + 43, y + 30, i == 0 ? L"1" : L"2", GetColor(240, 237, 218), FontSize::Header);

        DrawFitted(x + 30, y + 344, names[i], GetColor(247, 239, 222), FontSize::Title, 1.4f, cardWidth - 60);
        MenuStyle::Text(x + 30, y + 393, taglines[i], accent, FontSize::Medium);
        for (int row = 0; row < 3; ++row)
            DrawFitted(x + 30, y + 437 + row * 27, rules[i][row], GetColor(196, 207, 210), FontSize::Medium, 1.0f, cardWidth - 60);

        DrawLine(x + 30, y + 532, x + cardWidth - 30, y + 532, GetColor(48, 57, 67));
        if (isSelected)
        {
            DrawCircle(x + 35, y + 550, 3, accent, TRUE);
            MenuStyle::Text(x + 48, y + 541, TR(L"選択中"), accent, FontSize::Body);
        }
    }

    const unsigned int accent = selected == 0 ? GetColor(139, 220, 202) : GetColor(239, 141, 113);
    MenuStyle::Button(sw / 2 - 280, 783, 560, TR(L"ENTER / SPACE   決定"), true, accent);
    DrawCentered(sw / 2, 848, TR(L"A / D・左右キーで選択     1 / 2で開始"), MenuStyle::Muted(), FontSize::Body);
    SetDrawMode(previousDrawMode);
    SetDrawBlendMode(previousBlend, previousAlpha);
}
