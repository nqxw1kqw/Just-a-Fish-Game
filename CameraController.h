#pragma once
#include "Vector3.h"
#include "Consts.h"

class EffectSystem;

class CameraController
{
public:
    void Reset(const Vec3& initialTargetPos = Vec3{ 0.0f, 0.0f, 0.0f });
    void Update(float dt, const Vec3& targetPos, const EffectSystem& effects);

    const Vec3& GetEye() const { return camEye_; }
    const Vec3& GetTarget() const { return camTarget_; }

    float GetPitchDeg() const { return camPitchDeg_; }
    void SetPitchDeg(float deg) { camPitchDeg_ = deg; }

    float GetDistance() const { return camDistance_; }
    void SetDistance(float dist) { camDistance_ = dist; }

    float GetShakeScale() const { return camShakeScale_; }
    void SetShakeScale(float scale) { camShakeScale_ = scale; }

    float GetFollowSpeed() const { return camFollowSpeed_; }
    void SetFollowSpeed(float speed) { camFollowSpeed_ = speed; }

    float GetDeadZone() const { return camDeadZone_; }
    void SetDeadZone(float deadZone) { camDeadZone_ = deadZone; }

private:
    Vec3 camTarget_{ 0.0f, 0.0f, 0.0f };
    Vec3 camEye_{ 0.0f, 480.0f, -420.0f };
    float camPitchDeg_ = GameConsts::CAM_PITCH_DEG;
    float camDistance_ = GameConsts::CAM_DISTANCE;
    float camFollowSpeed_ = GameConsts::CAM_FOLLOW_SPEED;
    float camDeadZone_ = GameConsts::CAM_DEAD_ZONE;
    float camShakeScale_ = 1.0f;
};
