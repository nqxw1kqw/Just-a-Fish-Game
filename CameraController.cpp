#include "CameraController.h"
#include "RenderInterp.h"
#include "Effects.h"
#include "DxConv.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

void CameraController::Reset(const Vec3& initialTargetPos)
{
    camTarget_ = Vec3{ initialTargetPos.x, 0.0f, initialTargetPos.z };
    camPitchDeg_ = GameConsts::CAM_PITCH_DEG;
    camDistance_ = GameConsts::CAM_DISTANCE;
    camFollowSpeed_ = GameConsts::CAM_FOLLOW_SPEED;
    camDeadZone_ = GameConsts::CAM_DEAD_ZONE;
    camShakeScale_ = 1.0f;
    cineOn_ = false;
    cineBlend_ = 0.0f;
}

void CameraController::Update(float dt, const Vec3& targetPos, const EffectSystem& effects)
{
    // C1: Bỏ hẳn Y nảy của cá!
    Vec3 wantTarget = Vec3{ targetPos.x, 0.0f, targetPos.z };

    // C3: Vùng chết (Dead Zone)
    Vec3 diff = wantTarget - camTarget_;
    diff.y = 0.0f;
    float distFromTarget = diff.Length();
    if (distFromTarget > camDeadZone_)
    {
        Vec3 leadPos = camTarget_ + (diff / distFromTarget) * (distFromTarget - camDeadZone_);
        // C2: Làm mượt độc lập FPS qua hàm 1 - exp(-k * dt)
        float a = 1.0f - std::exp(-camFollowSpeed_ * dt);
        camTarget_.x += (leadPos.x - camTarget_.x) * a;
        camTarget_.z += (leadPos.z - camTarget_.z) * a;
    }
    camTarget_.y = 0.0f;

    // C8: Góc nghiêng & Khoảng cách camera
    // Cận cảnh (Shark Hunt): mượt vào/ra theo 1 - exp(-k dt)
    float cineA = 1.0f - std::exp(-GameConsts::CAM_CINE_BLEND_SPEED * dt);
    cineBlend_ += ((cineOn_ ? 1.0f : 0.0f) - cineBlend_) * cineA;
    float dist = camDistance_ + (GameConsts::CAM_CINE_DISTANCE - camDistance_) * cineBlend_;
    float pitchDeg = camPitchDeg_ + (GameConsts::CAM_CINE_PITCH_DEG - camPitchDeg_) * cineBlend_;
    float pitchRad = pitchDeg * 3.14159265f / 180.0f;
    camEye_ = camTarget_ + Vec3{
        0.0f,
        dist * std::sin(pitchRad),
        -dist * std::cos(pitchRad)
    };

    // C5 & C7: Rung 2 trục kết hợp tỷ lệ camShakeScale_
    Vec3 shake = effects.GetCameraShakeOffset(camShakeScale_);
    DxLib::SetCameraNearFar(5.0f, 4000.0f);
    DxLib::SetCameraPositionAndTargetAndUpVec(
        DxConv::ToVECTOR(camEye_ + shake),
        DxConv::ToVECTOR(camTarget_ + shake),
        VGet(0.0f, 1.0f, 0.0f)
    );
}

void CameraController::ApplyInterpolated(float alpha, const EffectSystem& effects) const
{
    Vec3 eye = camEye_;
    Vec3 target = camTarget_;
    if (!RenderInterp::ShouldSnap(prevTarget_, camTarget_))
    {
        eye = RenderInterp::Lerp(prevEye_, camEye_, alpha);
        target = RenderInterp::Lerp(prevTarget_, camTarget_, alpha);
    }
    Vec3 shake = effects.GetCameraShakeOffset(camShakeScale_);
    DxLib::SetCameraNearFar(5.0f, 4000.0f);
    DxLib::SetCameraPositionAndTargetAndUpVec(
        DxConv::ToVECTOR(eye + shake),
        DxConv::ToVECTOR(target + shake),
        VGet(0.0f, 1.0f, 0.0f)
    );
}
