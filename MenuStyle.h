#pragma once
#include "FontManager.h"
#include <algorithm>
#include <cwchar>

// Shared typography and controls for the illustrated menus, including bilingual text.
namespace MenuStyle
{
    class DrawState
    {
    public:
        DrawState() : drawMode_(GetDrawMode())
        {
            GetDrawBlendMode(&blendMode_, &alpha_);
            SetDrawMode(DX_DRAWMODE_BILINEAR);
        }
        ~DrawState()
        {
            SetDrawMode(drawMode_);
            SetDrawBlendMode(blendMode_, alpha_);
        }
        DrawState(const DrawState&) = delete;
        DrawState& operator=(const DrawState&) = delete;
    private:
        int drawMode_, blendMode_ = DX_BLENDMODE_NOBLEND, alpha_ = 255;
    };
    inline unsigned int Paper() { return GetColor(247, 238, 217); }
    inline unsigned int Muted() { return GetColor(155, 180, 192); }
    inline unsigned int Accent() { return GetColor(245, 205, 137); }
    inline unsigned int Background() { return GetColor(7, 19, 31); }
    inline int Font(FontSize size) { return FontManager::GetHandleJP(size); }
    inline int Width(const wchar_t* text, FontSize size)
    {
        return GetDrawStringWidthToHandle(text, static_cast<int>(std::wcslen(text)), Font(size));
    }
    inline void Text(int x, int y, const wchar_t* text, unsigned int color,
                     FontSize size, float scale = 1.0f, int maxWidth = 1400)
    {
        const int width = Width(text, size);
        if (width > 0) scale = (std::min)(scale, static_cast<float>(maxWidth) / width);
        DrawExtendStringToHandle(x, y, scale, scale, text, color, Font(size));
    }
    inline void Center(int cx, int y, const wchar_t* text, unsigned int color,
                       FontSize size, float scale = 1.0f, int maxWidth = 1400)
    {
        const int width = Width(text, size);
        if (width > 0) scale = (std::min)(scale, static_cast<float>(maxWidth) / width);
        Text(cx - static_cast<int>(width * scale) / 2, y, text, color, size, scale, maxWidth);
    }
    // Inherits the caller's blend state so result fades apply to the whole button.
    inline void Button(int x, int y, int width, const wchar_t* text, bool selected,
                       unsigned int accent)
    {
        DrawBox(x, y, x + width, y + 52, selected ? GetColor(40, 56, 64) : GetColor(12, 29, 42), TRUE);
        DrawBox(x, y, x + width, y + 52, selected ? accent : GetColor(47, 69, 85), FALSE);
        if (selected)
        {
            DrawBox(x, y, x + 4, y + 52, accent, TRUE);
            DrawCircle(x + 25, y + 26, 3, accent, TRUE);
        }
        Center(x + width / 2, y + 13, text, selected ? Paper() : Muted(), FontSize::Medium, 1.0f, width - 88);
    }
}
