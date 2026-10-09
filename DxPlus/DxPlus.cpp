// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "DxPlus.h"

namespace DxPlus
{
    int Initialize(int clientWidth, int clientHeight, bool bWindowed)
    {
        int result = DxWrapper::GetInstance().Initialize(clientWidth, clientHeight, bWindowed);
        if (result < 0) { return result; }
        DxWrapper::GetInstance().GetInputManager().Initialize();

        const wchar_t* defaultFontName = L"Meiryo";
        int fontSize = 24;
        int fontThickness = 1;
        Text::InitializeDefaultFont(defaultFontName, fontSize, fontThickness);

        // デバッグ行間オフセット
        constexpr int DebugLineSpacingOffset = 4;
        Debug::Initialize(fontSize + DebugLineSpacingOffset);  //デバッグ情報の行間を設定
        return 0;
    }

    void Shutdown()
    {
        Text::FinalizeFont();
        DxWrapper::GetInstance().Finalize();
    }

    bool GameLoop(bool showFps)
    {
        return DxWrapper::GetInstance().GameLoop(showFps);
    }
}
