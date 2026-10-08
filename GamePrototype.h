#pragma once
#include "PlayerFish.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Effects.h"
#include "CardManager.h"
#include "Boss.h"
#include "DebugController.h"
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

    GameMode GetGameMode() const { return mode_; }
    void SetGameMode(GameMode mode) { mode_ = mode; }
    const HighscoreData& GetHighscore() const { return highscore_; }
    int GetTotalKills() const { return enemies_.GetTotalKills(); }

private:
    void StartWave(int wave);
    void StartBossFight();
    void OnWaveComplete();
    void ReapplyAllStats();

    void LoadHighscore();
    void SaveHighscore();

    // Hỗ trợ Debug (Phần D)
    void SkipCurrentWave();
    void SkipToWave(int waveTarget);
    void SkipToEndlessWave(int waveTarget);
    void SkipToBoss();

    void DrawArena() const;

    const wchar_t* GetLastSummary() const { return lastSummaryBuffer_; }
    float GetWaveTimer() const { return waveTimer_; }
    float GetWaveDuration() const { return waveDuration_; }

    GameState state_ = GameState::Title;
    GameMode mode_ = GameMode::Classic;
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
    PlayerStats playerStats_;

    // Camera 3D bám theo cá
    CameraController cameraController_;

    float hitStopTimer_ = 0.0f;
    float gameTime_ = 0.0f;

    DebugController debugController_;
    UIRenderer uiRenderer_;
};
