# 分離リザルト画面の画像生成記録

組み込み画像生成ツールで、各見出し、魚、統計ボード、ボタンを個別の透過 PNG として抽出しました。背景と選択矢印は既存の分離メニュー素材を共用します。

## fish-win

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the HAPPY BLUE FISH with its small splash and two little golden stars from the LEFT of the victory screen as one standalone transparent PNG. Preserve the round blue body, white belly, three blue dorsal fins, blue forehead marking, pink cheeks, joyful eyes, smiling open mouth and raised fins, and a compact little water splash directly below. Remove all wooden floor, scenery, UI signs, score board, buttons, ropes and text. Everything outside fish/splash/star silhouettes must be genuine transparent alpha, including gaps between stars and fish. Do not include a large water oval or rectangular scene. Keep full fish, all fins, stars and splash uncut, centered on square transparent canvas with safe padding. Same clean navy-outlined hand-drawn anime cartoon and simple cel shading as source, no extra characters, no text, no borders or panels. One independent character illustration layer for a modular game result UI.
```

## fish-lose

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the TIRED BLUE FISH and its compact little water puddle from the LEFT of the defeat screen into one standalone transparent PNG. Preserve the rounded blue body, white belly, three blue dorsal fins, blue forehead marking, pink cheeks, droopy gentle eyes and small worried mouth, resting safely in a shallow puddle, with a few bubbles attached nearby. Keep the complete fish and fins uncut. Remove every wooden floor/scenery region, shell foreground prop, UI sign, frame, score board, buttons and text. All space outside the fish, tiny puddle and nearby bubble silhouettes must be actual transparent alpha; no rectangular background or large scene. Center the whole illustration on square transparent canvas with safe padding. Same clean navy-outlined anime cartoon and restrained cel shading as original, no extra characters or text. Single independent character layer for a modular result UI.
```

## header-win-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the TOP LEFT VICTORY hanging wooden sign as one independent UI PNG. Keep golden painted wood, dark navy outline, attached small shells/droplets/leaves and short rope loops, with EXACT large cream hand lettering "VICTORY!". Remove any long ropes connecting to ceiling and the ceiling itself. All space outside the sign silhouette must be actual transparent alpha. No fish, stars away from the sign, scenery, board, buttons, floor or other text. Whole sign front-facing, wide horizontal composition about 3.5:1, with transparent padding, no clipped wood/ornaments. Preserve original clean cartoon style and spelling. One header asset, no atlas or full-screen scene.
```

## header-lose-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its TOP LEFT coral-painted GAME OVER wooden sign as one independent transparent PNG. Preserve complete wooden silhouette, dark navy outline, tiny droplets and short rope loops, and EXACT large cream lettering "GAME OVER". Remove ceiling, long ropes, fish, any scenery, board, buttons, floor and all other text. Everything outside the header silhouette must be genuine transparent alpha. One wide front-facing horizontal sign, approximately 3.5:1 width:height, centered with transparent safety margins and no cut-off wood, ropes or decorations. Keep source cartoon linework, proportions and lettering. Not an atlas or finished menu.
```

## header-win-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its TOP LEFT JAPANESE VICTORY wooden sign as one independent transparent PNG. Preserve the golden-painted wood, attached shells/droplets, dark navy outline, short rope loops and EXACT large cream hand lettering "勝利！". Japanese glyphs must remain correctly formed. Remove ceiling and long ropes, fish, scenery, score board, buttons, floor and other text. All space outside the sign silhouette must be genuine transparent alpha. Single wide front-facing horizontal sign about 3.5:1, centered with transparent safety padding, no clipped wood or decorations. Preserve source spelling and clean anime cartoon style. Not a full-screen scene or atlas.
```

## header-lose-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its TOP LEFT JAPANESE GAME OVER wooden sign as one independent transparent PNG. Preserve coral painted wood, dark navy outline, attached tiny droplets and short rope loops, EXACT large cream hand lettering "ゲームオーバー". Keep Japanese correctly formed. Remove ceiling and long ropes, every fish/scenery/board/button/floor region and other text. Everything outside the complete header silhouette must be actual transparent alpha. Single wide front-facing horizontal sign about 3.5:1, centered with transparent safety padding, no clipping. Preserve source style and proportions. One isolated header, not an atlas or full menu.
```

## board-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the large RIGHT HAND SCORE BOARD as one independent transparent PNG. Preserve its entire front-facing wooden/rope frame, attached small shells/leaves, opaque ivory paper interior, vertical label/value divider, horizontal row dividers, the top sign EXACT "DIVE RESULTS", and five row labels EXACT "SCORE", "TIME", "KILLS", "LEVEL", "STAGE" in that order. The complete RIGHT COLUMN must remain blank ivory: no digits, placeholder lines, dashes, fake values or icons. All space OUTSIDE the board silhouette is genuine transparent alpha. No fish, scenery, floor, victory header, buttons or other text. Keep five evenly spaced rows and same label/value proportions as source, straight-on board about 1.4:1 width:height, centered with modest transparent safety margins, nothing clipped. Clean source cartoon style. This is one score-board UI layer to accept live numeric overlays, not a full-screen composition or atlas.
```

## board-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the large RIGHT HAND JAPANESE SCORE BOARD as one independent transparent PNG. Keep full wooden/rope frame, attached shell/leaf decorations, opaque ivory paper inside, top sign EXACT "今回の記録", and five labels EXACT "スコア", "時間", "撃破", "レベル", "ステージ" in that order. Preserve five evenly spaced rows, horizontal separators and vertical label/value divider in the same proportions as source. RIGHT VALUE COLUMN completely BLANK ivory, no digits, placeholders, underscores, dashes, icons or fake values. All outside the complete board silhouette must become genuine transparent alpha. No fish, scenery, floor, game-over header, buttons or extra writing. Single straight front-facing board about 1.4:1 width:height centered with transparent margins, nothing clipped. Correctly formed readable Japanese, source clean cartoon linework. One modular scoreboard asset, not an atlas or full screen.
```

## button-retry-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its upper teal wooden "RETRY" button, underneath the score board, as ONE independent transparent PNG. Keep full rounded rope/wood edging, teal painted center, attached starfish, shell, leaves and bubbles, with EXACT cream/navy hand lettering "RETRY". Everything outside the button silhouette must be genuine transparent alpha. No floor, water, fish, score board, other button, header or additional writing. Center the entire horizontal front-facing button around 5:1 width:height with transparent safety padding, no clipped decorations or border. Preserve source cartoon style and proportions. No cursor, glow, perspective or atlas.
```

## button-title-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its lower teal wooden "TITLE" button as ONE independent transparent PNG. Preserve whole rounded rope/wood edging, teal center, shell/leaf/bubble decorations and EXACT cream/navy hand lettering "TITLE". Genuine transparent alpha everywhere outside the button silhouette. No floor, scenery, fish, board, other button, header, or additional writing. One complete horizontal straight-on button around 5:1 width:height, centered with transparent safety margins, no clipping. Keep source cartoon linework, proportions and spelling. No selection cursor, glow, perspective or atlas.
```

## button-retry-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its upper teal wooden Japanese retry button as ONE independent transparent PNG. Keep complete rounded wood/rope edging, teal painted center, attached starfish, shell, leaves and bubbles, with EXACT cream/navy Japanese hand lettering "リトライ". Japanese glyphs must remain correct. Genuine transparent alpha everywhere outside the button silhouette. No floor, fish, scenery, score board, other button, header, extra writing, glow or cursor. Single complete horizontal front-facing button around 5:1 width:height centered with transparent safety padding, no clipping. Preserve source cartoon style and proportions. Not a full-screen composition or atlas.
```

## button-title-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its lower teal wooden Japanese title button as ONE independent transparent PNG. Preserve rounded wood/rope border, teal center, shell, leaves and bubble decorations and EXACT cream/navy Japanese lettering "タイトル". Correct Japanese glyphs. All outside the button silhouette must be actual transparent alpha. Remove floor, fish, scenery, score board, other button, header and all additional text. Single full horizontal straight-on button about 5:1 width:height centered with transparent margins, no clipped ornaments. Preserve source cartoon linework and proportions. No selection cursor, glow or atlas.
```


## board-stage-ja（区間表示版）

組み込み画像生成ツールの編集モードを使用。編集元は `board-ja.png`、保存先は `board-stage-ja.png`。1484×1060、透過背景、描画矩形は元画像と同じです。元ファイルと日本語素材のコピーは変更していません。

```text
Use case: text-localization. Asset type: production modular Japanese game results scoreboard, transparent PNG. Image 1 is the exact edit target. Change ONLY fourth row label from "レベル" to exact Japanese "区間". Preserve the other text exactly: header "今回の記録" and labels "スコア", "時間", "撃破", "ステージ". Keep the five row positions, row separators, divider, overall frame silhouette, shells, leaves, rope knots, cream paper, navy lettering, cartoon linework, proportions and blank right value column unchanged. Keep genuine transparent alpha outside the complete scoreboard silhouette, no clipped edges. No digits, added writing, backgrounds, shadows outside the silhouette, watermarks or extra objects. The fourth row must read exactly "区間" in the same dark navy hand-lettered style.
```
