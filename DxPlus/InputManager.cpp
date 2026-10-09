// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "InputManager.h"
#include <limits>
#include "DxLib.h"

namespace DxPlus
{
    // SHORT 型の最大値と最小値
    constexpr int SHORT_MAX = (std::numeric_limits<short>::max)(); // 32767
    constexpr int SHORT_MIN = (std::numeric_limits<short>::min)(); // -32768

    void InputManager::Initialize()
    {
        using namespace DxPlus::Input;
        std::vector<ActionMapping> keyboardMappingsP1
        {
            { BUTTON_UP,        KEY_INPUT_W },          // WASDキーのW
            { BUTTON_DOWN,      KEY_INPUT_S },          // WASDキーのS
            { BUTTON_LEFT,      KEY_INPUT_A },          // WASDキーのA
            { BUTTON_RIGHT,     KEY_INPUT_D },          // WASDキーのD

            { BUTTON_START,     KEY_INPUT_RETURN },     // エンターキー (メニュー決定など)
            { BUTTON_SELECT,    KEY_INPUT_BACK },       // バックスペースキー (メニュー戻るなど)

            { BUTTON_TRIGGER1,  KEY_INPUT_SPACE },      // ジャンプ (UnityのJumpに対応)
            { BUTTON_TRIGGER2,  KEY_INPUT_LCONTROL },   // Fire1 (UnityのFire1に対応)
            { BUTTON_TRIGGER3,  KEY_INPUT_LALT },       // Fire2 (UnityのFire2に対応)
            { BUTTON_TRIGGER4,  KEY_INPUT_LSHIFT },     // Fire3 (UnityのFire3に対応)
        };

        std::vector<ActionMapping> mouseMappingsP1
        {
            { BUTTON_TRIGGER2,  MOUSE_INPUT_LEFT },     // Fire1 (UnityのFire1に対応)
            { BUTTON_TRIGGER3,  MOUSE_INPUT_RIGHT },    // Fire2 (UnityのFire2に対応)
        };

        std::vector<ActionMapping> controllerMappingsP1
        {
            { BUTTON_UP,        XINPUT_BUTTON_DPAD_UP },
            { BUTTON_DOWN,      XINPUT_BUTTON_DPAD_DOWN },
            { BUTTON_LEFT,      XINPUT_BUTTON_DPAD_LEFT },
            { BUTTON_RIGHT,     XINPUT_BUTTON_DPAD_RIGHT },

            { BUTTON_START,     XINPUT_BUTTON_START },
            { BUTTON_SELECT,    XINPUT_BUTTON_BACK },

            { BUTTON_TRIGGER1,  XINPUT_BUTTON_A },
            { BUTTON_TRIGGER2,  XINPUT_BUTTON_B },
            { BUTTON_TRIGGER3,  XINPUT_BUTTON_X },
            { BUTTON_TRIGGER4,  XINPUT_BUTTON_Y },

            { BUTTON_L1,        XINPUT_BUTTON_LEFT_SHOULDER },
            { BUTTON_R1,        XINPUT_BUTTON_RIGHT_SHOULDER },
            { BUTTON_L3,        XINPUT_BUTTON_LEFT_THUMB },
            { BUTTON_R3,        XINPUT_BUTTON_RIGHT_THUMB },
        };
        std::vector<ActionMapping> keyboardMappingP2
        {
            { BUTTON_UP,        KEY_INPUT_UP },         // 矢印キー上
            { BUTTON_DOWN,      KEY_INPUT_DOWN },       // 矢印キー下
            { BUTTON_LEFT,      KEY_INPUT_LEFT },       // 矢印キー左
            { BUTTON_RIGHT,     KEY_INPUT_RIGHT },      // 矢印キー右
        };

        // プレイヤー1
        _players[PLAYER1].SetKeyboardMappings(keyboardMappingsP1);
        _players[PLAYER1].SetMouseMappings(mouseMappingsP1);
        _players[PLAYER1].SetControllerMappings(controllerMappingsP1);

        // プレイヤー2
        _players[PLAYER2].SetKeyboardMappings(keyboardMappingP2);
        _players[PLAYER2].SetControllerMappings(controllerMappingsP1);
    }

    void InputManager::Update()
    {
        // 全プレイヤーの入力を更新
        for (int i = 0; i < DxPlus::Input::MaxPlayers; ++i)
        {
            auto& inputState = _players[i];
            int oldButton = inputState.button;
            inputState.button = 0;

            // キーボードの入力を処理
            for (auto& assign : inputState.keyboardAssign)
            {
                if (DxLib::CheckHitKey(assign.inputCode))
                {
                    inputState.button |= assign.action;
                }
            }

            // マウスの入力を処理
            int mouseState = DxLib::GetMouseInput();
            for (auto& assign : inputState.mouseAssign)
            {
                if (mouseState & assign.inputCode)
                {
                    inputState.button |= assign.action;
                }
            }

            // コントローラー
            XINPUT_STATE state{};
            if (GetJoypadXInputState(DX_INPUT_PAD1 + i, &state) == 0)
            {
                for (auto& assign : inputState.controllerAssign)
                {
                    if (state.Buttons[assign.inputCode])
                    {
                        inputState.button |= assign.action;
                    }
                }

                // トリガーのアナログ入力を取得
                inputState.leftTrigger = NormalizeTriggerInput(state.LeftTrigger);
                inputState.rightTrigger = NormalizeTriggerInput(state.RightTrigger);
            }

            // トリガー入力の設定
            inputState.buttonDown = ~oldButton & inputState.button;   // ボタンを押した瞬間
            inputState.buttonUp = oldButton & ~inputState.button;     // ボタンを離した瞬間

            // 左右スティックの値取得
            inputState.leftStick = {
                ApplyDeadZone(NormalizeAnalogInput(state.ThumbLX)),
                ApplyDeadZone(NormalizeAnalogInput(state.ThumbLY))
            };
            inputState.rightStick = {
                ApplyDeadZone(NormalizeAnalogInput(state.ThumbRX)),
                ApplyDeadZone(NormalizeAnalogInput(state.ThumbRY))
            };
        }
    }

    int InputManager::GetButton(int playerIndex) const
    {
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return 0; }
        return _players[playerIndex].button;
    }

    int InputManager::GetButtonDown(int playerIndex) const
    {
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return 0; }
        return _players[playerIndex].buttonDown;
    }

    int InputManager::GetButtonUp(int playerIndex) const
    {
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return 0; }
        return _players[playerIndex].buttonUp;
    }

    const DxPlus::Vec2& InputManager::GetLeftStick(int playerIndex) const
    {
        static Vector2<float> empty{};
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return empty; }
        return _players[playerIndex].leftStick;
    }

    const DxPlus::Vec2& InputManager::GetRightStick(int playerIndex) const
    {
        static Vector2<float> empty{};
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return empty; }
        return _players[playerIndex].rightStick;
    }

    float InputManager::GetLeftTrigger(int playerIndex) const
    {
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return 0.0f; }
        return _players[playerIndex].leftTrigger;
    }

    float InputManager::GetRightTrigger(int playerIndex) const
    {
        if (playerIndex < 0 || playerIndex >= DxPlus::Input::MaxPlayers) { return 0.0f; }
        return _players[playerIndex].rightTrigger;
    }

    float InputManager::NormalizeAnalogInput(int input)
    {
        if (input < 0)
        {
            return static_cast<float>(input) / static_cast<float>(-SHORT_MIN);
        }
        else
        {
            return static_cast<float>(input) / static_cast<float>(SHORT_MAX);
        }
    }

    float InputManager::NormalizeTriggerInput(unsigned char triggerValue)
    {
        return static_cast<float>(triggerValue) / static_cast<float>(UCHAR_MAX);
    }

    float InputManager::ApplyDeadZone(float value)
    {
        if (std::fabs(value) < DxPlus::Input::STICK_DEAD_ZONE) { return 0.0f; }
        return (value - std::copysign(DxPlus::Input::STICK_DEAD_ZONE, value)) / (1.0f - DxPlus::Input::STICK_DEAD_ZONE);
    }
}
