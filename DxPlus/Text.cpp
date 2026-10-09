// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Text.h"
#include "Utils.h"

namespace DxPlus::Text
{
    void InitializeDefaultFont(const wchar_t* fontName, int fontSize, int fontThickness)
    {
        if (CheckFontHandleValid(defaultFontHandle))
        {
            DeleteFontToHandle(defaultFontHandle);
        }

        defaultFontHandle = CreateFontToHandle(fontName, fontSize, fontThickness, DX_FONTTYPE_NORMAL, -1, TRUE);

        if (defaultFontHandle == -1)
        {
            DxPlus::Utils::FatalError(L"Failed to create font handle");
        }
    }

    void FinalizeFont()
    {
        if (defaultFontHandle != -1)
        {
            DeleteFontToHandle(defaultFontHandle);
            defaultFontHandle = -1;
        }
    }

    int InitializeFont(const wchar_t* fontName, int fontSize, int fontThickness)
    {
        return DxLib::CreateFontToHandle(fontName, fontSize, fontThickness, DX_FONTTYPE_NORMAL, -1, TRUE);
    }

    void DeleteFont(int fontHandle)
    {
        if (fontHandle != -1)
        {
            DxLib::DeleteFontToHandle(fontHandle);
        }
    }

    void DrawString(const wchar_t* text, Vec2 position, int color, TextAlign align, Vec2 scale, double angle, int fontHandle)
    {
        int roundedX = static_cast<int>(std::floor(position.x));
        int roundedY = static_cast<int>(std::floor(position.y));
        DrawString(text, roundedX, roundedY, color, align, scale.x, scale.y, angle, fontHandle);
    }

    void DrawString(const wchar_t* text, int x, int y, int color, TextAlign align, double scaleX, double scaleY, double angle, int fontHandle)
    {
        if (fontHandle == -1)
        {
            if (defaultFontHandle == -1)
            {
                InitializeDefaultFont(L"MS ゴシック", 16, 1);
            }
            fontHandle = defaultFontHandle;
        }

        int textWidth = GetDrawStringWidthToHandle(text, -1, fontHandle);
        int textHeight = GetFontSizeToHandle(fontHandle);

        int offsetX = 0, offsetY = 0;

        switch (align)
        {
        case TOP_LEFT: offsetX = 0; offsetY = 0; break;
        case TOP_CENTER: offsetX = textWidth / 2; offsetY = 0; break;
        case TOP_RIGHT: offsetX = textWidth; offsetY = 0; break;
        case MIDDLE_LEFT: offsetX = 0; offsetY = textHeight / 2; break;
        case MIDDLE_CENTER: offsetX = textWidth / 2; offsetY = textHeight / 2; break;
        case MIDDLE_RIGHT: offsetX = textWidth; offsetY = textHeight / 2; break;
        case BOTTOM_LEFT: offsetX = 0; offsetY = textHeight; break;
        case BOTTOM_CENTER: offsetX = textWidth / 2; offsetY = textHeight; break;
        case BOTTOM_RIGHT: offsetX = textWidth; offsetY = textHeight; break;
        }

        SetDrawMode(DX_DRAWMODE_BILINEAR);
        DrawRotaStringToHandle(
            x, y,
            scaleX, scaleY,
            offsetX, offsetY,
            angle,
            color,
            fontHandle,
            static_cast<unsigned int>(-1),
            FALSE,
            text
        );
    }

    std::wstring Format(const wchar_t* format, ...)
    {
        constexpr size_t BUFFER_SIZE = 1024;
        wchar_t buffer[BUFFER_SIZE];

        va_list args;
        va_start(args, format);
        vswprintf(buffer, BUFFER_SIZE, format, args);
        va_end(args);

        return buffer;
    }

    void DrawTextWithAutoWrap(
        const wchar_t* text,
        const DxPlus::Vec2& position, 
        const DxPlus::Vec2& size, 
        int framePerChar,
        int color,
        int lineSpacing
    )
    {
        if (defaultFontHandle == -1)
        {
            InitializeDefaultFont(L"MS ゴシック", 16, 1);
        }

        int textHeight = GetFontSizeToHandle(defaultFontHandle);
        int lineHeight = textHeight + lineSpacing;

        int currentX = static_cast<int>(position.x);
        int currentY = static_cast<int>(position.y);
        static int totalFrame = 0;
        static int visibleCharCount = 0;

        totalFrame++;
        if (totalFrame / framePerChar > visibleCharCount)
        {
            visibleCharCount++;
        }

        int length = static_cast<int>(wcslen(text));
        int count = 0;

        for (int i = 0; i < length && count < visibleCharCount; ++i)
        {
            if (text[i] == L'\n')
            {
                currentX = static_cast<int>(position.x);
                currentY += lineHeight;
                continue;
            }

            wchar_t ch[2] = { text[i], L'\0' };
            int charWidth = GetDrawStringWidthToHandle(ch, -1, defaultFontHandle);

            if (currentX + charWidth > position.x + size.x)
            {
                currentX = static_cast<int>(position.x);
                currentY += lineHeight;

                if (currentY + textHeight > position.y + size.y)
                {
                    break; // 枠を超えるので終了
                }
            }

            DrawString(ch, currentX, currentY, color);
            currentX += charWidth;
            count++;
        }
    }

    // UTF-8 文字列を UTF-16 文字列に変換
    std::wstring Utf8ToUtf16(const std::string& utf8) {
        int wstr_len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
        if (wstr_len == 0) {
            return std::wstring();
        }

        std::wstring wide_str(wstr_len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &wide_str[0], wstr_len);
        return wide_str;
    }

    void DxPlus::Text::DrawOutlinedString(
        const wchar_t* text,
        Vec2 position,
        int fontID,
        int fillColor,
        int outlineColor,
        int thickness,
        TextAlign align,
        double angle)
    {
        if (!text || thickness < 0) return;

        const int textWidth = GetDrawStringWidthToHandle(text, (int)wcslen(text), fontID);
        const int textHeight = GetFontSizeToHandle(fontID);

        double rotCenterX = 0.0;
        double rotCenterY = 0.0;

        // 揃えに応じた回転中心補正
        switch (align)
        {
        case TOP_CENTER:
        case MIDDLE_CENTER:
        case BOTTOM_CENTER:
            rotCenterX = textWidth / 2.0;
            break;
        case TOP_RIGHT:
        case MIDDLE_RIGHT:
        case BOTTOM_RIGHT:
            rotCenterX = static_cast<double>(textWidth);
            break;
        default: break; // LEFT系 → 0.0
        }

        switch (align)
        {
        case MIDDLE_LEFT:
        case MIDDLE_CENTER:
        case MIDDLE_RIGHT:
            rotCenterY = textHeight / 2.0;
            break;
        case BOTTOM_LEFT:
        case BOTTOM_CENTER:
        case BOTTOM_RIGHT:
            rotCenterY = static_cast<double>(textHeight);
            break;
        default: break; // TOP系 → 0.0
        }

        // 縁取り描画（8方向）
        for (int dx = -thickness; dx <= thickness; ++dx)
        {
            for (int dy = -thickness; dy <= thickness; ++dy)
            {
                if (dx == 0 && dy == 0) continue;

                ::DrawRotaStringFToHandle(
                    position.x + dx,
                    position.y + dy,
                    1.0, 1.0,
                    rotCenterX, rotCenterY,
                    angle,
                    outlineColor,
                    fontID,
                    0,
                    FALSE,
                    text);
            }
        }

        // 本体描画
        ::DrawRotaStringFToHandle(
            position.x,
            position.y,
            1.0, 1.0,
            rotCenterX, rotCenterY,
            angle,
            fillColor,
            fontID,
            0,
            FALSE,
            text);
    }
}
