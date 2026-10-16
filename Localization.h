#pragma once
// Localization: bảng chuỗi EN/JP tập trung.
// Khoá = chuỗi tiếng Anh gốc (giữ nguyên trong code), TR(L"...") trả về bản dịch theo ngôn ngữ hiện tại.
// Chuỗi định dạng (%d, %s...) phải giữ đúng thứ tự specifier ở bản dịch.

enum class Language
{
    English = 0,
    Japanese = 1,
    Count
};

namespace L10N
{
    // Đọc lựa chọn đã lưu (Data/settings.ini cạnh exe). Gọi 1 lần lúc khởi động.
    void Init();
    Language Get();
    // Đổi ngôn ngữ (save = ghi lại vào settings.ini)
    void Set(Language lang, bool save = true);
    bool HasSavedChoice();
    // Trả về bản dịch của chuỗi gốc tiếng Anh; không có bản dịch -> trả lại chính nó
    const wchar_t* Tr(const wchar_t* en);
}

#define TR(s) L10N::Tr(s)
