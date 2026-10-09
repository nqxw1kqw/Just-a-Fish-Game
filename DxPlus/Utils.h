// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once
#include <tchar.h>

namespace DxPlus::Utils
{
	// 個々のRGB成分を取得
	int GetColorR(unsigned int color);
	int GetColorG(unsigned int color);
	int GetColorB(unsigned int color);
	// HSV値からRGB値への変換
	unsigned int HSVtoRGB(float h, float s, float v);
	// 色相を変化させる関数
	unsigned int GetChangingColor(float time);

	void FatalError(const TCHAR* message);
}
