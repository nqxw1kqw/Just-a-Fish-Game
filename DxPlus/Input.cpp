// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Input.h"
#include "DxLib.h"
#include "DxWrapper.h"
#include "InputManager.h"

namespace DxPlus::Input
{
    void Initialize()
    {
        DxWrapper::GetInstance().GetInputManager().Initialize();
    }

    void Update()
    {
        DxWrapper::GetInstance().GetInputManager().Update();
    }

    int GetButton(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetButton(playerIndex);
    }

    int GetButtonDown(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetButtonDown(playerIndex);
    }

    int GetButtonUp(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetButtonUp(playerIndex);
    }

    Vec2 GetLStick(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetLeftStick(playerIndex);
    }

    Vec2 GetRStick(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetRightStick(playerIndex);
    }

    float GetLTrigger(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetLeftTrigger(playerIndex);
    }

    float GetRTrigger(int playerIndex)
    {
        return DxWrapper::GetInstance().GetInputManager().GetRightTrigger(playerIndex);
    }

    Vec2Int GetMousePosition()
    {
        int mouseX, mouseY;
        DxLib::GetMousePoint(&mouseX, &mouseY);
        return Vector2<int>(mouseX, mouseY);
    }
}
