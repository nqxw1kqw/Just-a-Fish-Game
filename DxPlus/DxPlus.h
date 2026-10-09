// ============================================================================
//  DxPlus.h - OIC教材用 共通ヘッダファイル
//  
//  作成者：Y.Tanaka（OIC 大阪情報コンピュータ専門学校）
//  概要　：DxLibベースの共通定義とモジュールの統合用ヘッダ
//  更新日：2025年3月28日
// ============================================================================

#pragma once
#define NOMINMAX

// 標準ライブラリのインクルード
#include <string>
#include <sstream>

// 共通型やユーティリティのインクルード
#include "Utils.h"
#include "DxPlusConstants.h"

// 各機能モジュールのインクルード
#include "InputManager.h"
#include "Sprite.h"
#include "Primitive2D.h"
#include "Text.h"
#include "Debug.h"
#include "TextAnimator.h"
#include "FadeController.h"

// DxWrapperのインクルード
#include "DxWrapper.h"

// 他に必要なモジュールがあればここに追加

// もし初期化や終了処理が必要ならば、ここに追加可能
namespace DxPlus
{
	int Initialize(int clientWidth, int clientHeight, bool bWindowed);
	void Shutdown();
	bool GameLoop(bool showFps = false);
}
