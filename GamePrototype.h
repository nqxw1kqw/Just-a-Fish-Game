#pragma once
#include "PlayerFish.h"
#include "Enemy.h"
#include "Bullet.h"
#include "Effects.h"
#include "CardManager.h"
#include "Boss.h"
#include "SharkBoss.h"
#include "SquidBoss.h"
#include "TurtleBoss.h"
#include "SwordfishBoss.h"
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

// 選択できる難易度はありません。1回の冒険の中の進行段階です。
enum class RunPhase { Tutorial, Campaign };

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

    RunPhase GetRunPhase() const { return phase_; }
    bool IsCampaign() const { return phase_ == RunPhase::Campaign; }
    int GetProgressionStage() const
    {
        if (!IsCampaign()) return 0;
        const int stage = (currentWave_ - 1) / GameConsts::WAVES_PER_BOSS + 1;
        return stage < 1 ? 1 : (stage > GameConsts::CAMPAIGN_BOSSES ? GameConsts::CAMPAIGN_BOSSES : stage);
    }
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
    void StartRun();        // Điểm vào DUY NHẤT để bắt đầu / chơi lại một ván (Title, R, Retry)
    // チュートリアル → 25秒の通常ウェーブを3回 → ボス。5体撃破で勝利。
    void BeginBossIntro();
    void AdvanceAfterCardPick();         // カード選択後に次のウェーブまたはボスへ進む
    void FinishTutorial(); // 所持カードを引き継ぎ、通常ウェーブ1へ進む
    void ApplyPhaseRules();               // 練習中と通常プレイの補正を適用
    float UpdateTutorialTips(float dt);  // Trả về hệ số thời gian (làm chậm nhẹ khi gợi ý vừa hiện)

    enum TipId { Tip_Move, Tip_Jump, Tip_Hit, Tip_Card, Tip_Boss, Tip_Count };
    bool tipShown_[Tip_Count] = {};      // Mỗi gợi ý chỉ hiện 1 lần / phiên (không reset khi Retry)
    int activeTip_ = -1;
    float tipTimer_ = 0.0f;
    bool tipsSkipped_ = false;
    Vec3 tipMoveStart_{};
    void RefreshDamageTakenMul();        // modeMul x (1 - Def)
    void CompleteCampaign();            // 5体のボスを撃破すると勝利
    void OnWaveComplete();
    void ReapplyAllStats();

    void LoadHighscore();
    void SaveHighscore();
    void SaveRunRecord();

    // Hỗ trợ Debug (Phần D)
    void SkipCurrentWave();
    void SkipToWave(int waveTarget);
    void SkipToCampaignWave(int waveTarget);
    void SkipToBoss();
    void SkipToSharkBoss();   // Debug: nhảy thẳng tới Boss Cá Mập (Endless)
    void SkipToSquidBoss();   // Debug: nhảy thẳng tới Boss Mực (Endless)
    void SkipToTurtleBoss();  // Debug: nhảy thẳng tới Boss Rùa Cổ (Endless)
    void SkipToSwordfishBoss(); // Debug: nhảy thẳng tới Boss Cá Kiếm (Endless)
    bool IsSharkFight() const { return sharkFight_; }

    void DrawArena() const;

    const wchar_t* GetLastSummary() const { return lastSummaryBuffer_; }
    float GetWaveTimer() const { return waveTimer_; }
    float GetWaveDuration() const { return waveDuration_; }

    GameState state_ = GameState::Title;
    bool panelsVisible_ = GameConsts::INFO_PANELS_DEFAULT_VISIBLE; // F11: bảng stats/thẻ/debug/hướng dẫn
    RunPhase phase_ = RunPhase::Tutorial;
    HighscoreData highscore_{};
    int bossesDefeated_ = 0;
    int completedWaves_ = 0;
    std::array<int, GameConsts::CAMPAIGN_BOSSES> bossOrder_ = {0, 1, 2, 3, 4};

    int currentWave_ = 1;
    float waveTimer_ = 0.0f;
    float waveDuration_ = 30.0f;
    float bossIntroTimer_ = 0.0f;
    wchar_t lastSummaryBuffer_[128] = L"まだ記録がありません";

    PlayerFish player_;
    EnemyManager enemies_;
    WeaponSystem weapon_;
    EffectSystem effects_;
    CardManager cardManager_;
    WhaleBoss boss_;
    SharkBoss shark_;          // Boss Cá Mập (chỉ Endless)
    SquidBoss squid_;          // Boss #3: Giant Squid (chỉ Endless)
    TurtleBoss turtle_;        // Boss #4: Ancient Turtle (chỉ Endless)
    SwordfishBoss swordfish_;  // Boss #5: Swordfish (chỉ Endless)
    bool sharkFight_ = false;  // Trận boss hiện tại là cá mập?
    int bossKind_ = 0;         // 0 Whale, 1 Shark, 2 Squid, 3 Turtle, 4 Swordfish
    void PrepareBossForRound(int bossIndex);
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
