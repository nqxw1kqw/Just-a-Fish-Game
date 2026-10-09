// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Debug.h"
#include "DxLib.h"
#include "Text.h"

namespace DxPlus
{
    std::vector<std::wstring> Debug::debugMessages;
    int Debug::startX = 0;
    int Debug::startY = 0;
    int Debug::fontHandle;
    int Debug::defaultColor;
    int Debug::lineSpacing;

    void Debug::Initialize(int initSpacing, int initX, int initY, int initColor)
    {
        startX = initX;
        startY = initY;
        defaultColor = initColor;
        lineSpacing = initSpacing;
    }

    void Debug::Finalize()
    {
        DeleteFontToHandle(fontHandle);
    }

    void Debug::SetString(const std::wstring& text)
    {
        debugMessages.emplace_back(text);
    }

    void Debug::SetFormatString(const wchar_t* format, ...)
    {
        constexpr size_t BUFFER_SIZE = 1024;
        wchar_t buffer[BUFFER_SIZE];

        va_list args;
        va_start(args, format);
        vswprintf(buffer, BUFFER_SIZE, format, args);
        va_end(args);

        debugMessages.emplace_back(buffer);
    }

    void Debug::Draw()
    {
        int y = startY;

        for (const auto& message : debugMessages)
        {
            DxPlus::Text::DrawString(message.c_str(), startX, y, defaultColor);
            y += lineSpacing;
        }

        Clear();
    }

    void Debug::Clear()
    {
        debugMessages.clear();
    }
}
