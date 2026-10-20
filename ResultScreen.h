#pragma once

#include "LayeredResultScreen.h"

class Input;

// =======================================================================
// ResultScreen.h
// ゲームオーバー・勝利画面: fade-in, tiêu đề chuyển động, bảng thống kê
// (thời gian, số quái hạ, level, wave, điểm) và menu Retry / Title điều khiển
// bằng bàn phím (W/S hoặc ↑/↓, Enter/Space xác nhận khi NHẢ phím như màn Pause).
// =======================================================================

struct ResultStats
{
    bool victory = false;
    bool campaign = false;
    int bossesDefeated = 0;
    bool retryTutorial = true;
    bool diedAtBoss = false;
    bool debugUsed = false;
    float time = 0.0f;
    int kills = 0;
    int level = 1;
    int wave = 1;
    int wavesCleared = 0;
    // Kỷ lục Endless (chỉ hiển thị khi endless)
    int bestWave = 0;
    int bestKills = 0;
    float bestTime = 0.0f;
    int bestScore = 0;
};

class ResultScreen
{
public:
    enum class Action { None, Retry, Title };

    void Init();
    void Shutdown();

    // Gọi khi vừa chuyển sang GameOver / Victory
    void Enter(bool victory, bool diedAtBoss);
    Action Update(float dt, const Input& input);
    void Draw(const ResultStats& stats) const;

    bool DiedAtBoss() const { return diedAtBoss_; }
    static int ComputeScore(const ResultStats& s);

private:
    LayeredResultScreen layeredScreen_;
    bool DrawIllustrated(const ResultStats& stats) const;
    // [loss / victory][English / Japanese], with lettering baked into the PNGs.
    int resultArtwork_[2][2] = { { -1, -1 }, { -1, -1 } };
    int selectionArtwork_ = -1;
    void DrawDefeat(const ResultStats& stats) const;
    int defeatArtwork_ = -1;
    static constexpr float INPUT_DELAY = 0.6f;  // chặn phím đang giữ từ lúc chơi
    float timer_ = 0.0f;
    int selected_ = 0;   // 0 = Retry, 1 = Title
    bool victory_ = false;
    bool diedAtBoss_ = false;
    bool confirmPrev_ = true;
    bool confirmReady_ = false;
    int mouseX_ = -1, mouseY_ = -1;
    bool mousePrev_ = true;
    bool mouseReady_ = false;
    int mousePressedItem_ = -1;
};
