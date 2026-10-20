# 分離レイヤーの画像生成記録

組み込み画像生成ツールで、背景を再構成し、各カード・見出し・ボタンを個別の透過 PNG として抽出しました。元の承認済み画面は `../Illustrated-v1/` に保持しています。

## background

```text
Use case: precise-object-edit. Image 1 is the edit target, a flat game menu. Turn it into the BACKGROUND-ONLY layer for a modular game UI. REMOVE every UI element: both large framed picture cards and their illustrations/name plaques/badges, the large central header wooden sign and its hanging ropes, the teal confirm button, all writing, all selection markers. Reconstruct the scenery behind them seamlessly: an open cozy wooden seaside veranda, roof beams at upper edges, green vines at sides, a little hanging wind chime far left, broad open turquoise ocean and sunny sky in the middle, a distant wooden coastal village far right, and a continuous wooden plank floor across the bottom quarter. Preserve the existing clean anime cartoon drawing style, dark navy outlines, restrained cel shading, warm brown wood, turquoise water and sky. Keep the CENTER and broad left/right middle areas calm and clear for separate UI panels to be overlaid later. No fish characters, no frames, no signs, no boards, no labels, no button, no text, no numbers anywhere. Full-bleed opaque 16:9 landscape, same framing as original, no transparent holes. Do not leave ghosts or silhouettes of removed elements. This is a usable empty game-menu background, not a finished menu.
```

## language-english

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the LEFT ENGLISH picture card as ONE independent game UI asset. Include its complete wooden/rope frame, badge "1", blue fish and open book/ocean-window illustration inside the frame, the bottom name plaque with exact lettering "ENGLISH" and "英語", and the attached shells/leaves. Preserve the existing artwork, spelling, style and proportions closely. Everything OUTSIDE that single card must be genuinely transparent alpha. No veranda scenery outside its wooden outline, no floor, no other card, no header, no confirm button. Keep the inside of the picture card and its wooden name plate opaque. Center the complete card, no clipped frame, with a modest clear transparent margin. Card approximately 6:5 width:height, straight front-facing, no perspective. No glow or selection cursor. This is a single isolated sprite PNG, not a full-screen menu, not an atlas.
```

## language-japanese

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the RIGHT JAPANESE picture card as ONE independent transparent game UI asset. Keep its entire wood/rope frame, corner badge "2", blue fish illustration with red lantern, cherry blossoms and water inside, bottom wooden plaque with exact "日本語" and "JAPANESE", attached shells and leaves. Preserve existing style, spelling and proportions closely. Remove everything OUTSIDE the single card silhouette to genuine transparent alpha: no veranda behind it, no floor, no English card, no header and no confirm button. The scene INSIDE the card must remain opaque. Center the complete card on a landscape canvas, approximately 6:5 card width:height, with a modest transparent safety margin; do not clip the leaves, badge or ropes. Straight front-facing, no perspective distortion, no selection glow/cursor. One isolated card sprite PNG, no atlas, no other UI.
```

## header-language

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its TOP CENTER LANGUAGE HEADER wooden sign as one independent UI sprite. Preserve the golden wooden board, small shells/leaves/starfish/droplets attached, and exact hand-lettered text "言語を選ぼう" with smaller "CHOOSE YOUR LANGUAGE" beneath it. Keep a little of the two short rope loops attached to the top edge, but NO roof beam and NO long ropes going to the canvas edge. Make all space outside the header silhouette genuinely transparent alpha. No cards, no background, no fish, no button, no other text. Single wide horizontal sign, approximately 4:1 width:height, front-facing and centered with transparent padding so no leaf or rope is clipped. Preserve original spelling and clean navy-outlined cartoon style. Not an entire menu or a sprite sheet.
```

## confirm

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the BOTTOM TEAL CONFIRM BUTTON as one independent PNG with genuine transparent alpha outside its silhouette. Keep its complete wooden and rope border, turquoise painted wood, attached shells, green leaves, bubbles and EXACT two lines of lettering "ENTER / SPACE" and "決定 / CONFIRM". Preserve original style, spelling and proportions. Do not include any floor or scene around the button. No header, no cards, no extra text, no other objects, no rectangular background. Single wide front-facing horizontal button, roughly 4.5:1 width:height, centered on a transparent canvas with modest safe padding. All wood, rope and letters fully visible and opaque inside the button; do not clip any decorative edge. No cursor or selection glow. This sprite will be reused in both language and mode menus.
```

## mode-normal-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the LEFT NORMAL card, including complete wood/rope frame, corner badge "1", illustration of blue fish with distant friendly whale in turquoise sea, the wooden name plaque and ivory rules area with its decorations. Preserve spelling and style exactly: "NORMAL", "Choose upgrades each stage", "Enemies reset each stage", "30 stages / Boss every 5". Keep all three rules on their separate lines. Everything outside that single card silhouette must become genuinely transparent alpha. Keep the scene and ivory rules panel INSIDE the frame fully opaque. No scenery around the card, no floor, no header sign, no confirm button, no Hard card, no selection cursor. Center the whole straight-on card, approximately 6:5 width:height, on a transparent landscape canvas with safe padding, no clipped badge, rope, shells or leaves. One isolated game UI card PNG, not an atlas or full-screen composition.
```

## mode-hard-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the RIGHT HARD card as one isolated transparent UI PNG. Preserve its entire wooden/rope frame, corner badge "2", determined blue fish with looming shark in dark teal sea illustration, name plaque "HARD", ivory panel and EXACT three separate rule lines "1.5x damage taken", "Random upgrades in battle", "Enemies stay between stages". Keep the shells, starfish and leaves attached to the frame. Everything outside the card silhouette must be genuinely transparent alpha, while the illustration and ivory panel inside remain opaque. No surrounding veranda, no floor, no header, no confirm button, no Normal card, no cursor. Center the complete straight-on card on a landscape transparent canvas with modest safety padding, no cropped frame decorations. Approximately 6:5 width:height. Preserve existing spelling, artwork and clean navy-outlined cartoon style, one standalone card only, not an atlas.
```

## mode-normal-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY the LEFT JAPANESE NORMAL card as one isolated transparent game UI PNG. Preserve the wood/rope frame, corner badge "1", happy blue fish with distant friendly whale illustration, shells/leaves, wooden name plaque exact "ノーマル", and ivory panel with EXACT three separate lines "毎ステージ強化を選択", "毎ステージ敵をリセット", "全30ステージ／5ステージごとにボス". Japanese glyphs must remain accurately formed and all three lines readable. Everything outside the card silhouette must be genuine transparent alpha; inside the illustration and ivory panel remain opaque. No scene around the card, no floor, no other card, no header, no button, no cursor. Whole card centered, straight front-facing, approximately 6:5 width:height, landscape canvas with modest transparent margins, no clipped decorations. Preserve existing artwork and proportions, not an atlas or full-screen menu.
```

## mode-hard-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its RIGHT JAPANESE HARD card. Include the whole wooden/rope frame, corner badge "2", determined blue fish with looming shark illustration, wooden name plaque EXACT "ハード", and ivory panel with EXACT three separate lines "受けるダメージ1.5倍", "戦闘中にランダム強化", "ステージを越えて敵が残る", plus attached shells/leaves/starfish. Japanese must remain accurately formed and legible. Make everything OUTSIDE the card silhouette genuinely transparent alpha; keep all interior picture and ivory areas opaque. No veranda, floor, header, other card, button or cursor. Whole card front-facing, approximately 6:5 width:height, centered on landscape transparent canvas with modest safe padding, no clipped decoration. Preserve source artwork/proportions; one isolated UI card, not an atlas.
```

## header-mode-en

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its TOP CENTER MODE HEADER wooden sign as ONE independent transparent PNG. Preserve the golden wood, tiny shell/starfish/leaf decorations, attached short rope loops (no roof or long ropes to image edge), large exact lettering "CHOOSE YOUR DIVE", and the integrated ivory lower strip exact "Tutorial cleared - upgrades carry over". Preserve lettering exactly and existing clean anime cartoon style. Everything outside this header silhouette must be genuine transparent alpha. No cards, fish, background, floor, button, extra letters or cursor. Single wide horizontal front-facing sign, around 4:1 width:height, centered with clear safety margins. Include the entire sign and all attached decorations without clipping. Not a full-screen image or atlas.
```

## header-mode-ja

```text
Use case: background-extraction. Image 1 is the exact edit target. Extract ONLY its TOP CENTER JAPANESE MODE HEADER wooden sign as ONE independent transparent game UI sprite. Preserve golden wooden board, attached shell/starfish/leaf/droplet decorations and short rope loops (no ceiling or long ropes continuing to canvas edge). Preserve EXACT Japanese headline "次の海を選ぼう" and ivory subtitle strip "チュートリアルクリア！強化を引き継ぎます". Keep all Japanese glyphs correctly formed and readable, and the clean navy-outlined anime cartoon style. Everything outside the sign silhouette must become genuine transparent alpha. No background, cards, fish, floor, button, extra text or cursor. Single wide straight-on horizontal sign around 4:1 width:height, centered with modest transparent safe margins, no clipped edges or ornaments. Not an entire menu or atlas.
```
