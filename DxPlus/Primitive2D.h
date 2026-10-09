// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <vector>
#include "DxLib.h"
#include "Vector2.h"

namespace DxPlus::Primitive2D
{
    /// <summary>
    /// 2D平面上で点を回転させるヘルパー関数
    /// </summary>
    /// <param name="point">点の座標</param>
    /// <param name="center">回転の中心点</param>
    /// <param name="angle">角度</param>
    /// <returns>回転後の点の座標</returns>
    Vec2 RotatePoint(const Vec2& point, const Vec2& center, float angle);

    /// <summary>
    /// 指定された開始点と終了点を結ぶ線を描画します。
    /// 線の色と太さを指定できます。
    /// </summary>
    /// <param name="start">線の開始点の座標（Vec2 型）</param>
    /// <param name="end">線の終了点の座標（Vec2 型）</param>
    /// <param name="color">線の色（ARGB形式、例: 0xFF0000 は赤色）</param>
    /// <param name="thickness">線の太さ（1.0f 以上を指定可能）</param>
    /// <remarks>
    /// 太さが 1.0f の場合は通常の線を描画します。
    /// 太さが 1.0f を超える場合、アンチエイリアス付きの線を描画します。
    /// </remarks>
    void DrawLine(const Vec2& start, const Vec2& end, int color = DxLib::GetColor(255,255,255), float thickness = 1.0f);

    /// <summary>
    /// 矩形を描画する関数
    /// </summary>
    /// <param name="position">矩形の位置（基準点）</param>
    /// <param name="size">矩形のサイズ（幅、高さ）</param>
    /// <param name="color">矩形の色（ARGB）</param>
    /// <param name="fill">矩形を塗りつぶすかどうか</param>
    /// <param name="pivot">基準点（回転の基準点、位置を基準にするオフセット）</param>
    /// <param name="rotation">回転角度（ラジアン）</param>
    void DrawRect(
        const Vec2& position, 
        const Vec2& size, 
        int color = GetColor(255, 255, 255), 
        bool fill = true, 
        const Vec2& pivot = Vec2(0, 0), 
        float rotation = 0.0f
    );

    /// <summary>
    /// 楕円を描画する関数
    /// </summary>
    /// <param name="center">楕円の中心点</param>
    /// <param name="radius">楕円の基本半径（x軸方向の半径として扱う）</param>
    /// <param name="color">楕円の色（ARGB）</param>
    /// <param name="fill">楕円を塗りつぶすかどうか（デフォルトは塗りつぶし）</param>
    /// <param name="scale">楕円のスケール（x方向、y方向）</param>
    /// <param name="rotation">楕円の回転角度（ラジアン）</param>
    /// <param name="numSegments">セグメント数（多いほど滑らか）</param>
    void DrawCircle(
        const Vec2& center,  // 
        float radius,                  // 
        int color = GetColor(255, 255, 255),  
        bool fill = true,               
        const Vec2& scale = Vec2(1.0f, 1.0f),
        float rotation = 0.0f, 
        int numSegments = 128
    );

    /// <summary>
    /// 多角形を描画する関数
    /// </summary>
    /// <param name="vertices">頂点のリスト</param>
    /// <param name="color">色</param>
    /// <param name="fill">塗りつぶしフラグ</param>
    void DrawPolygon(
        const std::vector<Vec2>& vertices, 
        int color, 
        bool fill = true
    );
}
