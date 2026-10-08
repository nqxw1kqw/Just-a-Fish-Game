#include "DebugController.h"
#include "GamePrototype.h"
#include "FontManager.h"
#include "Consts.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

#ifdef _DEBUG
#include <atomic>
extern std::atomic<size_t> g_gameplayAllocCount;
#endif

void DebugController::Reset()
{
    showDebugHUD_ = true;
    debugUsed_ = false;
    debugEnemyTypeIdx_ = 1;
}

void DebugController::Update(float /*dt*/, GamePrototype& game)
{
    // Phím bật tắt Debug HUD (F1)
    static bool f1Prev = false;
    bool f1Now = CheckHitKey(KEY_INPUT_F1) != 0;
    if (f1Now && !f1Prev)
    {
        showDebugHUD_ = !showDebugHUD_;
    }
    f1Prev = f1Now;

    // Phím bật tắt rung camera (F2 - C7)
    static bool f2Prev = false;
    bool f2Now = CheckHitKey(KEY_INPUT_F2) != 0;
    if (f2Now && !f2Prev)
    {
        float scale = game.cameraController_.GetShakeScale();
        game.cameraController_.SetShakeScale((scale > 0.0f) ? 0.0f : 1.0f);
    }
    f2Prev = f2Now;

    // Phím chỉnh nóng góc nghiêng Pitch [ ] (C8)
    static bool lbrkPrev = false;
    bool lbrkNow = CheckHitKey(KEY_INPUT_LBRACKET) != 0;
    if (lbrkNow && !lbrkPrev)
    {
        game.cameraController_.SetPitchDeg((std::max)(25.0f, game.cameraController_.GetPitchDeg() - 5.0f));
    }
    lbrkPrev = lbrkNow;

    static bool rbrkPrev = false;
    bool rbrkNow = CheckHitKey(KEY_INPUT_RBRACKET) != 0;
    if (rbrkNow && !rbrkPrev)
    {
        game.cameraController_.SetPitchDeg((std::min)(85.0f, game.cameraController_.GetPitchDeg() + 5.0f));
    }
    rbrkPrev = rbrkNow;

    // Phím chỉnh khoảng cách camera - / = (C8)
    static bool minusPrev = false;
    bool minusNow = CheckHitKey(KEY_INPUT_MINUS) != 0;
    if (minusNow && !minusPrev)
    {
        game.cameraController_.SetDistance((std::min)(1000.0f, game.cameraController_.GetDistance() + 40.0f));
    }
    minusPrev = minusNow;

    static bool plusPrev = false;
    bool plusNow = (CheckHitKey(KEY_INPUT_SEMICOLON) != 0 || CheckHitKey(KEY_INPUT_COLON) != 0);
    if (plusNow && !plusPrev)
    {
        game.cameraController_.SetDistance((std::max)(260.0f, game.cameraController_.GetDistance() - 40.0f));
    }
    plusPrev = plusNow;

    // Phím K (Debug Kill quái)
    static bool kPrev = false;
    bool kNow = CheckHitKey(KEY_INPUT_K) != 0;
    if (kNow && !kPrev)
    {
        game.enemies_.KillAll(game.effects_);
    }
    kPrev = kNow;

    // --- CHẾ ĐỘ DEBUG & CÁC PHÍM TEST (PHẦN D) ---
    if constexpr (GameConsts::ALLOW_DEBUG_KEYS)
    {
        if (showDebugHUD_)
        {
            // Chuyển loại quái cần căn chỉnh 3D bằng phím N
            static bool nPrev = false;
            bool nNow = CheckHitKey(KEY_INPUT_N) != 0;
            if (nNow && !nPrev)
            {
                debugEnemyTypeIdx_ = (debugEnemyTypeIdx_ + 1) % 5;
            }
            nPrev = nNow;

            EnemyType curDebugEnemyType = static_cast<EnemyType>(debugEnemyTypeIdx_);

            // Chỉnh nóng Scale và góc quay của Mô hình 3D quái đang chọn (phím <, >, M)
            static bool commaPrev = false;
            bool commaNow = CheckHitKey(KEY_INPUT_COMMA) != 0;
            if (commaNow && !commaPrev)
            {
                float curScale = game.enemies_.GetModelScale(curDebugEnemyType);
                float step = (curScale <= 0.6f) ? 0.02f : 0.1f;
                game.enemies_.SetModelScale(curDebugEnemyType, (std::max)(0.02f, curScale - step));
            }
            commaPrev = commaNow;

            static bool periodPrev = false;
            bool periodNow = CheckHitKey(KEY_INPUT_PERIOD) != 0;
            if (periodNow && !periodPrev)
            {
                float curScale = game.enemies_.GetModelScale(curDebugEnemyType);
                float step = (curScale < 0.6f) ? 0.02f : 0.1f;
                game.enemies_.SetModelScale(curDebugEnemyType, (std::min)(10.0f, curScale + step));
            }
            periodPrev = periodNow;

            static bool mPrev = false;
            bool mNow = CheckHitKey(KEY_INPUT_M) != 0;
            if (mNow && !mPrev)
            {
                float curRot = game.enemies_.GetModelRotYDeg(curDebugEnemyType);
                float nextRot = curRot + 90.0f;
                if (nextRot >= 360.0f) nextRot -= 360.0f;
                game.enemies_.SetModelRotYDeg(curDebugEnemyType, nextRot);
            }
            mPrev = mNow;

            // Chỉnh nóng Scale và góc quay của Mô hình 3D Cá Chính (phím O, P, L)
            static bool oPrev = false;
            bool oNow = CheckHitKey(KEY_INPUT_O) != 0;
            if (oNow && !oPrev)
            {
                float curScale = game.player_.GetFishModelScale();
                float step = (curScale <= 0.6f) ? 0.02f : 0.1f;
                game.player_.SetFishModelScale((std::max)(0.02f, curScale - step));
            }
            oPrev = oNow;

            static bool pPrev = false;
            bool pNow = CheckHitKey(KEY_INPUT_P) != 0;
            if (pNow && !pPrev)
            {
                float curScale = game.player_.GetFishModelScale();
                float step = (curScale < 0.6f) ? 0.02f : 0.1f;
                game.player_.SetFishModelScale((std::min)(15.0f, curScale + step));
            }
            pPrev = pNow;

            static bool lPrev = false;
            bool lNow = CheckHitKey(KEY_INPUT_L) != 0;
            if (lNow && !lPrev)
            {
                float curRot = game.player_.GetFishModelRotYDeg();
                float nextRot = curRot + 90.0f;
                if (nextRot >= 360.0f) nextRot -= 360.0f;
                game.player_.SetFishModelRotYDeg(nextRot);
            }
            lPrev = lNow;

            bool shiftHeld = (CheckHitKey(KEY_INPUT_LSHIFT) != 0 || CheckHitKey(KEY_INPUT_RSHIFT) != 0);
            bool ctrlHeld = (CheckHitKey(KEY_INPUT_LCONTROL) != 0 || CheckHitKey(KEY_INPUT_RCONTROL) != 0);

            // F8 / Shift+F8: Bỏ qua wave / Diệt trùm / Bỏ qua chọn thẻ
            static bool f8Prev = false;
            bool f8Now = CheckHitKey(KEY_INPUT_F8) != 0;
            if (f8Now && !f8Prev)
            {
                debugUsed_ = true;
                if (shiftHeld)
                {
                    if (game.state_ == GameState::Boss)
                    {
                        game.boss_.Kill();
                    }
                    else if (game.state_ == GameState::CardPick)
                    {
                        if (game.currentWave_ >= 5)
                        {
                            game.state_ = GameState::BossIntro;
                            game.bossIntroTimer_ = 2.0f;
                            game.enemies_.KillAll(game.effects_);
                        }
                        else
                        {
                            game.StartWave(game.currentWave_ + 1);
                        }
                    }
                    else if (game.state_ == GameState::Wave)
                    {
                        // Shift+F8: bỏ qua wave và bỏ luôn lượt chọn thẻ (D2)
                        game.enemies_.KillAll(game.effects_);
                        game.effects_.Reset();
                        if (game.currentWave_ >= 5)
                        {
                            game.state_ = GameState::BossIntro;
                            game.bossIntroTimer_ = 2.0f;
                        }
                        else
                        {
                            game.StartWave(game.currentWave_ + 1);
                        }
                    }
                }
                else
                {
                    game.SkipCurrentWave();
                }
            }
            f8Prev = f8Now;

            // Ctrl + 1..5: Nhảy tới wave N (nhận ngẫu nhiên N - 1 thẻ)
            if (ctrlHeld)
            {
                static bool key1Prev = false, key2Prev = false, key3Prev = false, key4Prev = false, key5Prev = false, key6Prev = false;
                bool k1 = (CheckHitKey(KEY_INPUT_1) != 0 || CheckHitKey(KEY_INPUT_NUMPAD1) != 0);
                bool k2 = (CheckHitKey(KEY_INPUT_2) != 0 || CheckHitKey(KEY_INPUT_NUMPAD2) != 0);
                bool k3 = (CheckHitKey(KEY_INPUT_3) != 0 || CheckHitKey(KEY_INPUT_NUMPAD3) != 0);
                bool k4 = (CheckHitKey(KEY_INPUT_4) != 0 || CheckHitKey(KEY_INPUT_NUMPAD4) != 0);
                bool k5 = (CheckHitKey(KEY_INPUT_5) != 0 || CheckHitKey(KEY_INPUT_NUMPAD5) != 0);
                bool k6 = (CheckHitKey(KEY_INPUT_6) != 0 || CheckHitKey(KEY_INPUT_NUMPAD6) != 0);

                static bool key7Prev = false, key8Prev = false, key9Prev = false;
                bool k7 = (CheckHitKey(KEY_INPUT_7) != 0 || CheckHitKey(KEY_INPUT_NUMPAD7) != 0);
                bool k8 = (CheckHitKey(KEY_INPUT_8) != 0 || CheckHitKey(KEY_INPUT_NUMPAD8) != 0);
                bool k9 = (CheckHitKey(KEY_INPUT_9) != 0 || CheckHitKey(KEY_INPUT_NUMPAD9) != 0);

                if (k1 && !key1Prev) game.SkipToWave(1);
                if (k2 && !key2Prev) game.SkipToWave(2);
                if (k3 && !key3Prev) game.SkipToWave(3);
                if (k4 && !key4Prev) game.SkipToWave(4);
                if (k5 && !key5Prev) game.SkipToWave(5);
                if (k6 && !key6Prev) game.SkipToBoss();
                if (k7 && !key7Prev) game.SkipToEndlessWave(10);
                if (k8 && !key8Prev) game.SkipToEndlessWave(30);
                if (k9 && !key9Prev) game.SkipToEndlessWave(50);

                key1Prev = k1; key2Prev = k2; key3Prev = k3;
                key4Prev = k4; key5Prev = k5; key6Prev = k6;
                key7Prev = k7; key8Prev = k8; key9Prev = k9;
            }

            // F6: Nhảy thẳng đến Boss (hoặc Ctrl+6)
            static bool f6Prev = false;
            bool f6Now = CheckHitKey(KEY_INPUT_F6) != 0;
            if (f6Now && !f6Prev)
            {
                game.SkipToBoss();
            }
            f6Prev = f6Now;

            // F9 / Shift+F9: God Mode / Instant Lose
            static bool f9Prev = false;
            bool f9Now = CheckHitKey(KEY_INPUT_F9) != 0;
            if (f9Now && !f9Prev)
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
            f9Prev = f9Now;

            // F4: Hồi đầy máu & đặt lại thời gian hồi chiêu nhảy
            static bool f4Prev = false;
            bool f4Now = CheckHitKey(KEY_INPUT_F4) != 0;
            if (f4Now && !f4Prev)
            {
                debugUsed_ = true;
                game.player_.FullHealAndResetJumpCooldown();
            }
            f4Prev = f4Now;

            // F7 / Ctrl+F7 / Shift+F7: Buộc Boss tung chiêu (Task 10 T3f)
            static bool f7Prev = false;
            bool f7Now = CheckHitKey(KEY_INPUT_F7) != 0;
            if (f7Now && !f7Prev)
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
            f7Prev = f7Now;
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
        FontManager::Draw(sw - 120, 14, L"[DEBUG]", GetColor(255, 255, 255), FontSize::Body);
    }

    // 5. Debug Panel góc phải (F1)
    if (showDebugHUD_)
    {
        int panW = 460;
        int panH = (game.state_ == GameState::Boss) ? 690 : 590;
        int panX = sw - panW - 30;
        int panY = 24;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 195);
        DrawBox(panX, panY, panX + panW, panY + panH, GetColor(10, 15, 25), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawBox(panX, panY, panX + panW, panY + panH, GetColor(60, 100, 160), FALSE);

        int textX = panX + 15;
        int textY = panY + 12;
        FontManager::Draw(textX, textY, L"=== ODORU DEBUG (JUMP & SLAM) ===", GetColor(255, 220, 80), FontSize::Body);
        textY += 22;

#ifdef _DEBUG
        unsigned int allocCount = static_cast<unsigned int>(g_gameplayAllocCount.load());
        int allocColor = (allocCount == 0) ? GetColor(100, 255, 140) : GetColor(255, 90, 90);
        FontManager::DrawFormat(textX, textY, allocColor, FontSize::Small, L"Heap Allocs (Game Loop - B3): %u", allocCount);
        textY += 18;
#endif

        // --- SỐ ĐO WAVE / BOSS THEO TASK 10 T1 ---
        if (game.state_ == GameState::Wave)
        {
            FontManager::Draw(textX, textY, L"--- WAVE METRICS (TASK 10 T1) ---", GetColor(100, 220, 255), FontSize::Small);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 230, 120), FontSize::Small,
                L"Wave: %d/5 | Time: %.1fs / %.1fs",
                game.currentWave_, game.waveTimer_, game.waveDuration_);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(200, 230, 255), FontSize::Small,
                L"Alive: %d (Cost: %.1f) | Spawn: %d | Kill: %d",
                game.enemies_.GetActiveCount(), game.enemies_.GetAliveCost(),
                game.enemies_.GetSpawnedThisWave(), game.enemies_.GetKilledThisWave());
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 140, 140), FontSize::Small,
                L"Wave Dmg Taken: %.1f", game.player_.GetDamageTakenThisWave());
            textY += 18;
        }
        else if (game.state_ == GameState::Boss)
        {
            FontManager::Draw(textX, textY, L"--- BOSS METRICS (TASK 10 T1) ---", GetColor(255, 120, 120), FontSize::Small);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 230, 120), FontSize::Small,
                L"Boss Time: %.1fs | Phase: %s",
                game.boss_.GetBossFightTimer(), (game.boss_.GetPhase() == BossPhase::Phase1) ? L"Phase 1" : L"Phase 2");
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(255, 160, 140), FontSize::Small,
                L"Boss HP: %.0f / %.0f (%.1f%%) | Wave Dmg: %.1f",
                game.boss_.GetHp(), game.boss_.GetMaxHp(),
                (game.boss_.GetHp() / (std::max)(1.0f, game.boss_.GetMaxHp())) * 100.0f,
                game.player_.GetDamageTakenThisWave());
            textY += 17;

            // 5 chiêu gần nhất của Boss
            wchar_t actBuf[128] = L"";
            int histCount = game.boss_.GetActionHistoryCount();
            if (histCount == 0)
            {
                wcscpy_s(actBuf, L"Chưa có");
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
                L"Recent Actions: %s", actBuf);
            textY += 18;
        }

        FontManager::DrawFormat(textX, textY, GetColor(220, 220, 220), FontSize::Small, L"Airborne: %s", game.player_.IsAirborne() ? L"YES (INVINCIBLE)" : L"NO (GROUND)");
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(220, 220, 220), FontSize::Small, L"Height (Y): %.1f / %.1f", game.player_.GetHeightY(), GameConsts::JUMP_HEIGHT);
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(100, 220, 255), FontSize::Small, L"Slam Damage: %.1f  |  Radius: %.0f", game.player_.GetSlamDamage(), game.player_.GetSlamRadius());
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(120, 240, 180), FontSize::Small, L"Jump CD: %.2fs / %.2fs", game.player_.GetJumpCooldownTimer(), game.player_.GetJumpCooldownMax());
        textY += 18;

        FontManager::DrawFormat(textX, textY, (game.player_.IsGodMode()) ? GetColor(255, 215, 60) : GetColor(180, 180, 180), FontSize::Small,
            L"God Mode (F9): %s", game.player_.IsGodMode() ? L"ON (IMMUNE)" : L"OFF");
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(120, 220, 255), FontSize::Small, L"Cam Pitch: %.0f deg  |  Dist: %.0f", game.cameraController_.GetPitchDeg(), game.cameraController_.GetDistance());
        textY += 18;

        float shakeScale = game.cameraController_.GetShakeScale();
        FontManager::DrawFormat(textX, textY, (shakeScale > 0.0f) ? GetColor(100, 255, 150) : GetColor(255, 140, 100), FontSize::Small,
            L"Cam Shake: %s (Scale: %.1f)", (shakeScale > 0.0f) ? L"ON" : L"OFF", shakeScale);
        textY += 18;

        const wchar_t* enemyTypeNames[5] = { L"Minion", L"Hopper", L"Tanker", L"Float", L"Charge" };
        EnemyType curDebugEnemyType = static_cast<EnemyType>(debugEnemyTypeIdx_);
        FontManager::DrawFormat(textX, textY, GetColor(255, 185, 110), FontSize::Small,
            L"Enemy 3D [%s]: Scale=%.2f (< / >) | RotY=%.0f deg (M) | Switch (N)",
            enemyTypeNames[debugEnemyTypeIdx_], game.enemies_.GetModelScale(curDebugEnemyType), game.enemies_.GetModelRotYDeg(curDebugEnemyType));
        textY += 18;

        FontManager::DrawFormat(textX, textY, GetColor(100, 220, 255), FontSize::Small,
            L"Fish 3D: Scale=%.2f (O / P) | RotY=%.0f deg (L)",
            game.player_.GetFishModelScale(), game.player_.GetFishModelRotYDeg());
        textY += 18;

        if (game.player_.IsComboBuffActive())
        {
            FontManager::Draw(textX, textY, L"Combo Buff: [+50% DMG (CHAIN BOUNCE)]", GetColor(255, 200, 50), FontSize::Small);
            textY += 18;
        }

        if (game.state_ == GameState::Boss)
        {
            FontManager::Draw(textX, textY, L"--- RADIAL TSUNAMI (BOSS - A8) ---", GetColor(100, 220, 255), FontSize::Small);
            textY += 17;
            FontManager::DrawFormat(textX, textY, GetColor(200, 230, 255), FontSize::Small,
                L"Thick/Spd: %.2fs | Dodge Window: %.2fs",
                game.boss_.GetTsunamiThicknessOverSpeed(), game.boss_.GetTsunamiJumpWindow());
            textY += 17;
            float timeHit = game.boss_.GetTimeToHitPlayer(game.player_.GetPosition());
            if (timeHit >= 0.0f)
            {
                FontManager::DrawFormat(textX, textY, GetColor(255, 220, 100), FontSize::Small,
                    L"Active Waves: %d | Time To Impact: %.2fs", game.boss_.GetActiveTsunamiCount(), timeHit);
            }
            else
            {
                FontManager::DrawFormat(textX, textY, GetColor(180, 200, 220), FontSize::Small,
                    L"Active Waves: %d | Time To Impact: --", game.boss_.GetActiveTsunamiCount());
            }
            textY += 18;
        }

        // Dòng tóm tắt gần nhất (Summary)
        FontManager::Draw(textX, textY, L"--------------------------------------------", GetColor(70, 90, 120), FontSize::Small);
        textY += 13;
        FontManager::DrawFormat(textX, textY, GetColor(140, 255, 170), FontSize::Small,
            L"Summary: %s", game.GetLastSummary());
        textY += 18;

        FontManager::Draw(textX, textY, L"[DEBUG TEST KEYS]:", GetColor(255, 205, 80), FontSize::Small); textY += 17;
        FontManager::Draw(textX, textY, L"F8: Skip Wave  |  Shift+F8: Kill Boss/Skip Card", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"Ctrl+1..5: Jump Wave  |  F6/Ctrl+6: Go To Boss", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"F9: God Mode  |  Shift+F9: Instant Defeat", GetColor(255, 180, 120), FontSize::Small); textY += 16;
        FontManager::Draw(textX, textY, L"F4: Full HP + Reset CD  |  F7: Boss Tsunami", GetColor(255, 180, 120), FontSize::Small); textY += 16;
    }
}
