// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <string>
#include "DxLib.h"
#include "Vector2.h"

namespace DxPlus::Text
{
    static int defaultFontHandle = -1;

    enum TextAlign
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        MIDDLE_LEFT,
        MIDDLE_CENTER,
        MIDDLE_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };

    void InitializeDefaultFont(const wchar_t* fontName, int fontSize, int fontThickness);
    void FinalizeFont();
    int InitializeFont(const wchar_t* fontName, int fontSize, int fontThickness);
    void DeleteFont(int fontHandle);

    /// <summary>
    /// 文字列を指定位置に描画する
    /// </summary>
    /// <param name="text">表示する文字列</param>
    /// <param name="position">描画位置（ピクセル座標）</param>
    /// <param name="color">文字色（GetColorで指定）</param>
    /// <param name="align">整列方法（中央揃えなど）</param>
    /// <param name="scale">拡大率（1.0fで等倍）</param>
    /// <param name="angle">回転角度（ラジアン）</param>
    /// <param name="fontHandle">使用するフォントID（-1で標準）</param>
    void DrawString(
        const wchar_t* text,
        Vec2 position,
        int color = GetColor(255, 255, 255),
        TextAlign align = TextAlign::TOP_LEFT,
        Vec2 scale = { 1.0f, 1.0f },
        double angle = 0.0,
        int fontHandle = -1);

    void DrawString(
        const wchar_t* text,
        int x,
        int y,
        int color = GetColor(255, 255, 255),
        TextAlign align = TextAlign::TOP_LEFT,
        double scaleX = 1.0,
        double scaleY = 1.0,
        double angle = 0.0,
        int fontHandle = -1);

    std::wstring Format(const wchar_t* format, ...);

    void DrawTextWithAutoWrap(
        const wchar_t* text,
        const DxPlus::Vec2& position,
        const DxPlus::Vec2& size,
        int framePerChar,
        int color,
        int lineSpacing
    );

    // UTF-8 文字列を UTF-16 文字列に変換
    std::wstring Utf8ToUtf16(const std::string& utf8);

    /// <summary>
    /// 指定した文字列を縁取り付きで描画します（回転・整列対応）。
    /// DXライブラリの DrawRotaStringFToHandle を使用し、文字の中心を回転の軸にしています。
    /// 拡大率は1.0固定、縁取りの太さ・色・整列・角度指定が可能です。
    /// </summary>
    /// <param name="text">描画する文字列</param>
    /// <param name="position">描画位置（Vec2）</param>
    /// <param name="fontID">フォントハンドル</param>
    /// <param name="fillColor">文字色</param>
    /// <param name="outlineColor">縁取りの色</param>
    /// <param name="thickness">縁取りの太さ（ピクセル単位）</param>
    /// <param name="align">描画基準位置（TextAlign）</param>
    /// <param name="angle">回転角度（ラジアン）</param>
    void DrawOutlinedString(
        const wchar_t* text,
        Vec2 position,
        int fontID,
        int fillColor = GetColor(255, 255, 255),
        int outlineColor = GetColor(0, 0, 0),
        int thickness = 1,
        TextAlign align = TextAlign::TOP_LEFT,
        double angle = 0.0);
}
