// =============================
// Core/Math/Vector3.h
// =============================
#pragma once
#include <cmath>

template <typename T>
struct Vector3
{
    T x{};
    T y{};
    T z{};

    constexpr Vector3() = default;
    constexpr Vector3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

    constexpr Vector3<T> operator+(const Vector3<T>& other) const
    {
        return Vector3<T>(x + other.x, y + other.y, z + other.z);
    }

    constexpr Vector3<T> operator-(const Vector3<T>& other) const
    {
        return Vector3<T>(x - other.x, y - other.y, z - other.z);
    }

    constexpr Vector3<T> operator*(T scalar) const
    {
        return Vector3<T>(x * scalar, y * scalar, z * scalar);
    }

    constexpr Vector3<T> operator/(T scalar) const
    {
        return Vector3<T>(x / scalar, y / scalar, z / scalar);
    }

    Vector3<T>& operator+=(const Vector3<T>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3<T>& operator-=(const Vector3<T>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3<T>& operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3<T>& operator/=(T scalar)
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    constexpr Vector3<T> operator-() const
    {
        return Vector3<T>(-x, -y, -z);
    }

    T LengthSq() const
    {
        return x * x + y * y + z * z;
    }

    T Length() const
    {
        return static_cast<T>(std::sqrt(LengthSq()));
    }

    Vector3<T> Normalized() const
    {
        T length = Length();
        if (length <= static_cast<T>(0))
        {
            return Vector3<T>(0, 0, 0);
        }
        return Vector3<T>(x / length, y / length, z / length);
    }

    static constexpr float Dot(const Vector3<T>& a, const Vector3<T>& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static constexpr Vector3<T> Cross(const Vector3<T>& a, const Vector3<T>& b)
    {
        return Vector3<T>(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static constexpr Vector3<T> Up()
    {
        return Vector3<T>(static_cast<T>(0), static_cast<T>(1), static_cast<T>(0));
    }
};

using Vec3 = Vector3<float>;
using Vec3Int = Vector3<int>;
