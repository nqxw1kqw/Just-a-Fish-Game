# 分離レイヤー版のリザルト画面

現在は日本語専用です。見出し・記録ボード・ボタンは日本語画像だけを読み込みます。英語画像は保管用で、配布用 ZIP には含めません。

勝利・敗北画面を、背景、見出し、魚、記録ボード、再挑戦ボタン、タイトルボタンに分離しています。このフォルダーのパーツは透過 PNG です。以前の一枚絵は `../Illustrated-v1/` に保存しています。

## 画像一覧

| ファイル | 内容 |
|---|---|
| `fish-win.png` | 勝利した魚 |
| `fish-lose.png` | 敗北した魚 |
| `header-win-en.png` / `header-win-ja.png` | 勝利の見出し |
| `header-lose-en.png` / `header-lose-ja.png` | 敗北の見出し |
| `board-stage-ja.png` | 現行の記録ボード。「区間」を表示 |
| `board-en.png` / `board-ja.png` | 旧ボード。保管用 |
| `button-retry-en.png` / `button-retry-ja.png` | 再挑戦ボタン |
| `button-title-en.png` / `button-title-ja.png` | タイトルへ戻るボタン |

背景は `Data/UI/ChoiceMenus/Layered-v1/background.png`、選択矢印は `Data/UI/Results/Illustrated-v1/selection-arrow.png` を共用します。見出し、項目名、ボタン文字、枠は画像内に含まれます。実際のスコア、時間、撃破数、区間、ステージ、モード、記録、操作案内はゲーム内で描画します。

## 配置の調整

`layout.ini` の `[Result]` は論理画面 1600 × 900 px の座標です。`Header`、`Fish`、`Board`、`Retry`、`Title` の `X/Y/Width/Height` で各画像を収める領域を変更できます。画像の縦横比は保持します。

`[ValuesEN]` と `[ValuesJP]` の座標は元のボード PNG 上の位置です。`X` は数値列の中心、`Width` は列の最大幅、`Row0` から `Row4` はスコア、時間、撃破数、区間、ステージの行中心です。ボードを移動・拡大すると数値も追従します。

`ModeX/Y` はモード名、`NotesX/Y` は再挑戦の開始位置などの案内です。`SelectedLift`、`SelectedScalePercent`、`PressScalePercent` は選択・押下時の見た目、`NumberScalePercent`、`ScoreScalePercent` は数値の大きさを調整します。結果画面へ入り直すと再読込するため、配置だけの変更は再ビルド不要です。

## 描画と入力

`LayeredResultScreen` は画像の読込・配置・描画を担当します。`ResultScreen` が入力、待機時間、スコア計算、再挑戦、タイトルへの遷移を管理します。マウス判定は描画と同じ配置・拡大率から求めます。画面に入る前から押されているキーやマウスでは決定せず、押してから同じボタン上で離すと選択します。静止したマウスはキーボードの選択を上書きしません。

必要な画像が欠けている場合は日本語パネルで予備表示します。旧一枚絵は読み込みません。PNG の透明余白は `asset-bounds.ini` の描画元矩形で除き、画素は変更していません。別寸法の PNG に差し替えた場合は画像全体を描画します。現行の描画矩形は `asset-bounds.ini`、初期素材の寸法・透明画素数は `asset-bounds.json` にあります。

## 生成記録

組み込み画像生成ツールで、承認済みの画風を参照しながら 12 枚を個別に抽出・再構成しました。新しい `board-stage-ja.png` は組み込み画像生成ツールの編集で旧「レベル」行を「区間」に変更し、元画像は保存しました。`PROMPTS.md` に原文プロンプトを保存しています。
