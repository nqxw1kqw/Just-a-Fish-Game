// ============================================================================
// OIC教材用モジュール - 大阪情報コンピュータ専門学校
// 作成者：Y.Tanaka
// このファイルは授業用教材として作成されています。
// ============================================================================

#pragma once

namespace DxPlus
{
	class FadeController
	{
	public:
		enum class State { FadeIn, Stay, FadeOut };

		FadeController();

		// フェードイン・アウトを開始する
		void StartFadeIn(float duration = 1.0f);
		void StartFadeOut(float duration = 1.0f);
		void Update();		// 更新処理
		void Draw() const;	// 描画処理

		// 状態判定
		State GetState() const;
		bool IsFadeOutDone() const;
		bool IsStable() const;

	private:
		float timer = 0.0f;
		float duration = 1.0f;
		State state;
	};
} // namespace DxPlus
