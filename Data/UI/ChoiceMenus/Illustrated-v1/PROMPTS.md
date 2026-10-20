# 選択メニューの画像生成記録

組み込み画像生成ツールで制作した、言語選択とモード選択の原文プロンプトです。枠、見出し、説明文、ボタン文字は画像に含まれます。

## choicePromptLanguage

```text
Use case: ui-mockup, production raster game menu asset. Create one complete 16:9 landscape 1600x900 language-selection screen for a cute fish game, ready to load directly as an opaque PNG. Image 1 is the exact established UI STYLE reference: hand-drawn wooden seaside signs, rope borders, chunky cream/navy lettering, clean cel shading. Image 2 and image 3 are reference illustrations for the LEFT English choice and RIGHT Japanese choice, preserve their blue fish identity and motifs. They are style/subject references, not edit targets.
Make it attractive, clean bold anime/cartoon outlines, moderate detail easy to redraw. Warm wooden seaside room and turquoise ocean backdrop, not photorealistic, not painterly, no glossy mobile-game chrome.
Composition is a straight-on functional menu, no perspective tilt on the UI.
Top central hanging wooden sign, x390..1210 y35..180, with large hand-lettered Japanese EXACT "言語を選ぼう" and smaller English EXACT "CHOOSE YOUR LANGUAGE".
Two equally prominent large wooden picture cards side by side:
LEFT card approximately x180..750 y220..735, rope/wood frame, illustration of the same cute blue round fish with white belly, blue forehead stripe, pink cheeks, navy eyes, beside an open book in a sunny wooden cabin with round ocean window. Large bottom name plaque reads EXACT "ENGLISH", and smaller subtitle EXACT "英語". Small wooden corner badge EXACT "1".
RIGHT card approximately x850..1420 y220..735, same frame, illustration of that same blue fish by a red lantern, a few cherry blossoms and seaside water. Large bottom name plaque reads EXACT "日本語", smaller subtitle EXACT "JAPANESE". Small wooden corner badge EXACT "2".
Keep both cards neutral with identical frame weight, no baked selection cursor or glow. Both whole picture cards will be clickable.
Bottom central teal wood-and-rope confirm button approximately x545..1055 y780..850, two text lines EXACT "ENTER / SPACE" and "決定 / CONFIRM", large clear hand lettering.
Leave clear uncluttered space BELOW each card, centered x465 and x1135 y790..885, for the game to overlay an animated selection arrow. These zones must not contain text or objects. Do not draw arrows or selected marks.
All of the headline, names, subtitles, badges, confirm button and lettering must actually be drawn into the image. No additional text, no flags, no logos, no stats, no extra characters. Text must be spelled exactly, legible Japanese. Full-bleed 16:9 landscape.
```

## choicePromptModeEn

```text
Use case: ui-mockup, production raster game menu asset. Image 1 is the established language-menu STYLE and layout reference. Make a matching ENGLISH MODE SELECTION screen for the same cute blue-fish game, full-bleed opaque 16:9 landscape 1600x900. This is the next menu after the tutorial.
Preserve the same cozy wooden seaside environment, warm wood and rope frames, shells, teal confirm button, cream and navy hand lettering, clean anime/cartoon outlines, restrained cel shading. Moderate attractive detail that can be redrawn, no photorealism or painterly AI texture. Use the same blue round fish, white belly, blue forehead stripe, pink cheeks, large dark eyes in both choices.
Top center wooden sign exact large text "CHOOSE YOUR DIVE". Under it a slim cream ribbon exact smaller text "Tutorial cleared - upgrades carry over".
Two equally sized tall straight-on wooden picture cards side by side, neutral with NO selected marks:
LEFT approximately x170..750 y220..745: calm turquoise underwater scene, blue fish swimming confidently in warm sunlight with a friendly large whale in the distance. Compact illustration occupies upper part of the card; lower part is an ivory parchment rules panel. Large wooden name plaque reads EXACT "NORMAL". Three short hand-lettered navy rule lines on ivory, EACH on its own line, exact:
"Choose upgrades each stage"
"Enemies reset each stage"
"30 stages / Boss every 5"
Small top-left wooden badge "1".
RIGHT approximately x850..1430 y220..745: dramatic deep navy/teal underwater scene, same blue fish with determined expression, distant looming shark silhouette, a little muted coral-red underwater light. Avoid horror or excessive effects. Same ivory rules panel and same neutral frame weight. Large name plaque exact "HARD". Three exact rule lines:
"1.5x damage taken"
"Random upgrades in battle"
"Enemies stay between stages"
Small top-left wooden badge "2".
The two fish illustrations MUST be distinct but the characters must match. Keep names and short rules clearly legible, generously spaced and aligned. Do not omit or duplicate rule lines.
Bottom center teal wood-and-rope button approximately x545..1055 y780..850, EXACT two-line text "ENTER / SPACE" then "DIVE IN".
Leave uncluttered empty wood floor BELOW the two cards, around x360 and x1240 y805..900, for runtime selection arrows. No baked arrows, no cursor, no flags, no extra text, no fake UI values. All signs, frames and every specified text element must actually be generated inside the image.
```

## choicePromptModeJa

```text
Use case: text-localization. Image 1 is the exact edit target: English mode-selection menu PNG for a cute aquatic game. Produce its Japanese version, retaining the EXACT same 16:9 canvas, all geometry, both fish illustrations, cards, rope and wood frames, badges, shells and bottom confirm button. Change ONLY the drawn lettering. Do not alter card/button locations, frame weight or illustration.
Exact replacements:
"CHOOSE YOUR DIVE" -> "次の海を選ぼう"
"Tutorial cleared - upgrades carry over" -> "チュートリアルクリア！強化を引き継ぎます"
"NORMAL" -> "ノーマル"
"HARD" -> "ハード"
"Choose upgrades each stage" -> "毎ステージ強化を選択"
"Enemies reset each stage" -> "毎ステージ敵をリセット"
"30 stages / Boss every 5" -> "全30ステージ／5ステージごとにボス"
"1.5x damage taken" -> "受けるダメージ1.5倍"
"Random upgrades in battle" -> "戦闘中にランダム強化"
"Enemies stay between stages" -> "ステージを越えて敵が残る"
"DIVE IN" -> "スタート"
Keep "ENTER / SPACE", badge "1" and badge "2" unchanged.
Use accurately formed, correctly spelled Japanese, chunky hand lettering matching existing style. Preserve each rule on a single separate line, center and fit text within existing ivory rows. No omitted or duplicated text, no extra letters, no arrow or selected marker. Everything else unchanged.
```
