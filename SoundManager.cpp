#include "SoundManager.h"
#include <algorithm>
#include <cstdio>
#include <windows.h>
#include "Consts.h"

// Helper cấu hình SFX tĩnh lúc khởi tạo
struct SfxInitConfig
{
    SfxId id;
    const char* name;
    const wchar_t* wname;
    int poolSize;
    int baseVolume;
    const wchar_t* customFallback;
};

// Bảng cấu hình SFX chuẩn theo Yêu cầu 2
static const SfxInitConfig kSfxConfigs[] = {
    { SfxId::SwimPaddle,     "sfx_swim_paddle",     L"sfx_swim_paddle",     2, 60,  nullptr },
    { SfxId::JumpLaunch,     "sfx_jump_launch",     L"sfx_jump_launch",     4, 90,  L"Data/Sound/SE/player-jump-se.mp3" },
    { SfxId::SlamImpact,     "sfx_slam_impact",     L"sfx_slam_impact",     6, 100, L"Data/Sound/SE/player-splash-se.mp3" },
    { SfxId::PlayerHit,      "sfx_player_hit",      L"sfx_player_hit",      4, 95,  L"Data/Sound/SE/player-hit-se.mp3" },
    { SfxId::EnemySquish,    "sfx_enemy_squish",    L"sfx_enemy_squish",    4, 85,  nullptr },
    { SfxId::SharkTelegraph, "sfx_shark_telegraph", L"sfx_shark_telegraph", 2, 88,  nullptr },
    { SfxId::SharkDash,      "sfx_shark_dash",      L"sfx_shark_dash",      3, 92,  nullptr },
    { SfxId::SharkDive,      "sfx_shark_dive",      L"sfx_shark_dive",      2, 95,  nullptr },
    { SfxId::SharkErupt,     "sfx_shark_erupt",     L"sfx_shark_erupt",     3, 100, nullptr },
    { SfxId::SharkGrab,      "sfx_shark_grab",      L"sfx_shark_grab",      2, 95,  nullptr },
    { SfxId::SharkThrow,     "sfx_shark_throw",     L"sfx_shark_throw",     2, 92,  nullptr },
    { SfxId::SharkStrafe,    "sfx_shark_strafe",    L"sfx_shark_strafe",    2, 80,  nullptr },
    { SfxId::SharkVortexLoop,"sfx_shark_vortex_loop",L"sfx_shark_vortex_loop",1, 82, nullptr },
    { SfxId::SharkFrenzy,    "sfx_shark_frenzy",    L"sfx_shark_frenzy",    2, 100, nullptr },
    { SfxId::SharkStun,      "sfx_shark_stun",      L"sfx_shark_stun",      2, 88,  nullptr },
    { SfxId::PufferSpikes,   "sfx_puffer_spikes",   L"sfx_puffer_spikes",   4, 90,  nullptr },
    { SfxId::ExpPickup,      "sfx_exp_pickup",      L"sfx_exp_pickup",      4, 75,  nullptr },
    { SfxId::WhaleGroan,     "sfx_whale_groan",     L"sfx_whale_groan",     4, 100, nullptr },
    { SfxId::TsunamiCharge,  "sfx_tsunami_charge",  L"sfx_tsunami_charge",  4, 95,  nullptr },
    { SfxId::TsunamiSurge,   "sfx_tsunami_surge",   L"sfx_tsunami_surge",   4, 100, nullptr },
    { SfxId::BossHeavySlam,  "sfx_boss_heavy_slam", L"sfx_boss_heavy_slam", 4, 100, L"Data/Sound/SE/sfx_boss_breach.wav" },
    { SfxId::BossRoar,       "sfx_boss_roar",       L"sfx_boss_roar",       2, 100, nullptr },
    { SfxId::BossTailSlap,   "sfx_boss_tail_slap",  L"sfx_boss_tail_slap",  2, 95,  nullptr },
    { SfxId::BossBubbleNet,  "sfx_boss_bubble_net", L"sfx_boss_bubble_net", 2, 90,  nullptr },
    { SfxId::BossDiveCharge, "sfx_boss_dive_charge",L"sfx_boss_dive_charge",2, 95,  nullptr },
    { SfxId::BossInhaleLoop, "sfx_boss_inhale_loop",L"sfx_boss_inhale_loop",1, 85,  nullptr },
    { SfxId::BossCallPack,   "sfx_boss_call_pack",  L"sfx_boss_call_pack",  2, 95,  nullptr },
    { SfxId::BossStun,       "sfx_stun",            L"sfx_stun",            2, 85,  nullptr },
    { SfxId::EnemyHit,       "sfx_enemy_hit",       L"sfx_enemy_hit",       6, 70,  nullptr },
    { SfxId::EnemyHitCrit,   "sfx_enemy_hit_crit",  L"sfx_enemy_hit_crit",  4, 85,  nullptr },
    { SfxId::EnemyDeathPop,  "sfx_enemy_death_pop", L"sfx_enemy_death_pop", 6, 75,  nullptr },
    { SfxId::PlayerHurt,     "sfx_player_hurt",     L"sfx_player_hurt",     2, GameConsts::SFX_PLAYER_HURT_BASE_VOL, nullptr },
    { SfxId::PlayerSlow,     "sfx_player_slow",     L"sfx_player_slow",     2, 80,  nullptr },
    { SfxId::UiHover,        "sfx_ui_hover",        L"sfx_ui_hover",        2, 60,  nullptr },
    { SfxId::CardFlip,       "sfx_card_flip",       L"sfx_card_flip",       4, 80,  nullptr },
    { SfxId::CardSelect,     "sfx_card_select",     L"sfx_card_select",     4, 90,  nullptr },
};

// Bảng cấu hình BGM chuẩn theo Yêu cầu 3
struct BgmInitConfig
{
    BgmId id;
    const char* name;
    const wchar_t* wname;
    int baseVolume;
    const wchar_t* customFallback;
};

static const BgmInitConfig kBgmConfigs[] = {
    { BgmId::Title,    "bgm_title",    L"bgm_title",    75, L"Data/Sound/SE/game-loop-bgm.ogg" },
    { BgmId::Battle,   "bgm_battle",   L"bgm_battle",   75, L"Data/Sound/SE/game-loop-bgm.ogg" },
    { BgmId::Boss,     "bgm_boss",     L"bgm_boss",     85, nullptr },
    { BgmId::Victory,  "bgm_victory",  L"bgm_victory",  80, nullptr },
    { BgmId::GameOver, "bgm_gameover", L"bgm_gameover", 70, nullptr },
};

// Helper tìm và nạp file âm thanh theo danh sách đường dẫn khả dĩ
static int TryLoadSoundFile(const char* baseName, bool isBgm, const wchar_t* customFallback)
{
    wchar_t pathBuf[260];
    const wchar_t* extensions[] = { L".wav", L".mp3", L".ogg" };
    const wchar_t* primaryDirs[] = {
        isBgm ? L"Data/Audio/BGM/" : L"Data/Audio/SFX/",
        isBgm ? L"Data/Sound/BGM/" : L"Data/Sound/SE/",
        L"Data/Sound/"
    };

    wchar_t wBaseName[128] = L"";
    MultiByteToWideChar(CP_UTF8, 0, baseName, -1, wBaseName, 128);

    // 1. Thử đường dẫn theo chuẩn đặc tả (Data/Audio/ và Data/Sound/)
    for (const wchar_t* dir : primaryDirs)
    {
        for (const wchar_t* ext : extensions)
        {
            swprintf_s(pathBuf, L"%s%s%s", dir, wBaseName, ext);
            if (FileRead_size(pathBuf) > 0)
            {
                int h = LoadSoundMem(pathBuf);
                if (h != -1) return h;
            }
        }
    }

    // 2. Thử đường dẫn fallback người dùng đã tải sẵn (nếu có)
    if (customFallback != nullptr)
    {
        // Ưu tiên nạp định dạng .wav trước (nếu có) để triệt tiêu hoàn toàn độ trễ giải mã
        wchar_t wavFallback[260];
        wcscpy_s(wavFallback, customFallback);
        wchar_t* dot = wcsrchr(wavFallback, L'.');
        if (dot != nullptr)
        {
            wcscpy_s(dot, 5, L".wav");
            if (FileRead_size(wavFallback) > 0)
            {
                int h = LoadSoundMem(wavFallback);
                if (h != -1) return h;
            }
        }

        if (FileRead_size(customFallback) > 0)
        {
            int h = LoadSoundMem(customFallback);
            if (h != -1) return h;
        }
    }

    return -1;
}

SoundManager& SoundManager::Instance()
{
    static SoundManager s_instance;
    return s_instance;
}

void SoundManager::Init()
{
    AppLogAdd(L"[SOUND] Khởi tạo SoundManager (Odoru)...\n");

    // 1. Nạp SFX và khởi tạo Object Pool tĩnh (DuplicateSoundMem)
    for (const auto& cfg : kSfxConfigs)
    {
        int idx = static_cast<int>(cfg.id);
        auto& entry = sfxList_[idx];
        entry.name = cfg.name;
        entry.wname = cfg.wname;
        entry.poolSize = cfg.poolSize;
        entry.baseVolume = cfg.baseVolume;
        // Chống chồng tiếng cho SFX trúng đòn / chết khi hạ hàng loạt
        if (cfg.id == SfxId::EnemyHit || cfg.id == SfxId::EnemyHitCrit) entry.minIntervalMs = GameConsts::SFX_HIT_MIN_INTERVAL_MS;
        else if (cfg.id == SfxId::EnemyDeathPop || cfg.id == SfxId::EnemySquish) entry.minIntervalMs = GameConsts::SFX_DEATH_MIN_INTERVAL_MS;
        else if (cfg.id == SfxId::SharkStrafe) entry.minIntervalMs = GameConsts::SFX_SHARK_STRAFE_MIN_INTERVAL_MS;
        else if (cfg.id == SfxId::SharkErupt || cfg.id == SfxId::SharkDash) entry.minIntervalMs = GameConsts::SFX_SHARK_IMPACT_MIN_INTERVAL_MS;

        int baseHandle = TryLoadSoundFile(cfg.name, false, cfg.customFallback);
        if (baseHandle != -1)
        {
            entry.handles[0] = baseHandle;
            // Nhân bản các handle phụ trong pool cố định (polyphony)
            for (int h = 1; h < entry.poolSize; ++h)
            {
                entry.handles[h] = DuplicateSoundMem(baseHandle);
            }
            AppLogAdd(L"[SOUND] SFX loaded: %s (base=%d, poolSize=%d)\n", cfg.wname, baseHandle, entry.poolSize);
        }
        else
        {
            AppLogAdd(L"[SOUND] SFX missing: %s (chưa có file, bỏ qua an toàn)\n", cfg.wname);
            for (int h = 0; h < MAX_SFX_POOL_HANDLES; ++h) entry.handles[h] = -1;
        }
    }

    // 2. Nạp BGM
    for (const auto& cfg : kBgmConfigs)
    {
        int idx = static_cast<int>(cfg.id);
        auto& entry = bgmList_[idx];
        entry.name = cfg.name;
        entry.wname = cfg.wname;
        entry.baseVolume = cfg.baseVolume;

        int handle = TryLoadSoundFile(cfg.name, true, cfg.customFallback);
        entry.handle = handle;
        if (handle != -1)
        {
            AppLogAdd(L"[SOUND] BGM loaded: %s (handle=%d)\n", cfg.wname, handle);
        }
        else
        {
            AppLogAdd(L"[SOUND] BGM missing: %s (chưa có file, bỏ qua an toàn)\n", cfg.wname);
        }
    }

    ApplyAllVolumes();
    AppLogAdd(L"[SOUND] SoundManager khởi tạo hoàn tất.\n");
}

void SoundManager::Shutdown()
{
    AppLogAdd(L"[SOUND] Giải phóng toàn bộ tài nguyên SoundManager...\n");

    // Dừng và giải phóng SFX
    for (int i = 0; i < SFX_COUNT; ++i)
    {
        auto& entry = sfxList_[i];
        for (int h = 0; h < entry.poolSize; ++h)
        {
            if (entry.handles[h] != -1)
            {
                StopSoundMem(entry.handles[h]);
                DeleteSoundMem(entry.handles[h]);
                entry.handles[h] = -1;
            }
        }
        entry.poolSize = 0;
    }

    // Dừng và giải phóng BGM
    for (int i = 0; i < BGM_COUNT; ++i)
    {
        auto& entry = bgmList_[i];
        if (entry.handle != -1)
        {
            StopSoundMem(entry.handle);
            DeleteSoundMem(entry.handle);
            entry.handle = -1;
        }
    }

    currentBgm_ = BgmId::Count;
    nextBgm_ = BgmId::Count;
    fadeState_ = FadeState::None;
    AppLogAdd(L"[SOUND] SoundManager giải phóng thành công 0 rò rỉ.\n");
}

int SoundManager::CalculateVolume255(int baseVolume, SoundChannel ch) const
{
    int chVol = (ch == SoundChannel::BGM) ? bgmVolume_ : sfxVolume_;
    float net = (masterVolume_ / 100.0f) * (chVol / 100.0f) * (baseVolume / 100.0f);
    int vol255 = static_cast<int>(net * 255.0f + 0.5f);
    return std::clamp(vol255, 0, 255);
}

void SoundManager::ApplyBgmVolume(int handle, int baseVolume, float fadeRatio)
{
    if (handle == -1) return;
    int maxVol255 = CalculateVolume255(baseVolume, SoundChannel::BGM);
    int curVol255 = static_cast<int>(maxVol255 * fadeRatio + 0.5f);
    ChangeVolumeSoundMem(std::clamp(curVol255, 0, 255), handle);
}

void SoundManager::PlaySFXLoop(SfxId id)
{
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= SFX_COUNT) return;
    auto& entry = sfxList_[idx];
    for (int h = 0; h < entry.poolSize; ++h)
    {
        int handle = entry.handles[h];
        if (handle == -1) continue;
        if (CheckSoundMem(handle) == 1) return; // đang lặp rồi
    }
    for (int h = 0; h < entry.poolSize; ++h)
    {
        int handle = entry.handles[h];
        if (handle == -1) continue;
        ChangeVolumeSoundMem(CalculateVolume255(entry.baseVolume, SoundChannel::SFX), handle);
        PlaySoundMem(handle, DX_PLAYTYPE_LOOP, TRUE);
        return;
    }
}

void SoundManager::StopSFX(SfxId id)
{
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= SFX_COUNT) return;
    auto& entry = sfxList_[idx];
    for (int h = 0; h < entry.poolSize; ++h)
    {
        if (entry.handles[h] != -1 && CheckSoundMem(entry.handles[h]) == 1) StopSoundMem(entry.handles[h]);
    }
}

void SoundManager::ApplyAllVolumes()
{
    // SFX đang phát (vd. Inhale lặp) cũng theo âm lượng SFX mới
    for (auto& entry : sfxList_)
    {
        for (int h = 0; h < entry.poolSize; ++h)
        {
            int handle = entry.handles[h];
            if (handle != -1 && CheckSoundMem(handle) == 1)
                ChangeVolumeSoundMem(CalculateVolume255(entry.baseVolume, SoundChannel::SFX), handle);
        }
    }

    // Cập nhật âm lượng BGM hiện tại
    if (currentBgm_ != BgmId::Count)
    {
        int idx = static_cast<int>(currentBgm_);
        ApplyBgmVolume(bgmList_[idx].handle, bgmList_[idx].baseVolume, fadeRatio_);
    }
}

void SoundManager::SetVolume(SoundChannel ch, int value0to100)
{
    int clamped = std::clamp(value0to100, 0, 100);
    switch (ch)
    {
    case SoundChannel::Master: masterVolume_ = clamped; break;
    case SoundChannel::BGM:    bgmVolume_    = clamped; break;
    case SoundChannel::SFX:    sfxVolume_    = clamped; break;
    }
    ApplyAllVolumes();
}

int SoundManager::GetVolume(SoundChannel ch) const
{
    switch (ch)
    {
    case SoundChannel::Master: return masterVolume_;
    case SoundChannel::BGM:    return bgmVolume_;
    case SoundChannel::SFX:    return sfxVolume_;
    default:                   return 0;
    }
}

void SoundManager::PlaySFX(SfxId id)
{
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= SFX_COUNT) return;

    auto& entry = sfxList_[idx];
    if (entry.poolSize <= 0) return;

    if (entry.minIntervalMs > 0)
    {
        int now = GetNowCount();
        if (now - entry.lastPlayMs < entry.minIntervalMs) return;
        entry.lastPlayMs = now;
    }

    // Duyệt pool tìm handle đang rảnh (CheckSoundMem == 0)
    for (int h = 0; h < entry.poolSize; ++h)
    {
        int handle = entry.handles[h];
        if (handle != -1 && CheckSoundMem(handle) == 0)
        {
            int vol255 = CalculateVolume255(entry.baseVolume, SoundChannel::SFX);
            ChangeVolumeSoundMem(vol255, handle);
            PlaySoundMem(handle, DX_PLAYTYPE_BACK, TRUE);
            return;
        }
    }

    // Nếu toàn bộ handle trong pool đều đang phát, bỏ qua lần phát này (không cấp phát động)
}

void SoundManager::PlayBGM(BgmId id, bool fade)
{
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= BGM_COUNT) return;

    // Đang phát chính bài này và không trong quá trình FadeOut thì giữ nguyên
    if (currentBgm_ == id && fadeState_ != FadeState::FadingOut) return;

    int newHandle = bgmList_[idx].handle;

    if (!fade)
    {
        // Dừng ngay lập tức bài cũ
        if (currentBgm_ != BgmId::Count)
        {
            int curIdx = static_cast<int>(currentBgm_);
            if (bgmList_[curIdx].handle != -1) StopSoundMem(bgmList_[curIdx].handle);
        }

        currentBgm_ = id;
        nextBgm_ = BgmId::Count;
        fadeState_ = FadeState::None;
        fadeRatio_ = 1.0f;

        if (newHandle != -1)
        {
            ApplyBgmVolume(newHandle, bgmList_[idx].baseVolume, 1.0f);
            PlaySoundMem(newHandle, DX_PLAYTYPE_LOOP, TRUE);
        }
    }
    else
    {
        // Có fade
        if (currentBgm_ != BgmId::Count && bgmList_[static_cast<int>(currentBgm_)].handle != -1)
        {
            // Đang có BGM chạy -> Fade out bài cũ rồi Fade in bài mới
            nextBgm_ = id;
            fadeState_ = FadeState::FadingOut;
            fadeTimer_ = 0.0f;
            fadeDuration_ = 0.5f;
        }
        else
        {
            // Chưa có BGM nào -> Fade in ngay bài mới
            currentBgm_ = id;
            nextBgm_ = BgmId::Count;
            fadeState_ = FadeState::FadingIn;
            fadeTimer_ = 0.0f;
            fadeDuration_ = 0.5f;
            fadeRatio_ = 0.0f;

            if (newHandle != -1)
            {
                ApplyBgmVolume(newHandle, bgmList_[idx].baseVolume, 0.0f);
                PlaySoundMem(newHandle, DX_PLAYTYPE_LOOP, TRUE);
            }
        }
    }
}

void SoundManager::StopBGM(bool fade)
{
    if (currentBgm_ == BgmId::Count) return;

    if (!fade)
    {
        int curIdx = static_cast<int>(currentBgm_);
        if (bgmList_[curIdx].handle != -1) StopSoundMem(bgmList_[curIdx].handle);
        currentBgm_ = BgmId::Count;
        nextBgm_ = BgmId::Count;
        fadeState_ = FadeState::None;
    }
    else
    {
        nextBgm_ = BgmId::Count;
        fadeState_ = FadeState::FadingOut;
        fadeTimer_ = 0.0f;
        fadeDuration_ = 0.5f;
    }
}

void SoundManager::Update(float dt)
{
    if (fadeState_ == FadeState::None) return;

    if (fadeState_ == FadeState::FadingOut)
    {
        fadeTimer_ += dt;
        if (fadeTimer_ < fadeDuration_)
        {
            fadeRatio_ = 1.0f - (fadeTimer_ / fadeDuration_);
            if (currentBgm_ != BgmId::Count)
            {
                int curIdx = static_cast<int>(currentBgm_);
                ApplyBgmVolume(bgmList_[curIdx].handle, bgmList_[curIdx].baseVolume, fadeRatio_);
            }
        }
        else
        {
            // Fade out hoàn thành: Dừng bài cũ
            if (currentBgm_ != BgmId::Count)
            {
                int curIdx = static_cast<int>(currentBgm_);
                if (bgmList_[curIdx].handle != -1) StopSoundMem(bgmList_[curIdx].handle);
            }

            currentBgm_ = nextBgm_;
            nextBgm_ = BgmId::Count;

            if (currentBgm_ != BgmId::Count && bgmList_[static_cast<int>(currentBgm_)].handle != -1)
            {
                int newIdx = static_cast<int>(currentBgm_);
                fadeState_ = FadeState::FadingIn;
                fadeTimer_ = 0.0f;
                fadeRatio_ = 0.0f;
                ApplyBgmVolume(bgmList_[newIdx].handle, bgmList_[newIdx].baseVolume, 0.0f);
                PlaySoundMem(bgmList_[newIdx].handle, DX_PLAYTYPE_LOOP, TRUE);
            }
            else
            {
                fadeState_ = FadeState::None;
                fadeRatio_ = 1.0f;
            }
        }
    }
    else if (fadeState_ == FadeState::FadingIn)
    {
        fadeTimer_ += dt;
        if (fadeTimer_ < fadeDuration_)
        {
            fadeRatio_ = fadeTimer_ / fadeDuration_;
            if (currentBgm_ != BgmId::Count)
            {
                int curIdx = static_cast<int>(currentBgm_);
                ApplyBgmVolume(bgmList_[curIdx].handle, bgmList_[curIdx].baseVolume, fadeRatio_);
            }
        }
        else
        {
            // Fade in hoàn tất
            fadeRatio_ = 1.0f;
            fadeState_ = FadeState::None;
            if (currentBgm_ != BgmId::Count)
            {
                int curIdx = static_cast<int>(currentBgm_);
                ApplyBgmVolume(bgmList_[curIdx].handle, bgmList_[curIdx].baseVolume, 1.0f);
            }
        }
    }
}

const char* SoundManager::GetSfxName(SfxId id) const
{
    int idx = static_cast<int>(id);
    if (idx >= 0 && idx < SFX_COUNT) return sfxList_[idx].name;
    return "Unknown";
}

const char* SoundManager::GetBgmName(BgmId id) const
{
    int idx = static_cast<int>(id);
    if (idx >= 0 && idx < BGM_COUNT) return bgmList_[idx].name;
    return "None";
}

const wchar_t* SoundManager::GetSfxNameW(SfxId id) const
{
    int idx = static_cast<int>(id);
    if (idx >= 0 && idx < SFX_COUNT) return sfxList_[idx].wname;
    return L"Unknown";
}

const wchar_t* SoundManager::GetBgmNameW(BgmId id) const
{
    int idx = static_cast<int>(id);
    if (idx >= 0 && idx < BGM_COUNT) return bgmList_[idx].wname;
    return L"None";
}
