#include "FontManager.h"
#include "Localization.h"
#include "Consts.h"
#include <cstdarg>
#include <cstdio>
#include <cwchar>

int FontManager::fontHandles_[5] = { -1, -1, -1, -1, -1 };
int FontManager::jpHandles_[5] = { -1, -1, -1, -1, -1 };
bool FontManager::initialized_ = false;
bool FontManager::fontLoaded_ = false;
bool FontManager::jpFontLoaded_ = false;

void FontManager::CreateSet(int* handles, const wchar_t* fontName)
{
    handles[static_cast<int>(FontSize::Title)]  = CreateFontToHandle(fontName, 32, 3, DX_FONTTYPE_ANTIALIASING_8X8);
    handles[static_cast<int>(FontSize::Header)] = CreateFontToHandle(fontName, 24, 2, DX_FONTTYPE_ANTIALIASING_8X8);
    handles[static_cast<int>(FontSize::Medium)] = CreateFontToHandle(fontName, 20, 2, DX_FONTTYPE_ANTIALIASING_8X8);
    handles[static_cast<int>(FontSize::Body)]   = CreateFontToHandle(fontName, 16, 1, DX_FONTTYPE_ANTIALIASING_8X8);
    handles[static_cast<int>(FontSize::Small)]  = CreateFontToHandle(fontName, 13, 1, DX_FONTTYPE_ANTIALIASING_8X8);
}

void FontManager::DeleteSet(int* handles)
{
    for (int i = 0; i < 5; ++i)
    {
        if (handles[i] != -1)
        {
            DeleteFontToHandle(handles[i]);
            handles[i] = -1;
        }
    }
}

void FontManager::Init()
{
    if (initialized_) return;

    // Font Latin: Yabikoma (Data/Font/)
    int addRes = AddFontResourceExW(GameConsts::FONT_LATIN_PATH, FR_PRIVATE, NULL);
    fontLoaded_ = (addRes > 0);
    CreateSet(fontHandles_, fontLoaded_ ? GameConsts::FONT_LATIN_NAME : L"Consolas");

    // Font Nhật: Rounded M+ 1c Bold (SIL OFL 1.1, Data/Font/OFL.txt). Không có file -> Meiryo của Windows
    int addJp = AddFontResourceExW(GameConsts::FONT_JP_PATH, FR_PRIVATE, NULL);
    jpFontLoaded_ = (addJp > 0);
    CreateSet(jpHandles_, jpFontLoaded_ ? GameConsts::FONT_JP_NAME : GameConsts::FONT_JP_FALLBACK_NAME);

    initialized_ = true;
    ApplyLanguage();
}

void FontManager::ApplyLanguage()
{
    // Cấu hình font mặc định cho các hàm DxLib DrawString không dùng handle
    const bool jp = (L10N::Get() == Language::Japanese);
    const wchar_t* name = jp ? (jpFontLoaded_ ? GameConsts::FONT_JP_NAME : GameConsts::FONT_JP_FALLBACK_NAME)
                             : (fontLoaded_ ? GameConsts::FONT_LATIN_NAME : L"Consolas");
    ChangeFont(name);
    SetFontSize(16);
    ChangeFontType(DX_FONTTYPE_ANTIALIASING_8X8);
}

void FontManager::Shutdown()
{
    if (!initialized_) return;

    DeleteSet(fontHandles_);
    DeleteSet(jpHandles_);

    if (fontLoaded_)
    {
        RemoveFontResourceExW(GameConsts::FONT_LATIN_PATH, FR_PRIVATE, NULL);
        fontLoaded_ = false;
    }
    if (jpFontLoaded_)
    {
        RemoveFontResourceExW(GameConsts::FONT_JP_PATH, FR_PRIVATE, NULL);
        jpFontLoaded_ = false;
    }

    initialized_ = false;
}

int FontManager::GetHandle(FontSize size)
{
    if (L10N::Get() == Language::Japanese) return GetHandleJP(size);
    int idx = static_cast<int>(size);
    if (idx >= 0 && idx < 5 && fontHandles_[idx] != -1)
    {
        return fontHandles_[idx];
    }
    return -1;
}

int FontManager::GetHandleJP(FontSize size)
{
    int idx = static_cast<int>(size);
    if (idx >= 0 && idx < 5 && jpHandles_[idx] != -1)
    {
        return jpHandles_[idx];
    }
    return -1;
}

void FontManager::Draw(int x, int y, const wchar_t* text, unsigned int color, FontSize size)
{
    int handle = GetHandle(size);
    if (handle != -1)
    {
        DrawStringToHandle(x, y, text, color, handle);
    }
    else
    {
        DrawString(x, y, text, color);
    }
}

void FontManager::DrawJP(int x, int y, const wchar_t* text, unsigned int color, FontSize size)
{
    int handle = GetHandleJP(size);
    if (handle != -1)
    {
        DrawStringToHandle(x, y, text, color, handle);
    }
    else
    {
        DrawString(x, y, text, color);
    }
}

int FontManager::GetWidth(const wchar_t* text, FontSize size)
{
    int handle = GetHandle(size);
    int len = static_cast<int>(wcslen(text));
    return (handle != -1) ? GetDrawStringWidthToHandle(text, len, handle) : GetDrawStringWidth(text, len);
}

void FontManager::DrawFormat(int x, int y, unsigned int color, FontSize size, const wchar_t* fmt, ...)
{
    wchar_t buffer[1024];
    va_list args;
    va_start(args, fmt);
    vswprintf_s(buffer, fmt, args);
    va_end(args);

    int handle = GetHandle(size);
    if (handle != -1)
    {
        DrawStringToHandle(x, y, buffer, color, handle);
    }
    else
    {
        DrawString(x, y, buffer, color);
    }
}
