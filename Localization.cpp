#include "Localization.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string_view>
#include <unordered_map>

namespace
{
    struct Entry { const wchar_t* en; const wchar_t* ja; };

    // Bảng EN -> JP (日本語). Thêm chuỗi mới: thêm 1 dòng tại đây và bọc chuỗi trong code bằng TR(L"...").
    const Entry kTable[] = {
        { L"NORMAL STAGE %d / %d", L"ノーマル ステージ %d / %d" },
        { L"HELL STAGE %d / %d", L"ヘル ステージ %d / %d" },
        { L"TUTORIAL %d / %d", L"チュートリアル %d / %d" },
        { L"TIME LEFT: %02ds", L"残り時間: %02d秒" },
        { L"ENEMIES: %d | KILLS: %d", L"敵: %d | 撃破: %d" },
        { L"[STATS]  Lv %d  EXP %d/%d", L"[ステータス]  Lv %d  EXP %d/%d" },
        { L"ATK +%d   SLAM %.0f  BULLET %.0f", L"攻撃 +%d   スラム %.0f  弾 %.0f" },
        { L"DEF +%d   -%d%% dmg taken   HP %.0f", L"防御 +%d   被ダメ -%d%%   HP %.0f" },
        { L"SPD +%d   %.0f", L"速度 +%d   %.0f" },
        { L"CD  +%d   jump %.2fs", L"CD  +%d   ジャンプ %.2f秒" },
        { L"LUCK +%d  rarity +%d", L"運 +%d  レア度 +%d" },
        { L"[ACTIVE CARDS]:", L"[所持カード]:" },
        { L"- %s  Lv %d", L"- %s  Lv %d" },
        { L"[WASD]: Swim  |  [SPACE]: Jump Slam  |  [A/D+Enter / 1,2,3]: Pick Cards  |  [R]: Reset  |  [F1]: Debug  |  [F2]: Shake Cam  |  [F11]: Panels", L"[WASD]: 泳ぐ  |  [SPACE]: ジャンプスラム  |  [A/D+Enter / 1,2,3]: カード選択  |  [R]: リセット  |  [F1]: デバッグ  |  [F2]: 画面揺れ  |  [F11]: パネル" },
        { L"=== ODORU : JUST A FISH GAME ===", L"=== オドル : ジャスト・ア・フィッシュ・ゲーム ===" },
        { L"CORE MECHANIC: JUMP-TO-ATTACK", L"基本アクション: ジャンプで攻撃！" },
        { L"- Controls: [WASD] or Arrow Keys to swim smoothly in deep waters.", L"- 操作: [WASD] または矢印キーで深海をスイスイ泳ごう。" },
        { L"- Attack: Press [SPACE] to launch into the air and slam down!", L"- 攻撃: [SPACE] で空中へ跳び、真下へスラム！" },
        { L"- Aerial Invincibility: While airborne, completely immune to damage.", L"- 空中無敵: 空中にいる間はダメージを受けない。" },
        { L"- Landing Shockwave: Deals 40 damage, knocks back and breaks armor.", L"- 着地衝撃波: 40ダメージ＋ノックバック＋装甲破壊。" },
        { L"> [1] STORY MODE :", L"> [1] ストーリー :" },
        { L"  [1] STORY MODE :", L"  [1] ストーリー :" },
        { L"Tutorial (3x15s) + Whale, then Normal / Hell", L"チュートリアル(15秒x3)＋クジラ → ノーマル/ヘル" },
        { L"> [2] NORMAL MODE :", L"> [2] ノーマル :" },
        { L"  [2] NORMAL MODE :", L"  [2] ノーマル :" },
        { L"30 Stages (20s), Stacking Cards, Boss every 5", L"全30ステージ(20秒)・カード重ねがけ・5ステージ毎にボス" },
        { L"> [3] HELL MODE :", L"> [3] ヘル :" },
        { L"  [3] HELL MODE :", L"  [3] ヘル :" },
        { L"Dmg taken x1.5, Random Cards, No Wipe", L"被ダメージx1.5・ランダムカード・敵が残る" },
        { L"[NORMAL RECORD]: Max Stage %d  |  Kills: %d  |  Time: %02d:%02d  |  Score: %d", L"[ノーマル記録]: 最高ステージ %d  |  撃破: %d  |  時間: %02d:%02d  |  スコア: %d" },
        { L"[HELL RECORD]: Max Stage %d  |  Kills: %d  |  Score: %d", L"[ヘル記録]: 最高ステージ %d  |  撃破: %d  |  スコア: %d" },
        { L"Press [1] [2] [3], or [UP/DOWN] + [ENTER/SPACE] to START!", L"[1][2][3] または [↑/↓]+[ENTER/SPACE] でスタート！" },
        { L"[F1]: Toggle Debug HUD  |  [F2]: Toggle Camera Shake  |  [F11]: Panels  |  [L]: Language", L"[F1]: デバッグ表示  |  [F2]: 画面揺れ  |  [F11]: パネル  |  [L]: 言語" },
        { L"TUTORIAL CLEAR!  CHOOSE YOUR MODE", L"チュートリアルクリア！ モードを選ぼう" },
        { L"Your cards and the whale-clear heal carry over.", L"カードとクジラ撃破の回復は引き継がれます。" },
        { L"[1] NORMAL", L"[1] ノーマル" },
        { L"[2] HELL", L"[2] ヘル" },
        { L"30 stages (20s), pick 1 of 3 cards, field cleared each stage, boss every 5", L"全30ステージ(20秒)・3枚から1枚選択・毎ステージ敵リセット・5毎にボス" },
        { L"Damage taken x1.5, random card (no pause), enemies stay between stages", L"被ダメージx1.5・ランダムカード(停止なし)・敵はステージを越えて残る" },
        { L"[UP/DOWN] + [ENTER/SPACE], or [1] / [2]", L"[↑/↓] + [ENTER/SPACE] または [1] / [2]" },
        { L"+1 ATTACK", L"攻撃 +1" },
        { L"+1 DEFENSE", L"防御 +1" },
        { L"+1 SPEED", L"速度 +1" },
        { L"+1 COOLDOWN", L"クールダウン +1" },
        { L"+1 LUCK", L"運 +1" },
        { L"LEVEL UP!  Lv %d", L"レベルアップ！  Lv %d" },
        { L"HELL CARD GET!", L"ヘルカード獲得！" },
        { L"HELL ROULETTE...", L"ヘルルーレット…" },
        { L"MOVE: Swim with [WASD] / Arrow keys", L"移動: [WASD] / 矢印キーで泳ぐ" },
        { L"JUMP / SLAM: Press [SPACE] - airborne = invincible, landing hits", L"ジャンプ/スラム: [SPACE] ― 空中は無敵、着地で攻撃" },
        { L"ATTACK: Land a slam on an enemy to defeat it", L"攻撃: 敵の上にスラムして倒そう" },
        { L"CARDS: Pick 1 upgrade - [A/D] + [ENTER] or [1] [2] [3]", L"カード: 強化を1枚選ぶ ― [A/D]+[ENTER] または [1][2][3]" },
        { L"BOSS: Dodge its attacks in the air, slam it when it is open!", L"ボス: 空中で攻撃をかわし、スキを見てスラム！" },
        { L"[TAB] Skip tips", L"[TAB] ヒントをスキップ" },
        { L"! WARNING: DEEP SEA ENERGY ANOMALY !", L"！ 警告: 深海エネルギー異常 ！" },
        { L"BOSS: GREAT SHARK IS HUNTING YOU!", L"ボス: 巨大ザメが狙っている！" },
        { L"FINAL BOSS: GIANT WHALE HAS APPEARED!", L"ラスボス: 巨大クジラが現れた！" },
        { L"The battle for survival begins now...", L"生き残りをかけた戦いが今始まる…" },
        { L"PAUSED", L"一時停止" },
        { L"Resume", L"再開" },
        { L"BGM Volume:  < %3d >", L"BGM音量:  < %3d >" },
        { L"SFX Volume:  < %3d >", L"効果音量:  < %3d >" },
        { L"Screen:  < %s >", L"画面:  < %s >" },
        { L"Window", L"ウィンドウ" },
        { L"Fullscreen", L"フルスクリーン" },
        { L"Return to Title", L"タイトルへ戻る" },
        { L"[W/S]: Select  [A/D]: Adjust  [Enter]: OK  [ESC/P]: Resume", L"[W/S]: 選択  [A/D]: 調整  [Enter]: 決定  [ESC/P]: 再開" },
        { L"=== ALL CARDS FULLY STACKED! ===", L"=== 全カード最大まで強化済み！ ===" },
        { L"Card pool is empty. Received 20% Max HP Heal bonus!", L"カードがもうありません。最大HPの20%回復ボーナス！" },
        { L"Press [ENTER] to continue swimming...", L"[ENTER] で泳ぎを続ける…" },
        { L"=== CHOOSE 1 UPGRADE CARD (WAVE CLEARED!) ===", L"=== 強化カードを1枚選ぼう (ウェーブクリア！) ===" },
        { L"Press [Enter] or [1] to confirm upgrade", L"[Enter] または [1] で決定" },
        { L"Use [A/D] to select + [Enter] to confirm, or press keys [1] / [2]", L"[A/D] で選択 + [Enter] で決定、または [1] / [2]" },
        { L"Use [A/D] to select + [Enter] to confirm, or press keys [1] / [2] / [3]", L"[A/D] で選択 + [Enter] で決定、または [1] / [2] / [3]" },
        { L"Key [%d] %s (x%d/%d)", L"キー [%d] %s (x%d/%d)" },
        { L"Key [%d]  %s", L"キー [%d]  %s" },
        { L"[NO IMAGE]", L"[画像なし]" },
        { L"> SELECTED (ENTER) <", L"> 選択中 (ENTER) <" },
        { L"=== ACTIVE CARDS ===", L"=== 所持カード ===" },
        { L"COMMON", L"コモン" },
        { L"RARE", L"レア" },
        { L"EPIC", L"エピック" },
        { L"Deep Trench", L"深海の溝" },
        { L"+ Slam Radius +25%", L"+ スラム範囲 +25%" },
        { L"Heavy Slam", L"ヘビースラム" },
        { L"+ Slam Damage +40%", L"+ スラムダメージ +40%" },
        { L"Nimble Leap", L"身軽なジャンプ" },
        { L"+ Jump Cooldown x0.8", L"+ ジャンプCD x0.8" },
        { L"Swift Fins", L"疾風のヒレ" },
        { L"+ Swim Speed +15%", L"+ 泳ぐ速さ +15%" },
        { L"Thick Scales", L"分厚いウロコ" },
        { L"+ Max HP +30%", L"+ 最大HP +30%" },
        { L"Echo Wave", L"エコーウェーブ" },
        { L"+ Slam Radius +35%, Push +30%", L"+ スラム範囲 +35%、押し出し +30%" },
        { L"Puffer Needles", L"フグの針" },
        { L"+ Counter 8 Spikes on Hit", L"+ 被弾時にトゲ8本で反撃" },
        { L"Crushing Impact", L"粉砕インパクト" },
        { L"+ Knockback +100%, DMG +30%", L"+ ノックバック +100%、ダメージ +30%" },
        { L"VICTORY!", L"勝利！" },
        { L"GAME OVER", L"ゲームオーバー" },
        { L"You defeated the Giant Whale!", L"巨大クジラを倒した！" },
        { L"Fell before the Giant Whale...", L"巨大クジラの前に力尽きた…" },
        { L"Your fish was stranded...", L"お魚は打ち上げられてしまった…" },
        { L"%d / %d  CLEAR", L"%d / %d  クリア" },
        { L"Stage %d", L"ステージ %d" },
        { L"Tutorial %d / %d", L"チュートリアル %d / %d" },
        { L"TIME", L"時間" },
        { L"KILLS", L"撃破" },
        { L"LEVEL", L"レベル" },
        { L"WAVE", L"ウェーブ" },
        { L"SCORE", L"スコア" },
        { L"Best: Wave %d  |  Kills %d  |  %02d:%02d  |  Score %d", L"ベスト: ウェーブ %d  |  撃破 %d  |  %02d:%02d  |  スコア %d" },
        { L"[DEBUG MODE USED - RECORD NOT SAVED]", L"[デバッグ使用 ― 記録は保存されません]" },
        { L"[DEBUG MODE USED]", L"[デバッグ使用]" },
        { L"Play Again", L"もう一度遊ぶ" },
        { L"Retry", L"リトライ" },
        { L"[W/S]: Select   [Enter/Space]: OK   [R]: Retry   [T]: Title", L"[W/S]: 選択   [Enter/Space]: 決定   [R]: リトライ   [T]: タイトル" },
        { L"AIRBORNE  - INVINCIBLE", L"空中 ― 無敵" },
        { L"RECOVERING...", L"回復中…" },
        { L"[SPACE] SLAM READY", L"[SPACE] スラム準備OK" },
        { L"SLAM  %.1fs", L"スラム  %.1f秒" },
        { L"x%d COMBO!", L"x%d コンボ！" },
        { L"x%d combo", L"x%d コンボ" },
        { L"GIANT WHALE", L"巨大クジラ" },
        { L"PHASE 2 - ENRAGED", L"フェーズ2 ― 激怒" },
        { L"PHASE 1", L"フェーズ1" },
        { L"GREAT SHARK", L"巨大ザメ" },
        { L"FRENZY", L"狂乱" },
        { L"HUNTING", L"狩り中" },
        { L"[F11] Show panels", L"[F11] パネル表示" },
    };

    Language g_lang = Language::English;
    bool g_hasSaved = false;

    const std::unordered_map<std::wstring_view, const wchar_t*>& JaMap()
    {
        static std::unordered_map<std::wstring_view, const wchar_t*> map = [] {
            std::unordered_map<std::wstring_view, const wchar_t*> m;
            m.reserve(sizeof(kTable) / sizeof(kTable[0]));
            for (const Entry& e : kTable) m.emplace(e.en, e.ja);
            return m;
        }();
        return map;
    }

    void SettingsPath(wchar_t* out, size_t count)
    {
        wchar_t exe[MAX_PATH] = {};
        DWORD n = GetModuleFileNameW(nullptr, exe, MAX_PATH);
        if (n == 0 || n >= MAX_PATH)
        {
            swprintf_s(out, count, L"Data\\settings.ini");
            return;
        }
        wchar_t* slash = wcsrchr(exe, L'\\');
        if (slash) *(slash + 1) = L'\0';
        swprintf_s(out, count, L"%sData\\settings.ini", exe);
    }
}

namespace L10N
{
    void Init()
    {
        wchar_t path[MAX_PATH];
        SettingsPath(path, MAX_PATH);
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, path, L"r") != 0 || !fp) return;
        char line[64] = {};
        while (fgets(line, sizeof(line), fp))
        {
            if (strncmp(line, "lang=ja", 7) == 0) { g_lang = Language::Japanese; g_hasSaved = true; }
            else if (strncmp(line, "lang=en", 7) == 0) { g_lang = Language::English; g_hasSaved = true; }
        }
        fclose(fp);
    }

    Language Get() { return g_lang; }
    bool HasSavedChoice() { return g_hasSaved; }

    void Set(Language lang, bool save)
    {
        g_lang = lang;
        if (!save) return;
        wchar_t path[MAX_PATH];
        SettingsPath(path, MAX_PATH);
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, path, L"w") != 0 || !fp) return;
        fputs(lang == Language::Japanese ? "lang=ja\n" : "lang=en\n", fp);
        fclose(fp);
        g_hasSaved = true;
    }

    const wchar_t* Tr(const wchar_t* en)
    {
        if (!en || g_lang == Language::English) return en;
        const auto& m = JaMap();
        auto it = m.find(std::wstring_view(en));
        return (it != m.end()) ? it->second : en;
    }
}
