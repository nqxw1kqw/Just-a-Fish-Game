# 分離画像のポーズ・設定メニュー

現在は日本語専用です。音量行は row-music-ja.png と row-sfx-ja.png を読み込み、見出し・再開・表示モード・タイトルボタンも日本語画像だけを使用します。

ゲームを一時停止すると、停止中の画面を暗くし、木枠・ロープ・貝殻を使った設定メニューを重ねます。背景パネル、見出し、各行、スライダー、つまみ、画面モードの文字は個別の透過 PNG です。

## 素材一覧

| ファイル | 内容 |
|---|---|
| `panel.png` | 文字や操作項目のない台紙 |
| `header-en.png` / `header-ja.png` | 一時停止の見出し |
| `resume-en.png` / `resume-ja.png` | 再開ボタン |
| `row-bgm.png` | 音符と BGM の文字を含む設定行 |
| `row-sfx.png` | スピーカーと SFX の文字を含む設定行 |
| `row-display.png` | モニターのアイコンを含む設定行 |
| `slider-track.png` | 音量に応じて表示幅を変えるゲージ |
| `slider-knob.png` | 貝殻のつまみ |
| `window-en.png` / `window-ja.png` | ウィンドウ表示の文字 |
| `fullscreen-en.png` / `fullscreen-ja.png` | 全画面表示の文字 |

このフォルダーの 14 枚は組み込み画像生成ツールで個別に作成しています。タイトルへ戻るボタンは `Data/UI/Results/Layered-v1/button-title-en.png` と `button-title-ja.png`、選択矢印は `Data/UI/Results/Illustrated-v1/selection-arrow.png` を共用します。

固定の見出し・ボタン名・設定ラベルは画像内の文字です。音量の数値と操作案内はゲーム内で描画します。BGM と SFX の略称は両言語で共通です。

## 操作

- `Esc` または `P`：一時停止・再開（デバッグ画面表示中の `P` は除く）
- `W/S` または上下キー：項目を選択
- `A/D` または左右キー：音量を 5 ずつ変更、画面モードを切替
- `Enter/Space`：再開・画面モード切替・タイトルへ戻る
- マウス：ボタン上で押して離すと決定。ゲージまたは貝殻を押してドラッグすると音量を変更

停止前から押されているマウスは、いったん離すまで決定に使いません。静止したマウスはキーボードの選択を戻しません。ドラッグ中はゲージ外へ動かしても音量を 0〜100 に制限します。効果音の試聴はドラッグを離した時に行います。

`PauseMenuScreen` は画像、配置、マウスの判定を担当し、`GamePrototype` は音量、再開、タイトルへの移動、画面モード変更要求を処理します。画面モードの実際の切替は従来どおりメインループ側で行います。音量は現在の実行中の値を使用します。

## 配置変更

`layout.ini` の座標は 1600 × 900 基準です。`Panel`、`Header`、`Resume`、`Bgm`、`Sfx`、`Display`、`Title` の `X/Y/Width/Height` で画像の配置領域を指定します。縦横比は保持されます。

`TrackStartPercent/TrackEndPercent` は設定行の幅に対するゲージ両端、`ValueCenterPercent` は数値の中心です。音量・つまみ・当たり判定は行の拡大移動に追従します。`HelpY` は案内文、`SelectedLift/SelectedScalePercent/PressScalePercent` は選択と押下の見た目です。ポーズを開き直すと再読込します。

`asset-bounds.ini` は透明な余白を除く描画元矩形、`asset-bounds.json` は寸法と透明画素数の記録です。PNG の画素は変更していません。画像が不足した場合は以前のポーズメニューを表示します。生成した原文プロンプトは `PROMPTS.md` に保存しています。
