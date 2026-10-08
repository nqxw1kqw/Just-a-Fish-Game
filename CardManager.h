#pragma once
#include "Card.h"
#include "CardDatabase.h"
#include <vector>
#include <array>

class CardManager
{
public:
    void Init();
    void Shutdown();
    void Reset();

#if defined(_DEBUG) || !defined(NDEBUG)
    void RunSelfTest();
#endif

    // Roll 3 thẻ ngẫu nhiên dựa trên số wave vừa hoàn thành
    void Roll3Cards(int completedWave);

    // Cấp ngẫu nhiên count thẻ hợp lệ (Dùng cho Debug Ctrl+1..6)
    void GrantRandomCards(int count);

    // Xử lý phím chọn thẻ (A/D/Left/Right + Enter, hoặc phím 1/2/3)
    bool Update(float dt);

    // Vẽ giao diện chọn thẻ
    void Draw() const;

    // Áp dụng chỉ số thẻ lên PlayerStats
    void ApplyToStats(PlayerStats& stats) const;

    // Kiểm tra hook & số lượng thẻ sở hữu (Task 10 T4d)
    bool HasHook(HookType hook) const;
    bool HasCard(const std::string& cardId) const;
    int GetCardCount(const std::string& cardId) const;
    int GetAvailableCardsPoolCount() const;

    void SetEndless(bool enable) { isEndless_ = enable; }
    bool IsEndless() const { return isEndless_; }
    int GetOfferedCardCount() const { return offeredCount_; }

    const std::vector<Card>& GetActiveCards() const { return activeCards_; }
    const std::array<Card, 3>& GetCurrentOffer() const { return currentOffer_; }

private:
    CardRarity PickRarity(int completedWave) const;

    bool isEndless_ = false;
    std::vector<Card> activeCards_;
    std::array<Card, 3> currentOffer_{};
    int offeredCount_ = 3;
    int selectedIndex_ = 0; // 0, 1, 2
    float inputDelay_ = 0.0f;
};
