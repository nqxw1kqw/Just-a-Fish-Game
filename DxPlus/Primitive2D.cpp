// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#include "Primitive2D.h"

namespace DxPlus::Primitive2D
{
    // 2D平面上で点を回転させるヘルパー関数
    Vec2 RotatePoint(const Vec2& point, const Vec2& center, float angle)
    {
        float s = sin(angle);
        float c = cos(angle);

        // 原点に平行移動
        Vec2 p = point - center;

        // 回転
        float xnew = p.x * c - p.y * s;
        float ynew = p.x * s + p.y * c;

        // 平行移動を元に戻す
        p.x = xnew + center.x;
        p.y = ynew + center.y;

        return p;
    }

    void DrawLine(const Vec2& start, const Vec2& end, int color, float thickness)
    {
        if (thickness <= 1.0f)
        {
            DxLib::DrawLine(
                static_cast<int>(start.x), static_cast<int>(start.y),
                static_cast<int>(end.x), static_cast<int>(end.y),
                color
            );
        }
        else
        {
            DxLib::DrawLineAA(
                start.x, start.y,
                end.x, end.y,
                color, thickness
            );
        }
    }

    void DrawRect(
        const Vec2& position,       // 矩形の位置（基準点）
        const Vec2& size,           // 矩形のサイズ（幅、高さ）
        int color,                  // 矩形の色（ARGB）
        bool fill,                  // 矩形を塗りつぶすかどうか
        const Vec2& pivot,          // 基準点（回転の基準点、位置を基準にするオフセット）
        float rotation              // 回転角度（ラジアン）
    )
    {
        DxLib::SetUseZBuffer3D(FALSE);  // Zバッファを無効化
        DxLib::SetWriteZBuffer3D(FALSE);  // Zバッファへの書き込みも無効化

        // 矩形の頂点の相対位置を計算
        Vec2 halfSize = size / 2.0f;
        Vec2 points[4] = {
            { -pivot.x, -pivot.y },
            { size.x - pivot.x, -pivot.y },
            { size.x - pivot.x, size.y - pivot.y },
            { -pivot.x, size.y - pivot.y }
        };

        // 回転を適用して、頂点の最終位置を計算
        float cosRot = std::cos(rotation);
        float sinRot = std::sin(rotation);
        Vec2 finalPoints[4];

        for (int i = 0; i < 4; ++i)
        {
            // 回転を適用
            float rotatedX = points[i].x * cosRot - points[i].y * sinRot;
            float rotatedY = points[i].x * sinRot + points[i].y * cosRot;

            // 描画位置の計算
            finalPoints[i].x = position.x + rotatedX;
            finalPoints[i].y = position.y + rotatedY;
        }

        if (fill)
        {
            // 塗りつぶしの矩形を描画
            DrawTriangle(
                static_cast<int>(finalPoints[0].x), static_cast<int>(finalPoints[0].y),
                static_cast<int>(finalPoints[1].x), static_cast<int>(finalPoints[1].y),
                static_cast<int>(finalPoints[2].x), static_cast<int>(finalPoints[2].y),
                color, TRUE
            );
            DrawTriangle(
                static_cast<int>(finalPoints[0].x), static_cast<int>(finalPoints[0].y),
                static_cast<int>(finalPoints[2].x), static_cast<int>(finalPoints[2].y),
                static_cast<int>(finalPoints[3].x), static_cast<int>(finalPoints[3].y),
                color, TRUE
            );
        }
        else
        {
            // 線画の矩形を描画
            for (int i = 0; i < 4; ++i)
            {
                int next = (i + 1) % 4;
                DxPlus::Primitive2D::DrawLine(finalPoints[i], finalPoints[next], color);
            }
        }

        DxLib::SetUseZBuffer3D(TRUE);  // Zバッファを有効化
        DxLib::SetWriteZBuffer3D(TRUE);  // Zバッファへの書き込みも有効化
    }

    void DrawCircle(const Vec2& center, float radius, int color, bool fill, const Vec2& scale, float rotation, int numSegments)
    {
        DxLib::SetUseZBuffer3D(FALSE);  // Zバッファを無効化
        DxLib::SetWriteZBuffer3D(FALSE);  // Zバッファへの書き込みも無効化

        float angleStep = static_cast<float>(2.0f * DX_PI / numSegments);

        // 回転を事前に計算
        float cosRot = std::cos(rotation);
        float sinRot = std::sin(rotation);

        if (fill)
        {
            // 塗りつぶしの楕円を描画
            for (int i = 0; i < numSegments; ++i)
            {
                float angle1 = i * angleStep;
                float angle2 = (i + 1) * angleStep;

                // 最初の点を計算
                float x1 = std::cos(angle1) * radius * scale.x;
                float y1 = std::sin(angle1) * radius * scale.y;
                float rotatedX1 = x1 * cosRot - y1 * sinRot;
                float rotatedY1 = x1 * sinRot + y1 * cosRot;
                int drawX1 = static_cast<int>(center.x + rotatedX1);
                int drawY1 = static_cast<int>(center.y + rotatedY1);

                // 次の点を計算
                float x2 = std::cos(angle2) * radius * scale.x;
                float y2 = std::sin(angle2) * radius * scale.y;
                float rotatedX2 = x2 * cosRot - y2 * sinRot;
                float rotatedY2 = x2 * sinRot + y2 * cosRot;
                int drawX2 = static_cast<int>(center.x + rotatedX2);
                int drawY2 = static_cast<int>(center.y + rotatedY2);

                // 三角形を描画して塗りつぶす
                DrawTriangle(

                    static_cast<int>(center.x),
                    static_cast<int>(center.y),
                    drawX1, drawY1, drawX2, drawY2, color, TRUE);
            }
        }
        else
        {
            // 輪郭の楕円を描画
            float prevX = radius * scale.x;
            float prevY = 0.0f;
            float rotatedPrevX = prevX * cosRot - prevY * sinRot;
            float rotatedPrevY = prevX * sinRot + prevY * cosRot;
            int drawPrevX = static_cast<int>(center.x + rotatedPrevX);
            int drawPrevY = static_cast<int>(center.y + rotatedPrevY);

            for (int i = 1; i <= numSegments; ++i)
            {
                float angle = i * angleStep;
                float x = std::cos(angle) * radius * scale.x;
                float y = std::sin(angle) * radius * scale.y;

                // 回転を適用
                float rotatedX = x * cosRot - y * sinRot;
                float rotatedY = x * sinRot + y * cosRot;

                int drawX = static_cast<int>(center.x + rotatedX);
                int drawY = static_cast<int>(center.y + rotatedY);

                // 輪郭を描画
                DxPlus::Primitive2D::DrawLine(
                    { static_cast<float>(drawPrevX), static_cast<float>(drawPrevY) },
                    { static_cast<float>(drawX), static_cast<float>(drawY) }, color);

                drawPrevX = drawX;
                drawPrevY = drawY;
            }
        }

        DxLib::SetUseZBuffer3D(TRUE);  // Zバッファを有効化
        DxLib::SetWriteZBuffer3D(TRUE);  // Zバッファへの書き込みも有効化
    }

    void DrawPolygon(const std::vector<Vec2>& vertices, int color, bool fill)
    {
        DxLib::SetUseZBuffer3D(FALSE);  // Zバッファを無効化
        DxLib::SetWriteZBuffer3D(FALSE);  // Zバッファへの書き込みも無効化

        size_t numVertices = vertices.size();
        if (numVertices < 3) return; // 頂点数が3未満の場合、描画しない

        if (fill)
        {
            // ポリゴンを複数の三角形に分割して描画
            for (size_t i = 1; i < numVertices - 1; ++i)
            {
                DrawTriangleAA(
                    vertices[0].x, vertices[0].y,
                    vertices[i].x, vertices[i].y,
                    vertices[i + 1].x, vertices[i + 1].y,
                    color, TRUE
                );
            }
        }
        else
        {
            // 頂点を順番に線で結ぶことでポリゴンを描画
            for (size_t i = 0; i < numVertices; ++i)
            {
                size_t next = (i + 1) % numVertices;
                DxPlus::Primitive2D::DrawLine(vertices[i], vertices[next], color);
            }
        }

        DxLib::SetUseZBuffer3D(TRUE);  // Zバッファを有効化
        DxLib::SetWriteZBuffer3D(TRUE);  // Zバッファへの書き込みも有効化
    }
}
