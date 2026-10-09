// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <string>
#include <vector>
#include "DxLib.h"

namespace DxPlus
{
    class Debug
    {
    public:
        Debug() = default;
        static void Initialize(int lineSpacing, int startX = 0, int startY = 0, int color = GetColor(255, 255, 255));
        static void Finalize();
        static void SetString(const std::wstring& text);
        static void SetFormatString(const wchar_t* format, ...);
        static void Draw();
    private:
        static void Clear();

        static std::vector<std::wstring> debugMessages;
        static int startX;
        static int startY;
        static int fontHandle;
        static int defaultColor;
        static int lineSpacing;
    };
}
