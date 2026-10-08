#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include "DxLib.h"
#include "Vector3.h"
#include "DxConv.h"

namespace Render3D
{
    // Vẽ khối hộp 3D trục tọa độ (thay thế DrawBox3D)
    inline void DrawBox3D(VECTOR Pos1, VECTOR Pos2, unsigned int Color, int FillFlag)
    {
        DrawCube3D(Pos1, Pos2, Color, Color, FillFlag);
    }

    // Vẽ đoạn thẳng 3D với Vec3
    inline void DrawLine3D(const Vec3& p1, const Vec3& p2, unsigned int color)
    {
        ::DrawLine3D(DxConv::ToVECTOR(p1), DxConv::ToVECTOR(p2), color);
    }

    // Vẽ hình tròn 3D trên mặt phẳng XZ
    inline void DrawCircle3D(const Vec3& center, float radius, int divNum, unsigned int color, bool fill = false)
    {
        if (radius <= 0.1f || divNum < 3) return;
        float angleStep = 2.0f * 3.14159265f / divNum;
        VECTOR c = DxConv::ToVECTOR(center);

        for (int i = 0; i < divNum; ++i)
        {
            float a1 = i * angleStep;
            float a2 = (i + 1) * angleStep;
            VECTOR p1 = VGet(center.x + std::cos(a1) * radius, center.y, center.z + std::sin(a1) * radius);
            VECTOR p2 = VGet(center.x + std::cos(a2) * radius, center.y, center.z + std::sin(a2) * radius);

            if (fill)
            {
                DrawTriangle3D(c, p1, p2, color, TRUE);
                DrawTriangle3D(c, p2, p1, color, TRUE);
            }
            else
            {
                DrawLine3D(p1, p2, color);
            }
        }
    }

    // Vẽ khối hộp 3D có thể xoay theo góc Yaw (quay quanh trục Y)
    inline void DrawOrientedBox3D(
        const Vec3& center,
        const Vec3& halfSize,
        float yaw,
        unsigned int fillColor,
        unsigned int edgeColor = 0,
        bool drawEdges = true)
    {
        float cosY = std::cos(yaw);
        float sinY = std::sin(yaw);

        // 8 góc hộp ở tọa độ local
        Vec3 localPts[8] = {
            { -halfSize.x, -halfSize.y, -halfSize.z }, // 0
            {  halfSize.x, -halfSize.y, -halfSize.z }, // 1
            {  halfSize.x,  halfSize.y, -halfSize.z }, // 2
            { -halfSize.x,  halfSize.y, -halfSize.z }, // 3
            { -halfSize.x, -halfSize.y,  halfSize.z }, // 4
            {  halfSize.x, -halfSize.y,  halfSize.z }, // 5
            {  halfSize.x,  halfSize.y,  halfSize.z }, // 6
            { -halfSize.x,  halfSize.y,  halfSize.z }, // 7
        };

        VECTOR worldPts[8];
        for (int i = 0; i < 8; ++i)
        {
            float rx = localPts[i].x * cosY + localPts[i].z * sinY;
            float rz = -localPts[i].x * sinY + localPts[i].z * cosY;
            worldPts[i] = VGet(center.x + rx, center.y + localPts[i].y, center.z + rz);
        }

        // 6 mặt của khối hộp (mỗi mặt gồm 2 tam giác)
        int faces[6][4] = {
            { 4, 5, 6, 7 }, // Front (+Z)
            { 1, 0, 3, 2 }, // Back (-Z)
            { 3, 2, 6, 7 }, // Top (+Y)
            { 0, 1, 5, 4 }, // Bottom (-Y)
            { 0, 4, 7, 3 }, // Left (-X)
            { 5, 1, 2, 6 }, // Right (+X)
        };

        for (int f = 0; f < 6; ++f)
        {
            int i0 = faces[f][0];
            int i1 = faces[f][1];
            int i2 = faces[f][2];
            int i3 = faces[f][3];

            DrawTriangle3D(worldPts[i0], worldPts[i1], worldPts[i2], fillColor, TRUE);
            DrawTriangle3D(worldPts[i0], worldPts[i2], worldPts[i3], fillColor, TRUE);
            DrawTriangle3D(worldPts[i0], worldPts[i2], worldPts[i1], fillColor, TRUE);
            DrawTriangle3D(worldPts[i0], worldPts[i3], worldPts[i2], fillColor, TRUE);
        }

        if (drawEdges)
        {
            int edges[12][2] = {
                {0,1}, {1,2}, {2,3}, {3,0},
                {4,5}, {5,6}, {6,7}, {7,4},
                {0,4}, {1,5}, {2,6}, {3,7}
            };
            for (int e = 0; e < 12; ++e)
            {
                DrawLine3D(worldPts[edges[e][0]], worldPts[edges[e][1]], edgeColor);
            }
        }
    }

    // Vẽ bóng đổ dẹt trên sàn đấu
    inline void DrawFlatShadow(const Vec3& groundPos, float radius, float alphaRatio)
    {
        if (radius <= 1.0f || alphaRatio <= 0.01f) return;
        int alpha = static_cast<int>(std::clamp(alphaRatio * 150.0f, 0.0f, 150.0f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        int shadowColor = GetColor(10, 15, 25);
        Vec3 p{ groundPos.x, 1.2f, groundPos.z };
        DrawCircle3D(p, radius, 24, shadowColor, true);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}
