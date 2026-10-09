#pragma once
#include <array>
#include "DxLib.h"

// =======================================================================
//  HỆ THỐNG QUẢN LÝ ÂM THANH - SOUND MANAGER (ODORU GAME)
//  - Quản lý BGM (Phát lặp, Fade-in, Fade-out)
//  - Quản lý SFX (Polyphony mảng tĩnh, không cấp phát động trong 120Hz loop)
//  - 3 Kênh âm lượng: Master, BGM, SFX (0 - 100 quy đổi sang 0 - 255 của DxLib)
// =======================================================================

enum class SoundChannel
{
    Master,
    BGM,
    SFX
};

// Định danh các hiệu ứng âm thanh (Yêu cầu 2)
enum class SfxId
{
    // Nhân vật cá chính
    SwimPaddle,         // sfx_swim_paddle: quạt nước khi bơi
    JumpLaunch,         // sfx_jump_launch: phóng mình lên không trung
    SlamImpact,         // sfx_slam_impact: dập đất tạo sóng nổ (quan trọng nhất)
    PlayerHit,          // sfx_player_hit: cá bị quái cắn trúng

    // Quái vật & Vũ khí
    EnemySquish,        // sfx_enemy_squish: quái bị sóng chấn động nghiền nát
    SharkTelegraph,     // sfx_shark_telegraph: cá mập khóa mục tiêu laser đỏ
    SharkDash,          // sfx_shark_dash: cá mập phóng lao
    PufferSpikes,       // sfx_puffer_spikes: cá nóc bắn 8 gai phản đòn
    ExpPickup,          // sfx_exp_pickup: nhặt ngọc kinh nghiệm / hồi phục

    // Trùm cuối Cá Voi (Whale Titan Boss)
    WhaleGroan,         // sfx_whale_groan: cá voi gầm lúc Boss Intro
    TsunamiCharge,      // sfx_tsunami_charge: hút nước nạp sóng thần
    TsunamiSurge,       // sfx_tsunami_surge: vành sóng thần quét qua sàn đấu
    BossHeavySlam,      // sfx_boss_heavy_slam: địa chấn khi Boss nhảy đập

    // Giao diện người dùng
    UiHover,            // sfx_ui_hover: lướt qua thẻ bài / nút
    CardFlip,           // sfx_card_flip: mở màn hình chọn thẻ
    CardSelect,         // sfx_card_select: khóa chọn thẻ nâng cấp

    Count
};

// Định danh nhạc nền BGM (Yêu cầu 3)
enum class BgmId
{
    Title,              // bgm_title.mp3: piano & synth êm dịu, đại dương đêm
    Battle,             // bgm_battle.mp3: nhịp trống dồn dập cho các Wave thường
    Boss,               // bgm_boss.mp3: giao hưởng căng thẳng trận trùm
    Victory,            // bgm_victory.mp3: kèn đồng tươi sáng mừng chiến thắng
    GameOver,           // bgm_gameover.mp3: cello u buồn lúc cá mắc cạn

    Count
};

class SoundManager
{
public:
    static constexpr int SFX_COUNT = static_cast<int>(SfxId::Count);
    static constexpr int BGM_COUNT = static_cast<int>(BgmId::Count);
    static constexpr int MAX_SFX_POOL_HANDLES = 6; // Đủ cho sfx_slam_impact (6 handles)

    // Lấy instance duy nhất
    static SoundManager& Instance();

    // Khởi tạo & nạp toàn bộ âm thanh (gọi 1 lần sau DxLib_Init)
    void Init();

    // Giải phóng toàn bộ handle âm thanh (gọi trước DxLib_End)
    void Shutdown();

    // Cập nhật fade BGM mỗi tick 120Hz (không cấp phát động)
    void Update(float dt);

    // Phát hiệu ứng âm thanh (tự động chọn handle rảnh trong pool tĩnh)
    void PlaySFX(SfxId id);

    // Phát nhạc nền BGM (hỗ trợ fade-out BGM cũ và fade-in BGM mới)
    void PlayBGM(BgmId id, bool fade = true);

    // Dừng nhạc nền hiện tại
    void StopBGM(bool fade = true);

    // Quản lý âm lượng 3 kênh (0 .. 100)
    void SetVolume(SoundChannel ch, int value0to100);
    int  GetVolume(SoundChannel ch) const;

    // Hỗ trợ kiểm thử Debug
    const char*    GetSfxName(SfxId id) const;
    const char*    GetBgmName(BgmId id) const;
    const wchar_t* GetSfxNameW(SfxId id) const;
    const wchar_t* GetBgmNameW(BgmId id) const;
    BgmId GetCurrentBgm() const { return currentBgm_; }

private:
    SoundManager() = default;
    ~SoundManager() = default;
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    // Cấu trúc một mục SFX trong mảng tĩnh
    struct SfxEntry
    {
        std::array<int, MAX_SFX_POOL_HANDLES> handles{ -1, -1, -1, -1, -1, -1 };
        int poolSize = 0;
        int baseVolume = 100;
        const char* name = "";
        const wchar_t* wname = L"";
    };

    // Cấu trúc một mục BGM trong mảng tĩnh
    struct BgmEntry
    {
        int handle = -1;
        int baseVolume = 100;
        const char* name = "";
        const wchar_t* wname = L"";
    };

    // Máy trạng thái Fade BGM
    enum class FadeState
    {
        None,
        FadingOut,
        FadingIn
    };

    void ApplyAllVolumes();
    void ApplyBgmVolume(int handle, int baseVolume, float fadeRatio);
    int  CalculateVolume255(int baseVolume, SoundChannel ch) const;

    // Mảng tĩnh cố định
    std::array<SfxEntry, SFX_COUNT> sfxList_{};
    std::array<BgmEntry, BGM_COUNT> bgmList_{};

    // 3 Kênh âm lượng (0..100)
    int masterVolume_ = 80;
    int bgmVolume_    = 75;
    int sfxVolume_    = 85;

    // Trạng thái BGM hiện tại
    BgmId currentBgm_ = BgmId::Count;
    BgmId nextBgm_    = BgmId::Count;
    FadeState fadeState_ = FadeState::None;
    float fadeTimer_  = 0.0f;
    float fadeDuration_ = 0.5f; // Mặc định 0.5s theo yêu cầu
    float fadeRatio_  = 1.0f;
};
