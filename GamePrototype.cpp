#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include "GamePrototype.h"
#include "SeabedFloor.h"
#include "LowPolyModels.h"
#include "DxConv.h"
#include "Render3DUtil.h"
#include "DxLib.h"
#include "FontManager.h"
#include "Localization.h"
#include "SoundManager.h"
#include "Input.h"

namespace
{
    // Định dạng file kỷ lục v2: có số phiên bản + checksum.
    // File cũ (v1) chỉ là HighscoreData thô (16 byte) - vẫn đọc được để tương thích ngược.
    constexpr unsigned int HIGHSCORE_MAGIC = 0x4F444F52; // 'ODOR'
    constexpr unsigned int HIGHSCORE_VERSION_V2 = 2;
    constexpr unsigned int HIGHSCORE_VERSION = 3;
    // Đường dẫn cũ (tương đối theo thư mục làm việc) - chỉ dùng để đọc/migrate

    struct HighscoreFileV1 // struct thô cũ, 16 byte
    {
        unsigned int magic;
        int maxWave;
        int maxKills;
        float maxSurvivalTime;
    };

    struct HighscoreFileV2
    {
        unsigned int magic = HIGHSCORE_MAGIC;
        unsigned int version = HIGHSCORE_VERSION_V2;
        int maxWave = 0;
        int maxKills = 0;
        float maxSurvivalTime = 0.0f;
        unsigned int checksum = 0; // FNV-1a của toàn bộ các byte phía trước
    };

    struct HighscoreFileV3
    {
        unsigned int magic = HIGHSCORE_MAGIC;
        unsigned int version = HIGHSCORE_VERSION;
        int maxWave = 0;
        int maxKills = 0;
        float maxSurvivalTime = 0.0f;
        int maxScore = 0;
        unsigned int checksum = 0; // FNV-1a của toàn bộ các byte phía trước
    };
    static_assert(sizeof(HighscoreFileV1) != sizeof(HighscoreFileV2) && sizeof(HighscoreFileV2) != sizeof(HighscoreFileV3),
        "Các phiên bản file kỷ lục phải khác kích thước để phân biệt");

    template <typename T>
    unsigned int ComputeHighscoreChecksum(const T& f)
    {
        const unsigned char* p = reinterpret_cast<const unsigned char*>(&f);
        const size_t len = offsetof(T, checksum);
        unsigned int h = 2166136261u;
        for (size_t i = 0; i < len; ++i)
        {
            h ^= p[i];
            h *= 16777619u;
        }
        return h;
    }

    // <thư mục chứa exe>\Data\<name> : không phụ thuộc thư mục làm việc (VS / double-click / shortcut)
    void BuildExeDataPath(wchar_t* out, size_t outCount, const wchar_t* name)
    {
        wchar_t exe[MAX_PATH] = {};
        DWORD n = GetModuleFileNameW(nullptr, exe, MAX_PATH);
        if (n == 0 || n >= MAX_PATH)
        {
            swprintf_s(out, outCount, L"Data\\%s", name);
            return;
        }
        wchar_t* slash = wcsrchr(exe, L'\\');
        if (slash) *(slash + 1) = L'\0';
        swprintf_s(out, outCount, L"%sData\\%s", exe, name);
    }

    // Đọc file kỷ lục (v1/v2/v3). Trả về false nếu không có file / không hợp lệ.
    bool ReadHighscoreFile(const wchar_t* path, HighscoreData& out)
    {
        FILE* fp = nullptr;
        if (_wfopen_s(&fp, path, L"rb") != 0 || !fp) return false;
        unsigned char buf[sizeof(HighscoreFileV3) + 1] = {};
        size_t size = fread(buf, 1, sizeof(buf), fp);
        fclose(fp);

        if (size == sizeof(HighscoreFileV3))
        {
            HighscoreFileV3 f{};
            memcpy(&f, buf, sizeof(f));
            if (f.magic != HIGHSCORE_MAGIC || f.version != HIGHSCORE_VERSION || f.checksum != ComputeHighscoreChecksum(f)) return false;
            out.maxWave = f.maxWave; out.maxKills = f.maxKills; out.maxSurvivalTime = f.maxSurvivalTime; out.maxScore = f.maxScore;
            return true;
        }
        if (size == sizeof(HighscoreFileV2))
        {
            HighscoreFileV2 f{};
            memcpy(&f, buf, sizeof(f));
            if (f.magic != HIGHSCORE_MAGIC || f.version != HIGHSCORE_VERSION_V2 || f.checksum != ComputeHighscoreChecksum(f)) return false;
            out.maxWave = f.maxWave; out.maxKills = f.maxKills; out.maxSurvivalTime = f.maxSurvivalTime; out.maxScore = 0;
            return true;
        }
        if (size == sizeof(HighscoreFileV1))
        {
            HighscoreFileV1 f{};
            memcpy(&f, buf, sizeof(f));
            if (f.magic != HIGHSCORE_MAGIC) return false;
            out.maxWave = f.maxWave; out.maxKills = f.maxKills; out.maxSurvivalTime = f.maxSurvivalTime; out.maxScore = 0;
            return true;
        }
        return false;
    }
}

void GamePrototype::LoadHighscore()
{
    highscore_ = HighscoreData{};
    wchar_t path[MAX_PATH] = {};
    BuildExeDataPath(path, MAX_PATH, L"highscore_campaign.dat");
    ReadHighscoreFile(path, highscore_);
}

void GamePrototype::SaveHighscore()
{
    // Task 10 T4e: Không lưu kỷ lục nếu đã dùng phím debug
    if (debugController_.WasDebugUsed()) return;

    HighscoreData& hs = highscore_;
    bool updated = false;
    if (currentWave_ > hs.maxWave)
    {
        hs.maxWave = currentWave_;
        updated = true;
    }
    int totalKills = enemies_.GetTotalKills();
    if (totalKills > hs.maxKills)
    {
        hs.maxKills = totalKills;
        updated = true;
    }
    if (gameTime_ > hs.maxSurvivalTime)
    {
        hs.maxSurvivalTime = gameTime_;
        updated = true;
    }
    int score = ResultScreen::ComputeScore(BuildResultStats());
    if (score > hs.maxScore)
    {
        hs.maxScore = score;
        updated = true;
    }
    if (!updated) return;

    wchar_t dir[MAX_PATH] = {}, path[MAX_PATH] = {}, tmpPath[MAX_PATH] = {};
    BuildExeDataPath(dir, MAX_PATH, L"");
    BuildExeDataPath(path, MAX_PATH, L"highscore_campaign.dat");
    BuildExeDataPath(tmpPath, MAX_PATH, L"highscore_campaign.tmp");
    CreateDirectoryW(dir, NULL); // đã tồn tại thì bỏ qua

    HighscoreFileV3 f{};
    f.maxWave = hs.maxWave;
    f.maxKills = hs.maxKills;
    f.maxSurvivalTime = hs.maxSurvivalTime;
    f.maxScore = hs.maxScore;
    f.checksum = ComputeHighscoreChecksum(f);

    // Ghi an toàn: ghi ra file tạm, kiểm tra lỗi, rồi mới thay thế file thật
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, tmpPath, L"wb") != 0 || !fp)
    {
        AppLogAdd(L"[HIGHSCORE] Không mở được file tạm để ghi: %s\n", tmpPath);
        return;
    }
    bool ok = (fwrite(&f, sizeof(f), 1, fp) == 1);
    ok = (fflush(fp) == 0) && ok;
    ok = (fclose(fp) == 0) && ok;

    if (ok && MoveFileExW(tmpPath, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        AppLogAdd(L"[HIGHSCORE] Saved %s: Wave=%d, Kills=%d, Time=%.1fs, Score=%d\n",
            path, hs.maxWave, hs.maxKills, hs.maxSurvivalTime, hs.maxScore);
    }
    else
    {
        DeleteFileW(tmpPath);
        AppLogAdd(L"[HIGHSCORE] Lưu kỷ lục thất bại, giữ nguyên file cũ\n");
    }
}

void GamePrototype::SaveRunRecord()
{
    // Chỉ lưu khi đang có một ván Endless (không lưu ở màn Title)
    if (IsCampaign() && state_ != GameState::Title)
    {
        SaveHighscore();
    }
}

void GamePrototype::Init()
{
    effects_.Init();
    LowPolyModels::Load(); // Mô hình low-poly dùng chung (nạp 1 lần)
    player_.Init();
    enemies_.Init();
    weapon_.Init();
    cardManager_.Init();
    uiRenderer_.Init();
    resultScreen_.Init();
    boss_.Init();
    LoadHighscore();
    Reset();
    // 日本語専用版はタイトルから開始します。
    state_ = GameState::Title;
}

void GamePrototype::Shutdown()
{
    // Thoát game (ESC / đóng cửa sổ) giữa ván Endless: vẫn lưu kỷ lục
    SaveRunRecord();

    enemies_.Shutdown();
    cardManager_.Shutdown();
    uiRenderer_.Shutdown();
    resultScreen_.Shutdown();
    effects_.Shutdown();
    LowPolyModels::Release();
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
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    sharkFight_ = false;
    bossKind_ = 0;
    bossesDefeated_ = 0;
    completedWaves_ = 0;
    bossOrder_ = {0, 1, 2, 3, 4};
    // 最初は必ずクジラ。残る4体だけをシャッフルし、重複させません。
    for (int i = GameConsts::CAMPAIGN_BOSSES - 1; i > 1; --i)
        std::swap(bossOrder_[i], bossOrder_[1 + rand() % i]);

    enemies_.SetEndless(IsCampaign());
    cardManager_.SetEndless(IsCampaign());
    activeTip_ = -1;
    ApplyPhaseRules();

    hitStopTimer_ = 0.0f;

    ReapplyAllStats();

    // Lưu ý: Reset() chỉ dọn trạng thái, không tự bắt đầu wave.
    // Người gọi tự gọi StartWave(1) (tránh gọi StartWave(1) hai lần).

    cameraController_.Reset(player_.GetPosition());

    gameTime_ = 0.0f;
    debugController_.Reset();
}

void GamePrototype::ReapplyAllStats()
{
    playerStats_ = PlayerStats{};
    cardManager_.ApplyToStats(playerStats_);
    if (phase_ == RunPhase::Tutorial) // Tutorial: thêm HP tối đa
        playerStats_.maxHp.AddModifier("tutorial", ModifierType::Percent, GameConsts::TUTORIAL_PLAYER_HP_BONUS);
    if (IsCampaign())
    {
        const float step = static_cast<float>(GetProgressionStage() - 1);
        playerStats_.maxHp.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_HP_PCT * step);
        playerStats_.slamDamage.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_DAMAGE_PCT * step);
        playerStats_.shockwaveDamage.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_DAMAGE_PCT * step);
        playerStats_.bulletDamage.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_DAMAGE_PCT * step);
        playerStats_.moveSpeed.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_SPEED_PCT * step);
        playerStats_.airSpeed.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_SPEED_PCT * step);
        playerStats_.jumpCooldown.AddModifier("stage", ModifierType::Percent, GameConsts::STAGE_COOLDOWN_PCT * step);
    }

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

// Điểm vào duy nhất cho mọi lần bắt đầu / chơi lại: Reset() đúng một lần rồi StartStage(1) đúng một lần.
void GamePrototype::StartRun()
{
    phase_ = RunPhase::Tutorial;
    uiRenderer_.LeavePause();
    paused_ = false;
    Reset();
    StartWave(1);
    Input::Instance().Flush();
}

void GamePrototype::StartWave(int wave)
{
    currentWave_ = wave;
    ReapplyAllStats();
    RefreshDamageTakenMul();
    state_ = GameState::Wave;
    waveTimer_ = 0.0f;

    waveDuration_ = IsCampaign() ? GameConsts::CAMPAIGN_WAVE_DURATION : GameConsts::TUTORIAL_WAVE_DURATION;

    player_.ResetDamageTakenThisWave();
    enemies_.SetWave(wave, IsCampaign());
    enemies_.SetAutoSpawn(true);
    SoundManager::Instance().PlayBGM(BgmId::Battle, true);
}

void GamePrototype::StartBossFight()
{
    state_ = GameState::Boss;
    ReapplyAllStats();
    player_.ResetDamageTakenThisWave();
    enemies_.KillAll(effects_);
    enemies_.SetAutoSpawn(false);
    RefreshDamageTakenMul();
    boss_.SetDamageTakenMul(1.0f); shark_.SetDamageTakenMul(1.0f);
    squid_.SetDamageTakenMul(1.0f); turtle_.SetDamageTakenMul(1.0f); swordfish_.SetDamageTakenMul(1.0f);
    boss_.SetHellMode(false); shark_.SetHellMode(false); squid_.SetHellMode(false);
    turtle_.SetHellMode(false); swordfish_.SetHellMode(false);
    const Vec3 center{0,0,0};
    if (bossKind_ == 1) shark_.Spawn(center);
    else if (bossKind_ == 2) squid_.Spawn(center);
    else if (bossKind_ == 3) turtle_.Spawn(center);
    else if (bossKind_ == 4) swordfish_.Spawn(center);
    else boss_.Spawn(center);
    effects_.AddTrauma(0.5f);
    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
}

// ボスの出現順と難易度を、その周回の撃破数から決定します。
void GamePrototype::PrepareBossForRound(int bossIndex)
{
    bossIndex = std::clamp(bossIndex, 0, GameConsts::CAMPAIGN_BOSSES - 1);
    bossKind_ = bossOrder_[bossIndex];
    sharkFight_ = (bossKind_ == 1);
    const int tier = bossIndex + 1;
    const float hpMul = 1.0f + GameConsts::CAMPAIGN_BOSS_HP_PER_CLEAR * bossIndex;
    if (bossKind_ == 1) { shark_.SetBossDifficulty(tier); shark_.SetHpMultiplier(hpMul); }
    else if (bossKind_ == 2) { squid_.SetBossDifficulty(tier); squid_.SetHpMultiplier(hpMul); }
    else if (bossKind_ == 3) { turtle_.SetBossDifficulty(tier); turtle_.SetHpMultiplier(hpMul); }
    else if (bossKind_ == 4) { swordfish_.SetBossDifficulty(tier); swordfish_.SetHpMultiplier(hpMul); }
    else { boss_.SetBossDifficulty(tier); boss_.SetHpMultiplier(hpMul); }
    AppLogAdd(L"[CAMPAIGN] Boss %d/5 kind=%d hpMul=%.2f\n", tier, bossKind_, hpMul);
}

void GamePrototype::BeginBossIntro()
{
    state_ = GameState::BossIntro;
    bossIntroTimer_ = 2.0f;
    enemies_.KillAll(effects_);
    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
    SoundManager::Instance().PlaySFX(bossKind_ == 0 ? SfxId::WhaleGroan : SfxId::BossRoar);
}

void GamePrototype::AdvanceAfterCardPick()
{
    if (!IsCampaign())
    {
        if (currentWave_ >= GameConsts::TUTORIAL_WAVES) FinishTutorial();
        else StartWave(currentWave_ + 1);
        return;
    }
    // ボス戦は通常ウェーブを置き換えません。撃破数で同じボスへの再入場を防ぎます。
    if (currentWave_ % GameConsts::WAVES_PER_BOSS == 0 &&
        bossesDefeated_ < currentWave_ / GameConsts::WAVES_PER_BOSS)
    {
        PrepareBossForRound(bossesDefeated_);
        BeginBossIntro();
        return;
    }
    StartWave(currentWave_ + 1);
}

void GamePrototype::FinishTutorial()
{
    player_.Heal(player_.GetMaxHp() * GameConsts::TUTORIAL_BOSS_CLEAR_HEAL);
    phase_ = RunPhase::Campaign;
    completedWaves_ = 0;
    activeTip_ = -1;
    enemies_.SetEndless(true);
    cardManager_.SetEndless(true);
    ApplyPhaseRules();
    ReapplyAllStats();
    // カードを引き継ぎ、通常ウェーブの番号を1から開始します。
    StartWave(1);
    Input::Instance().Flush();
}

void GamePrototype::ApplyPhaseRules()
{
    RefreshDamageTakenMul();
    enemies_.SetPlayerDmgMul(IsCampaign() ? 1.0f : GameConsts::TUTORIAL_DMG_MUL_VS_ENEMY);
    effects_.SetParticleScale(1.0f);
    enemies_.SetMaxAlive(0);
    enemies_.SetHell(false);
    cardManager_.SetBonusScale(1.0f);
}

void GamePrototype::RefreshDamageTakenMul()
{
    float mul = IsCampaign() ? 1.0f : GameConsts::TUTORIAL_DAMAGE_TAKEN_MUL;
    if (IsCampaign()) mul *= 1.0f - GameConsts::STAGE_DEFENSE_PCT * (GetProgressionStage() - 1);
    player_.SetDamageTakenMul(mul);
}

float GamePrototype::UpdateTutorialTips(float dt)
{
    if (phase_ != RunPhase::Tutorial || tipsSkipped_) { activeTip_ = -1; return 1.0f; }

    const Input& input = Input::Instance();
    if (activeTip_ >= 0 && input.IsPressed(KEY_INPUT_TAB))
    {
        tipsSkipped_ = true; // Bỏ qua toàn bộ gợi ý còn lại
        activeTip_ = -1;
        return 1.0f;
    }

    // Kích hoạt gợi ý kế tiếp theo thứ tự: Move -> Jump -> Hit -> Card -> Boss
    if (activeTip_ < 0)
    {
        int next = -1;
        if (state_ == GameState::Wave)
        {
            if (!tipShown_[Tip_Move]) { if (waveTimer_ >= GameConsts::TIP_START_DELAY) next = Tip_Move; }
            else if (!tipShown_[Tip_Jump]) next = Tip_Jump;
            else if (!tipShown_[Tip_Hit]) next = Tip_Hit;
        }
        else if (state_ == GameState::CardPick && !tipShown_[Tip_Card]) next = Tip_Card;
        else if (state_ == GameState::Boss && !tipShown_[Tip_Boss]) next = Tip_Boss;
        if (next >= 0)
        {
            activeTip_ = next;
            tipShown_[next] = true;
            tipTimer_ = 0.0f;
            tipMoveStart_ = player_.GetPosition();
        }
    }
    if (activeTip_ < 0) return 1.0f;

    tipTimer_ += dt;
    bool done = tipTimer_ >= GameConsts::TIP_MAX_TIME;
    switch (activeTip_)
    {
    case Tip_Move:
    {
        Vec3 d = player_.GetPosition() - tipMoveStart_;
        if (d.x * d.x + d.z * d.z >= GameConsts::TIP_MOVE_DISTANCE * GameConsts::TIP_MOVE_DISTANCE) done = true;
        break;
    }
    case Tip_Jump: if (player_.IsJumping()) done = true; break;
    case Tip_Hit:  if (enemies_.GetTotalKills() > 0) done = true; break;
    case Tip_Card: if (state_ != GameState::CardPick) done = true; break;
    case Tip_Boss: if (tipTimer_ >= GameConsts::TIP_BOSS_SHOW_TIME || state_ != GameState::Boss) done = true; break;
    }
    if (done) { activeTip_ = -1; return 1.0f; }

    // Làm chậm nhẹ (không dừng hẳn) chỉ trong giây đầu khi đang chiến đấu
    if ((state_ == GameState::Wave || state_ == GameState::Boss) && tipTimer_ < GameConsts::TIP_SOFT_PAUSE_TIME)
        return GameConsts::TIP_SOFT_PAUSE_SCALE;
    return 1.0f;
}

void GamePrototype::CompleteCampaign()
{
    AppLogAdd(L"[CAMPAIGN] All %d bosses defeated!\n", bossesDefeated_);
    state_ = GameState::Victory;
    resultScreen_.Enter(true, false);
    effects_.AddTrauma(0.7f);
    enemies_.KillAll(effects_);
    SoundManager::Instance().PlayBGM(BgmId::Victory, true);
    SaveHighscore();
}

void GamePrototype::OnWaveComplete()
{
    completedWaves_ = (std::max)(completedWaves_, currentWave_);
    // Tóm tắt Wave: sinh X, hạ Y, nhận Z sát thương, dọn trong T giây (Task 10 T1)
    int spawned = enemies_.GetSpawnedThisWave();
    int killed = enemies_.GetKilledThisWave();
    float dmgTaken = player_.GetDamageTakenThisWave();
    float timeSpent = waveTimer_;
    swprintf_s(lastSummaryBuffer_, L"ウェーブ %d: 出現 %d、撃破 %d、被ダメ %.1f、経過 %.1f秒",
               currentWave_, spawned, killed, dmgTaken, timeSpent);
    AppLogAdd(L"[SUMMARY] %s\n", lastSummaryBuffer_);

    // 時間終了で出現を止め、残りの敵を片付けてカード選択へ。
    enemies_.SetAutoSpawn(false);
    enemies_.KillAll(effects_);

    // G3: Cuối mỗi wave hồi 30% máu tối đa
    player_.Heal(player_.GetMaxHp() * GameConsts::WAVE_HEAL_PERCENT);
    effects_.PlayFx(FxId::HealWaveEnd, player_.GetPosition() + Vec3{ 0, 15, 0 });

    // Hook 11: Vảy hồi phục (Heal thêm 15% max HP - Phần C boost)
    if (cardManager_.HasHook(HookType::OnWaveEnd))
    {
        player_.Heal(player_.GetMaxHp() * 0.15f);
        effects_.PlayFx(FxId::HealCardScale, player_.GetPosition() + Vec3{ 0, 15, 0 });
    }

    // Task 10 T4d: Trong Endless, nếu pool thẻ đã hết sạch
    if (IsCampaign() && cardManager_.GetAvailableCardsPoolCount() == 0)
    {
        player_.Heal(player_.GetMaxHp() * GameConsts::ENDLESS_EMPTY_POOL_HEAL);
        effects_.PlayFx(FxId::HealPoolBonus, player_.GetPosition() + Vec3{ 0, 20, 0 });

        AdvanceAfterCardPick();
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
    if (waveTarget < 1 || waveTarget > GameConsts::TUTORIAL_WAVES) return;
    debugController_.MarkDebugUsed();
    phase_ = RunPhase::Tutorial;
    completedWaves_ = waveTarget - 1;
    bossesDefeated_ = 0;
    cardManager_.SetEndless(false);
    enemies_.SetEndless(false);

    // Reset thẻ và cấp ngẫu nhiên (waveTarget - 1) thẻ
    cardManager_.Reset();
    cardManager_.GrantRandomCards(waveTarget - 1);

    // Dọn dẹp trạng thái các thực thể
    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    sharkFight_ = false;
    bossKind_ = 0;

    ApplyPhaseRules();
    ReapplyAllStats();
    StartWave(waveTarget);
}

void GamePrototype::SkipToCampaignWave(int waveTarget)
{
    if (waveTarget < 1) return;
    waveTarget = (std::min)(waveTarget, GameConsts::CAMPAIGN_WAVES);
    debugController_.MarkDebugUsed();
    phase_ = RunPhase::Campaign;
    bossesDefeated_ = (waveTarget - 1) / GameConsts::WAVES_PER_BOSS;
    completedWaves_ = waveTarget - 1;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);

    cardManager_.Reset();
    int cardsToGrant = (std::min)(15, waveTarget - 1);
    cardManager_.GrantRandomCards(cardsToGrant);

    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    sharkFight_ = false;
    bossKind_ = 0;

    ApplyPhaseRules();
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
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    sharkFight_ = false;
    bossKind_ = 0;

    phase_ = RunPhase::Campaign;
    bossesDefeated_ = 0;
    completedWaves_ = GameConsts::WAVES_PER_BOSS;
    currentWave_ = GameConsts::WAVES_PER_BOSS;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);
    ApplyPhaseRules();
    PrepareBossForRound(0);
    ReapplyAllStats();
    StartBossFight();
}

void GamePrototype::SkipToSharkBoss()
{
    debugController_.MarkDebugUsed();
    phase_ = RunPhase::Campaign;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);
    cardManager_.Reset();
    cardManager_.GrantRandomCards(5);

    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();

    ReapplyAllStats();
    bossesDefeated_ = 0;
    completedWaves_ = GameConsts::WAVES_PER_BOSS;
    currentWave_ = GameConsts::WAVES_PER_BOSS;
    PrepareBossForRound(bossesDefeated_);
    sharkFight_ = true; // Debug: luôn Cá Mập
    bossKind_ = 1;
    shark_.SetBossDifficulty(1);
    shark_.SetHpMultiplier(1.0f);
    StartBossFight();
}

void GamePrototype::SkipToSquidBoss()
{
    debugController_.MarkDebugUsed();
    phase_ = RunPhase::Campaign;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);
    cardManager_.Reset();
    cardManager_.GrantRandomCards(5);
    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    ReapplyAllStats();
    bossesDefeated_ = 0;
    completedWaves_ = GameConsts::WAVES_PER_BOSS;
    currentWave_ = GameConsts::WAVES_PER_BOSS;
    bossKind_ = 2;
    sharkFight_ = false;
    squid_.SetBossDifficulty(1);
    squid_.SetHpMultiplier(1.0f);
    StartBossFight();
}

void GamePrototype::SkipToTurtleBoss()
{
    debugController_.MarkDebugUsed();
    phase_ = RunPhase::Campaign;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);
    cardManager_.Reset();
    cardManager_.GrantRandomCards(5);
    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    ReapplyAllStats();
    bossesDefeated_ = 0;
    completedWaves_ = GameConsts::WAVES_PER_BOSS;
    currentWave_ = GameConsts::WAVES_PER_BOSS;
    bossKind_ = 3;
    sharkFight_ = false;
    turtle_.SetBossDifficulty(1);
    turtle_.SetHpMultiplier(1.0f);
    StartBossFight();
}

void GamePrototype::SkipToSwordfishBoss()
{
    debugController_.MarkDebugUsed();
    phase_ = RunPhase::Campaign;
    cardManager_.SetEndless(true);
    enemies_.SetEndless(true);
    cardManager_.Reset();
    cardManager_.GrantRandomCards(5);
    enemies_.Reset();
    effects_.Reset();
    player_.Reset();
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    turtle_.Reset();
    swordfish_.Reset();
    ReapplyAllStats();
    bossesDefeated_ = 0;
    completedWaves_ = GameConsts::WAVES_PER_BOSS;
    currentWave_ = GameConsts::WAVES_PER_BOSS;
    bossKind_ = 4;
    sharkFight_ = false;
    swordfish_.SetBossDifficulty(1);
    swordfish_.SetHpMultiplier(1.0f);
    StartBossFight();
}

bool GamePrototype::ConsumeWindowModeToggleRequest()
{
    bool requested = windowModeToggleRequested_;
    windowModeToggleRequested_ = false;
    return requested;
}

// Màn hình tạm dừng (Task #13): ESC / P để mở - đóng.
// ESC ở các màn khác (Title, chọn thẻ, Boss Intro, Thắng/Thua) vẫn thoát game như trước.
bool GamePrototype::UpdatePauseInput()
{
    static bool escPrev = true, pPrev = true, upPrev = true, downPrev = true;
    static bool leftPrev = true, rightPrev = true, enterPrev = true;
    static bool confirmReady = false;
    bool escNow = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
    bool pNow = CheckHitKey(KEY_INPUT_P) != 0;
    bool upNow = (CheckHitKey(KEY_INPUT_UP) || CheckHitKey(KEY_INPUT_W)) != 0;
    bool downNow = (CheckHitKey(KEY_INPUT_DOWN) || CheckHitKey(KEY_INPUT_S)) != 0;
    bool leftNow = (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_A)) != 0;
    bool rightNow = (CheckHitKey(KEY_INPUT_RIGHT) || CheckHitKey(KEY_INPUT_D)) != 0;
    bool enterNow = (CheckHitKey(KEY_INPUT_RETURN) || CheckHitKey(KEY_INPUT_SPACE)) != 0;

    bool escPressed = escNow && !escPrev;
    // Phím P cũng dùng để chỉnh scale model trong Debug HUD, nên chỉ dùng P khi Debug HUD tắt
    bool pPressed = pNow && !pPrev && !debugController_.IsDebugHUDVisible();
    bool upPressed = upNow && !upPrev;
    bool downPressed = downNow && !downPrev;
    bool leftPressed = leftNow && !leftPrev;
    bool rightPressed = rightNow && !rightPrev;
    // Chỉ nhận một lần nhấn mới sau khi đã nhả phím khi mở Pause.
    bool enterReleased = !enterNow && enterPrev && confirmReady;

    escPrev = escNow; pPrev = pNow; upPrev = upNow; downPrev = downNow;
    leftPrev = leftNow; rightPrev = rightNow; enterPrev = enterNow;

    if (!paused_)
    {
        bool canPause = (state_ == GameState::Wave || state_ == GameState::Boss || state_ == GameState::BossIntro)
                        && player_.GetHp() > 0.0f;
        if (canPause && (escPressed || pPressed))
        {
            paused_ = true;
            confirmReady = false;
            pauseSelected_ = PauseItem_Resume;
            uiRenderer_.EnterPause();
            return true;
        }
        if (escPressed && !canPause)
        {
            quitRequested_ = true;
        }
        return false;
    }

    if (escPressed || pPressed)
    {
        uiRenderer_.LeavePause();
        paused_ = false;
        Input::Instance().Flush();
        return true; // tick này vẫn bỏ qua để phím vừa nhấn không lọt vào game
    }

    if (!enterNow) confirmReady = true;

    const PauseMenuScreen::PointerResult pointer = uiRenderer_.UpdatePausePointer(pauseSelected_);

    if (upPressed) pauseSelected_ = (pauseSelected_ + PauseItem_Count - 1) % PauseItem_Count;
    if (downPressed) pauseSelected_ = (pauseSelected_ + 1) % PauseItem_Count;

    SoundManager& sound = SoundManager::Instance();
    if (pointer.volumeItem == PauseItem_BgmVolume || pointer.volumeItem == PauseItem_SfxVolume)
    {
        const SoundChannel channel = pointer.volumeItem == PauseItem_BgmVolume ? SoundChannel::BGM : SoundChannel::SFX;
        if (sound.GetVolume(channel) != pointer.volume) sound.SetVolume(channel, pointer.volume);
        if (pointer.volumeReleased && channel == SoundChannel::SFX) sound.PlaySFX(SfxId::CardFlip);
    }
    int volumeDelta = (rightPressed ? 5 : 0) - (leftPressed ? 5 : 0);
    if (volumeDelta != 0)
    {
        if (pauseSelected_ == PauseItem_BgmVolume)
        {
            sound.SetVolume(SoundChannel::BGM, sound.GetVolume(SoundChannel::BGM) + volumeDelta);
        }
        else if (pauseSelected_ == PauseItem_SfxVolume)
        {
            sound.SetVolume(SoundChannel::SFX, sound.GetVolume(SoundChannel::SFX) + volumeDelta);
            sound.PlaySFX(SfxId::CardFlip); // nghe thử âm lượng SFX
        }
    }

    const bool pointerConfirm = pointer.activated >= 0;
    if (pointerConfirm) pauseSelected_ = pointer.activated;
    bool toggleWindow = (pauseSelected_ == PauseItem_WindowMode) && (enterReleased || pointerConfirm || leftPressed || rightPressed);
    if (toggleWindow)
    {
        // Thực hiện ở main.cpp (ngoài vòng Update) vì ChangeWindowMode reset hệ thống đồ hoạ
        windowModeToggleRequested_ = true;
    }
    else if (enterReleased || pointerConfirm)
    {
        if (pauseSelected_ == PauseItem_Resume)
        {
            uiRenderer_.LeavePause();
            paused_ = false;
            Input::Instance().Flush();
        }
        else if (pauseSelected_ == PauseItem_Title)
        {
            uiRenderer_.LeavePause();
            paused_ = false;
            Input::Instance().Flush();
            SaveRunRecord(); // Pause -> Title giữa ván Endless: không mất kỷ lục
            Reset();
            state_ = GameState::Title;
            sound.PlayBGM(BgmId::Title, true);
        }
    }
    return true;
}

void GamePrototype::BeginRenderInterp(float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    if (!renderInterpEnabled_) alpha = 1.0f; // tắt: vẽ đúng trạng thái tick mới nhất (như trước)

    player_.BeginRenderInterp(alpha);
    enemies_.BeginRenderInterp(alpha);
    boss_.BeginRenderInterp(alpha);
    shark_.BeginRenderInterp(alpha);
    squid_.BeginRenderInterp(alpha);
    turtle_.BeginRenderInterp(alpha);
    swordfish_.BeginRenderInterp(alpha);
    weapon_.BeginRenderInterp(alpha);
    cameraController_.ApplyInterpolated(alpha, effects_);
    renderInterpActive_ = true;
}

void GamePrototype::EndRenderInterp()
{
    if (!renderInterpActive_) return;
    player_.EndRenderInterp();
    enemies_.EndRenderInterp();
    boss_.EndRenderInterp();
    shark_.EndRenderInterp();
    squid_.EndRenderInterp();
    turtle_.EndRenderInterp();
    swordfish_.EndRenderInterp();
    weapon_.EndRenderInterp();
    renderInterpActive_ = false;
}

void GamePrototype::Update(float dt)
{
    // Render interpolation (#26): lưu transform đầu tick (kể cả khi pause -> prev == current)
    player_.SavePrevTransform();
    enemies_.SavePrevTransforms();
    boss_.SavePrevTransform();
    shark_.SavePrevTransform();
    squid_.SavePrevTransform();
    turtle_.SavePrevTransform();
    swordfish_.SavePrevTransform();
    weapon_.SavePrevTransforms();
    cameraController_.SavePrev();

    // Màn hình tạm dừng: đóng băng toàn bộ logic game (chỉ âm thanh vẫn cập nhật fade)
    if (UpdatePauseInput())
    {
        SoundManager::Instance().Update(dt);
        return;
    }

    // Cập nhật hệ thống âm thanh (Fade BGM mỗi tick 120Hz)
    SoundManager::Instance().Update(dt);

    // Chụp trạng thái bàn phím một lần cho cả tick (dùng chung cho mọi module)
    Input::Instance().Update();
    const Input& input = Input::Instance();

    // F10: bật/tắt render interpolation (so sánh độ mượt)
    if (input.IsPressed(KEY_INPUT_F10))
    {
        renderInterpEnabled_ = !renderInterpEnabled_;
    }

    // F11: ẩn/hiện bảng thông tin (stats, thẻ, debug panel, hướng dẫn phím) - chỉ hiển thị, không bật cheat
    if (input.IsPressed(GameConsts::KEY_TOGGLE_PANELS))
    {
        panelsVisible_ = !panelsVisible_;
    }

    // Xử lý các phím Debug / Cheat / Camera tinh chỉnh (Phần D)
    // Debug/cheat keys chỉ chạy trong lúc chơi (không chạy sau màn Language/Title)
    if (state_ != GameState::Title)
        debugController_.Update(dt, *this);

    // 0. Màn hình Tiêu đề (Title - G1, Task 10 T4a)
    if (state_ == GameState::Title)
    {
        SoundManager::Instance().PlayBGM(BgmId::Title, true);

        // SPACE / ENTER: チュートリアルから1つの冒険を開始。
        if (input.IsPressed(KEY_INPUT_SPACE) || input.IsPressed(KEY_INPUT_RETURN))
        {
            StartRun();
            return;
        }

        return;
    }

    // Kiểm tra chết (Game Over)
    if (state_ != GameState::GameOver && state_ != GameState::Victory && state_ != GameState::Title)
    {
        if (player_.GetHp() <= 0.0f)
        {
            resultScreen_.Enter(false, state_ == GameState::Boss || state_ == GameState::BossIntro);
            state_ = GameState::GameOver;
            effects_.AddTrauma(0.85f);
            SoundManager::Instance().PlayBGM(BgmId::GameOver, true);
            if (IsCampaign())
            {
                SaveHighscore();
            }
            return;
        }
    }

    if (state_ == GameState::GameOver || state_ == GameState::Victory)
    {
        ResultScreen::Action action = resultScreen_.Update(dt, input);
        if (action == ResultScreen::Action::Retry)
        {
            StartRun(); // chơi lại đúng chế độ lúc bắt đầu (tutorial -> Endless không làm đổi Retry)
            return;
        }
        if (action == ResultScreen::Action::Title)
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

    // Phím R: chơi lại từ wave 1 (cùng chế độ) - chỉ trong các trạng thái đang chơi,
    // chỉ khi vừa nhấn (giữ phím không reset liên tục). Lưu kỷ lục Endless trước khi reset.
    // Chỉ khi đang chiến đấu (Wave / Boss): không reset khi đang chọn thẻ hoặc Boss Intro.
    if (input.IsPressed(KEY_INPUT_R) && (state_ == GameState::Wave || state_ == GameState::Boss))
    {
        SaveRunRecord();
        StartRun();
        return;
    }

    // Gợi ý tutorial: làm chậm nhẹ khi gợi ý vừa hiện
    dt *= UpdateTutorialTips(dt);

    // --- XỬ LÝ THEO TRẠNG THÁI GAME ---
    if (state_ == GameState::Wave)
    {
        gameTime_ += dt;
        waveTimer_ += dt;

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

        }
        // ヒットストップ中も25秒の進行は止めません。
        if (waveTimer_ >= waveDuration_)
        {
            OnWaveComplete();
        }
    }
    else if (state_ == GameState::CardPick)
    {
        bool picked = cardManager_.Update(dt);
        effects_.Update(dt, player_.GetPosition());

        if (picked)
        {
            ReapplyAllStats();

            AdvanceAfterCardPick();
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
            const Vec3 bossPos = (bossKind_ == 1) ? shark_.GetPosition() : (bossKind_ == 2) ? squid_.GetPosition() : (bossKind_ == 3) ? turtle_.GetPosition() : (bossKind_ == 4) ? swordfish_.GetPosition() : boss_.GetPosition();
            const float bossRad = (bossKind_ == 1) ? shark_.GetHitboxRadius() : (bossKind_ == 2) ? squid_.GetHitboxRadius() : (bossKind_ == 3) ? turtle_.GetHitboxRadius() : (bossKind_ == 4) ? swordfish_.GetHitboxRadius() : boss_.GetHitboxRadius();
            float dBoss = (bossPos - player_.GetPosition()).Length();
            if (dBoss <= player_.GetSlamRadius() + bossRad)
            {
                if (bossKind_ == 1) shark_.TakeDamage(player_.GetSlamDamage(), effects_);
                else if (bossKind_ == 2) squid_.TakeDamageAt(player_.GetPosition(), player_.GetSlamDamage(), effects_);
                else if (bossKind_ == 3) turtle_.TakeDamageSlam(player_.GetSlamDamage(), effects_);
                else if (bossKind_ == 4) swordfish_.TakeDamageAt(player_.GetPosition(), player_.GetSlamDamage(), effects_);
                else boss_.TakeDamage(player_.GetSlamDamage(), effects_);
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
            if (bossKind_ == 1) shark_.Update(dt, player_, effects_);
            else if (bossKind_ == 2) squid_.Update(dt, player_, effects_);
            else if (bossKind_ == 3) turtle_.Update(dt, player_, effects_);
            else if (bossKind_ == 4) swordfish_.Update(dt, player_, effects_);
            else boss_.Update(dt, player_, enemies_, effects_);

            // 5. Cập nhật quái đệ do Boss triệu hồi
            enemies_.Update(dt, player_, effects_);

            // 6. Cập nhật vũ khí bắn quái & Boss (Gai phản đòn Pufferfish & súng)
            weapon_.Update(dt, player_, enemies_, effects_, bossKind_ == 0 ? &boss_ : nullptr, bossKind_ == 1 ? &shark_ : nullptr, bossKind_ == 2 ? &squid_ : nullptr, bossKind_ == 3 ? &turtle_ : nullptr, bossKind_ == 4 ? &swordfish_ : nullptr);

            // 7. Cập nhật hiệu ứng
            effects_.Update(dt, player_.GetPosition());

            // 9. Kiểm tra Boss bị hạ gục
            const bool bossDead = (bossKind_ == 1) ? shark_.IsDead() : (bossKind_ == 2) ? squid_.IsDead() : (bossKind_ == 3) ? turtle_.IsDead() : (bossKind_ == 4) ? swordfish_.IsDead() : boss_.IsDead();
            const float bossTime = (bossKind_ == 1) ? shark_.GetBossFightTimer() : (bossKind_ == 2) ? squid_.GetBossFightTimer() : (bossKind_ == 3) ? turtle_.GetBossFightTimer() : (bossKind_ == 4) ? swordfish_.GetBossFightTimer() : boss_.GetBossFightTimer();
            if (bossDead && player_.GetHp() > 0.0f)
            {
                ++bossesDefeated_;
                effects_.AddTrauma(0.7f);
                enemies_.KillAll(effects_);
                swprintf_s(lastSummaryBuffer_, L"ボス %d / %d 撃破: %.1f秒", bossesDefeated_, GameConsts::CAMPAIGN_BOSSES, bossTime);
                player_.Heal(player_.GetMaxHp() * 0.50f);
                effects_.PlayFx(FxId::HealBossReward, player_.GetPosition() + Vec3{0,20,0});
                if (bossesDefeated_ >= GameConsts::CAMPAIGN_BOSSES) CompleteCampaign();
                else
                {
                    state_ = GameState::CardPick;
                    SoundManager::Instance().PlaySFX(SfxId::CardFlip);
                    cardManager_.Roll3Cards(currentWave_);
                }
            }
        }
    }

    // --- CẬP NHẬT CAMERA 3D (C1, C2, C3) ---
    // Shark Hunt: cận cảnh khi cá mập đang ngoạm người chơi
    cameraController_.SetCinematic(state_ == GameState::Boss && sharkFight_ && shark_.IsCinematic());
    cameraController_.Update(dt, player_.GetPosition(), effects_);
}

void GamePrototype::DrawArena() const
{
    float half = GameConsts::ARENA_HALF_SIZE;

    // 1. Mặt sàn chính: model 3D đáy biển (FLOOR_USE_MODEL) -> SeabedFloor texture -> hộp màu + lưới cũ.
    //    Hạ cát dưới mặt nước; logic mặt đất/collision giữ nguyên y=0.
    bool floorDrawn = false;
    if (GameConsts::FLOOR_USE_MODEL)
    {
        int floorModel = LowPolyModels::GetHandle(LowPolyId::Floor);
        if (floorModel >= 0)
        {
            MV1SetMatrix(floorModel, MGetTranslate(VGet(0.0f, GameConsts::FLOOR_MODEL_Y, 0.0f)));
            MV1SetDifColorScale(floorModel, GetColorF(GameConsts::FLOOR_MODEL_TINT_R,
                GameConsts::FLOOR_MODEL_TINT_G, GameConsts::FLOOR_MODEL_TINT_B, 1.0f));
            floorDrawn = MV1DrawModel(floorModel) == 0;
        }
    }
    if (!floorDrawn && !SeabedFloor::Draw(half))
    {
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
    }

    // 3. Đường viền tâm sàn đấu
    Render3D::DrawCircle3D(Vec3{ 0.0f, 0.4f, 0.0f }, 160.0f, 48, GetColor(50, 90, 130), false);

    // 3b. Mặt nước biển nông 3D lấp lánh (Shallow Ocean Surface & Caustics)
    effects_.DrawShallowOcean(half);

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
    if (state_ == GameState::GameOver || state_ == GameState::Victory)
    {
        SetFogEnable(FALSE);
        if (state_ == GameState::Victory) uiRenderer_.DrawVictoryScreen(*this);
        else uiRenderer_.DrawGameOverScreen(*this);
        return;
    }

    // The illustrated menu is a complete screen; combat HUD/tips must not cover it.


    // Language / Title: chỉ vẽ đúng màn hình đó, không vẽ world/HUD phía sau (tránh bleed + không sim nền)

    if (state_ == GameState::Title)
    {
        SetFogEnable(FALSE);
        uiRenderer_.DrawTitleScreen(*this);
        return;
    }
    // #15: Sương mù xanh dưới nước chỉ cho cảnh 3D lúc chơi (tắt ở màn Title)
    const bool useFog = (state_ != GameState::Title);
    if (useFog)
    {
        SetFogEnable(TRUE);
        SetFogColor(GameConsts::FOG_COLOR_R, GameConsts::FOG_COLOR_G, GameConsts::FOG_COLOR_B);
        SetFogStartEnd(GameConsts::FOG_START, GameConsts::FOG_END);
    }

    // 1. Vẽ môi trường sàn đấu 3D
    DrawArena();

    // 2. Vẽ Decal báo hiệu trước đòn đánh của Boss trên mặt sàn
    if (state_ == GameState::Boss || state_ == GameState::BossIntro)
    {
        if (bossKind_ == 1) shark_.Draw();
        else if (bossKind_ == 2) squid_.Draw();
        else if (bossKind_ == 3) turtle_.Draw();
        else if (bossKind_ == 4) swordfish_.Draw();
        else boss_.Draw();
    }

    // 3. Vẽ sóng nước và hạt vụn 3D
    effects_.Draw();

    // 4. Vẽ quái vật 3D
    enemies_.Draw();

    // 5. Vẽ cá chính 3D
    player_.Draw();

    // 6. Vẽ súng tự ngắm và đạn 3D
    weapon_.Draw(player_);

    // 6b. 光の筋: tia sáng từ mặt nước + caustics (additive, vẽ sau vật thể đục)
    if (state_ != GameState::Title)
    {
        godRays_.Draw();
    }

    // Tắt sương mù trước khi vẽ số sát thương và UI 2D
    SetFogEnable(FALSE);

    // 7. Vẽ số sát thương nổi (Floating Damage Numbers) trên đầu quái
    effects_.DrawDamagePopups();

    // 8. Vẽ giao diện 2D (HUD)
    uiRenderer_.DrawHUD(*this);

    // 8. Vẽ thanh máu Boss nếu đang đấu Boss
    if (state_ == GameState::Boss)
    {
        if (bossKind_ == 1) uiRenderer_.DrawBossHealthBar(shark_);
        else if (bossKind_ == 2) uiRenderer_.DrawBossHealthBar(squid_);
        else if (bossKind_ == 3) uiRenderer_.DrawBossHealthBar(turtle_);
        else if (bossKind_ == 4) uiRenderer_.DrawBossHealthBar(swordfish_);
        else uiRenderer_.DrawBossHealthBar(boss_);
    }

    // 9. Vẽ màn hình chọn thẻ
    if (state_ == GameState::CardPick)
    {
        cardManager_.Draw();
    }

    // 10. Màn hình Chuyển cảnh Boss Intro
    if (state_ == GameState::BossIntro)
    {
        uiRenderer_.DrawBossIntroScreen(bossKind_);
    }

    uiRenderer_.DrawFlowOverlay(*this); // gợi ý tutorial + thẻ Hell

    // (11. Language/Title đã vẽ ở đầu Draw và return)


    // 12. Màn hình Thắng / Thua
    if (state_ == GameState::Victory)
    {
        uiRenderer_.DrawVictoryScreen(*this);
    }
    else if (state_ == GameState::GameOver)
    {
        uiRenderer_.DrawGameOverScreen(*this);
    }

    // 13. Màn hình tạm dừng (vẽ đè lên khung hình game đang đóng băng)
    if (paused_)
    {
        uiRenderer_.DrawPauseScreen(*this);
    }
}

ResultStats GamePrototype::BuildResultStats() const
{
    ResultStats s;
    s.victory = (state_ == GameState::Victory);
    s.campaign = IsCampaign();
    s.retryTutorial = true;
    s.bossesDefeated = bossesDefeated_;
    s.diedAtBoss = resultScreen_.DiedAtBoss();
    s.debugUsed = debugController_.WasDebugUsed();
    s.time = gameTime_;
    s.kills = GetTotalKills();
    s.stage = GetProgressionStage();
    s.wave = currentWave_;
    s.wavesCleared = completedWaves_;
    const HighscoreData& hs = highscore_;
    s.bestWave = hs.maxWave;
    s.bestKills = hs.maxKills;
    s.bestTime = hs.maxSurvivalTime;
    s.bestScore = hs.maxScore;
    return s;
}
