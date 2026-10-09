// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <array>
#include <vector>
#include "Vector2.h"
#include "Input.h"
#include "InputConstants.h"

namespace DxPlus
{
    struct ActionMapping
    {
        int action;
        int inputCode;
    };

    struct InputState
    {
        InputState()
            :button(0)
            , buttonDown(0)
            , buttonUp(0)
            , leftStick()
            , rightStick()
            , leftTrigger(0.0f)
            , rightTrigger(0.0f) {}

        void SetKeyboardMappings(const std::vector<ActionMapping>& data)
        {
            keyboardAssign = data;
        }
        void SetMouseMappings(const std::vector<ActionMapping>& data)
        {
            mouseAssign = data;
        }
        void SetControllerMappings(const std::vector<ActionMapping>& data)
        {
            controllerAssign = data;
        }

        int button;
        int buttonDown;
        int buttonUp;
        DxPlus::Vec2 leftStick;
        DxPlus::Vec2 rightStick;
        float leftTrigger;
        float rightTrigger;
        std::vector<ActionMapping> keyboardAssign;
        std::vector<ActionMapping> mouseAssign;
        std::vector<ActionMapping> controllerAssign;
    };

    class InputManager
    {
    public:
        InputManager() = default;

        void Initialize();
        void Update();

        int GetButton(int playerIndex) const;
        int GetButtonDown(int playerIndex) const;
        int GetButtonUp(int playerIndex) const;

        const DxPlus::Vec2& GetLeftStick(int playerIndex) const;
        const DxPlus::Vec2& GetRightStick(int playerIndex) const;
        float GetLeftTrigger(int playerIndex) const;
        float GetRightTrigger(int playerIndex) const;

    private:
        float NormalizeAnalogInput(int input);
        float NormalizeTriggerInput(unsigned char triggerValue);
        float ApplyDeadZone(float value);

        std::array<InputState, DxPlus::Input::MaxPlayers> _players;
    };
}
