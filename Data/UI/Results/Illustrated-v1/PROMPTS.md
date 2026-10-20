# リザルト画面の画像生成記録

生成方法: 組み込み画像生成ツール。背景、装飾文字、統計欄の見出し、ボタンを一枚の画像に統合しています。数値は実行時に描画します。

画像: lose-en.png / lose-ja.png / win-en.png / win-ja.png。選択カーソル: selection-arrow.png（透過 PNG）。

以下は再生成用の原文プロンプトです。

## resultPromptEnLose

```text
Use case: ui-mockup / production raster game UI asset.
Create one complete 16:9, 1600x900 result screen background for a cute aquatic indie game, intended to be loaded directly by DxLib. This is the ENGLISH LOSS version, not a screenshot with fake data. Image 1 is character identity / simple linework reference; image 2 is game theme reference (wooden seaside Japanese room, blue fish, water).
Style: attractive hand-drawn anime cartoon, bold clean navy outlines, restrained cel shading, charming crafted wooden signs, warm ivory paper, muted teal sea, small rope and shell motifs. Polished but easy for a student artist to redraw. No dense AI painterly detail, no photorealism, no metallic mobile-game chrome, no busy sparkles. Preserve the blue fish design: round body, white belly, three blue dorsal fins, blue head marking, pink cheeks, dark eyes.
LAYOUT is a real functional game menu and must be exact, with no perspective tilt on the UI:
Canvas 1600x900. Calm pale aqua seaside background. Large expressive tired-but-safe blue fish on the left, approximately x130..680 y300..680, resting in a little pool on a wooden jetty; a few simple bubbles and shells. Keep the entire RIGHT half clear of character art.
Top headline centered over left half at x410 y150: exact text "GAME OVER" in large original hand-lettered chunky cream lettering with dark navy outline and soft coral backing ribbon.
At x800..1490, y180..650: straight front-facing wood-and-rope framed ivory score board, clean and flat so numbers added by game code remain readable.
Board top centered x1145 y210 exact small title "DIVE RESULTS".
Five horizontal rows, labels only in LEFT column x850..1100; the RIGHT column x1170..1430 MUST remain completely BLANK ivory (no digits, placeholders, underscores, dashes or decorative marks).
Row label centers Y: 300 "SCORE", 365 "TIME", 430 "KILLS", 495 "LEVEL", 560 "STAGE". Use clear large hand-lettered navy text and small subtle separators between rows.
Two large front-facing rounded hand-drawn wooden buttons in a vertical stack at x820..1470: first y700..765 with EXACT text "RETRY"; second y785..850 with EXACT text "TITLE". Text centered, bold cream hand lettering outlined navy, large and readable. Both same style with teal painted centers, rope/wood edging, small shell detail outside the text zone. No selection cursor baked in; the game will add it.
No other text whatsoever. No numbers anywhere. No fake mouse pointer. No keyboard hints. No extra UI or logo. All static headings, labels, button text and frames must be actually drawn in the image. Do not leave heading labels for code. Keep practical empty value column and stable straight geometry. Full bleed opaque 16:9 landscape.
```

## resultPromptJaLose

```text
Use case: text-localization. Image 1 is the exact edit target, the English loss result-screen game asset. Create its JAPANESE version. Preserve the entire illustration, character identity, shapes, panel positions, dividers, blank value cells, and buttons EXACTLY. Do not redraw or rearrange the UI. Keep the same 16:9 canvas and resolution.
Change ONLY the drawn text, retaining bold hand-lettered cream letters/navy outlines on signs and navy letters on ivory labels:
"GAME OVER" -> "ゲームオーバー"
"DIVE RESULTS" -> "今回の記録"
"SCORE" -> "スコア"
"TIME" -> "時間"
"KILLS" -> "撃破"
"LEVEL" -> "レベル"
"STAGE" -> "ステージ"
"RETRY" -> "リトライ"
"TITLE" -> "タイトル"
Japanese text must be correctly formed and spelled exactly as above, readable chunky hand lettering fitting the SAME existing areas. All right-hand value cells MUST remain completely blank. No extra text, no digits, no keyboard hints. Change only those text regions, keep all other pixels and geometry as close as possible.
```

## resultPromptEnWin

```text
Use case: precise-object-edit. Image 1 is the EXACT base layout for a production game result screen. Create the ENGLISH VICTORY version.
KEEP EXACTLY the same 16:9 canvas, all score board positions and dimensions, every row divider and label, blank value cells, and both button positions/text. Retain all labels "DIVE RESULTS", "SCORE", "TIME", "KILLS", "LEVEL", "STAGE", "RETRY", "TITLE" perfectly unchanged. The whole right-side layout must remain identical because live game values are drawn over it.
Change the left headline from "GAME OVER" to EXACT "VICTORY!" in the same bold hand-drawn cream lettering/navy outline. Change that hanging sign's paint from coral to warm golden yellow/sea green accents, keep shape/location.
Change ONLY the blue fish's expression/pose to joyous achievement: same round blue fish, white belly, three blue dorsal fins, blue forehead mark and pink cheeks, now hovering just above the shallow water with cheerful determined eyes and happy open smiling mouth, fins lifted, small simple water splash underneath. Keep character entirely in the left half and similar size. A few tiny hand-drawn golden celebration stars by its head, restrained, no shower of particles. Keep seaside wooden setting and all UI geometry. Same clean hand-drawn anime cartoon/cel shading, moderate detail easy to redraw. Not photorealistic, not a glossy 3D render.
No extra text or numbers. ALL right value cells still completely blank. All static lettering must remain drawn into the image, no font overlay placeholders.
```

## resultPromptJaWin

```text
Use case: text-localization. Image 1 is the exact edit target: ENGLISH VICTORY result-screen game asset. Create its JAPANESE version. Preserve the entire illustration, happy blue fish, all shapes, score board positions, dividers, blank value cells, and both button positions EXACTLY. Same 16:9 canvas and resolution. Change ONLY the drawn text:
"VICTORY!" -> "勝利！"
"DIVE RESULTS" -> "今回の記録"
"SCORE" -> "スコア"
"TIME" -> "時間"
"KILLS" -> "撃破"
"LEVEL" -> "レベル"
"STAGE" -> "ステージ"
"RETRY" -> "リトライ"
"TITLE" -> "タイトル"
Use correctly formed Japanese exactly as above. Large chunky hand lettering, cream fill/navy outline on signs/buttons and navy on ivory stat labels. Fill the existing headline sign with balanced large "勝利！". Keep all right-hand value cells completely blank. No extra text, no digits, no keyboard hints. Do not alter composition or geometry or fish mood.
```

## resultPromptCursor

```text
Use case: illustration-story. Asset type: a single transparent game menu selection cursor for a cute hand-drawn seaside fish game.
One simple chunky right-pointing arrow made from warm golden-yellow painted wood, a tiny white seashell at its left end and two small aqua bubble accents. Strong dark navy outline, cream highlight, restrained cel shading, same clean anime/cartoon linework as a cozy aquatic wooden UI. Clearly readable at 64 pixels. Broad compact arrowhead, no thin details. Center the complete icon with generous transparent padding; the arrow points horizontally RIGHT. No text, no letters, no numbers, no panel, no background, no drop-shadow rectangle, no checkerboard baked in. Actual transparent alpha. Square canvas.
```
