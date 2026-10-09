// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Utils.h"
#include <cmath>
#include <Windows.h>

namespace DxPlus::Utils
{
    // 個々のRGB成分を取得
    int GetColorR(unsigned int color) {
        return (color >> 16) & 0xFF;
    }

    int GetColorG(unsigned int color) {
        return (color >> 8) & 0xFF;
    }

    int GetColorB(unsigned int color) {
        return color & 0xFF;
    }

    // HSVからRGBへの変換関数
    unsigned int HSVtoRGB(float h, float s, float v) {
        float r = 0.0f, g = 0.0f, b = 0.0f;

        int i = static_cast<int>(h * 6);
        float f = h * 6 - i;
        float p = v * (1 - s);
        float q = v * (1 - f * s);
        float t = v * (1 - (1 - f) * s);

        switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
        }

        return (static_cast<int>(r * 255) << 16) | (static_cast<int>(g * 255) << 8) | static_cast<int>(b * 255);
    }

    // 色相を変化させる関数
    unsigned int GetChangingColor(float time) {
        // 色相 (Hue) は0.0〜1.0の範囲で変化
        float hue = std::fmod(time * 0.1f, 1.0f);  // 時間に応じて色相を変化
        float saturation = 1.0f;  // 鮮やかな色
        float value = 1.0f;  // 最大の明るさ

        return HSVtoRGB(hue, saturation, value);
    }

    void FatalError(const TCHAR* message)
    {
        MessageBox(NULL, message, _T("DxLib"), MB_OK);
        exit(-1);
    }
}
