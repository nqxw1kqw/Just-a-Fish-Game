#pragma once
#include "PlayerFish.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Effects.h"
#include "CardManager.h"
#include "Boss.h"
#include "SharkBoss.h"
#include "SquidBoss.h"
#include "DebugController.h"
#include "GodRays.h"
#include "ResultScreen.h"
#include "CameraController.h"
#include "UIRenderer.h"
#include "Consts.h"

enum class GameState
{
    LanguageSelect, // Màn đầu tiên: chọn ngôn ngữ EN / JP
    Title,        // Màn hình tiêu đề bắt đầu game (G1)
    Wave,         // Đang trong wave chiến đấu (G2)
    CardPick,     // Đang chọn 1 trong 3 thẻ nâng cấp (C2)
    BossIntro,    // Chuyển cảnh giới thiệu trùm cuối (G1, B4)
    Boss,         // Trận đấu Boss Cá Voi (B)
    ModeSelect,   // Phase 1: Sau khi hạ Cá Voi tutorial -> chọn Normal / Hell
    Victory,      // Màn Thắng cuộc (Result Win)
    GameOver      // Màn Thất bại (Result Lose)
};

enum class GameMode
{
    Classic, // Chế độ Cổ điển: 5 wave + Boss Cá Voi (Gốc)
    Endless, // NORMAL: 30 stage 20s, chọn thẻ, dọn quái giữa stage, Boss mỗi 5 stage
    Hell     // HELL (Phase 1 stub): sát thương nhận x1.5, thẻ ngẫu nhiên tự chọn, không dọn quái
};

struct HighscoreData
{
    unsigned int magic = 0x4F444F52; // 'ODOR'
    int maxWave = 0;
    int maxKills = 0;
    float maxSurvivalTime = 0.0f;
    int maxScore = 0; // v3: điểm cao nhất (ResultScreen::ComputeScore)
};

class GamePrototype
{
    friend class DebugController;
    friend class UIRenderer;

public:
    void Init();
    void Shutdown();
    void Reset();
    void Update(float dt);
    void Draw() const;

    // Render interpolation (#26): đổi transform sang trạng thái nội suy trước Draw, khôi phục sau Draw
    void BeginRenderInterp(float alpha);
    void EndRenderInterp();
    bool IsRenderInterpEnabled() const { return renderInterpEnabled_; }

    GameMode GetGameMode() const { return mode_; }
    void SetGameMode(GameMode mode) { mode_ = mode; }
    const HighscoreData& GetHighscore() const { return highscore_; }
    const HighscoreData& GetHellHighscore() const { return highscoreHell_; }
    bool IsEndlessLike() const { return mode_ != GameMode::Classic; }
    int GetTotalKills() const { return enemies_.GetTotalKills(); }

    // Màn hình tạm dừng (Task #13)
    bool IsPaused() const { return paused_; }
    bool IsQuitRequested() const { return quitRequested_; }
    bool ConsumeWindowModeToggleRequest();

private:
    bool renderInterpEnabled_ = true; // F10: bật/tắt nội suy hình ảnh
    bool renderInterpActive_ = false;
    void StartWave(int wave);
    void StartBossFight();
    void StartRun(GameMode mode);        // Điểm vào DUY NHẤT để bắt đầu / chơi lại một ván (Title, R, Retry)
    // Balance pass: luồng Tutorial (3 wave + Cá Voi) -> Endless (30 stage, Boss mỗi 5 stage)
    void StartStage(int stage);          // Endless: stage chia hết cho ENDLESS_BOSS_EVERY -> Boss
    void BeginBossIntro();
    void AdvanceAfterCardPick();         // Sau khi chọn thẻ: sang wave/stage kế tiếp hoặc Boss
    void EnterEndlessFromTutorial(GameMode mode); // Màn chọn chế độ -> vào Normal/Hell stage 1 (giữ thẻ)
    void ApplyModeRules();               // Áp luật riêng của chế độ (Hell: x1.5 dmg, trần quái)
    void HellAutoPickCard();             // Hell: tự chọn 1 thẻ ngẫu nhiên, không dừng game
    float UpdateTutorialTips(float dt);  // Trả về hệ số thời gian (làm chậm nhẹ khi gợi ý vừa hiện)

    enum TipId { Tip_Move, Tip_Jump, Tip_Hit, Tip_Card, Tip_Boss, Tip_Count };
    bool tipShown_[Tip_Count] = {};      // Mỗi gợi ý chỉ hiện 1 lần / phiên (không reset khi Retry)
    int activeTip_ = -1;
    float tipTimer_ = 0.0f;
    bool tipsSkipped_ = false;
    Vec3 tipMoveStart_{};
    int modeSelectIdx_ = 0;              // 0 = Normal, 1 = Hell
    float hellCardToastTimer_ = 0.0f;
    // Phase 2: vòng quay thẻ Hell
    void UpdateHellRoulette(float dt);
    void FinishHellRoulette();
    bool rouletteActive_ = false;
    bool rouletteLanded_ = false;
    int rouletteCount_ = 0;
    int rouletteIdx_ = 0;
    int rouletteTarget_ = 0;
    float rouletteTimer_ = 0.0f;
    float rouletteStepTimer_ = 0.0f;
    std::array<Card, 3> rouletteCards_{};
    // Hell: lên cấp -> +1 bậc chỉ số ngẫu nhiên
    enum LvStat { LS_Atk, LS_Def, LS_Spd, LS_Cd, LS_Luck, LS_Count };
    int lvStat_[LS_Count] = {};          // Normal/Hell: điểm chỉ số từ lên cấp
    int levelUpStat_ = 0;
    float levelUpBannerTimer_ = 0.0f;
    void GrantLevelStat();
    void RefreshDamageTakenMul();        // modeMul x (1 - Def)
    wchar_t hellCardToast_[96] = L"";
    void EnterEndlessClear();            // Qua stage ENDLESS_MAX_STAGE -> màn CLEAR
    void OnWaveComplete();
    void ReapplyAllStats();

    void LoadHighscore();
    void SaveHighscore();
    void SaveHighscoreIfEndlessRun();

    // Hỗ trợ Debug (Phần D)
    void SkipCurrentWave();
    void SkipToWave(int waveTarget);
    void SkipToEndlessWave(int waveTarget);
    void SkipToBoss();
    void SkipToSharkBoss();   // Debug: nhảy thẳng tới Boss Cá Mập (Endless)
    void SkipToSquidBoss();   // Debug: nhảy thẳng tới Boss Mực (Endless)
    bool IsSharkFight() const { return sharkFight_; }

    void DrawArena() const;

    const wchar_t* GetLastSummary() const { return lastSummaryBuffer_; }
    float GetWaveTimer() const { return waveTimer_; }
    float GetWaveDuration() const { return waveDuration_; }

    GameState state_ = GameState::LanguageSelect;
    int langSelectIdx_ = 0;              // 0 = English, 1 = 日本語
    bool panelsVisible_ = GameConsts::INFO_PANELS_DEFAULT_VISIBLE; // F11: bảng stats/thẻ/debug/hướng dẫn
    GameMode mode_ = GameMode::Classic;
    GameMode runStartMode_ = GameMode::Classic; // Chế độ lúc bắt đầu ván (tutorial tự chuyển sang Endless giữa ván)
    int titleSelectedMode_ = 0; // 0 = Classic, 1 = Endless
    HighscoreData highscore_{};
    HighscoreData highscoreHell_{}; // Kỷ lục riêng của Hell (highscore_hell.dat)

    int currentWave_ = 1;
    float waveTimer_ = 0.0f;
    float waveDuration_ = 30.0f;
    float bossIntroTimer_ = 0.0f;
    wchar_t lastSummaryBuffer_[128] = L"Chưa có tóm tắt wave";

    PlayerFish player_;
    EnemyManager enemies_;
    WeaponSystem weapon_;
    EffectSystem effects_;
    CardManager cardManager_;
    WhaleBoss boss_;
    SharkBoss shark_;          // Boss Cá Mập (chỉ Endless)
    SquidBoss squid_;          // Boss #3: Giant Squid (chỉ Endless)
    bool sharkFight_ = false;  // Trận boss hiện tại là cá mập?
    int bossKind_ = 0;         // 0 Whale, 1 Shark, 2 Squid
    void PrepareEndlessBoss(int bossIdx);
    PlayerStats playerStats_;

    // Camera 3D bám theo cá
    CameraController cameraController_;

    float hitStopTimer_ = 0.0f;
    float gameTime_ = 0.0f;

    DebugController debugController_;
    GodRays godRays_; // 光の筋 (phím G bật/tắt)
    ResultScreen resultScreen_; // ゲームオーバー・勝利画面
    ResultStats BuildResultStats() const;
    UIRenderer uiRenderer_;

    // Màn hình tạm dừng (Task #13)
    enum PauseItem
    {
        PauseItem_Resume,
        PauseItem_BgmVolume,
        PauseItem_SfxVolume,
        PauseItem_WindowMode,
        PauseItem_Title,
        PauseItem_Count
    };
    bool UpdatePauseInput(); // true = đang tạm dừng, bỏ qua logic game trong tick này
    bool paused_ = false;
    int pauseSelected_ = PauseItem_Resume;
    bool quitRequested_ = false;
    bool windowModeToggleRequested_ = false;
};
