#pragma once

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
    bool endless = false;
    bool diedAtBoss = false;
    bool debugUsed = false;
    float time = 0.0f;
    int kills = 0;
    int level = 1;
    int wave = 1;
    // Kỷ lục Endless (chỉ hiển thị khi endless)
    int bestWave = 0;
    int bestKills = 0;
    float bestTime = 0.0f;
};

class ResultScreen
{
public:
    enum class Action { None, Retry, Title };

    // Gọi khi vừa chuyển sang GameOver / Victory
    void Enter(bool victory, bool diedAtBoss);
    Action Update(float dt, const Input& input);
    void Draw(const ResultStats& stats) const;

    bool DiedAtBoss() const { return diedAtBoss_; }
    static int ComputeScore(const ResultStats& s);

private:
    static constexpr float INPUT_DELAY = 0.6f;  // chặn phím đang giữ từ lúc chơi
    float timer_ = 0.0f;
    int selected_ = 0;   // 0 = Retry, 1 = Title
    bool victory_ = false;
    bool diedAtBoss_ = false;
    bool confirmPrev_ = true;
};
