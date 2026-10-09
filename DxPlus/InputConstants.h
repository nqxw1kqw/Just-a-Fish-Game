// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once

namespace DxPlus::Input
{
    // プレイヤーの数を定義
    constexpr int MaxPlayers = 4;

    // プレイヤー定数
    constexpr int PLAYER1 = 0;
    constexpr int PLAYER2 = 1;
    constexpr int PLAYER3 = 2;
    constexpr int PLAYER4 = 3;

    // ボタンのアクションラベル
    constexpr int BUTTON_UP = 1 << 0;
    constexpr int BUTTON_DOWN = 1 << 1;
    constexpr int BUTTON_LEFT = 1 << 2;
    constexpr int BUTTON_RIGHT = 1 << 3;
    constexpr int BUTTON_START = 1 << 4;
    constexpr int BUTTON_TRIGGER1 = 1 << 5;
    constexpr int BUTTON_TRIGGER2 = 1 << 6;
    constexpr int BUTTON_TRIGGER3 = 1 << 7;
    constexpr int BUTTON_SELECT = 1 << 8;
    constexpr int BUTTON_TRIGGER4 = 1 << 9;
    constexpr int BUTTON_L1 = 1 << 10;
    constexpr int BUTTON_R1 = 1 << 11;
//  constexpr int BUTTON_L2 = 1 << 12;//※未使用（アナログスティックを使用するため）
//  constexpr int BUTTON_R2 = 1 << 13;//※未使用（アナログスティックを使用するため）
    constexpr int BUTTON_L3 = 1 << 14;
    constexpr int BUTTON_R3 = 1 << 15;

    constexpr float STICK_DEAD_ZONE = 0.1f;//スティックの遊び
}
