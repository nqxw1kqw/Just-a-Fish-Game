#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <atomic>
#include "GamePrototype.h"
#include "DxConv.h"
#include "Render3DUtil.h"
#include "DxLib.h"
#include "FontManager.h"
#include "SoundManager.h"

void GamePrototype::LoadHighscore()
{
    highscore_ = HighscoreData{};
    FILE* fp = nullptr;
    if (fopen_s(&fp, "Data/highscore.dat", "rb") == 0 && fp)
    {
        HighscoreData temp{};
        size_t read = fread(&temp, sizeof(HighscoreData), 1, fp);
        if (read == 1 && temp.magic == 0x4F444F52)
        {
            highscore_ = temp;
        }
        fclose(fp);
    }
}

void GamePrototype::SaveHighscore()
{
    // Task 10 T4e: Không lưu kỷ lục nếu đã dùng phím debug
    if (debugController_.WasDebugUsed()) return;

    bool updated = false;
    if (currentWave_ > highscore_.maxWave)
    {
        highscore_.maxWave = currentWave_;
        updated = true;
    }
    int totalKills = enemies_.GetTotalKills();
    if (totalKills > highscore_.maxKills)
    {
        highscore_.maxKills = totalKills;
        updated = true;
    }
    if (gameTime_ > highscore_.maxSurvivalTime)
    {
        highscore_.maxSurvivalTime = gameTime_;
        updated = true;
    }

    if (updated)
    {
        CreateDirectory(L"Data", NULL);
        FILE* fp = nullptr;
        if (fopen_s(&fp, "Data/highscore.dat", "wb") == 0 && fp)
        {
            fwrite(&highscore_, sizeof(HighscoreData), 1, fp);
            fclose(fp);
            AppLogAdd(L"[HIGHSCORE] Saved: Wave=%d, Kills=%d, Time=%.1fs\n",
                highscore_.maxWave, highscore_.maxKills, highscore_.maxSurvivalTime);
        }
    }
}

void GamePrototype::Init()
{
    effects_.Init();
    player_.Init();
    enemies_.Init();
    weapon_.Init();
    cardManager_.Init();
    boss_.Init();
    LoadHighscore();
    Reset();
    state_ = GameState::Title; // Luôn bắt đầu từ màn Title (G1, Task 10 T4a)
}

void GamePrototype::Shutdown()
{
    enemies_.Shutdown();
    cardManager_.Shutdown();
}

void GamePrototype::Reset()
{
    playerStats_ = PlayerStats{};

    effects_.Reset();
    player_.Reset();
    enemies_.Reset();
    weapon_.Reset();
    cardManager_.Reset();
    boss_.Reset();

    enemies_.SetEndless(mode_ == GameMode::Endless);
    cardManager_.SetEndless(mode_ == GameMode::Endless);

    hitStopTimer_ = 0.0f;

    ReapplyAllStats();

    StartWave(1);

    cameraController_.Reset(player_.GetPosition());

    gameTime_ = 0.0f;
    debugController_.Reset();
}

void GamePrototype::ReapplyAllStats()
{
    playerStats_ = PlayerStats{};
    cardManager_.ApplyToStats(playerStats_);

    player_.ApplyStats(playerStats_);
    player_.SetHooks(
        cardManager_.HasHook(HookType::OnHitTaken), // Cá nóc
        cardManager_.HasHook(HookType::OnLand),     // Mưa đạn
        cardManager_.HasHook(HookType::OnBounce)    // Nảy liên hoàn
    );

    weapon_.ApplyStats(playerStats_);
    weapon_.SetHooks(
        cardManager_.HasHook(HookType::OnHitDealt), // Vây điện
        cardManager_.HasHook(HookType::OnApex),     // Cá chép vượt thác
        cardManager_.HasHook(HookType::OnKill)      // Cá mập đói
    );
}

void GamePrototype::StartWave(int wave)
{
    currentWave_ = wave;
    state_ = GameState::Wave;
    waveTimer_ = 0.0f;

    if (mode_ == GameMode::Classic)
    {
        // G2: 5 wave, mỗi wave khoảng 30s - 40s
        if (wave <= 2) waveDuration_ = 30.0f;
        else if (wave <= 4) waveDuration_ = 35.0f;
        else waveDuration_ = 40.0f;
    }
    else
    {
        // Task 10 T4b: Chế độ Endless thời lượng mỗi wave cố định 35 giây
        waveDuration_ = GameConsts::ENDLESS_WAVE_DURATION;
    }

    player_.ResetDamageTakenThisWave();
    enemies_.SetWave(wave, mode_ == GameMode::Endless);
    enemies_.SetAutoSpawn(true);
    SoundManager::Instance().PlayBGM(BgmId::Battle, true);
}

void GamePrototype::StartBossFight()
{
    state_ = GameState::Boss;
    player_.ResetDamageTakenThisWave();
    enemies_.KillAll(effects_); // Dọn quái nhỏ chuẩn bị sàn đấu
    enemies_.SetAutoSpawn(false); // Trong trận Boss, quái chỉ xuất hiện theo đòn của Boss
    boss_.Spawn(Vec3{ 0.0f, 0.0f, 0.0f });
    effects_.AddTrauma(0.5f);
    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
}

void GamePrototype::OnWaveComplete()
{
    // Tóm tắt Wave: sinh X, hạ Y, nhận Z sát thương, dọn trong T giây (Task 10 T1)
    int spawned = enemies_.GetSpawnedThisWave();
    int killed = enemies_.GetKilledThisWave();
    float dmgTaken = player_.GetDamageTakenThisWave();
    float timeSpent = waveTimer_;
    swprintf_s(lastSummaryBuffer_, L"Wave %d: sinh %d, hạ %d, nhận %.1f sát thương, dọn trong %.1fs",
               currentWave_, spawned, killed, dmgTaken, timeSpent);
    AppLogAdd(L"[SUMMARY] %s\n", lastSummaryBuffer_);

    // Hết giờ wave: Quái còn lại biến mất (G2)
    enemies_.KillAll(effects_);

    // G3: Cuối mỗi wave hồi 30% máu tối đa
    player_.Heal(player_.GetMaxHp() * GameConsts::WAVE_HEAL_PERCENT);
    effects_.SpawnHitParticles(player_.GetPosition() + Vec3{ 0, 15, 0 }, 25, GetColor(80, 255, 120));

    // Hook 11: Vảy hồi phục (Heal thêm 15% max HP - Phần C boost)
    if (cardManager_.HasHook(HookType::OnWaveEnd))
    {
        player_.Heal(player_.GetMaxHp() * 0.15f);
        effects_.SpawnHitParticles(player_.GetPosition() + Vec3{ 0, 15, 0 }, 15, GetColor(120, 255, 180));
    }

    // Task 10 T4d: Trong Endless, nếu pool thẻ đã hết sạch
    if (mode_ == GameMode::Endless && cardManager_.GetAvailableCardsPoolCount() == 0)
    {
        player_.Heal(player_.GetMaxHp() * GameConsts::ENDLESS_EMPTY_POOL_HEAL);
        effects_.SpawnHitParticles(player_.GetPosition() + Vec3{ 0, 20, 0 }, 30, GetColor(255, 215, 60));

        if (currentWave_ % 5 == 0)
        {
            state_ = GameState::BossIntro;
            bossIntroTimer_ = 2.0f;
            int bossIdx = currentWave_ / 5;
            boss_.SetBossDifficulty(bossIdx);
            SoundManager::Instance().PlayBGM(BgmId::Boss, true);
            SoundManager::Instance().PlaySFX(SfxId::WhaleGroan);
        }
        else
        {
            StartWave(currentWave_ + 1);
        }
        return;
    }

    // Chuyển sang CardPick
    state_ = GameState::CardPick;
    SoundManager::Instance().PlaySFX(SfxId::CardFlip);
    cardManager_.Roll3Cards(currentWave_);
}

void GamePrototype::SkipCurrentWave()
{
    if (state_ == GameState::Wave)
    {
        debugController_.MarkDebugUsed();
        effects_.Reset(); // D3: dọn sạch hiệu ứng wave
        OnWaveComplete();
    }
    // Ở trận Boss không có tác dụng (D2)
}

void GamePrototype::SkipToWave(int waveTarget)
{
    if (waveTarget < 1 || waveTarget > 5) return;
    debugController_.MarkDebugUsed();

    // Reset thẻ và cấp ngẫu nhiên (waveTarget - 1) thẻ
    cardManager_.Reset();
    cardManager_.GrantRandomCards(waveTarget - 1);

    // Dọn dẹp trạng thái các thực thể
    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();

    ReapplyAllStats();
    StartWave(waveTarget);
}

void GamePrototype::SkipToEndlessWave(int waveTarget)
{
    if (waveTarget < 1) return;
    debugController_.MarkDebugUsed();
    mode_ = GameMode::Endless;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);

    cardManager_.Reset();
    int cardsToGrant = (std::min)(15, waveTarget - 1);
    cardManager_.GrantRandomCards(cardsToGrant);

    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();

    ReapplyAllStats();
    StartWave(waveTarget);
}

void GamePrototype::SkipToBoss()
{
    debugController_.MarkDebugUsed();

    // Cấp ngẫu nhiên 5 thẻ tương ứng vượt qua 5 wave (D2: Ctrl+6)
    cardManager_.Reset();
    cardManager_.GrantRandomCards(5);

    // Dọn dẹp trạng thái các thực thể
    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();

    ReapplyAllStats();
    StartBossFight();
}

void GamePrototype::Update(float dt)
{
    // Cập nhật hệ thống âm thanh (Fade BGM mỗi tick 120Hz)
    SoundManager::Instance().Update(dt);

    // Phím Reset (R)
    if (CheckHitKey(KEY_INPUT_R))
    {
        Reset();
        return;
    }

    // Xử lý các phím Debug / Cheat / Camera tinh chỉnh (Phần D)
    debugController_.Update(dt, *this);

    // 0. Màn hình Tiêu đề (Title - G1, Task 10 T4a)
    if (state_ == GameState::Title)
    {
        SoundManager::Instance().PlayBGM(BgmId::Title, true);

        static bool upPrev = false, downPrev = false, k1Prev = false, k2Prev = false;
        bool upNow = (CheckHitKey(KEY_INPUT_UP) || CheckHitKey(KEY_INPUT_W)) != 0;
        bool downNow = (CheckHitKey(KEY_INPUT_DOWN) || CheckHitKey(KEY_INPUT_S)) != 0;
        bool k1Now = (CheckHitKey(KEY_INPUT_1) || CheckHitKey(KEY_INPUT_NUMPAD1)) != 0;
        bool k2Now = (CheckHitKey(KEY_INPUT_2) || CheckHitKey(KEY_INPUT_NUMPAD2)) != 0;

        if (upNow && !upPrev) titleSelectedMode_ = 0;
        if (downNow && !downPrev) titleSelectedMode_ = 1;

        if (k1Now && !k1Prev)
        {
            mode_ = GameMode::Classic;
            titleSelectedMode_ = 0;
            Reset();
            StartWave(1);
            return;
        }
        if (k2Now && !k2Prev)
        {
            mode_ = GameMode::Endless;
            titleSelectedMode_ = 1;
            Reset();
            StartWave(1);
            return;
        }
        upPrev = upNow; downPrev = downNow; k1Prev = k1Now; k2Prev = k2Now;

        if (CheckHitKey(KEY_INPUT_SPACE) || CheckHitKey(KEY_INPUT_RETURN))
        {
            mode_ = (titleSelectedMode_ == 1) ? GameMode::Endless : GameMode::Classic;
            Reset();
            StartWave(1);
            return;
        }

        effects_.Update(dt, player_.GetPosition());
        return;
    }

    // Kiểm tra chết (Game Over)
    if (state_ != GameState::GameOver && state_ != GameState::Victory && state_ != GameState::Title)
    {
        if (player_.GetHp() <= 0.0f)
        {
            state_ = GameState::GameOver;
            effects_.AddTrauma(0.85f);
            SoundManager::Instance().PlayBGM(BgmId::GameOver, true);
            if (mode_ == GameMode::Endless)
            {
                SaveHighscore();
            }
            return;
        }
    }

    if (state_ == GameState::GameOver || state_ == GameState::Victory)
    {
        if (CheckHitKey(KEY_INPUT_R))
        {
            Reset();
            StartWave(1);
            return;
        }
        if (CheckHitKey(KEY_INPUT_T))
        {
            Reset();
            state_ = GameState::Title;
            SoundManager::Instance().PlayBGM(BgmId::Title, true);
            return;
        }

        // Vẫn cập nhật hạt hiệu ứng và camera khi ở màn hình kết thúc
        effects_.Update(dt, player_.GetPosition());
        return;
    }

    // --- XỬ LÝ THEO TRẠNG THÁI GAME ---
    if (state_ == GameState::Wave)
    {
        gameTime_ += dt;

        // 1. Luôn cập nhật Player để bơi lội mượt mà liên tục, không bao giờ bị đứng lại sau khi đáp đất!
        player_.Update(dt, effects_);

        // 2. Tiếp đất -> Sinh sóng nước đẩy quái và gây sát thương (J6, J7)
        if (player_.ConsumeLandEvent())
        {
            int hitCount = enemies_.ApplyShockwave(
                player_.GetPosition(),
                player_.GetSlamRadius(),
                player_.GetSlamDamage(),
                player_.GetSlamKnockback(),
                effects_
            );

            // Hit stop & Camera Trauma khi tiếp đất trúng quái (J7)
            if (hitCount > 0)
            {
                // Khựng quái 15-30ms tạo độ nén uy lực, hoàn toàn không chặn chuyển động của cá
                float stopMs = (std::min)(30.0f, 15.0f + 3.0f * hitCount);
                hitStopTimer_ = stopMs / 1000.0f;

                float trauma = std::clamp(0.05f + 0.02f * hitCount, 0.05f, 0.35f);
                effects_.AddTrauma(trauma);
            }
        }

        // Tạm dừng nhịp quái khi đang trúng đòn hit-stop
        if (hitStopTimer_ > 0.0f)
        {
            hitStopTimer_ -= dt;
            effects_.Update(dt, player_.GetPosition());
        }
        else
        {
            waveTimer_ += dt;

            // 3. Hook 3: Mưa đạn khi tiếp đất
            if (player_.ConsumeBulletRainTrigger())
            {
                float rainDmg = playerStats_.bulletDamage.GetValue() * 0.75f;
                weapon_.ShootRadial(player_.GetPosition(), 8, rainDmg, playerStats_.bulletSpeed.GetValue());
            }

            // 4. Hook 6: Cá nóc bắn gai phản đòn khi bị trúng đòn
            if (player_.ConsumePufferfishTrigger())
            {
                float spikeDmg = playerStats_.bulletDamage.GetValue() * 1.0f;
                weapon_.ShootRadial(player_.GetPosition(), 8, spikeDmg, playerStats_.bulletSpeed.GetValue() * 0.9f);
            }

            // 5. Cập nhật kẻ địch & vũ khí (Gai phản đòn Pufferfish & súng)
            enemies_.Update(dt, player_, effects_);
            weapon_.Update(dt, player_, enemies_, effects_, nullptr);

            // 6. Cập nhật hạt hiệu ứng & sóng nước
            effects_.Update(dt, player_.GetPosition());

            // 7. Nhặt hạt kinh nghiệm / tiền
            int collectedExp = effects_.CollectExpOrbs(player_.GetPosition(), 45.0f);
            if (collectedExp > 0)
            {
                player_.AddExp(collectedExp);
            }

            // 8. Kiểm tra hoàn thành Wave (G2: Hết giờ thì quái biến mất rồi sang CardPick)
            if (waveTimer_ >= waveDuration_)
            {
                OnWaveComplete();
            }
        }
    }
    else if (state_ == GameState::CardPick)
    {
        bool picked = cardManager_.Update(dt);
        effects_.Update(dt, player_.GetPosition());

        if (picked)
        {
            ReapplyAllStats();

            if (mode_ == GameMode::Classic)
            {
                // Kiểm tra tiến sang Boss hoặc Wave tiếp theo (G1, G2)
                if (currentWave_ >= 5)
                {
                    state_ = GameState::BossIntro;
                    bossIntroTimer_ = 2.0f; // 2 giây giới thiệu Boss (G1, B4)
                    enemies_.KillAll(effects_);
                    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
                    SoundManager::Instance().PlaySFX(SfxId::WhaleGroan);
                }
                else
                {
                    StartWave(currentWave_ + 1);
                }
            }
            else
            {
                // Chế độ Endless: Boss xuất hiện định kỳ mỗi 5 wave (Task 10 T4c)
                if (currentWave_ % 5 == 0)
                {
                    state_ = GameState::BossIntro;
                    bossIntroTimer_ = 2.0f;
                    enemies_.KillAll(effects_);
                    int bossIdx = currentWave_ / 5;
                    boss_.SetBossDifficulty(bossIdx);
                    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
                    SoundManager::Instance().PlaySFX(SfxId::WhaleGroan);
                }
                else
                {
                    StartWave(currentWave_ + 1);
                }
            }
        }
    }
    else if (state_ == GameState::BossIntro)
    {
        bossIntroTimer_ -= dt;
        effects_.Update(dt, player_.GetPosition());
        if (bossIntroTimer_ <= 0.0f)
        {
            StartBossFight();
        }
    }
    else if (state_ == GameState::Boss)
    {
        gameTime_ += dt;

        // 1. Luôn cập nhật Player để bơi lội mượt mà liên tục, không bị đứng lại sau khi đáp đất!
        player_.Update(dt, effects_);

        // 2. Tiếp đất -> Gây sát thương sóng nước lên quái và Boss (J6, J7)
        if (player_.ConsumeLandEvent())
        {
            int hitCount = enemies_.ApplyShockwave(
                player_.GetPosition(),
                player_.GetSlamRadius(),
                player_.GetSlamDamage(),
                player_.GetSlamKnockback(),
                effects_
            );

            // Sóng nước trúng Boss nếu đứng gần
            float dBoss = (boss_.GetPosition() - player_.GetPosition()).Length();
            if (dBoss <= player_.GetSlamRadius() + boss_.GetHitboxRadius())
            {
                boss_.TakeDamage(player_.GetSlamDamage(), effects_);
                hitCount++;
            }

            if (hitCount > 0)
            {
                float stopMs = (std::min)(30.0f, 15.0f + 3.0f * hitCount);
                hitStopTimer_ = stopMs / 1000.0f;

                float trauma = std::clamp(0.05f + 0.02f * hitCount, 0.05f, 0.35f);
                effects_.AddTrauma(trauma);
            }
        }

        // Tạm dừng nhịp đòn tấn công của Boss/quái khi trúng đòn dập đất
        if (hitStopTimer_ > 0.0f)
        {
            hitStopTimer_ -= dt;
            effects_.Update(dt, player_.GetPosition());
        }
        else
        {

            // 3. Hooks
            if (player_.ConsumeBulletRainTrigger())
            {
                float rainDmg = playerStats_.bulletDamage.GetValue() * 0.75f;
                weapon_.ShootRadial(player_.GetPosition(), 8, rainDmg, playerStats_.bulletSpeed.GetValue());
            }

            if (player_.ConsumePufferfishTrigger())
            {
                float spikeDmg = playerStats_.bulletDamage.GetValue() * 1.0f;
                weapon_.ShootRadial(player_.GetPosition(), 8, spikeDmg, playerStats_.bulletSpeed.GetValue() * 0.9f);
            }

            // 4. Cập nhật Boss Cá Voi
            boss_.Update(dt, player_, enemies_, effects_);

            // 5. Cập nhật quái đệ do Boss triệu hồi
            enemies_.Update(dt, player_, effects_);

            // 6. Cập nhật vũ khí bắn quái & Boss (Gai phản đòn Pufferfish & súng)
            weapon_.Update(dt, player_, enemies_, effects_, &boss_);

            // 7. Cập nhật hiệu ứng
            effects_.Update(dt, player_.GetPosition());

            // 8. Thu thập hạt EXP (Trận Boss không có level-up theo yêu cầu)
            effects_.CollectExpOrbs(player_.GetPosition(), 45.0f);

            // 9. Kiểm tra Boss bị hạ gục
            if (boss_.IsDead())
            {
                if (mode_ == GameMode::Classic)
                {
                    state_ = GameState::Victory;
                    effects_.AddTrauma(0.7f);
                    enemies_.KillAll(effects_);
                    SoundManager::Instance().PlayBGM(BgmId::Victory, true);
                    swprintf_s(lastSummaryBuffer_, L"Boss: %.1f giây", boss_.GetBossFightTimer());
                    AppLogAdd(L"[SUMMARY] %s\n", lastSummaryBuffer_);
                }
                else
                {
                    // Endless Mode: Hạ gục 1 lượt Boss định kỳ! (Task 10 T4c)
                    effects_.AddTrauma(0.7f);
                    enemies_.KillAll(effects_);
                    swprintf_s(lastSummaryBuffer_, L"Boss Tier %d: %.1f giây", boss_.GetBossTier(), boss_.GetBossFightTimer());
                    AppLogAdd(L"[SUMMARY] %s\n", lastSummaryBuffer_);

                    // Hồi 50% máu tối đa thưởng diệt Boss
                    player_.Heal(player_.GetMaxHp() * 0.50f);
                    effects_.SpawnHitParticles(player_.GetPosition() + Vec3{ 0, 20, 0 }, 30, GetColor(255, 230, 80));

                    // Tiến sang wave kế tiếp qua CardPick
                    currentWave_++;
                    state_ = GameState::CardPick;
                    cardManager_.Roll3Cards(currentWave_);
                }
            }
        }
    }

    // --- CẬP NHẬT CAMERA 3D (C1, C2, C3) ---
    cameraController_.Update(dt, player_.GetPosition(), effects_);
}

void GamePrototype::DrawArena() const
{
    float half = GameConsts::ARENA_HALF_SIZE;

    // 1. Mặt sàn chính
    VECTOR floorP1 = VGet(-half, -2.0f, -half);
    VECTOR floorP2 = VGet(half, 0.0f, half);
    Render3D::DrawBox3D(floorP1, floorP2, GetColor(18, 26, 42), TRUE);

    // 2. Lưới ô vuông 3D trên sàn (Grid)
    int gridColor = GetColor(35, 55, 85);
    float step = 100.0f;
    for (float x = -half; x <= half; x += step)
    {
        DrawLine3D(VGet(x, 0.2f, -half), VGet(x, 0.2f, half), gridColor);
    }
    for (float z = -half; z <= half; z += step)
    {
        DrawLine3D(VGet(-half, 0.2f, z), VGet(half, 0.2f, z), gridColor);
    }

    // 3. Đường viền tâm sàn đấu
    Render3D::DrawCircle3D(Vec3{ 0.0f, 0.4f, 0.0f }, 160.0f, 48, GetColor(50, 90, 130), false);

    // 4. Bờ tường rào 4 cạnh (3D box viền neon xanh ngọc)
    float wallThick = 18.0f;
    float wallHeight = 24.0f;
    int wallColor = GetColor(30, 80, 140);
    int wallEdge = GetColor(80, 180, 255);

    // Tường Bắc & Nam
    Vec3 northCenter{ 0.0f, wallHeight * 0.5f, half + wallThick * 0.5f };
    Vec3 southCenter{ 0.0f, wallHeight * 0.5f, -half - wallThick * 0.5f };
    Vec3 wallHSizeZ{ half + wallThick, wallHeight * 0.5f, wallThick * 0.5f };
    Render3D::DrawOrientedBox3D(northCenter, wallHSizeZ, 0.0f, wallColor, wallEdge, true);
    Render3D::DrawOrientedBox3D(southCenter, wallHSizeZ, 0.0f, wallColor, wallEdge, true);

    // Tường Đông & Tây
    Vec3 eastCenter{ half + wallThick * 0.5f, wallHeight * 0.5f, 0.0f };
    Vec3 westCenter{ -half - wallThick * 0.5f, wallHeight * 0.5f, 0.0f };
    Vec3 wallHSizeX{ wallThick * 0.5f, wallHeight * 0.5f, half + wallThick };
    Render3D::DrawOrientedBox3D(eastCenter, wallHSizeX, 0.0f, wallColor, wallEdge, true);
    Render3D::DrawOrientedBox3D(westCenter, wallHSizeX, 0.0f, wallColor, wallEdge, true);
}

void GamePrototype::Draw() const
{
    // 1. Vẽ môi trường sàn đấu 3D
    DrawArena();

    // 2. Vẽ Decal báo hiệu trước đòn đánh của Boss trên mặt sàn
    if (state_ == GameState::Boss || state_ == GameState::BossIntro)
    {
        boss_.Draw();
    }

    // 3. Vẽ sóng nước và hạt vụn 3D
    effects_.Draw();

    // 4. Vẽ quái vật 3D
    enemies_.Draw();

    // 5. Vẽ cá chính 3D
    player_.Draw();

    // 6. Vẽ súng tự ngắm và đạn 3D
    weapon_.Draw(player_);

    // 7. Vẽ số sát thương nổi (Floating Damage Numbers) trên đầu quái
    effects_.DrawDamagePopups();

    // 8. Vẽ giao diện 2D (HUD)
    uiRenderer_.DrawHUD(*this);

    // 8. Vẽ thanh máu Boss nếu đang đấu Boss
    if (state_ == GameState::Boss)
    {
        uiRenderer_.DrawBossHealthBar(boss_);
    }

    // 9. Vẽ màn hình chọn thẻ
    if (state_ == GameState::CardPick)
    {
        cardManager_.Draw();
    }

    // 10. Màn hình Chuyển cảnh Boss Intro
    if (state_ == GameState::BossIntro)
    {
        uiRenderer_.DrawBossIntroScreen();
    }

    // 11. Màn hình Tiêu đề Title
    if (state_ == GameState::Title)
    {
        uiRenderer_.DrawTitleScreen(*this);
    }

    // 12. Màn hình Thắng / Thua
    if (state_ == GameState::Victory)
    {
        uiRenderer_.DrawVictoryScreen(*this);
    }
    else if (state_ == GameState::GameOver)
    {
        uiRenderer_.DrawGameOverScreen(*this);
    }
}
