#pragma once
#include "PlayerFish.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Effects.h"
#include "CardManager.h"
#include "Boss.h"
#include "SharkBoss.h"
#include "DebugController.h"
#include "GodRays.h"
#include "ResultScreen.h"
#include "CameraController.h"
#include "UIRenderer.h"
#include "Consts.h"

enum class GameState
{
    Title,        // Màn hình tiêu đề bắt đầu game (G1)
    Wave,         // Đang trong wave chiến đấu (G2)
    CardPick,     // Đang chọn 1 trong 3 thẻ nâng cấp (C2)
    BossIntro,    // Chuyển cảnh giới thiệu trùm cuối (G1, B4)
    Boss,         // Trận đấu Boss Cá Voi (B)
    Victory,      // Màn Thắng cuộc (Result Win)
    GameOver      // Màn Thất bại (Result Lose)
};

enum class GameMode
{
    Classic, // Chế độ Cổ điển: 5 wave + Boss Cá Voi (Gốc)
    Endless  // Chế độ Vô tận: wave vô hạn, scaling quái, Boss định kỳ mỗi 5 wave (Task 10 T4)
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
    void EnterEndlessFromTutorial();     // Hạ Cá Voi tutorial -> tự vào Endless stage 1 (giữ thẻ)
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
    bool IsSharkFight() const { return sharkFight_; }

    void DrawArena() const;

    const wchar_t* GetLastSummary() const { return lastSummaryBuffer_; }
    float GetWaveTimer() const { return waveTimer_; }
    float GetWaveDuration() const { return waveDuration_; }

    GameState state_ = GameState::Title;
    GameMode mode_ = GameMode::Classic;
    GameMode runStartMode_ = GameMode::Classic; // Chế độ lúc bắt đầu ván (tutorial tự chuyển sang Endless giữa ván)
    int titleSelectedMode_ = 0; // 0 = Classic, 1 = Endless
    HighscoreData highscore_{};

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
    bool sharkFight_ = false;  // Trận boss hiện tại là cá mập?
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
