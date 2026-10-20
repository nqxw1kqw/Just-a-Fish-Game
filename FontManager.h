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
    // 日本語フォントを既定の描画にも適用します。
    static void ApplyLanguage();
    static int GetHandle(FontSize size);
    // Handle font Nhật (luôn có, dùng để vẽ chữ JP khi ngôn ngữ đang là EN, ví dụ màn chọn ngôn ngữ)
    static int GetHandleJP(FontSize size);

    // Vẽ chuỗi ký tự bằng Font tùy chỉnh
    static void Draw(int x, int y, const wchar_t* text, unsigned int color, FontSize size = FontSize::Body);
    static void DrawJP(int x, int y, const wchar_t* text, unsigned int color, FontSize size = FontSize::Body);

    // Vẽ chuỗi định dạng (Format String) bằng Font tùy chỉnh
    static void DrawFormat(int x, int y, unsigned int color, FontSize size, const wchar_t* fmt, ...);

    // Độ rộng chuỗi (px) theo font hiện tại - dùng để căn giữa
    static int GetWidth(const wchar_t* text, FontSize size);

private:
    static void CreateSet(int* handles, const wchar_t* fontName);
    static void DeleteSet(int* handles);

    static int jpHandles_[5];     // bộ Nhật
    static bool initialized_;
    static bool jpFontLoaded_;
};
