#pragma once
#include <windows.h>
#include <wingdi.h>
#include "DxLib.h"

enum class FontSize
{
    Title,   // 32px - Tiêu đề lớn (Title / Victory / GameOver)
    Header,  // 24px - Tiêu đề phụ (Banners / Wave / Boss)
    Medium,  // 20px - Đề mục (Card titles / Selectors)
    Body,    // 16px - Văn bản chính (HUD / Mô tả chi tiết)
    Small    // 13px - Chữ phụ (Debug HUD / Thông tin nhỏ)
};

class FontManager
{
public:
    static void Init();
    static void Shutdown();
    static int GetHandle(FontSize size);

    // Vẽ chuỗi ký tự bằng Font tùy chỉnh (Yabikoma)
    static void Draw(int x, int y, const wchar_t* text, unsigned int color, FontSize size = FontSize::Body);

    // Vẽ chuỗi định dạng (Format String) bằng Font tùy chỉnh (Yabikoma)
    static void DrawFormat(int x, int y, unsigned int color, FontSize size, const wchar_t* fmt, ...);

private:
    static int fontHandles_[5];
    static bool initialized_;
    static bool fontLoaded_;
};
