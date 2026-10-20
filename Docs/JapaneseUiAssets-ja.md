# 日本語 UI 画像の生成記録

組み込みの画像生成ツールで既存画像の文字を置き換えました。外部 API や CLI は使用していません。透過は生成結果のまま保持し、画像ピクセルの後処理はしていません。`asset-bounds.ini` にはアルファ領域の範囲だけを追加しています。

日本語の指示要約：決定ボタンの英語説明を外し、音量ラベルを「音楽」「効果音」に変更します。元の枠、アイコン、色、透明部分、右側のスライダー用空白を保持します。キーボードのキー名は実際の表記を使います。

## 決定ボタン

- 保存先：`Data/UI/ChoiceMenus/Layered-v1/confirm-ja.png`
- 編集元：`Data/UI/ChoiceMenus/Layered-v1/confirm.png`

生成時に渡した指示の原文：

```text
Use case: text-localization. Edit target: the attached transparent game UI asset. Replace the second line "決定 / CONFIRM" with ONLY "決定". Keep the first line exactly "ENTER / SPACE" because those are keyboard key names. Center the Japanese word 決定, bold cream lettering outlined navy, in the same attractive hand drawn style. Preserve the full original wide canvas aspect ratio, transparent margins, wooden frame, rope knots, cream/turquoise background, all decorative details, placement and existing hand-drawn game art style. Change only the specified text. No new words, no watermark. Preserve transparency. Return a single standalone ready-to-use UI asset, not a mockup.
```

## 音楽音量

- 保存先：`Data/UI/Pause/Layered-v1/row-music-ja.png`
- 編集元：`Data/UI/Pause/Layered-v1/row-bgm.png`

生成時に渡した指示の原文：

```text
Use case: text-localization. Edit target: the attached transparent game UI asset. Replace ONLY the letters "BGM" with the exact Japanese word "音楽". Keep the turquoise musical-note icon. The label must occupy the same left-side area as the original, leaving the right two thirds empty for a runtime slider. Preserve the full original wide canvas aspect ratio, transparent margins, wooden frame, rope knots, cream/turquoise background, all decorative details, placement and existing hand-drawn game art style. Change only the specified text. No new words, no watermark. Preserve transparency. Return a single standalone ready-to-use UI asset, not a mockup.
```

## 効果音音量

- 保存先：`Data/UI/Pause/Layered-v1/row-sfx-ja.png`
- 編集元：`Data/UI/Pause/Layered-v1/row-sfx.png`

生成時に渡した指示の原文：

```text
Use case: text-localization. Edit target: the attached transparent game UI asset. Replace ONLY the letters "SFX" with the exact Japanese word "効果音". Keep the turquoise speaker icon. Fit the three bold Japanese characters entirely in the left-side label area; leave the right two thirds empty for a runtime slider. Preserve the full original wide canvas aspect ratio, transparent margins, wooden frame, rope knots, cream/turquoise background, all decorative details, placement and existing hand-drawn game art style. Change only the specified text. No new words, no watermark. Preserve transparency. Return a single standalone ready-to-use UI asset, not a mockup.
```
