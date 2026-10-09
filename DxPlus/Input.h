// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include "Vector2.h"

namespace DxPlus::Input
{
    void Initialize();
    void Update();
    int GetButton(int playerIndex);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="playerIndex"></param>
    /// <returns></returns>
    int GetButtonDown(int playerIndex);
    int GetButtonUp(int playerIndex);
    Vec2 GetLStick(int playerIndex);
    Vec2 GetRStick(int playerIndex);
    float GetLTrigger(int playerIndex);
    float GetRTrigger(int playerIndex);

    Vec2Int GetMousePosition();

    // ヘルパー関数
    inline bool IsButton(int playerIndex, int buttonFlag)
    {
        return (GetButton(playerIndex) & buttonFlag) != 0;
    }
    inline bool IsButtonDown(int playerIndex, int buttonFlag)
    {
        return (GetButtonDown(playerIndex) & buttonFlag) != 0;
    }
    inline bool IsButtonUp(int playerIndex, int buttonFlag)
    {
        return (GetButtonUp(playerIndex) & buttonFlag) != 0;
    }
}
