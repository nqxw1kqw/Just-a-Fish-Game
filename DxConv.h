// =============================
// Core/Math/DxConv.h
// =============================
#pragma once
#include "DxLib.h"
#include "Vector3.h"

namespace DxConv
{
    inline DxLib::VECTOR ToVECTOR(const Vec3& v) noexcept
    {
        return VGet(v.x, v.y, v.z);
    }

    inline Vec3 ToVec3(const DxLib::VECTOR& v) noexcept
    {
        return Vec3{ v.x, v.y, v.z };
    }
}
