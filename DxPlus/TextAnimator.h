// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <string>
#include "DxLib.h"

namespace DxPlus
{
    enum class TextAnimatorAlign
    {
        Left,
        Center,
        Right
    };

    class TextAnimator
    {
    public:
        TextAnimator();

        void Initialize(const wchar_t* fontName, int fontSize, int thickness);
        void SetFont(int fontID);
        void SetText(const std::wstring& text);
        void SetPosition(int x, int y);
        void SetAreaSize(int width, int height);
        void SetSpeed(int millisPerChar);
        void SetColor(int color);
        void SetLineSpacing(int spacing);
        void SetAlignment(TextAnimatorAlign align);
        void SetShowFrame(bool enabled);
        void Reset();

        void Update(int deltaMillis);
        void Draw();

        bool IsFinished() const;

    private:
        std::wstring text;
        int x = 0, y = 0;
        int width = 400, height = 200;
        int millisPerChar = 50;
        int color = GetColor(255, 255, 255);
        int lineSpacing = 4;
        int visibleCharCount = 0;
        int elapsedMillis = 0;
        int fontHandle = -1;
        TextAnimatorAlign alignment = TextAnimatorAlign::Left;
        bool showFrame = false;
    };
}
