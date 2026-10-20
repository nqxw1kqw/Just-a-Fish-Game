#include "DebugController.h"
#include "GamePrototype.h"
#include "FontManager.h"
#include "SoundManager.h"
#include "Consts.h"
#include "DxLib.h"
#include "Input.h"
#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include <atomic>
extern std::atomic<size_t> g_gameplayAllocCount;
#endif

namespace
{
    int s_debugSfxIdx = 0;
    int s_debugBgmIdx = 0;
}

void DebugController::Reset()
{
    showDebugHUD_ = GameConsts::DEBUG_HUD_DEFAULT_VISIBLE;
    debugUsed_ = false;
    debugEnemyTypeIdx_ = 1;
}

void DebugController::Update(float /*dt*/, GamePrototype& game)
{
    // Dùng trạng thái phím chung (Input) thay cho các biến static xxxPrev
    const Input& input = Input::Instance();

    // Phím bật tắt Debug HUD (F1)
    bool f1Now = input.IsPressed(KEY_INPUT_F1);
    if (f1Now)
    {
        showDebugHUD_ = !showDebugHUD_;
    }

    // Phím bật tắt rung camera (F2 - C7)
    bool f2Now = input.IsPressed(KEY_INPUT_F2);
    if (f2Now)
    {
        float scale = game.cameraController_.GetShakeScale();
        game.cameraController_.SetShakeScale((scale > 0.0f) ? 0.0f : 1.0f);
    }

    // Phím chỉnh nóng góc nghiêng Pitch [ ] (C8)
    bool lbrkNow = input.IsPressed(KEY_INPUT_LBRACKET);
    if (lbrkNow)
    {
        game.cameraController_.SetPitchDeg((std::max)(25.0f, game.cameraController_.GetPitchDeg() - 5.0f));
    }

    bool rbrkNow = input.IsPressed(KEY_INPUT_RBRACKET);
    if (rbrkNow)
    {
        game.cameraController_.SetPitchDeg((std::min)(85.0f, game.cameraController_.GetPitchDeg() + 5.0f));
    }

    // Phím chỉnh khoảng cách camera - / = (C8)
    bool minusNow = input.IsPressed(KEY_INPUT_MINUS);
    if (minusNow)
    {
        game.cameraController_.SetDistance((std::min)(1000.0f, game.cameraController_.GetDistance() + 40.0f));
    }

    bool plusNow = (input.IsPressed(KEY_INPUT_SEMICOLON) || input.IsPressed(KEY_INPUT_COLON));
    if (plusNow)
    {
        game.cameraController_.SetDistance((std::max)(260.0f, game.cameraController_.GetDistance() - 40.0f));
    }

    // Phím K (Debug Kill quái)
    bool kNow = input.IsPressed(KEY_INPUT_K);
    if (kNow)
    {
        game.enemies_.KillAll(game.effects_);
    }

    // --- CHẾ ĐỘ DEBUG & CÁC PHÍM TEST (PHẦN D) ---
    if constexpr (GameConsts::ALLOW_DEBUG_KEYS)
    {
        if (showDebugHUD_)
        {
            // Chuyển loại quái cần căn chỉnh 3D bằng phím N
            bool nNow = input.IsPressed(KEY_INPUT_N);
            if (nNow)
            {
                debugEnemyTypeIdx_ = (debugEnemyTypeIdx_ + 1) % 5;
            }

            EnemyType curDebugEnemyType = static_cast<EnemyType>(debugEnemyTypeIdx_);

            // Chỉnh nóng Scale và góc quay của Mô hình 3D quái đang chọn (phím <, >, M)
            bool commaNow = input.IsPressed(KEY_INPUT_COMMA);
            if (commaNow)
            {
                float curScale = game.enemies_.GetModelScale(curDebugEnemyType);
                float step = (curScale <= 0.6f) ? 0.02f : 0.1f;
                game.enemies_.SetModelScale(curDebugEnemyType, (std::max)(0.02f, curScale - step));
            }

            bool periodNow = input.IsPressed(KEY_INPUT_PERIOD);
            if (periodNow)
            {
                float curScale = game.enemies_.GetModelScale(curDebugEnemyType);
                float step = (curScale < 0.6f) ? 0.02f : 0.1f;
                game.enemies_.SetModelScale(curDebugEnemyType, (std::min)(10.0f, curScale + step));
            }

            bool mNow = input.IsPressed(KEY_INPUT_M);
            if (mNow)
            {
                float curRot = game.enemies_.GetModelRotYDeg(curDebugEnemyType);
                float nextRot = curRot + 90.0f;
                if (nextRot >= 360.0f) nextRot -= 360.0f;
                game.enemies_.SetModelRotYDeg(curDebugEnemyType, nextRot);
            }

            // Chỉnh nóng Scale và góc quay của Mô hình 3D Cá Chính (phím O, P, L)
            bool oNow = input.IsPressed(KEY_INPUT_O);
            if (oNow)
            {
                float curScale = game.player_.GetFishModelScale();
                float step = (curScale <= 0.6f) ? 0.02f : 0.1f;
                game.player_.SetFishModelScale((std::max)(0.02f, curScale - step));
            }

            bool pNow = input.IsPressed(KEY_INPUT_P);
            if (pNow)
            {
                float curScale = game.player_.GetFishModelScale();
                float step = (curScale < 0.6f) ? 0.02f : 0.1f;
                game.player_.SetFishModelScale((std::min)(15.0f, curScale + step));
            }

            bool lNow = input.IsPressed(KEY_INPUT_L);
            if (lNow)
            {
                float curRot = game.player_.GetFishModelRotYDeg();
                float nextRot = curRot + 90.0f;
                if (nextRot >= 360.0f) nextRot -= 360.0f;
                game.player_.SetFishModelRotYDeg(nextRot);
            }

            bool shiftHeld = (input.IsDown(KEY_INPUT_LSHIFT) || input.IsDown(KEY_INPUT_RSHIFT));
            bool ctrlHeld = (input.IsDown(KEY_INPUT_LCONTROL) || input.IsDown(KEY_INPUT_RCONTROL));

            // F8 / Shift+F8: Bỏ qua wave / Diệt trùm / Bỏ qua chọn thẻ
            bool f8Now = input.IsPressed(KEY_INPUT_F8);
            if (f8Now)
            {
                debugUsed_ = true;
                if (shiftHeld)
                {
                    if (game.state_ == GameState::Boss)
                    {
                        if (game.bossKind_ == 4) game.swordfish_.Kill();
                        else if (game.bossKind_ == 3) game.turtle_.Kill();
                        else if (game.bossKind_ == 2) game.squid_.Kill();
                        else if (game.sharkFight_) game.shark_.Kill();
                        else game.boss_.Kill();
                    }
                    else if (game.state_ == GameState::CardPick)
                    {
                        game.AdvanceAfterCardPick();
                    }
                    else if (game.state_ == GameState::Wave)
                    {
                        // Shift+F8: bỏ qua wave và bỏ luôn lượt chọn thẻ (D2)
                        game.enemies_.KillAll(game.effects_);
                        game.effects_.Reset();
                        game.AdvanceAfterCardPick();
                    }
                }
                else
                {
                    game.SkipCurrentWave();
                }
            }

            // Ctrl + 1..5: Nhảy tới wave N (nhận ngẫu nhiên N - 1 thẻ)
            if (ctrlHeld)
            {
                bool k1 = (input.IsPressed(KEY_INPUT_1) || input.IsPressed(KEY_INPUT_NUMPAD1));
                bool k2 = (input.IsPressed(KEY_INPUT_2) || input.IsPressed(KEY_INPUT_NUMPAD2));
                bool k3 = (input.IsPressed(KEY_INPUT_3) || input.IsPressed(KEY_INPUT_NUMPAD3));
                bool k4 = (input.IsPressed(KEY_INPUT_4) || input.IsPressed(KEY_INPUT_NUMPAD4));
                bool k5 = (input.IsPressed(KEY_INPUT_5) || input.IsPressed(KEY_INPUT_NUMPAD5));
                bool k6 = (input.IsPressed(KEY_INPUT_6) || input.IsPressed(KEY_INPUT_NUMPAD6));

                bool k7 = (input.IsPressed(KEY_INPUT_7) || input.IsPressed(KEY_INPUT_NUMPAD7));
                bool k8 = (input.IsPressed(KEY_INPUT_8) || input.IsPressed(KEY_INPUT_NUMPAD8));
                bool k9 = (input.IsPressed(KEY_INPUT_9) || input.IsPressed(KEY_INPUT_NUMPAD9));

                if (k1) game.SkipToWave(1);
                if (k2) game.SkipToWave(2);
                if (k3) game.SkipToWave(3);
                if (k4) game.SkipToWave(4);
                if (k5) game.SkipToEndlessWave(5);  // Ctrl+5: Endless stage 5 (Boss đầu tiên)
                if (k6) game.SkipToBoss();
                if (k7) game.SkipToEndlessWave(1);   // Ctrl+7: Endless stage 1
                if (k8 && shiftHeld) game.SkipToSwordfishBoss(); // Ctrl+Shift+8: Boss Cá Kiếm (Endless)
                else if (k8) game.SkipToEndlessWave(15);  // Ctrl+8: Endless stage 15 (Boss)
                if (k9 && shiftHeld) game.SkipToTurtleBoss(); // Ctrl+Shift+9: Boss Rùa Cổ (Endless)
                else if (k9) game.SkipToEndlessWave(GameConsts::ENDLESS_MAX_STAGE); // Ctrl+9: stage 30 (Boss cuối)
                bool k0 = (input.IsPressed(KEY_INPUT_0) || input.IsPressed(KEY_INPUT_NUMPAD0));
                if (k0 && shiftHeld) game.SkipToSquidBoss(); // Ctrl+Shift+0: Boss Mực (Endless)
                else if (k0) game.SkipToSharkBoss(); // Ctrl+0: Boss Cá Mập (Endless)
            }

            // F6: Nhảy thẳng đến Boss (hoặc Ctrl+6)
            bool f6Now = input.IsPressed(KEY_INPUT_F6);
            if (f6Now)
            {
                if (shiftHeld)
                {
                    // Shift+F6: Endless stage kế tiếp (Ctrl+Shift+F6: lùi 1 stage), giữ nguyên chế độ Endless
                    int target = game.currentWave_ + (ctrlHeld ? -1 : 1);
                    if (!game.IsEndlessLike()) target = 1;
                    game.SkipToEndlessWave((std::max)(1, target));
                }
                else
                {
                    game.SkipToBoss();
                }
            }

            // F9 / Shift+F9: God Mode / Instant Lose
            bool f9Now = input.IsPressed(KEY_INPUT_F9);
            if (f9Now)
            {
                debugUsed_ = true;
                if (shiftHeld)
                {
                    game.player_.Kill();
                }
                else
                {
                    game.player_.SetGodMode(!game.player_.IsGodMode());
                }
            }

            // F4: Hồi đầy máu & đặt lại thời gian hồi chiêu nhảy
            bool f4Now = input.IsPressed(KEY_INPUT_F4);
            if (f4Now)
            {
                debugUsed_ = true;
                game.player_.FullHealAndResetJumpCooldown();
            }

            // F7 / Ctrl+F7 / Shift+F7: Buộc Boss tung chiêu (Task 10 T3f)
            bool f7Now = input.IsPressed(KEY_INPUT_F7);
            if (f7Now)
            {
                debugUsed_ = true;
                if (game.state_ != GameState::Boss)
                {
                    game.SkipToBoss();
                }
                if (ctrlHeld)
                {
                    game.boss_.ForceDoubleSlam(game.player_);
                }
                else if (shiftHeld)
                {
                    game.boss_.ForceTsunamiWithGap(game.player_);
                }
                else
                {
                    game.boss_.ForceTsunami(game.player_);
                }
            }

            // Boss rework: Shift + số = buộc boss dùng chiêu mới
            if (shiftHeld && !ctrlHeld)
            {
                int forced = 0;
                if (input.IsPressed(KEY_INPUT_1)) forced = 1;
                else if (input.IsPressed(KEY_INPUT_2)) forced = 2;
                else if (input.IsPressed(KEY_INPUT_3)) forced = 3;
                else if (input.IsPressed(KEY_INPUT_4)) forced = 4;
                else if (input.IsPressed(KEY_INPUT_5)) forced = 5;
                else if (input.IsPressed(KEY_INPUT_6)) forced = 6;
                else if (input.IsPressed(KEY_INPUT_7)) forced = 7;
                else if (input.IsPressed(KEY_INPUT_8)) forced = 8;
                else if (input.IsPressed(KEY_INPUT_9)) forced = 9;
                else if (input.IsPressed(KEY_INPUT_0)) forced = 10;
                if (forced != 0 && game.bossKind_ == 4 && game.state_ == GameState::Boss)
                {
                    // Trận Cá Kiếm: Shift+1 Sword Dash | 2 Spin | 3 Rage Dive
                    debugUsed_ = true;
                    if (forced == 1) game.swordfish_.ForceDash(game.player_);
                    else if (forced == 2) game.swordfish_.ForceSpin(game.player_);
                    else if (forced == 3) game.swordfish_.ForceRage(game.effects_);
                }
                else if (forced != 0 && game.bossKind_ == 3 && game.state_ == GameState::Boss)
                {
                    // Trận Rùa Cổ: Shift+1 Roll | 2 Bite | 3 Tail Sweep | 4 Phase 2
                    debugUsed_ = true;
                    if (forced == 1) game.turtle_.ForceRoll(game.player_);
                    else if (forced == 2) game.turtle_.ForceBite(game.player_);
                    else if (forced == 3) game.turtle_.ForceTail(game.player_);
                    else if (forced == 4) game.turtle_.ForcePhase2(game.effects_);
                }
                else if (forced != 0 && game.bossKind_ == 2 && game.state_ == GameState::Boss)
                {
                    // Trận Mực: Shift+1 Slam | 2 Ink Spray | 3 Ink Jet | 4 Grab | 5 Enrage
                    debugUsed_ = true;
                    if (forced == 1) game.squid_.ForceSlam(game.player_);
                    else if (forced == 2) game.squid_.ForceInkSpray(game.player_);
                    else if (forced == 3) game.squid_.ForceInkJet(game.player_);
                    else if (forced == 4) game.squid_.ForceGrab(game.player_);
                    else if (forced == 5) game.squid_.ForceEnrage(game.effects_);
                }
                else if (forced != 0 && game.sharkFight_ && game.state_ == GameState::Boss)
                {
                    // Trận Cá Mập: Shift+1 Bite | 2 Strafe | 3 Vortex | 4 Frenzy | 5 Hunt
                    debugUsed_ = true;
                    if (forced == 1) game.shark_.ForceBite(game.player_);
                    else if (forced == 2) game.shark_.ForceStrafe(game.player_);
                    else if (forced == 3) game.shark_.ForceVortex(game.player_);
                    else if (forced == 4) game.shark_.ForceFrenzy(game.effects_);
                    else if (forced == 5) game.shark_.ForceHunt(game.player_);
                }
                else if (forced != 0)
                {
                    debugUsed_ = true;
                    if (game.state_ != GameState::Boss) game.SkipToBoss();
                    if (forced == 1) game.boss_.ForceBreach(game.player_);
                    else if (forced == 2) game.boss_.ForceTail(game.player_);
                    else if (forced == 3) game.boss_.Stun(2.0f);
                    else if (forced == 4) game.boss_.ForceDoubleSlam(game.player_); // Triple Slam
                    else if (forced == 5) game.boss_.ForceTsunamiDouble(game.player_);
                    else if (forced == 6) game.boss_.ForceBubbleNet(game.player_);
                    else if (forced == 7) game.boss_.ForceInhale(game.player_);
                    else if (forced == 8) game.boss_.ForcePack(game.player_);
                    else if (forced == 9) game.boss_.ForceDive(game.player_);
                    else if (forced == 10) game.boss_.ForcePhaseStep(game.effects_);
                }
            }

            // F3: Test SFX lần lượt
            bool f3Now = input.IsPressed(KEY_INPUT_F3);
            if (f3Now)
            {
                s_debugSfxIdx = (s_debugSfxIdx + 1) % SoundManager::SFX_COUNT;
                SoundManager::Instance().PlaySFX(static_cast<SfxId>(s_debugSfxIdx));
            }

            // F5: Test BGM lần lượt
            bool f5Now = input.IsPressed(KEY_INPUT_F5);
            if (f5Now)
            {
                s_debugBgmIdx = (s_debugBgmIdx + 1) % SoundManager::BGM_COUNT;
                SoundManager::Instance().PlayBGM(static_cast<BgmId>(s_debugBgmIdx));
            }

            // Chỉnh âm lượng: C/V (Master), Z/X (BGM), H/J (SFX)
            bool cNow = input.IsPressed(KEY_INPUT_C);
            bool vNow = input.IsPressed(KEY_INPUT_V);
            if (cNow) SoundManager::Instance().SetVolume(SoundChannel::Master, SoundManager::Instance().GetVolume(SoundChannel::Master) - 5);
            if (vNow) SoundManager::Instance().SetVolume(SoundChannel::Master, SoundManager::Instance().GetVolume(SoundChannel::Master) + 5);

            bool zNow = input.IsPressed(KEY_INPUT_Z);
            bool xNow = input.IsPressed(KEY_INPUT_X);
            if (zNow) SoundManager::Instance().SetVolume(SoundChannel::BGM, SoundManager::Instance().GetVolume(SoundChannel::BGM) - 5);
            if (xNow) SoundManager::Instance().SetVolume(SoundChannel::BGM, SoundManager::Instance().GetVolume(SoundChannel::BGM) + 5);

            bool hNow = input.IsPressed(KEY_INPUT_H);
            bool jNow = input.IsPressed(KEY_INPUT_J);
            if (hNow) SoundManager::Instance().SetVolume(SoundChannel::SFX, SoundManager::Instance().GetVolume(SoundChannel::SFX) - 5);
            if (jNow) SoundManager::Instance().SetVolume(SoundChannel::SFX, SoundManager::Instance().GetVolume(SoundChannel::SFX) + 5);
        }
    }
}

void DebugController::DrawHUD(const GamePrototype& game) const
{
    int sw = GameConsts::SCREEN_WIDTH;

    // Chỉ báo Debug Mode nếu đã dùng bất kỳ phím cheat nào (Phần D)
    if (debugUsed_)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 230);
        DrawBox(sw - 140, 10, sw - 20, 36, GetColor(180, 25, 25), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(sw - 140, 10, sw - 20, 36, GetColor(255, 215, 60), FALSE);
        FontManager::Draw(sw - 120, 14, L"[デバッグ]", GetColor(255, 255, 255), FontSize::Body);
    }

    // 5. Debug Panel góc phải (F1). F11 ẩn toàn bộ bảng thông tin
    if (showDebugHUD_ && game.panelsVisible_)
    {
        int panW = 460;
        int panH = (game.state_ == GameState::Boss) ? 770 : 670;
        int panX = sw - panW - 30;
        int panY = 24;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 195);
        DrawBox(panX, panY, panX + panW, panY + panH, GetColor(10, 15, 25), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(panX, panY, panX + panW, panY + panH, GetColor(60, 100, 160), FALSE);

        int textX = panX + 15;
        int textY = panY + 12;
        FontManager::Draw(textX, textY, L"=== オドル 動作情報 ===", GetColor(255, 220, 80), FontSize::Body);
        textY += 22;

#ifdef _DEBUG
        unsigned int allocCount = static_cast<unsigned int>(g_gameplayAllocCount.load());
        int allocColor = (allocCount == 0) ? GetColor(100, 255, 140) : GetColor(255, 90, 90);
        FontManager::DrawFormat(textX, textY, allocColor, FontSize::Small, L"動作中のメモリ確保: %u", allocCount);
        textY += 18;
#endif

        // --- SỐ ĐO WAVE / BOSS THEO TASK 10 T1 ---
        if (game.state_ == GameState::Wave)
        {
            FontManager::Draw(textX, textY, L"--- ウェーブ情報 ---", GetColor(100, 220, 255), FontSize::Small);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 230, 120), FontSize::Small,
                L"ウェーブ: %d | 時間: %.1f秒 / %.1f秒",
                game.currentWave_, game.waveTimer_, game.waveDuration_);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(200, 230, 255), FontSize::Small,
                L"生存: %d (コスト: %.1f) | 出現: %d | 撃破: %d",
                game.enemies_.GetActiveCount(), game.enemies_.GetAliveCost(),
                game.enemies_.GetSpawnedThisWave(), game.enemies_.GetKilledThisWave());
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 140, 140), FontSize::Small,
                L"今回の被ダメージ: %.1f", game.player_.GetDamageTakenThisWave());
            textY += 18;
        }
        else if (game.state_ == GameState::Boss)
        {
            FontManager::Draw(textX, textY, L"--- ボス情報 ---", GetColor(255, 120, 120), FontSize::Small);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 230, 120), FontSize::Small,
                L"経過: %.1f秒 | 段階: %s",
                game.boss_.GetBossFightTimer(), (game.boss_.GetPhase() == BossPhase::Phase1) ? L"第1段階" : L"第2段階");
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 160, 140), FontSize::Small,
                L"ボス体力: %.0f / %.0f (%.1f%%) | 被ダメ: %.1f",
                game.boss_.GetHp(), game.boss_.GetMaxHp(),
                (game.boss_.GetHp() / (std::max)(1.0f, game.boss_.GetMaxHp())) * 100.0f,
                game.player_.GetDamageTakenThisWave());
            textY += 17;

            // 5 chiêu gần nhất của Boss
            wchar_t actBuf[128] = L"";
            int histCount = game.boss_.GetActionHistoryCount();
            if (histCount == 0)
            {
                wcscpy_s(actBuf, L"なし");
            }
            else
            {
                for (int i = 0; i < histCount && i < 5; ++i)
                {
                    const wchar_t* a = game.boss_.GetRecentAction(i);
                    if (i > 0) wcscat_s(actBuf, L" < ");
                    wcscat_s(actBuf, a ? a : L"?");
                }
            }
            FontManager::DrawFormat(textX, textY, GetColor(220, 240, 255), FontSize::Small,
                L"最近の行動: %s", actBuf);
            textY += 18;
        }

        FontManager::DrawFormat(textX, textY, GetColor(220, 220, 220), FontSize::Small, L"空中: %s", game.player_.IsAirborne() ? L"はい（無敵）" : L"いいえ（地上）");
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(220, 220, 220), FontSize::Small, L"高さ: %.1f / %.1f", game.player_.GetHeightY(), GameConsts::JUMP_HEIGHT);
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(100, 220, 255), FontSize::Small, L"スラム威力: %.1f | 範囲: %.0f", game.player_.GetSlamDamage(), game.player_.GetSlamRadius());
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(120, 240, 180), FontSize::Small, L"ジャンプ待ち: %.2f秒 / %.2f秒", game.player_.GetJumpCooldownTimer(), game.player_.GetJumpCooldownMax());
        textY += 18;

        FontManager::DrawFormat(textX, textY, (game.player_.IsGodMode()) ? GetColor(255, 215, 60) : GetColor(180, 180, 180), FontSize::Small,
            L"無敵設定 (F9): %s", game.player_.IsGodMode() ? L"有効（無敵）" : L"無効");
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(120, 220, 255), FontSize::Small, L"カメラ角度: %.0f度 | 距離: %.0f", game.cameraController_.GetPitchDeg(), game.cameraController_.GetDistance());
        textY += 18;

        float shakeScale = game.cameraController_.GetShakeScale();
        FontManager::DrawFormat(textX, textY, (shakeScale > 0.0f) ? GetColor(100, 255, 150) : GetColor(255, 140, 100), FontSize::Small,
            L"画面揺れ: %s (倍率: %.1f)", (shakeScale > 0.0f) ? L"有効" : L"無効", shakeScale);
        textY += 18;

        const wchar_t* enemyTypeNames[5] = { L"小魚", L"カニ", L"カメ", L"クラゲ", L"突進ザメ" };
        EnemyType curDebugEnemyType = static_cast<EnemyType>(debugEnemyTypeIdx_);
        FontManager::DrawFormat(textX, textY, GetColor(255, 185, 110), FontSize::Small,
            L"敵 [%s]: 倍率 %.2f (< / >) | 向き %.0f度 (M) | 切替 (N)",
            enemyTypeNames[debugEnemyTypeIdx_], game.enemies_.GetModelScale(curDebugEnemyType), game.enemies_.GetModelRotYDeg(curDebugEnemyType));
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(100, 220, 255), FontSize::Small,
            L"魚: 倍率 %.2f (O / P) | 向き %.0f度 (L)",
            game.player_.GetFishModelScale(), game.player_.GetFishModelRotYDeg());
        textY += 18;

        if (game.player_.IsComboBuffActive())
        {
            FontManager::Draw(textX, textY, L"連続着地強化: ダメージ +50%", GetColor(255, 200, 50), FontSize::Small);
            textY += 18;
        }

        if (game.state_ == GameState::Boss)
        {
            FontManager::Draw(textX, textY, L"--- 放射状の大波 ---", GetColor(100, 220, 255), FontSize::Small);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(200, 230, 255), FontSize::Small,
                L"波の通過: %.2f秒 | 回避猶予: %.2f秒",
                game.boss_.GetTsunamiThicknessOverSpeed(), game.boss_.GetTsunamiJumpWindow());
            textY += 17;
            float timeHit = game.boss_.GetTimeToHitPlayer(game.player_.GetPosition());
            if (timeHit >= 0.0f)
            {
                FontManager::DrawFormat(textX, textY, GetColor(255, 220, 100), FontSize::Small,
                    L"波の数: %d | 到達まで: %.2f秒", game.boss_.GetActiveTsunamiCount(), timeHit);
            }
            else
            {
                FontManager::DrawFormat(textX, textY, GetColor(180, 200, 220), FontSize::Small,
                    L"波の数: %d | 到達まで: --", game.boss_.GetActiveTsunamiCount());
            }
            textY += 17;
            // AI pacing: đợt chiêu / thời gian nghỉ / 2 chiêu gần nhất
            FontManager::DrawFormat(textX, textY, GetColor(180, 255, 180), FontSize::Small,
                L"連続攻撃: %d/%d | 休息: %.2f秒 | 直前: %ls, %ls",
                game.boss_.GetBurstCount(), game.boss_.GetBurstSize(), game.boss_.GetRestTimer(),
                WhaleBoss::SkillName(game.boss_.GetLastSkill()), WhaleBoss::SkillName(game.boss_.GetPrevSkill()));
            textY += 18;
            if (game.sharkFight_)
            {
                FontManager::DrawFormat(textX, textY, GetColor(120, 220, 255), FontSize::Small,
                    L"サメ体力 %.0f/%.0f %ls | 連続 %d | 休息 %.2f秒 | 渦 %.1f秒 | 狩り待ち %.1f秒 | %ls, %ls",
                    game.shark_.GetHp(), game.shark_.GetMaxHp(), game.shark_.IsFrenzy() ? L"狂乱" : L"",
                    game.shark_.GetBurstCount(), game.shark_.GetRestTimer(), game.shark_.GetVortexTimer(), game.shark_.GetHuntCooldown(),
                    SharkBoss::SkillName(game.shark_.GetLastSkill()), SharkBoss::SkillName(game.shark_.GetPrevSkill()));
                textY += 18;
            }
        }

        // Dòng tóm tắt gần nhất (Summary)
        FontManager::Draw(textX, textY, L"--------------------------------------------", GetColor(70, 90, 120), FontSize::Small);
        textY += 13;
        FontManager::DrawFormat(textX, textY, GetColor(140, 255, 170), FontSize::Small,
            L"結果: %s", game.GetLastSummary());
        textY += 18;

        // Thông tin âm thanh & kiểm thử
        FontManager::Draw(textX, textY, L"--- 音声 ---", GetColor(120, 240, 255), FontSize::Small);
        textY += 17;
        FontManager::DrawFormat(textX, textY, GetColor(200, 230, 255), FontSize::Small,
            L"全体: %d%% (C/V) | 音楽: %d%% (Z/X) | 効果音: %d%% (H/J)",
            SoundManager::Instance().GetVolume(SoundChannel::Master),
            SoundManager::Instance().GetVolume(SoundChannel::BGM),
            SoundManager::Instance().GetVolume(SoundChannel::SFX));
        textY += 17;
        FontManager::DrawFormat(textX, textY, GetColor(255, 220, 140), FontSize::Small,
            L"F3 効果音 [%d]: %s",
            s_debugSfxIdx, SoundManager::Instance().GetSfxNameW(static_cast<SfxId>(s_debugSfxIdx)));
        textY += 17;
        FontManager::DrawFormat(textX, textY, GetColor(255, 200, 180), FontSize::Small,
            L"F5 音楽: %s",
            SoundManager::Instance().GetBgmNameW(SoundManager::Instance().GetCurrentBgm()));
        textY += 18;

        FontManager::Draw(textX, textY, L"[動作確認キー]:", GetColor(255, 205, 80), FontSize::Small); textY += 17;
        FontManager::Draw(textX, textY, L"F3: 効果音 | F5: 音楽 | C/V, Z/X, H/J: 音量", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"F8: ウェーブ省略 | Shift+F8: ボス撃破・カード省略", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"Ctrl+1～3: 練習 | F6: クジラ | Shift+F6: 次ステージ", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"F9: 無敵 | Shift+F9: 即敗北", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, game.IsRenderInterpEnabled() ? L"F10: 描画補間 有効" : L"F10: 描画補間 無効", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"F4: 全回復・待ち時間解除 | F7: ボスの大波", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"Shift+1～0: ボスの技を指定して実行", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"Ctrl+0: サメ | Shift+1～5: 噛む・旋回・渦・狂乱・狩り", GetColor(255, 180, 120), FontSize::Small); textY += 16;
    }
}
