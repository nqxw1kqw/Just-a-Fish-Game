#pragma once
#include "Card.h"

// =======================================================================
// CardFrame.h
// カード枠（レア度ごとの色）: khung thẻ theo độ hiếm.
//   Common = bạc / xanh lam nhạt, 1 ngọc
//   Rare   = tím, 2 ngọc, viền phát sáng nhịp thở
//   Epic   = vàng kim (ウルトラレア), 3 ngọc, phát sáng mạnh + vệt sáng quét chéo
// Vẽ bằng code (DxLib). Nếu có ảnh Data/UI/Cards/frame_{common,rare,epic}.png
// thì ảnh được phủ lên trên; thiếu file vẫn dùng khung vẽ bằng code.
// =======================================================================
namespace CardFrame
{
    struct Style
    {
        unsigned int edge;      // viền chính
        unsigned int edgeDark;  // viền trong
        int bgTop[3];           // gradient nền
        int bgBottom[3];
        int glow[3];            // màu glow (additive)
        float glowStrength;     // 0 = không glow
        int gems;               // số ngọc
        const wchar_t* label;
    };

    const Style& GetStyle(CardRarity rarity);

    // Nền + viền + glow + băng độ hiếm + ngọc. Gọi trước khi vẽ nội dung thẻ.
    void DrawBack(int x, int y, int w, int h, CardRarity rarity, bool selected, float time);
    // Hiệu ứng phủ trên nội dung (vệt sáng Epic, ảnh khung PNG nếu có). Gọi sau nội dung.
    void DrawFront(int x, int y, int w, int h, CardRarity rarity, bool selected, float time);

    // Giải phóng ảnh khung đã nạp (gọi khi tắt game, tuỳ chọn)
    void Release();
}
