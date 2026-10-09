// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "DxWrapper.h"
#include <string>
#include <mmsystem.h>
#include "DxLib.h"
#include "Utils.h"
#pragma comment(lib, "winmm.lib")

namespace DxPlus
{
    DxWrapper::DxWrapper()
        :inputManager(std::make_unique<InputManager>())
    {
    }

    int DxWrapper::Initialize(int clientWidth, int clientHeight, bool bWindowed)
    {
        DxLib::SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);       // 使用する文字コードを utf8 に設定
        DxLib::SetGraphMode(clientWidth, clientHeight, 32);
        DxLib::ChangeWindowMode(bWindowed);
        DxLib::SetUseDXArchiveFlag(FALSE);
        DxLib::SetUse3DFlag(FALSE);
        DxLib::SetAlwaysRunFlag(TRUE);
        DxLib::SetWaitVSyncFlag(TRUE);

        // DirectX 11を使用する設定に変更
        DxLib::SetUseDirect3DVersion(DX_DIRECT3D_11);
        DxLib::SetUseTSFFlag(TRUE);

        DxLib::SetBackgroundColor(0, 0, 0);

        if (DxLib::DxLib_Init() == -1)
        {
            DxPlus::Utils::FatalError(L"DxLib initialization failed!");
        }

        DxLib::SetWindowText(L"DxPlus");
        DxLib::SetDrawScreen(DX_SCREEN_BACK);

        timer.SetFrameRate(60.0);
        timeBeginPeriod(1);

        return 0;
    }

    bool DxWrapper::GameLoop(bool showFps)
    {
        // DxLibのメッセージ処理（WM_QUIT相当）
        if (DxLib::ProcessMessage() != 0) {
            return false;
        }

        // 固定フレームレートを守る（待ちループ）
        while (!timer.UpdateFrame()) {
            if (DxLib::ProcessMessage() != 0) {
                return false;
            }
            // 残り時間が短ければCPU回しっぱなし、長ければSleep(1)
            double remaining = timer.GetTimeUntilNextFrame(); // ※追加実装が必要
            if (remaining > 0.002) {
                Sleep(1);
            }
        }

        if (showFps)
        {
            DrawFrameStats(); // フレームレートをタイトルバーに表示
        }

        return true; // フレーム実行可能
    }

    void DxWrapper::Finalize()
    {
        timeEndPeriod(1);
        DxLib::DxLib_End();
    }

    void DxWrapper::DrawFrameStats() {
        double fps = 1.0 / timer.GetDeltaTime();
        std::wstring title = L"DxPlus - FPS: " + std::to_wstring(static_cast<float>(fps));
        DxLib::SetWindowText(title.c_str());
    }
}
