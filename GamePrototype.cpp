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
    const wchar_t* const HIGHSCORE_LEGACY_PATH = L"Data/highscore.dat";

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
    highscoreHell_ = HighscoreData{};
    {
        wchar_t hellPath[MAX_PATH] = {};
        BuildExeDataPath(hellPath, MAX_PATH, L"highscore_hell.dat");
        ReadHighscoreFile(hellPath, highscoreHell_); // không có file -> giữ 0
    }
    highscore_ = HighscoreData{};
    wchar_t path[MAX_PATH] = {};
    BuildExeDataPath(path, MAX_PATH, L"highscore.dat");
    if (ReadHighscoreFile(path, highscore_))
    {
        AppLogAdd(L"[HIGHSCORE] Loaded %s: Wave=%d Kills=%d Time=%.1fs Score=%d\n",
            path, highscore_.maxWave, highscore_.maxKills, highscore_.maxSurvivalTime, highscore_.maxScore);
        return;
    }
    // Migrate: file cũ theo thư mục làm việc (nếu exe được chạy từ chỗ khác)
    HighscoreData legacy{};
    if (ReadHighscoreFile(HIGHSCORE_LEGACY_PATH, legacy))
    {
        highscore_ = legacy;
        AppLogAdd(L"[HIGHSCORE] Loaded legacy file %s\n", HIGHSCORE_LEGACY_PATH);
        return;
    }
    highscore_ = HighscoreData{}; // không có file / hỏng: bắt đầu từ 0, không crash
    AppLogAdd(L"[HIGHSCORE] No valid highscore file (%s), starting fresh\n", path);
}

void GamePrototype::SaveHighscore()
{
    // Task 10 T4e: Không lưu kỷ lục nếu đã dùng phím debug
    if (debugController_.WasDebugUsed()) return;

    const bool hell = (mode_ == GameMode::Hell);
    HighscoreData& hs = hell ? highscoreHell_ : highscore_;
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
    BuildExeDataPath(path, MAX_PATH, hell ? L"highscore_hell.dat" : L"highscore.dat");
    BuildExeDataPath(tmpPath, MAX_PATH, hell ? L"highscore_hell.tmp" : L"highscore.tmp");
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

void GamePrototype::SaveHighscoreIfEndlessRun()
{
    // Chỉ lưu khi đang có một ván Endless (không lưu ở màn Title)
    if (IsEndlessLike() && state_ != GameState::Title && state_ != GameState::LanguageSelect && state_ != GameState::ModeSelect)
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
    boss_.Init();
    LoadHighscore();
    Reset();
    // Luôn bắt đầu từ màn chọn ngôn ngữ (lựa chọn đã lưu được chọn sẵn), sau đó tới Title
    state_ = GameState::LanguageSelect;
    langSelectIdx_ = (L10N::Get() == Language::Japanese) ? 1 : 0;
}

void GamePrototype::Shutdown()
{
    // Thoát game (ESC / đóng cửa sổ) giữa ván Endless: vẫn lưu kỷ lục
    SaveHighscoreIfEndlessRun();

    enemies_.Shutdown();
    cardManager_.Shutdown();
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
    sharkFight_ = false;
    bossKind_ = 0;

    enemies_.SetEndless(IsEndlessLike());
    cardManager_.SetEndless(IsEndlessLike());
    hellCardToastTimer_ = 0.0f;
    rouletteActive_ = false;
    for (int& v : lvStat_) v = 0;
    levelUpBannerTimer_ = 0.0f;
    activeTip_ = -1;
    ApplyModeRules();

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
    if (mode_ == GameMode::Classic) // Tutorial: thêm HP tối đa
        playerStats_.maxHp.AddModifier("tutorial", ModifierType::Percent, GameConsts::TUTORIAL_PLAYER_HP_BONUS);
    if (IsEndlessLike()) // Normal/Hell: điểm chỉ số nhận từ lên cấp (Attack/Def/Speed/Cooldown/Luck)
    {
        const float atk = GameConsts::LV_ATK_PCT * lvStat_[LS_Atk];
        if (atk > 0.0f)
        {
            playerStats_.slamDamage.AddModifier("lvAtk", ModifierType::Percent, atk);
            playerStats_.shockwaveDamage.AddModifier("lvAtk", ModifierType::Percent, atk);
            playerStats_.bulletDamage.AddModifier("lvAtk", ModifierType::Percent, atk);
        }
        if (lvStat_[LS_Spd]) playerStats_.moveSpeed.AddModifier("lvSpd", ModifierType::Percent, GameConsts::LV_SPD_PCT * lvStat_[LS_Spd]);
        if (lvStat_[LS_Cd])  playerStats_.jumpCooldown.AddModifier("lvCd", ModifierType::Percent, GameConsts::LV_CD_PCT * lvStat_[LS_Cd]);
    }
    cardManager_.SetLuck(IsEndlessLike() ? lvStat_[LS_Luck] : 0);

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
void GamePrototype::StartRun(GameMode mode)
{
    mode_ = mode;
    runStartMode_ = mode;
    titleSelectedMode_ = (mode == GameMode::Hell) ? 2 : (mode == GameMode::Endless) ? 1 : 0;
    paused_ = false;
    Reset();
    StartStage(1);
    Input::Instance().Flush(); // phím vừa nhấn (R / Enter / 1 / 2) không lọt sang tick sau
}

void GamePrototype::StartWave(int wave)
{
    currentWave_ = wave;
    state_ = GameState::Wave;
    waveTimer_ = 0.0f;

    if (mode_ == GameMode::Classic)
    {
        // Phase 1: Tutorial TUTORIAL_WAVES stage, mỗi stage 15s
        waveDuration_ = GameConsts::TUTORIAL_WAVE_DURATION;
    }
    else if (mode_ == GameMode::Hell)
    {
        waveDuration_ = GameConsts::HELL_WAVE_DURATION;
    }
    else
    {
        // Task 10 T4b: Chế độ Endless thời lượng mỗi wave cố định 35 giây
        waveDuration_ = GameConsts::ENDLESS_WAVE_DURATION;
    }

    player_.ResetDamageTakenThisWave();
    enemies_.SetWave(wave, IsEndlessLike());
    enemies_.SetAutoSpawn(true);
    SoundManager::Instance().PlayBGM(BgmId::Battle, true);
}

void GamePrototype::StartBossFight()
{
    state_ = GameState::Boss;
    player_.ResetDamageTakenThisWave();
    enemies_.KillAll(effects_); // Dọn quái nhỏ chuẩn bị sàn đấu
    if (mode_ == GameMode::Classic) player_.SetDamageTakenMul(GameConsts::TUTORIAL_BOSS_DAMAGE_TAKEN_MUL);
    enemies_.SetAutoSpawn(false); // Trong trận Boss, quái chỉ xuất hiện theo đòn của Boss
    if (bossKind_ == 1) shark_.Spawn(Vec3{ 0.0f, 0.0f, 0.0f });
    else if (bossKind_ == 2) squid_.Spawn(Vec3{ 0.0f, 0.0f, 0.0f });
    boss_.SetDamageTakenMul(mode_ == GameMode::Classic ? GameConsts::TUTORIAL_DMG_MUL_VS_BOSS
                          : mode_ == GameMode::Hell ? GameConsts::HELL_PLAYER_DMG_MUL : 1.0f);
    shark_.SetDamageTakenMul(mode_ == GameMode::Hell ? GameConsts::HELL_PLAYER_DMG_MUL : 1.0f);
    squid_.SetDamageTakenMul(mode_ == GameMode::Hell ? GameConsts::HELL_PLAYER_DMG_MUL : 1.0f);
    if (bossKind_ == 0) boss_.Spawn(Vec3{ 0.0f, 0.0f, 0.0f });
    boss_.SetHellMode(mode_ == GameMode::Hell);
    shark_.SetHellMode(mode_ == GameMode::Hell);
    squid_.SetHellMode(mode_ == GameMode::Hell);
    effects_.AddTrauma(0.5f);
    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
}

// Endless: tham số là số stage Boss (5, 10, ... 30). Boss ngẫu nhiên Cá Voi / Cá Mập.
// HP Boss: mul = 1 + ENDLESS_BOSS_HP_PER_STAGE * (stage - 5)  (stage 5 = x1.0, stage 30 = x2.25)
void GamePrototype::PrepareEndlessBoss(int stage)
{
    // Boss rotation: Whale / Shark / Giant Squid (random, tutorial stays Whale)
    bossKind_ = rand() % 3;
    sharkFight_ = (bossKind_ == 1);
    const int tier = (std::max)(1, stage / GameConsts::ENDLESS_BOSS_EVERY);
    const float hpMul = 1.0f + GameConsts::ENDLESS_BOSS_HP_PER_STAGE
        * static_cast<float>((std::max)(0, stage - GameConsts::ENDLESS_BOSS_EVERY));
    if (bossKind_ == 1) { shark_.SetBossDifficulty(tier); shark_.SetHpMultiplier(hpMul); }
    else if (bossKind_ == 2) { squid_.SetBossDifficulty(tier); squid_.SetHpMultiplier(hpMul); }
    else { boss_.SetBossDifficulty(tier); boss_.SetHpMultiplier(hpMul); }
    AppLogAdd(L"[ENDLESS] Stage %d boss kind=%d tier=%d hpMul=%.2f\n", stage, bossKind_, tier, hpMul);
}

void GamePrototype::BeginBossIntro()
{
    state_ = GameState::BossIntro;
    bossIntroTimer_ = 2.0f;
    enemies_.KillAll(effects_);
    SoundManager::Instance().PlayBGM(BgmId::Boss, true);
    SoundManager::Instance().PlaySFX(SfxId::WhaleGroan);
}

void GamePrototype::StartStage(int stage)
{
    if (IsEndlessLike() && stage % GameConsts::ENDLESS_BOSS_EVERY == 0)
    {
        currentWave_ = stage;
        waveTimer_ = 0.0f;
        player_.ResetDamageTakenThisWave();
        enemies_.SetWave(stage, true);
        PrepareEndlessBoss(stage);
        BeginBossIntro();
        return;
    }
    StartWave(stage);
}

void GamePrototype::AdvanceAfterCardPick()
{
    if (mode_ == GameMode::Classic)
    {
        if (currentWave_ >= GameConsts::TUTORIAL_WAVES)
        {
            sharkFight_ = false; bossKind_ = 0; // Tutorial: luôn là Cá Voi
            BeginBossIntro();
        }
        else
        {
            StartWave(currentWave_ + 1);
        }
        return;
    }
    if (currentWave_ >= GameConsts::ENDLESS_MAX_STAGE)
    {
        EnterEndlessClear();
        return;
    }
    StartStage(currentWave_ + 1);
}

void GamePrototype::EnterEndlessFromTutorial(GameMode mode)
{
    AppLogAdd(L"[FLOW] Tutorial cleared -> %s stage 1\n", mode == GameMode::Hell ? L"HELL" : L"NORMAL");
    mode_ = mode;
    titleSelectedMode_ = (mode == GameMode::Hell) ? 2 : 1;
    enemies_.SetEndless(true);
    cardManager_.SetEndless(true);
    ApplyModeRules();
    ReapplyAllStats(); // bỏ buff HP tutorial
    boss_.Reset();
    shark_.Reset();
    squid_.Reset();
    sharkFight_ = false;
    bossKind_ = 0;
    // Hồi máu đã được cộng lúc hạ Cá Voi (trước màn chọn chế độ); thẻ được giữ nguyên
    StartStage(1);
    Input::Instance().Flush();
}

void GamePrototype::ApplyModeRules()
{
    const bool hell = (mode_ == GameMode::Hell);
    const bool tutorial = (mode_ == GameMode::Classic);
    RefreshDamageTakenMul();
    enemies_.SetPlayerDmgMul(tutorial ? GameConsts::TUTORIAL_DMG_MUL_VS_ENEMY : hell ? GameConsts::HELL_PLAYER_DMG_MUL : 1.0f);
    if (!hell) effects_.SetParticleScale(1.0f);
    enemies_.SetMaxAlive(hell ? GameConsts::HELL_MAX_ALIVE : 0);
    enemies_.SetHell(hell);
    cardManager_.SetBonusScale(hell ? GameConsts::HELL_CARD_BONUS_SCALE : 1.0f);
}

void GamePrototype::HellAutoPickCard()
{
    // Phase 2: vòng quay thẻ chạy song song với trận đấu (không dừng game), thẻ áp dụng khi dừng quay
    if (rouletteActive_) FinishHellRoulette(); // vòng cũ chưa xong -> áp dụng ngay
    cardManager_.Roll3Cards(currentWave_);
    const int n = cardManager_.GetOfferedCardCount();
    if (n > 0)
    {
        const auto& offer = cardManager_.GetCurrentOffer();
        rouletteCount_ = n;
        for (int i = 0; i < n; ++i) rouletteCards_[i] = offer[i];
        rouletteTarget_ = rand() % n;
        rouletteIdx_ = rand() % n;
        rouletteTimer_ = 0.0f;
        rouletteStepTimer_ = 0.0f;
        rouletteLanded_ = false;
        rouletteActive_ = true;
        cardManager_.ClearOffer();
        SoundManager::Instance().PlaySFX(SfxId::CardFlip);
    }
    AdvanceAfterCardPick();
}

// "Level" = cấp EXP sẵn có (PlayerFish::AddExp, nhặt EXP orb). Normal/Hell: mỗi lần lên cấp -> +1 điểm ngẫu nhiên
// vào 1 trong 5 chỉ số. Attack -> sát thương, Def -> giảm sát thương nhận, Speed -> bơi, Cooldown -> hồi nhảy, Luck -> độ hiếm thẻ.
void GamePrototype::GrantLevelStat()
{
    static const wchar_t* kNames[LS_Count] = { L"ATTACK", L"DEFENSE", L"SPEED", L"COOLDOWN", L"LUCK" };
    int s = rand() % LS_Count;
    lvStat_[s]++;
    ReapplyAllStats();
    RefreshDamageTakenMul();
    levelUpStat_ = s;
    levelUpBannerTimer_ = GameConsts::LEVEL_UP_BANNER_TIME;
    SoundManager::Instance().PlaySFX(SfxId::CardSelect);
    AppLogAdd(L"[LEVEL] Lv %d: +1 %s\n", player_.GetLevel(), kNames[s]);
}

void GamePrototype::RefreshDamageTakenMul()
{
    const bool hell = (mode_ == GameMode::Hell);
    const bool tutorial = (mode_ == GameMode::Classic);
    float mul = hell ? GameConsts::HELL_DAMAGE_TAKEN_MUL : tutorial ? GameConsts::TUTORIAL_DAMAGE_TAKEN_MUL : 1.0f;
    if (tutorial && state_ == GameState::Boss) mul = GameConsts::TUTORIAL_BOSS_DAMAGE_TAKEN_MUL;
    if (!tutorial) mul *= 1.0f - (std::min)(GameConsts::LV_DEF_MAX, GameConsts::LV_DEF_PCT * lvStat_[LS_Def]);
    player_.SetDamageTakenMul(mul);
}

void GamePrototype::FinishHellRoulette()
{
    if (!rouletteActive_ || rouletteLanded_) return;
    rouletteLanded_ = true;
    rouletteIdx_ = rouletteTarget_;
    const Card& c = rouletteCards_[rouletteTarget_];
    cardManager_.AddActiveCard(c);
    ReapplyAllStats();
    SoundManager::Instance().PlaySFX(SfxId::CardSelect);
    AppLogAdd(L"[HELL] Roulette card: %s\n", c.name.c_str());
}

void GamePrototype::UpdateHellRoulette(float dt)
{
    if (!rouletteActive_) return;
    rouletteTimer_ += dt;
    if (!rouletteLanded_)
    {
        float k = (std::min)(1.0f, rouletteTimer_ / GameConsts::HELL_ROULETTE_SPIN_TIME);
        float step = GameConsts::HELL_ROULETTE_STEP_START + (GameConsts::HELL_ROULETTE_STEP_END - GameConsts::HELL_ROULETTE_STEP_START) * k * k;
        rouletteStepTimer_ += dt;
        if (rouletteStepTimer_ >= step && rouletteCount_ > 1)
        {
            rouletteStepTimer_ = 0.0f;
            rouletteIdx_ = (rouletteIdx_ + 1) % rouletteCount_;
            SoundManager::Instance().PlaySFX(SfxId::UiHover);
        }
        if (k >= 1.0f) { FinishHellRoulette(); rouletteTimer_ = 0.0f; }
    }
    else if (rouletteTimer_ >= GameConsts::HELL_ROULETTE_HOLD_TIME + GameConsts::HELL_ROULETTE_FADE_TIME)
    {
        rouletteActive_ = false;
    }
}

float GamePrototype::UpdateTutorialTips(float dt)
{
    if (hellCardToastTimer_ > 0.0f) hellCardToastTimer_ -= dt;
    if (mode_ != GameMode::Classic || tipsSkipped_) { activeTip_ = -1; return 1.0f; }

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

void GamePrototype::EnterEndlessClear()
{
    AppLogAdd(L"[ENDLESS] All %d stages cleared!\n", GameConsts::ENDLESS_MAX_STAGE);
    state_ = GameState::Victory;
    resultScreen_.Enter(true, false);
    effects_.AddTrauma(0.7f);
    enemies_.KillAll(effects_);
    SoundManager::Instance().PlayBGM(BgmId::Victory, true);
    SaveHighscore();
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

    // Hết giờ wave: Quái còn lại biến mất (G2). Hell: KHÔNG dọn quái giữa stage
    if (mode_ != GameMode::Hell) enemies_.KillAll(effects_);

    // G3: Cuối mỗi wave hồi 30% máu tối đa
    player_.Heal(player_.GetMaxHp() * (mode_ == GameMode::Hell ? GameConsts::HELL_WAVE_HEAL_PERCENT : GameConsts::WAVE_HEAL_PERCENT));
    effects_.PlayFx(FxId::HealWaveEnd, player_.GetPosition() + Vec3{ 0, 15, 0 });

    // Hook 11: Vảy hồi phục (Heal thêm 15% max HP - Phần C boost)
    if (cardManager_.HasHook(HookType::OnWaveEnd))
    {
        player_.Heal(player_.GetMaxHp() * 0.15f);
        effects_.PlayFx(FxId::HealCardScale, player_.GetPosition() + Vec3{ 0, 15, 0 });
    }

    // Task 10 T4d: Trong Endless, nếu pool thẻ đã hết sạch
    if (IsEndlessLike() && cardManager_.GetAvailableCardsPoolCount() == 0)
    {
        player_.Heal(player_.GetMaxHp() * GameConsts::ENDLESS_EMPTY_POOL_HEAL);
        effects_.PlayFx(FxId::HealPoolBonus, player_.GetPosition() + Vec3{ 0, 20, 0 });

        AdvanceAfterCardPick();
        return;
    }

    // Hell: thẻ ngẫu nhiên tự chọn, không dừng game
    if (mode_ == GameMode::Hell)
    {
        HellAutoPickCard();
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
    sharkFight_ = false;
    bossKind_ = 0;

    ReapplyAllStats();
    StartWave(waveTarget);
}

void GamePrototype::SkipToEndlessWave(int waveTarget)
{
    if (waveTarget < 1) return;
    waveTarget = (std::min)(waveTarget, GameConsts::ENDLESS_MAX_STAGE);
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
    shark_.Reset();
    squid_.Reset();
    sharkFight_ = false;
    bossKind_ = 0;

    ReapplyAllStats();
    StartStage(waveTarget);
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
    sharkFight_ = false;
    bossKind_ = 0;

    ReapplyAllStats();
    StartBossFight();
}

void GamePrototype::SkipToSharkBoss()
{
    debugController_.MarkDebugUsed();
    mode_ = GameMode::Endless;
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

    ReapplyAllStats();
    currentWave_ = GameConsts::ENDLESS_BOSS_EVERY;
    PrepareEndlessBoss(currentWave_);
    sharkFight_ = true; // Debug: luôn Cá Mập
    bossKind_ = 1;
    shark_.SetBossDifficulty(1);
    shark_.SetHpMultiplier(1.0f);
    StartBossFight();
}

void GamePrototype::SkipToSquidBoss()
{
    debugController_.MarkDebugUsed();
    mode_ = GameMode::Endless;
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
    ReapplyAllStats();
    currentWave_ = GameConsts::ENDLESS_BOSS_EVERY;
    bossKind_ = 2;
    sharkFight_ = false;
    squid_.SetBossDifficulty(1);
    squid_.SetHpMultiplier(1.0f);
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
    bool escNow = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
    bool pNow = CheckHitKey(KEY_INPUT_P) != 0;
    bool upNow = (CheckHitKey(KEY_INPUT_UP) || CheckHitKey(KEY_INPUT_W)) != 0;
    bool downNow = (CheckHitKey(KEY_INPUT_DOWN) || CheckHitKey(KEY_INPUT_S)) != 0;
    bool leftNow = (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_A)) != 0;
    bool rightNow = (CheckHitKey(KEY_INPUT_RIGHT) || CheckHitKey(KEY_INPUT_D)) != 0;
    bool enterNow = CheckHitKey(KEY_INPUT_RETURN) != 0;

    bool escPressed = escNow && !escPrev;
    // Phím P cũng dùng để chỉnh scale model trong Debug HUD, nên chỉ dùng P khi Debug HUD tắt
    bool pPressed = pNow && !pPrev && !debugController_.IsDebugHUDVisible();
    bool upPressed = upNow && !upPrev;
    bool downPressed = downNow && !downPrev;
    bool leftPressed = leftNow && !leftPrev;
    bool rightPressed = rightNow && !rightPrev;
    // Xác nhận khi NHẢ Enter, để Enter không "lọt" sang màn Title (Title đọc Enter đang giữ)
    bool enterReleased = !enterNow && enterPrev;

    escPrev = escNow; pPrev = pNow; upPrev = upNow; downPrev = downNow;
    leftPrev = leftNow; rightPrev = rightNow; enterPrev = enterNow;

    if (!paused_)
    {
        bool canPause = (state_ == GameState::Wave || state_ == GameState::Boss || state_ == GameState::BossIntro)
                        && player_.GetHp() > 0.0f;
        if (canPause && (escPressed || pPressed))
        {
            paused_ = true;
            pauseSelected_ = PauseItem_Resume;
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
        paused_ = false;
        Input::Instance().Flush();
        return true; // tick này vẫn bỏ qua để phím vừa nhấn không lọt vào game
    }

    if (upPressed) pauseSelected_ = (pauseSelected_ + PauseItem_Count - 1) % PauseItem_Count;
    if (downPressed) pauseSelected_ = (pauseSelected_ + 1) % PauseItem_Count;

    SoundManager& sound = SoundManager::Instance();
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

    bool toggleWindow = (pauseSelected_ == PauseItem_WindowMode) && (enterReleased || leftPressed || rightPressed);
    if (toggleWindow)
    {
        // Thực hiện ở main.cpp (ngoài vòng Update) vì ChangeWindowMode reset hệ thống đồ hoạ
        windowModeToggleRequested_ = true;
    }
    else if (enterReleased)
    {
        if (pauseSelected_ == PauseItem_Resume)
        {
            paused_ = false;
            Input::Instance().Flush();
        }
        else if (pauseSelected_ == PauseItem_Title)
        {
            paused_ = false;
            Input::Instance().Flush();
            SaveHighscoreIfEndlessRun(); // Pause -> Title giữa ván Endless: không mất kỷ lục
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
    debugController_.Update(dt, *this);

    // 00. Màn chọn ngôn ngữ (màn đầu tiên)
    if (state_ == GameState::LanguageSelect)
    {
        SoundManager::Instance().PlayBGM(BgmId::Title, true);
        if (input.IsPressed(KEY_INPUT_UP) || input.IsPressed(KEY_INPUT_W) || input.IsPressed(KEY_INPUT_DOWN) || input.IsPressed(KEY_INPUT_S)
            || input.IsPressed(KEY_INPUT_LEFT) || input.IsPressed(KEY_INPUT_RIGHT))
        {
            langSelectIdx_ = 1 - langSelectIdx_;
        }
        bool k1 = input.IsPressed(KEY_INPUT_1) || input.IsPressed(KEY_INPUT_NUMPAD1);
        bool k2 = input.IsPressed(KEY_INPUT_2) || input.IsPressed(KEY_INPUT_NUMPAD2);
        if (k1) langSelectIdx_ = 0;
        if (k2) langSelectIdx_ = 1;
        if (k1 || k2 || input.IsPressed(KEY_INPUT_SPACE) || input.IsPressed(KEY_INPUT_RETURN))
        {
            L10N::Set(langSelectIdx_ == 1 ? Language::Japanese : Language::English, true);
            FontManager::ApplyLanguage();
            state_ = GameState::Title;
            Input::Instance().Flush(); // tránh Enter/1/2 bị hiểu là chọn mode ở Title
            return;
        }
        effects_.Update(dt, player_.GetPosition());
        return;
    }

    // 0. Màn hình Tiêu đề (Title - G1, Task 10 T4a)
    if (state_ == GameState::Title)
    {
        SoundManager::Instance().PlayBGM(BgmId::Title, true);

        bool upPressed = input.IsPressed(KEY_INPUT_UP) || input.IsPressed(KEY_INPUT_W);
        bool downPressed = input.IsPressed(KEY_INPUT_DOWN) || input.IsPressed(KEY_INPUT_S);
        bool k1Pressed = input.IsPressed(KEY_INPUT_1) || input.IsPressed(KEY_INPUT_NUMPAD1);
        bool k2Pressed = input.IsPressed(KEY_INPUT_2) || input.IsPressed(KEY_INPUT_NUMPAD2);
        bool k3Pressed = input.IsPressed(KEY_INPUT_3) || input.IsPressed(KEY_INPUT_NUMPAD3);

        // [L]: quay lại màn chọn ngôn ngữ
        if (input.IsPressed(GameConsts::KEY_LANGUAGE_SELECT))
        {
            langSelectIdx_ = (L10N::Get() == Language::Japanese) ? 1 : 0;
            state_ = GameState::LanguageSelect;
            return;
        }

        if (upPressed) titleSelectedMode_ = (titleSelectedMode_ + 2) % 3;
        if (downPressed) titleSelectedMode_ = (titleSelectedMode_ + 1) % 3;
        if (k3Pressed)
        {
            StartRun(GameMode::Hell);
            return;
        }

        if (k1Pressed)
        {
            StartRun(GameMode::Classic);
            return;
        }
        if (k2Pressed)
        {
            StartRun(GameMode::Endless);
            return;
        }
        if (input.IsPressed(KEY_INPUT_SPACE) || input.IsPressed(KEY_INPUT_RETURN))
        {
            StartRun((titleSelectedMode_ == 2) ? GameMode::Hell : (titleSelectedMode_ == 1) ? GameMode::Endless : GameMode::Classic);
            return;
        }

        effects_.Update(dt, player_.GetPosition());
        return;
    }

    // Kiểm tra chết (Game Over)
    if (state_ != GameState::GameOver && state_ != GameState::Victory && state_ != GameState::Title && state_ != GameState::LanguageSelect)
    {
        if (player_.GetHp() <= 0.0f)
        {
            resultScreen_.Enter(false, state_ == GameState::Boss || state_ == GameState::BossIntro);
            state_ = GameState::GameOver;
            effects_.AddTrauma(0.85f);
            SoundManager::Instance().PlayBGM(BgmId::GameOver, true);
            if (IsEndlessLike())
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
            StartRun(runStartMode_); // chơi lại đúng chế độ lúc bắt đầu (tutorial -> Endless không làm đổi Retry)
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

    // Phase 1: Màn chọn chế độ sau tutorial (Normal / Hell)
    if (state_ == GameState::ModeSelect)
    {
        if (input.IsPressed(KEY_INPUT_UP) || input.IsPressed(KEY_INPUT_W) ||
            input.IsPressed(KEY_INPUT_DOWN) || input.IsPressed(KEY_INPUT_S))
        {
            modeSelectIdx_ = 1 - modeSelectIdx_;
            SoundManager::Instance().PlaySFX(SfxId::UiHover);
        }
        int choose = -1;
        if (input.IsPressed(KEY_INPUT_1) || input.IsPressed(KEY_INPUT_NUMPAD1)) choose = 0;
        if (input.IsPressed(KEY_INPUT_2) || input.IsPressed(KEY_INPUT_NUMPAD2)) choose = 1;
        if (input.IsPressed(KEY_INPUT_RETURN) || input.IsPressed(KEY_INPUT_NUMPADENTER) || input.IsPressed(KEY_INPUT_SPACE)) choose = modeSelectIdx_;
        if (choose >= 0)
        {
            SoundManager::Instance().PlaySFX(SfxId::CardSelect);
            EnterEndlessFromTutorial(choose == 1 ? GameMode::Hell : GameMode::Endless);
            return;
        }
        effects_.Update(dt, player_.GetPosition());
        return;
    }

    // Phím R: chơi lại từ wave 1 (cùng chế độ) - chỉ trong các trạng thái đang chơi,
    // chỉ khi vừa nhấn (giữ phím không reset liên tục). Lưu kỷ lục Endless trước khi reset.
    // Chỉ khi đang chiến đấu (Wave / Boss): không reset khi đang chọn thẻ hoặc Boss Intro.
    if (input.IsPressed(KEY_INPUT_R) && (state_ == GameState::Wave || state_ == GameState::Boss))
    {
        SaveHighscoreIfEndlessRun();
        StartRun(runStartMode_);
        return;
    }

    if (IsEndlessLike())
    {
        for (int n = player_.ConsumeLevelUps(); n > 0; --n) GrantLevelStat();
    }
    else player_.ConsumeLevelUps();
    if (levelUpBannerTimer_ > 0.0f) levelUpBannerTimer_ -= dt;
    if (mode_ == GameMode::Hell)
    {
        effects_.SetParticleScale(enemies_.GetActiveCount() >= GameConsts::HELL_FX_LIGHT_THRESHOLD ? GameConsts::HELL_FX_LIGHT_SCALE : 1.0f);
    }
    UpdateHellRoulette(dt); // Phase 2: vòng quay thẻ Hell (game vẫn chạy)
    // Gợi ý tutorial: làm chậm nhẹ khi gợi ý vừa hiện
    dt *= UpdateTutorialTips(dt);

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
            int collectedExp = effects_.CollectExpOrbs(player_.GetPosition(), GameConsts::EXP_PICKUP_RADIUS);
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
            const Vec3 bossPos = (bossKind_ == 1) ? shark_.GetPosition() : (bossKind_ == 2) ? squid_.GetPosition() : boss_.GetPosition();
            const float bossRad = (bossKind_ == 1) ? shark_.GetHitboxRadius() : (bossKind_ == 2) ? squid_.GetHitboxRadius() : boss_.GetHitboxRadius();
            float dBoss = (bossPos - player_.GetPosition()).Length();
            if (dBoss <= player_.GetSlamRadius() + bossRad)
            {
                if (bossKind_ == 1) shark_.TakeDamage(player_.GetSlamDamage(), effects_);
                else if (bossKind_ == 2) squid_.TakeDamageAt(player_.GetPosition(), player_.GetSlamDamage(), effects_);
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
            else boss_.Update(dt, player_, enemies_, effects_);

            // 5. Cập nhật quái đệ do Boss triệu hồi
            enemies_.Update(dt, player_, effects_);

            // 6. Cập nhật vũ khí bắn quái & Boss (Gai phản đòn Pufferfish & súng)
            weapon_.Update(dt, player_, enemies_, effects_, bossKind_ == 0 ? &boss_ : nullptr, bossKind_ == 1 ? &shark_ : nullptr, bossKind_ == 2 ? &squid_ : nullptr);

            // 7. Cập nhật hiệu ứng
            effects_.Update(dt, player_.GetPosition());

            // 8. Thu thập hạt EXP (Trận Boss không có level-up theo yêu cầu)
            effects_.CollectExpOrbs(player_.GetPosition(), GameConsts::EXP_PICKUP_RADIUS);

            // 9. Kiểm tra Boss bị hạ gục
            const bool bossDead = (bossKind_ == 1) ? shark_.IsDead() : (bossKind_ == 2) ? squid_.IsDead() : boss_.IsDead();
            const float bossTime = (bossKind_ == 1) ? shark_.GetBossFightTimer() : (bossKind_ == 2) ? squid_.GetBossFightTimer() : boss_.GetBossFightTimer();
            const int bossTier = (bossKind_ == 1) ? shark_.GetBossTier() : (bossKind_ == 2) ? squid_.GetBossTier() : boss_.GetBossTier();
            if (bossDead)
            {
                if (mode_ == GameMode::Classic)
                {
                    // Phase 1: hạ Cá Voi tutorial -> màn chọn chế độ Normal / Hell (giữ thẻ + hồi máu)
                    effects_.AddTrauma(0.7f);
                    enemies_.KillAll(effects_);
                    swprintf_s(lastSummaryBuffer_, L"Boss: %.1f giây", bossTime);
                    AppLogAdd(L"[SUMMARY] %s\n", lastSummaryBuffer_);
                    player_.Heal(player_.GetMaxHp() * GameConsts::TUTORIAL_BOSS_CLEAR_HEAL);
                    modeSelectIdx_ = 0;
                    state_ = GameState::ModeSelect;
                    SoundManager::Instance().PlayBGM(BgmId::Victory, true);
                    Input::Instance().Flush();
                }
                else
                {
                    // Endless Mode: Hạ gục 1 lượt Boss định kỳ! (Task 10 T4c)
                    effects_.AddTrauma(0.7f);
                    enemies_.KillAll(effects_);
                    swprintf_s(lastSummaryBuffer_, bossKind_ == 2 ? L"Squid Tier %d: %.1f giây" : sharkFight_ ? L"Shark Tier %d: %.1f giây" : L"Boss Tier %d: %.1f giây", bossTier, bossTime);
                    AppLogAdd(L"[SUMMARY] %s\n", lastSummaryBuffer_);

                    // Hồi 50% máu tối đa thưởng diệt Boss
                    player_.Heal(player_.GetMaxHp() * 0.50f);
                    effects_.PlayFx(FxId::HealBossReward, player_.GetPosition() + Vec3{ 0, 20, 0 });

                    // Stage cuối -> CLEAR; ngược lại chọn thẻ rồi sang stage kế tiếp
                    if (currentWave_ >= GameConsts::ENDLESS_MAX_STAGE)
                    {
                        EnterEndlessClear();
                    }
                    else if (mode_ == GameMode::Hell)
                    {
                        HellAutoPickCard();
                    }
                    else
                    {
                        state_ = GameState::CardPick;
                        SoundManager::Instance().PlaySFX(SfxId::CardFlip);
                        cardManager_.Roll3Cards(currentWave_);
                    }
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

    // 1. Mặt sàn chính: 海底の砂テクスチャ (fallback: hộp màu + lưới cũ nếu thiếu file)
    if (!SeabedFloor::Draw(half))
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
    // #15: Sương mù xanh dưới nước chỉ cho cảnh 3D lúc chơi (tắt ở màn Title)
    const bool useFog = (state_ != GameState::Title && state_ != GameState::LanguageSelect);
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
    if (state_ != GameState::Title && state_ != GameState::LanguageSelect)
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

    if (state_ == GameState::ModeSelect)
    {
        uiRenderer_.DrawModeSelectScreen(*this);
    }
    uiRenderer_.DrawFlowOverlay(*this); // gợi ý tutorial + thẻ Hell

    // 11. Màn hình Tiêu đề Title
    if (state_ == GameState::Title)
    {
        uiRenderer_.DrawTitleScreen(*this);
    }
    else if (state_ == GameState::LanguageSelect)
    {
        uiRenderer_.DrawLanguageSelectScreen(*this);
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
    s.endless = IsEndlessLike();
    s.diedAtBoss = resultScreen_.DiedAtBoss();
    s.debugUsed = debugController_.WasDebugUsed();
    s.time = gameTime_;
    s.kills = GetTotalKills();
    s.level = player_.GetLevel();
    s.wave = currentWave_;
    const HighscoreData& hs = (mode_ == GameMode::Hell) ? highscoreHell_ : highscore_;
    s.bestWave = hs.maxWave;
    s.bestKills = hs.maxKills;
    s.bestTime = hs.maxSurvivalTime;
    s.bestScore = hs.maxScore;
    return s;
}
