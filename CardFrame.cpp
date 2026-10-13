#include "CardFrame.h"
#include "FontManager.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

namespace
{
    const CardFrame::Style kStyles[3] =
    {
        // Common: bạc xanh
        { 0, 0, { 26, 36, 52 }, { 12, 18, 28 }, { 120, 170, 220 }, 0.0f, 1, L"COMMON" },
        // Rare: tím
        { 0, 0, { 44, 22, 70 }, { 16, 10, 30 }, { 170, 80, 255 }, 0.55f, 2, L"RARE" },
        // Epic (Ultra Rare): vàng kim
        { 0, 0, { 70, 50, 14 }, { 26, 16, 6 }, { 255, 190, 50 }, 1.0f, 3, L"EPIC" },
    };
    bool s_colorsReady = false;
    CardFrame::Style s_styles[3];

    int s_frameImages[3] = { -2, -2, -2 }; // -2 = chưa thử nạp, -1 = không có file

    int Index(CardRarity r)
    {
        return (r == CardRarity::Epic) ? 2 : (r == CardRarity::Rare) ? 1 : 0;
    }

    void EnsureColors()
    {
        if (s_colorsReady) return;
        for (int i = 0; i < 3; ++i) s_styles[i] = kStyles[i];
        s_styles[0].edge = GetColor(185, 205, 225); s_styles[0].edgeDark = GetColor(70, 90, 115);
        s_styles[1].edge = GetColor(200, 120, 255); s_styles[1].edgeDark = GetColor(90, 40, 140);
        s_styles[2].edge = GetColor(255, 215, 70);  s_styles[2].edgeDark = GetColor(150, 100, 20);
        s_colorsReady = true;
    }

    int FrameImage(int idx)
    {
        if (s_frameImages[idx] == -2)
        {
            static const wchar_t* paths[3] = { L"Data/UI/Cards/frame_common.png", L"Data/UI/Cards/frame_rare.png", L"Data/UI/Cards/frame_epic.png" };
            s_frameImages[idx] = (FileRead_size(paths[idx]) > 0) ? LoadGraph(paths[idx]) : -1;
        }
        return s_frameImages[idx];
    }

    // Ngọc hình thoi
    void DrawGem(float cx, float cy, float r, unsigned int fill, unsigned int rim)
    {
        DrawQuadrangleAA(cx, cy - r, cx + r * 0.75f, cy, cx, cy + r, cx - r * 0.75f, cy, fill, TRUE);
        DrawQuadrangleAA(cx, cy - r, cx + r * 0.75f, cy, cx, cy + r, cx - r * 0.75f, cy, rim, FALSE, 1.5f);
        DrawCircleAA(cx - r * 0.2f, cy - r * 0.35f, r * 0.18f, 10, GetColor(255, 255, 255), TRUE);
    }

    // Góc trang trí hình chữ L
    void DrawCorner(int x, int y, int dx, int dy, unsigned int col)
    {
        const int L = 18;
        DrawLine(x, y, x + dx * L, y, col, 3);
        DrawLine(x, y, x, y + dy * L, col, 3);
    }
}

const CardFrame::Style& CardFrame::GetStyle(CardRarity rarity)
{
    EnsureColors();
    return s_styles[Index(rarity)];
}

void CardFrame::DrawBack(int x, int y, int w, int h, CardRarity rarity, bool selected, float time)
{
    const Style& s = GetStyle(rarity);

    // 1. Glow ngoài (additive), nhịp thở; thẻ đang chọn sáng hơn
    float pulse = 0.75f + 0.25f * std::sin(time * 3.0f);
    float g = s.glowStrength * pulse + (selected ? 0.45f : 0.0f);
    if (g > 0.0f)
    {
        for (int i = 6; i >= 1; --i)
        {
            int a = static_cast<int>(std::clamp(g * 26.0f * (7 - i) / 6.0f, 0.0f, 255.0f));
            SetDrawBlendMode(DX_BLENDMODE_ADD, a);
            DrawRoundRect(x - i * 3, y - i * 3, x + w + i * 3, y + h + i * 3, 14 + i * 3, 14 + i * 3, GetColor(s.glow[0], s.glow[1], s.glow[2]), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 2. Nền gradient dọc
    const int steps = 24;
    for (int i = 0; i < steps; ++i)
    {
        float t = static_cast<float>(i) / (steps - 1);
        int r = static_cast<int>(s.bgTop[0] + (s.bgBottom[0] - s.bgTop[0]) * t);
        int gg = static_cast<int>(s.bgTop[1] + (s.bgBottom[1] - s.bgTop[1]) * t);
        int b = static_cast<int>(s.bgTop[2] + (s.bgBottom[2] - s.bgTop[2]) * t);
        if (selected) { r += 10; gg += 10; b += 14; }
        int y0 = y + h * i / steps, y1 = y + h * (i + 1) / steps;
        DrawBox(x, y0, x + w, y1, GetColor((std::min)(r, 255), (std::min)(gg, 255), (std::min)(b, 255)), TRUE);
    }

    // 3. Viền đôi + góc trang trí
    DrawBox(x, y, x + w, y + h, s.edge, FALSE);
    DrawBox(x + 1, y + 1, x + w - 1, y + h - 1, s.edge, FALSE);
    DrawBox(x + 5, y + 5, x + w - 5, y + h - 5, s.edgeDark, FALSE);
    if (selected) DrawBox(x - 3, y - 3, x + w + 3, y + h + 3, GetColor(255, 240, 120), FALSE);
    DrawCorner(x + 3, y + 3, 1, 1, s.edge);
    DrawCorner(x + w - 4, y + 3, -1, 1, s.edge);
    DrawCorner(x + 3, y + h - 4, 1, -1, s.edge);
    DrawCorner(x + w - 4, y + h - 4, -1, -1, s.edge);

    // 4. Băng độ hiếm ở đáy thẻ + ngọc
    int bandH = 22, by = y + h - bandH - 6;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(x + 6, by, x + w - 6, by + bandH, s.edgeDark, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawLine(x + 6, by, x + w - 6, by, s.edge);
    FontManager::Draw(x + 14, by + 4, s.label, s.edge, FontSize::Small);
    for (int i = 0; i < s.gems; ++i)
    {
        float gx = static_cast<float>(x + w - 22 - i * 20), gy = static_cast<float>(by + bandH / 2);
        DrawGem(gx, gy, 8.0f, s.edge, GetColor(255, 255, 255));
    }
}

void CardFrame::DrawFront(int x, int y, int w, int h, CardRarity rarity, bool selected, float time)
{
    const Style& s = GetStyle(rarity);
    int idx = Index(rarity);

    // Vệt sáng quét chéo (Epic luôn có, Rare khi được chọn)
    if (idx == 2 || (idx == 1 && selected))
    {
        float period = (idx == 2) ? 2.6f : 3.4f;
        float p = std::fmod(time, period) / period;      // 0..1
        float sx = x - 120.0f + (w + 240.0f) * p;
        SetDrawArea(x, y, x + w, y + h);
        SetDrawBlendMode(DX_BLENDMODE_ADD, idx == 2 ? 70 : 45);
        unsigned int col = GetColor(s.glow[0], s.glow[1], s.glow[2]);
        float fy0 = static_cast<float>(y), fy1 = static_cast<float>(y + h);
        DrawQuadrangleAA(sx, fy0, sx + 50.0f, fy0, sx - 70.0f, fy1, sx - 120.0f, fy1, col, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        SetDrawAreaFull();
    }

    // Ảnh khung PNG (tuỳ chọn) phủ lên trên
    int img = FrameImage(idx);
    if (img >= 0)
    {
        SetDrawMode(DX_DRAWMODE_BILINEAR);
        DrawExtendGraph(x - 8, y - 8, x + w + 8, y + h + 8, img, TRUE);
        SetDrawMode(DX_DRAWMODE_NEAREST);
    }
}

void CardFrame::Release()
{
    for (int& h : s_frameImages)
    {
        if (h >= 0) DeleteGraph(h);
        h = -2;
    }
}
