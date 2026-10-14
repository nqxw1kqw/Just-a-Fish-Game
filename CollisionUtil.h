#pragma once
// =============================
// CollisionUtil.h
// 当たり判定の共通関数 / Hàm va chạm dùng chung (Task #24)
// Tất cả phép tính giữ nguyên thứ tự như code cũ để kết quả float giống hệt.
// =============================
#include <cmath>
#include "Vector3.h"

namespace Collision
{
    // a - b, bỏ trục Y (mặt phẳng XZ)
    inline Vec3 FlatDiff(const Vec3& a, const Vec3& b)
    {
        Vec3 d = a - b;
        d.y = 0.0f;
        return d;
    }

    // Bình phương khoảng cách trên mặt phẳng XZ
    inline float DistSqXZ(const Vec3& a, const Vec3& b)
    {
        return FlatDiff(a, b).LengthSq();
    }

    // Khoảng cách trên mặt phẳng XZ
    inline float DistXZ(const Vec3& a, const Vec3& b)
    {
        return FlatDiff(a, b).Length();
    }

    // Hai hình tròn (XZ) chồng lên nhau? (biên tính là chạm)
    inline bool CirclesOverlapXZ(const Vec3& a, float radiusA, const Vec3& b, float radiusB)
    {
        float totalRad = radiusA + radiusB;
        return DistSqXZ(a, b) <= totalRad * totalRad;
    }

    // 円柱判定: hình trụ đứng — chồng XZ và chênh lệch Y < maxDeltaY
    inline bool CylinderHit(const Vec3& a, float radiusA, const Vec3& b, float radiusB, float maxDeltaY)
    {
        if (!(std::abs(a.y - b.y) < maxDeltaY)) return false;
        return CirclesOverlapXZ(a, radiusA, b, radiusB);
    }

    // Nón trên mặt XZ: point nằm trong nón đỉnh origin, hướng dir (không cần chuẩn hoá),
    // nửa góc halfAngleRad, tầm range (+ bán kính mục tiêu targetRadius)
    inline bool InConeXZ(const Vec3& origin, const Vec3& dir, float halfAngleRad, float range, const Vec3& point, float targetRadius = 0.0f)
    {
        Vec3 d = FlatDiff(point, origin);
        float distSq = d.x * d.x + d.z * d.z;
        float maxR = range + targetRadius;
        if (distSq > maxR * maxR) return false;
        if (distSq < 1.0f) return true;
        float dl = std::sqrt(dir.x * dir.x + dir.z * dir.z);
        if (dl < 1e-4f) return false;
        float cosA = (d.x * dir.x + d.z * dir.z) / (std::sqrt(distSq) * dl);
        return cosA >= std::cos(halfAngleRad);
    }
}
