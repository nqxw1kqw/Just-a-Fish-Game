#include "FontManager.h"
#include <cstdarg>
#include <cstdio>
#include <cwchar>

int FontManager::fontHandles_[5] = { -1, -1, -1, -1, -1 };
bool FontManager::initialized_ = false;
bool FontManager::fontLoaded_ = false;

void FontManager::Init()
{
    if (initialized_) return;

    // Tải font Yabikoma từ thư mục Data/Font/
    int addRes = AddFontResourceExW(L"Data/Font/Yabikoma-e9W16.otf", FR_PRIVATE, NULL);
    fontLoaded_ = (addRes > 0);

    const wchar_t* fontName = fontLoaded_ ? L"Yabikoma" : L"Consolas";

    // Khởi tạo các cấp độ kích thước font
    fontHandles_[static_cast<int>(FontSize::Title)]  = CreateFontToHandle(fontName, 32, 3, DX_FONTTYPE_ANTIALIASING_8X8);
    fontHandles_[static_cast<int>(FontSize::Header)] = CreateFontToHandle(fontName, 24, 2, DX_FONTTYPE_ANTIALIASING_8X8);
    fontHandles_[static_cast<int>(FontSize::Medium)] = CreateFontToHandle(fontName, 20, 2, DX_FONTTYPE_ANTIALIASING_8X8);
    fontHandles_[static_cast<int>(FontSize::Body)]   = CreateFontToHandle(fontName, 16, 1, DX_FONTTYPE_ANTIALIASING_8X8);
    fontHandles_[static_cast<int>(FontSize::Small)]  = CreateFontToHandle(fontName, 13, 1, DX_FONTTYPE_ANTIALIASING_8X8);

    // Cấu hình font mặc định cho toàn bộ hàm DxLib DrawString
    ChangeFont(fontName);
    SetFontSize(16);
    ChangeFontType(DX_FONTTYPE_ANTIALIASING_8X8);

    initialized_ = true;
}

void FontManager::Shutdown()
{
    if (!initialized_) return;

    for (int i = 0; i < 5; ++i)
    {
        if (fontHandles_[i] != -1)
        {
            DeleteFontToHandle(fontHandles_[i]);
            fontHandles_[i] = -1;
        }
    }

    if (fontLoaded_)
    {
        RemoveFontResourceExW(L"Data/Font/Yabikoma-e9W16.otf", FR_PRIVATE, NULL);
        fontLoaded_ = false;
    }

    initialized_ = false;
}

int FontManager::GetHandle(FontSize size)
{
    int idx = static_cast<int>(size);
    if (idx >= 0 && idx < 5 && fontHandles_[idx] != -1)
    {
        return fontHandles_[idx];
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
