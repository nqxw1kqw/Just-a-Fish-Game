// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "TextAnimator.h"
#include "Utils.h"
#include "Primitive2D.h"

namespace DxPlus
{
    TextAnimator::TextAnimator()
        : fontHandle(-1)
    {
    }

    void TextAnimator::Initialize(const wchar_t* fontName, int fontSize, int thickness)
    {
        fontHandle = CreateFontToHandle(fontName, fontSize, thickness, DX_FONTTYPE_NORMAL, -1, TRUE);
        if (fontHandle == -1)
        {
            DxPlus::Utils::FatalError(L"フォントの作成に失敗しました");
        }
    }

    void TextAnimator::SetFont(int fontID)
    {
        fontHandle = fontID;
    }

    void TextAnimator::SetText(const std::wstring& t) { text = t; Reset(); }
    void TextAnimator::SetPosition(int x_, int y_) { x = x_; y = y_; }
    void TextAnimator::SetAreaSize(int w, int h) { width = w; height = h; }
    void TextAnimator::SetSpeed(int ms) { millisPerChar = ms; }
    void TextAnimator::SetColor(int c) { color = c; }
    void TextAnimator::SetLineSpacing(int s) { lineSpacing = s; }
    void TextAnimator::SetAlignment(TextAnimatorAlign align) { alignment = align; }
    void TextAnimator::SetShowFrame(bool enabled) { showFrame = enabled; }
    void TextAnimator::Reset() { visibleCharCount = 0; elapsedMillis = 0; }

    void TextAnimator::Update(int deltaMillis)
    {
        elapsedMillis += deltaMillis;
        visibleCharCount = elapsedMillis / millisPerChar;
        if (visibleCharCount > static_cast<int>(text.size()))
            visibleCharCount = static_cast<int>(text.size());
    }

    void TextAnimator::Draw()
    {
        if (fontHandle == -1) return;

        if (showFrame)
        {
            Primitive2D::DrawRect(
                { static_cast<float>(x), static_cast<float>(y) },
                { static_cast<float>(width), static_cast<float>(height) },
                GetColor(200, 200, 200),
                false
            );
        }

        int textHeight = GetFontSizeToHandle(fontHandle);
        int lineHeight = textHeight + lineSpacing;

        int currentX = x;
        int currentY = y;
        int count = 0;
        std::wstring line;

        for (int i = 0; i < static_cast<int>(text.size()) && count < visibleCharCount; ++i)
        {
            wchar_t ch = text[i];
            if (ch == L'\n' || currentX >= x + width)
            {
                if (currentY + lineHeight > y + height) break;

                int lineWidth = GetDrawStringWidthToHandle(line.c_str(), -1, fontHandle);
                int drawX = x;
                if (alignment == TextAnimatorAlign::Center)
                    drawX = x + (width - lineWidth) / 2;
                else if (alignment == TextAnimatorAlign::Right)
                    drawX = x + (width - lineWidth);

                DrawStringToHandle(drawX, currentY, line.c_str(), color, fontHandle);

                currentY += lineHeight;
                currentX = x;
                line.clear();

                if (ch == L'\n') continue;
            }

            wchar_t chStr[2] = { ch, L'\0' };
            int charWidth = GetDrawStringWidthToHandle(chStr, -1, fontHandle);
            if (currentX + charWidth > x + width)
            {
                if (currentY + lineHeight > y + height) break;

                int lineWidth = GetDrawStringWidthToHandle(line.c_str(), -1, fontHandle);
                int drawX = x;
                if (alignment == TextAnimatorAlign::Center)
                    drawX = x + (width - lineWidth) / 2;
                else if (alignment == TextAnimatorAlign::Right)
                    drawX = x + (width - lineWidth);

                DrawStringToHandle(drawX, currentY, line.c_str(), color, fontHandle);

                currentY += lineHeight;
                currentX = x;
                line.clear();
            }

            currentX += charWidth;
            line += ch;
            count++;
        }

        if (!line.empty() && currentY + lineHeight <= y + height)
        {
            int lineWidth = GetDrawStringWidthToHandle(line.c_str(), -1, fontHandle);
            int drawX = x;
            if (alignment == TextAnimatorAlign::Center)
                drawX = x + (width - lineWidth) / 2;
            else if (alignment == TextAnimatorAlign::Right)
                drawX = x + (width - lineWidth);

            DrawStringToHandle(drawX, currentY, line.c_str(), color, fontHandle);
        }
    }

    bool TextAnimator::IsFinished() const
    {
        return visibleCharCount >= static_cast<int>(text.size());
    }
}
