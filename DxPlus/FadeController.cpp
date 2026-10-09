// =============================
// FadeController.cpp
// =============================
#include "FadeController.h"
#include "DxPlus.h"

namespace DxPlus
{
    FadeController::FadeController() : timer(0.0f), duration(1.0f), state(State::Stay) {}

    void FadeController::StartFadeIn(float duration_)
    {
        duration = duration_;
        if (duration <= 0.0f)
        {
            timer = 0.0f;
            state = State::Stay;
            return;
        }
        timer = duration;
        state = State::FadeIn;
    }

    void FadeController::StartFadeOut(float duration_)
    {
        duration = duration_;
        if (duration <= 0.0f)
        {
            timer = 0.0f;
            state = State::FadeOut;
            return;
        }
        timer = 0.0f;
        state = State::FadeOut;
    }

    void FadeController::Update()
    {
        if (duration <= 0.0f)
        {
            return;
        }

        float speed = 1.0f / (duration * 60.0f);
        if (state == State::FadeIn)
        {
            timer -= speed;
            if (timer <= 0.0f)
            {
                timer = 0.0f;
                state = State::Stay;
            }
        }
        else if (state == State::FadeOut)
        {
            timer += speed;
            if (timer >= duration)
            {
                timer = duration;
            }
        }
    }

    void FadeController::Draw() const
    {
        if (state == State::Stay) { return; }
        if (duration <= 0.0f) { return; }

        float alpha = timer / duration;
        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * alpha));
        DxPlus::Primitive2D::DrawRect(
            { 0.0f, 0.0f }, { DxPlus::CLIENT_WIDTH, DxPlus::CLIENT_HEIGHT },
            DxLib::GetColor(0, 0, 0));
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    FadeController::State DxPlus::FadeController::GetState() const
    {
        return state;
    }

    bool DxPlus::FadeController::IsFadeOutDone() const
    {
        return state == State::FadeOut && timer >= duration;
    }

    bool FadeController::IsStable() const
    {
        return state == State::Stay;
    }
} // namespace DxPlus
