// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <type_traits>
#include <cmath>

namespace DxPlus
{
    // テンプレートクラスの定義
    template<typename T>
    struct Vector2
    {
        T x;
        T y;

        // コンストラクタ
        constexpr Vector2() : x(0), y(0) {}
        constexpr Vector2(T x, T y) : x(x), y(y) {}

        // ベクトルの加算
        constexpr Vector2 operator+(const Vector2& other) const
        {
            return Vector2(x + other.x, y + other.y);
        }

        // ベクトルの減算
        constexpr Vector2 operator-(const Vector2& other) const
        {
            return Vector2(x - other.x, y - other.y);
        }

        // スカラーとの乗算
        constexpr Vector2 operator*(T scalar) const
        {
            return Vector2(x * scalar, y * scalar);
        }

        // スカラーとの除算
        constexpr Vector2 operator/(T scalar) const
        {
            // ゼロ除算を防ぐために、スカラーがゼロの場合にはゼロベクトルを返す
            if (scalar == 0)
            {
                return Vector2(0, 0);
            }
            return Vector2(x / scalar, y / scalar);
        }

        // ベクトルの加算代入
        Vector2& operator+=(const Vector2& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        // ベクトルの減算代入
        Vector2& operator-=(const Vector2& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        // スカラーとの乗算代入
        Vector2& operator*=(T scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        // スカラーとの除算代入
        Vector2& operator/=(T scalar)
        {
            if (scalar != 0)
            {
                x /= scalar;
                y /= scalar;
            }
            return *this;
        }

        // ベクトルの長さの二乗を計算
        T LengthSq() const
        {
            static_assert(std::is_floating_point<T>::value, "LengthSq() is only available for floating point types.");
            return x * x + y * y;
        }

        // ベクトルの長さ（大きさ）を計算
        T Length() const
        {
            static_assert(std::is_floating_point<T>::value, "Length() is only available for floating point types.");
            return static_cast<T>(std::sqrt(static_cast<double>(LengthSq())));
        }

        // ベクトルの正規化
        Vector2 Normalize() const
        {
            static_assert(std::is_floating_point<T>::value, "Normalize() is only available for floating point types.");
            T length = Length();
            if (length != 0)
            {
                return *this / length;
            }
            return Vector2(0, 0);
        }
    };

    using Vec2 = Vector2<float>;    // Vector2<float>は初学者には難しいため、最初はエイリアスを使わせておく
    using Vec2Int = Vector2<int>;   // int型のVector2

    inline Vec2Int ToVec2Int(const Vec2& v)
    {
        return Vec2Int(static_cast<int>(v.x), static_cast<int>(v.y));
    }
}
