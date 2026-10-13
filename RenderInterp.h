#pragma once
#include <cmath>
#include "Vector3.h"

// =======================================================================
// RenderInterp.h
// Nội suy hình ảnh (render interpolation) giữa 2 tick logic cố định 120Hz.
// Logic vẫn chạy bước cố định; khi vẽ, vị trí/góc được lerp theo alpha
// = phần dư của bộ tích luỹ / FIXED_TIMESTEP để chuyển động mượt trên mọi FPS.
// =======================================================================
namespace RenderInterp
{
    // Khoảng cách (unit) mà 1 tick không thể đi được -> coi là dịch chuyển tức thời, không lerp
    constexpr float SNAP_DISTANCE = 150.0f;

    inline Vec3 Lerp(const Vec3& a, const Vec3& b, float t)
    {
        return Vec3{ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
    }

    // Lerp góc theo đường ngắn nhất (radian)
    inline float LerpAngle(float a, float b, float t)
    {
        constexpr float PI = 3.14159265f;
        float d = b - a;
        while (d > PI) d -= 2.0f * PI;
        while (d < -PI) d += 2.0f * PI;
        return a + d * t;
    }

    inline bool ShouldSnap(const Vec3& a, const Vec3& b)
    {
        float dx = b.x - a.x, dy = b.y - a.y, dz = b.z - a.z;
        return (dx * dx + dy * dy + dz * dz) > SNAP_DISTANCE * SNAP_DISTANCE;
    }

    // Đổi pos/yaw sang giá trị nội suy; trả lại bản gốc qua savedPos/savedYaw
    inline void Apply(Vec3& pos, float& yaw, const Vec3& prevPos, float prevYaw, float alpha, Vec3& savedPos, float& savedYaw)
    {
        savedPos = pos;
        savedYaw = yaw;
        if (ShouldSnap(prevPos, pos)) return;
        pos = Lerp(prevPos, pos, alpha);
        yaw = LerpAngle(prevYaw, yaw, alpha);
    }
}
