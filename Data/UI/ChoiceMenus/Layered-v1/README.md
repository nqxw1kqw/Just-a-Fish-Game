# 分離レイヤー版の選択メニュー

現在は日本語専用です。言語選択は使用せず、モードは日本語画像、決定ボタンは confirm-ja.png を読み込みます。英語画像は元データとして保管し、日本語配布用 ZIP からは除外します。

背景、カード、見出し、決定ボタンを個別の PNG に分離しています。背景以外の 10 枚は透過 PNG です。元の画像は上書きせず、`../Illustrated-v1/` に保持しています。

## 画像一覧

| ファイル | 内容 |
|---|---|
| `background.png` | UI を取り除いて補完した海辺の背景 |
| `language-english.png` | 英語のカード（枠・魚・文字を含む） |
| `language-japanese.png` | 日本語のカード（枠・魚・文字を含む） |
| `header-language.png` | 日英併記の言語選択見出し |
| `mode-normal-en.png` | ノーマルカード・英語 |
| `mode-hard-en.png` | ハードカード・英語 |
| `mode-normal-ja.png` | ノーマルカード・日本語 |
| `mode-hard-ja.png` | ハードカード・日本語 |
| `header-mode-en.png` | モード選択見出し・英語 |
| `header-mode-ja.png` | モード選択見出し・日本語 |
| `confirm.png` | 両メニュー共通の日英併記決定ボタン |

選択矢印は `Data/UI/Results/Illustrated-v1/selection-arrow.png` を共用しています。

各画像は組み込み画像生成ツールで抽出・再構成しました。文字、装飾、枠は画像内に含まれます。PNG の画素は後処理で塗り替えていません。

## 位置を変更する

`layout.ini` の `[Language]` と `[Mode]` を編集します。座標は論理画面 1600 × 900 px 基準です。

- `HeaderX/Y/Width/Height`：見出し
- `LeftX/Y/Width/Height`：左カード
- `RightX/Y/Width/Height`：右カード
- `ConfirmX/Y/Width/Height`：決定ボタン
- `SelectedLift`：選択時に持ち上がる距離
- `SelectedScalePercent`：選択時の大きさ（103 = 1.03 倍）
- `PressScalePercent`：マウスで押している間の大きさ

幅と高さは収める領域です。画像の縦横比を保持して中央に配置します。メニューへ入り直すと設定を再読込するため、配置変更だけなら再ビルドは不要です。

## 透過余白と入力

`asset-bounds.ini` は各 PNG の透明な余白を除いた描画元矩形です。PNG 自体を切り貼りする代わりに、DxLib の描画元矩形で余白だけを除きます。`asset-bounds.json` には寸法、矩形、完全透明画素数を記録しています。別サイズの PNG に差し替えた場合、寸法が一致しない矩形は使わず画像全体を表示します。

`IllustratedChoiceMenu` が独立した画像を順番に描画します。マウス判定も同じ配置・縦横比・選択時の拡大率から算出します。選択カードは少し浮き、押している画像は小さくなります。キーボード操作と画面遷移は `GamePrototype` が管理します。

必要なレイヤーが読み込めない場合だけ、以前の一枚絵メニューを代替表示します。再挑戦、チュートリアル、難易度のルールは変更していません。

## 生成記録

`PROMPTS.md` に 11 点それぞれの原文プロンプトを保存しています。
