#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cstdlib>
#include "CardManager.h"
#include "Consts.h"
#include "SoundManager.h"
#include "DxLib.h"
#include "FontManager.h"
#include "Localization.h"
namespace
{
    // Vẽ chuỗi căn giữa theo trục X quanh cx (độ dài chữ EN/JP khác nhau)
    void DrawCX(int cx, int y, const wchar_t* text, unsigned int color, FontSize size)
    {
        FontManager::Draw(cx - FontManager::GetWidth(text, size) / 2, y, text, color, size);
    }
}

#include "CardFrame.h"
#include "Input.h"

void CardManager::Init()
{
    Reset();
    CardDatabase::LoadImages();
#if defined(_DEBUG) || !defined(NDEBUG)
    RunSelfTest();
#endif
}

void CardManager::Shutdown()
{
    CardDatabase::ReleaseImages();
}

void CardManager::Reset()
{
    activeCards_.clear();
    selectedIndex_ = 0;
}

CardRarity CardManager::PickRarity(int completedWave) const
{
    int r = (std::min)(99, rand() % 100 + luck_ * GameConsts::LV_LUCK_RARITY_SHIFT);
    if (completedWave <= 2)
    {
        // 75% Common, 25% Rare, 0% Epic
        return (r < 75) ? CardRarity::Common : CardRarity::Rare;
    }
    else if (completedWave == 3)
    {
        // 55% Common, 38% Rare, 7% Epic
        if (r < 55) return CardRarity::Common;
        if (r < 93) return CardRarity::Rare;
        return CardRarity::Epic;
    }
    else if (completedWave == 4)
    {
        // 40% Common, 45% Rare, 15% Epic
        if (r < 40) return CardRarity::Common;
        if (r < 85) return CardRarity::Rare;
        return CardRarity::Epic;
    }
    else
    {
        // 30% Common, 45% Rare, 25% Epic
        if (r < 30) return CardRarity::Common;
        if (r < 75) return CardRarity::Rare;
        return CardRarity::Epic;
    }
}

int CardManager::GetCardCount(const std::string& cardId) const
{
    int count = 0;
    for (const auto& card : activeCards_)
    {
        if (card.id == cardId) count++;
    }
    return count;
}

int CardManager::GetAvailableCardsPoolCount() const
{
    const auto& allCards = CardDatabase::GetAllCards();
    int count = 0;
    for (const auto& card : allCards)
    {
        int limit = isEndless_ ? card.maxStack : 1;
        if (card.stackable || GetCardCount(card.id) < limit)
        {
            count++;
        }
    }
    return count;
}

void CardManager::Roll3Cards(int completedWave)
{
    const auto& allCards = CardDatabase::GetAllCards();
    std::vector<Card> availableCards;

    // Lọc các thẻ hợp lệ: trong Classic tối đa 1 thẻ, trong Endless tối đa card.maxStack thẻ (Task 10 T4d)
    for (const auto& card : allCards)
    {
        int limit = isEndless_ ? card.maxStack : 1;
        if (card.stackable || GetCardCount(card.id) < limit)
        {
            availableCards.push_back(card);
        }
    }

    std::vector<Card> pickedCards;
    selectedIndex_ = 0;
    inputDelay_ = 0.35f;

    // Nếu pool thẻ rỗng (hết sạch thẻ có thể lấy)
    if (availableCards.empty())
    {
        offeredCount_ = 0;
        for (int i = 0; i < 3; ++i) currentOffer_[i] = Card{};
        return;
    }

    // Xác định số thẻ đưa ra (tối đa 3 hoặc số lượng còn lại trong pool)
    int countToOffer = (std::min)(3, static_cast<int>(availableCards.size()));
    offeredCount_ = countToOffer;

    for (int slot = 0; slot < countToOffer; ++slot)
    {
        CardRarity targetRarity = PickRarity(completedWave);

        std::vector<CardRarity> priority;
        if (targetRarity == CardRarity::Epic)
        {
            priority = { CardRarity::Epic, CardRarity::Rare, CardRarity::Common };
        }
        else if (targetRarity == CardRarity::Rare)
        {
            priority = { CardRarity::Rare, CardRarity::Common, CardRarity::Epic };
        }
        else
        {
            priority = { CardRarity::Common, CardRarity::Rare, CardRarity::Epic };
        }

        Card chosenCard;
        bool found = false;

        // 1. Thử từng độ hiếm theo thứ tự rơi xuống trong availableCards
        for (CardRarity r : priority)
        {
            std::vector<Card> candidates;
            for (const auto& card : availableCards)
            {
                bool alreadyPicked = false;
                for (const auto& p : pickedCards)
                {
                    if (p.id == card.id) { alreadyPicked = true; break; }
                }
                if (!alreadyPicked && card.rarity == r)
                {
                    candidates.push_back(card);
                }
            }

            if (!candidates.empty())
            {
                int idx = rand() % candidates.size();
                chosenCard = candidates[idx];
                found = true;
                break;
            }
        }

        // 2. Nếu không tìm thấy theo độ hiếm ưu tiên, lấy bất kỳ thẻ nào chưa pick trong availableCards
        if (!found)
        {
            std::vector<Card> candidates;
            for (const auto& card : availableCards)
            {
                bool alreadyPicked = false;
                for (const auto& p : pickedCards)
                {
                    if (p.id == card.id) { alreadyPicked = true; break; }
                }
                if (!alreadyPicked)
                {
                    candidates.push_back(card);
                }
            }

            if (!candidates.empty())
            {
                int idx = rand() % candidates.size();
                chosenCard = candidates[idx];
                found = true;
            }
        }

        if (found)
        {
            pickedCards.push_back(chosenCard);
        }
    }

    for (int i = 0; i < 3; ++i)
    {
        if (i < static_cast<int>(pickedCards.size()))
        {
            currentOffer_[i] = pickedCards[i];
        }
        else
        {
            currentOffer_[i] = Card{};
        }
    }
}

void CardManager::GrantRandomCards(int count)
{
    const auto& allCards = CardDatabase::GetAllCards();
    for (int i = 0; i < count; ++i)
    {
        std::vector<Card> candidates;
        for (const auto& c : allCards)
        {
            int limit = isEndless_ ? c.maxStack : 1;
            if (c.stackable || GetCardCount(c.id) < limit)
            {
                candidates.push_back(c);
            }
        }
        if (!candidates.empty())
        {
            int idx = rand() % candidates.size();
            activeCards_.push_back(candidates[idx]);
        }
    }
}

bool CardManager::Update(float dt)
{
    if (offeredCount_ <= 0) return true; // Không có thẻ để chọn, tự động vượt qua

    // Đọc phím "vừa nhấn" từ trạng thái phím chung (hỗ trợ cả phím số thường và Numpad)
    const Input& input = Input::Instance();
    bool leftPressed = input.IsPressed(KEY_INPUT_A) || input.IsPressed(KEY_INPUT_LEFT);
    bool rightPressed = input.IsPressed(KEY_INPUT_D) || input.IsPressed(KEY_INPUT_RIGHT);
    bool key1Pressed = input.IsPressed(KEY_INPUT_1) || input.IsPressed(KEY_INPUT_NUMPAD1);
    bool key2Pressed = input.IsPressed(KEY_INPUT_2) || input.IsPressed(KEY_INPUT_NUMPAD2);
    bool key3Pressed = input.IsPressed(KEY_INPUT_3) || input.IsPressed(KEY_INPUT_NUMPAD3);
    bool enterPressed = input.IsPressed(KEY_INPUT_RETURN) || input.IsPressed(KEY_INPUT_NUMPADENTER);

    // Khoảng đệm an toàn 0.35s sau khi vừa mở bảng chọn thẻ
    // (phím đang giữ từ trước sẽ không tạo cạnh "vừa nhấn" mới khi hết đệm)
    if (inputDelay_ > 0.0f)
    {
        inputDelay_ -= dt;
        return false;
    }

    // Phím A/D hoặc mũi tên để đổi con trỏ
    if (leftPressed)
    {
        selectedIndex_ = (selectedIndex_ + offeredCount_ - 1) % offeredCount_;
        SoundManager::Instance().PlaySFX(SfxId::UiHover);
    }

    if (rightPressed)
    {
        selectedIndex_ = (selectedIndex_ + 1) % offeredCount_;
        SoundManager::Instance().PlaySFX(SfxId::UiHover);
    }

    // Phím số 1, 2, 3 chọn trực tiếp (chỉ cho phép slot hợp lệ)
    int chooseSlot = -1;
    if (key1Pressed && offeredCount_ >= 1) chooseSlot = 0;
    if (key2Pressed && offeredCount_ >= 2) chooseSlot = 1;
    if (key3Pressed && offeredCount_ >= 3) chooseSlot = 2;

    // Phím Enter xác nhận ô đang chọn
    if (enterPressed)
    {
        chooseSlot = selectedIndex_;
    }

    if (chooseSlot >= 0 && chooseSlot < offeredCount_)
    {
        SoundManager::Instance().PlaySFX(SfxId::CardSelect);
        activeCards_.push_back(currentOffer_[chooseSlot]);
        return true; // Đã chọn xong 1 thẻ!
    }

    return false;
}

const Card* CardManager::AutoPickRandom()
{
    if (offeredCount_ <= 0) return nullptr;
    activeCards_.push_back(currentOffer_[rand() % offeredCount_]);
    offeredCount_ = 0;
    return &activeCards_.back();
}

void CardManager::ApplyToStats(PlayerStats& stats) const
{
    stats.ResetToBase();

    for (const auto& card : activeCards_)
    {
        for (const auto& baseMod : card.modifiers)
        {
            // Hell: giảm hiệu lực thẻ (Add/Percent x scale, More: 1 + (v-1) x scale). Chỉ số đếm nguyên giữ nguyên.
            auto mod = baseMod;
            if (bonusScale_ != 1.0f && mod.statName != "bulletCount" && mod.statName != "extraJumps")
            {
                if (mod.type == ModifierType::More) mod.value = 1.0f + (mod.value - 1.0f) * bonusScale_;
                else mod.value *= bonusScale_;
            }
            if (mod.statName == "maxHp") stats.maxHp.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "moveSpeed") stats.moveSpeed.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "jumpCooldown") stats.jumpCooldown.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "bounceDuration" || mod.statName == "jumpDuration") stats.bounceDuration.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "bounceHeight" || mod.statName == "jumpHeight") {
                stats.bounceHeight.AddModifier(card.id, mod.type, mod.value);
                stats.jumpHeight.AddModifier(card.id, mod.type, mod.value);
            }
            else if (mod.statName == "apexThreshold") stats.apexThreshold.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "shockwaveDamage" || mod.statName == "slamDamage") {
                stats.shockwaveDamage.AddModifier(card.id, mod.type, mod.value);
                stats.slamDamage.AddModifier(card.id, mod.type, mod.value);
            }
            else if (mod.statName == "shockwaveRadius" || mod.statName == "slamRadius") {
                stats.shockwaveRadius.AddModifier(card.id, mod.type, mod.value);
                stats.slamRadius.AddModifier(card.id, mod.type, mod.value);
            }
            else if (mod.statName == "slamKnockback") stats.slamKnockback.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "extraJumps") stats.extraJumps.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "airSpeed") stats.airSpeed.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "bulletDamage") stats.bulletDamage.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "fireCooldown") stats.fireCooldown.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "bulletSpeed") stats.bulletSpeed.AddModifier(card.id, mod.type, mod.value);
            else if (mod.statName == "bulletCount") stats.bulletCount.AddModifier(card.id, mod.type, mod.value);
        }
    }
}

bool CardManager::HasHook(HookType hook) const
{
    for (const auto& card : activeCards_)
    {
        if (card.hook == hook) return true;
    }
    return false;
}

bool CardManager::HasCard(const std::string& cardId) const
{
    for (const auto& card : activeCards_)
    {
        if (card.id == cardId) return true;
    }
    return false;
}

void CardManager::Draw() const
{
    int sw = GameConsts::SCREEN_WIDTH;
    int sh = GameConsts::SCREEN_HEIGHT;

    // 1. Phủ màn tối mờ
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 215);
    DrawBox(0, 0, sw, sh, GetColor(10, 16, 28), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    if (offeredCount_ <= 0)
    {
        DrawCX(sw / 2, sh / 2 - 30, TR(L"=== 全カード最大まで強化済み！ ==="), GetColor(255, 230, 80), FontSize::Header);
        DrawCX(sw / 2, sh / 2 + 10, TR(L"カードがもうありません。最大HPの20%回復ボーナス！"), GetColor(100, 255, 160), FontSize::Medium);
        DrawCX(sw / 2, sh / 2 + 45, TR(L"[ENTER] で泳ぎを続ける…"), GetColor(200, 225, 255), FontSize::Body);
        return;
    }

    // 2. Tiêu đề
    DrawCX(sw / 2, 90, TR(L"=== 強化カードを1枚選ぼう (ウェーブクリア！) ==="), GetColor(255, 230, 80), FontSize::Header);
    if (offeredCount_ == 1)
    {
        DrawCX(sw / 2, 125, TR(L"[Enter] または [1] で決定"), GetColor(180, 205, 235), FontSize::Body);
    }
    else if (offeredCount_ == 2)
    {
        DrawCX(sw / 2, 125, TR(L"[A/D] で選択 + [Enter] で決定、または [1] / [2]"), GetColor(180, 205, 235), FontSize::Body);
    }
    else
    {
        DrawCX(sw / 2, 125, TR(L"[A/D] で選択 + [Enter] で決定、または [1] / [2] / [3]"), GetColor(180, 205, 235), FontSize::Body);
    }

    // 3. Bố cục thẻ Card linh hoạt (T3, T4d: căn giữa theo số thẻ offeredCount_)
    int cardW = GameConsts::CARD_WIDTH;
    int cardH = GameConsts::CARD_HEIGHT;
    int spacing = GameConsts::CARD_SPACING;
    int imgSize = GameConsts::CARD_IMAGE_SIZE;
    int totalW = offeredCount_ * cardW + (offeredCount_ - 1) * spacing;
    int startX = (sw - totalW) / 2;
    const int cardBaseY = 175;
    const float frameTime = static_cast<float>(GetNowCount()) / 1000.0f;

    for (int i = 0; i < offeredCount_; ++i)
    {
        const auto& card = currentOffer_[i];
        int cx = startX + i * (cardW + spacing);
        bool isSelected = (i == selectedIndex_);
        // Thẻ đang chọn được nhấc lên nhẹ (選択演出)
        int cardY = cardBaseY - (isSelected ? 10 : 0);

        // カード枠: khung theo độ hiếm (Common bạc / Rare tím / Epic vàng kim)
        const CardFrame::Style& frame = CardFrame::GetStyle(card.rarity);
        int edgeColor = static_cast<int>(frame.edge);
        wchar_t rarityStr[24];
        swprintf_s(rarityStr, L"[%s]", TR(frame.label));
        CardFrame::DrawBack(cx, cardY, cardW, cardH, card.rarity, isSelected, frameTime + i * 0.7f);

        // Thanh tiêu đề số phím tắt [1..3], độ hiếm và thông tin stack trong Endless (Task 10 T4d)
        wchar_t slotHeader[48];
        if (isEndless_)
        {
            int currentCount = GetCardCount(card.id);
            swprintf_s(slotHeader, TR(L"キー [%d] %s (x%d/%d)"), i + 1, rarityStr, currentCount, card.maxStack);
        }
        else
        {
            swprintf_s(slotHeader, TR(L"キー [%d]  %s"), i + 1, rarityStr);
        }
        FontManager::Draw(cx + 12, cardY + 12, slotHeader, edgeColor, FontSize::Small);

        // Ô ảnh minh họa 220 x 220 (T2 & T3)
        int imgX = cx + (cardW - imgSize) / 2;
        int imgY = cardY + 36;
        if (card.imageHandle != -1)
        {
            SetDrawMode(DX_DRAWMODE_BILINEAR);
            DrawExtendGraph(imgX, imgY, imgX + imgSize, imgY + imgSize, card.imageHandle, TRUE);
            SetDrawMode(DX_DRAWMODE_NEAREST);
            DrawBox(imgX, imgY, imgX + imgSize, imgY + imgSize, edgeColor, FALSE);
        }
        else
        {
            // Fallback nếu không có ảnh: vẽ ô màu trơn kèm cảnh báo (T2)
            DrawBox(imgX, imgY, imgX + imgSize, imgY + imgSize, GetColor(30, 42, 60), TRUE);
            DrawBox(imgX, imgY, imgX + imgSize, imgY + imgSize, GetColor(70, 90, 120), FALSE);
            FontManager::Draw(imgX + 25, imgY + imgSize / 2 - 15, TR(L"[画像なし]"), GetColor(255, 120, 100), FontSize::Body);
        }

        // Tên thẻ (chữ trắng)
        int textY = imgY + imgSize + 14;
        FontManager::Draw(cx + 16, textY, TR(card.name.c_str()), GetColor(255, 255, 255), FontSize::Medium);

        // Đường kẻ ngăn cách mảnh
        DrawLine(cx + 16, textY + 28, cx + cardW - 16, textY + 28, GetColor(50, 75, 110));

        // Dòng lợi ích (chữ xanh lá)
        FontManager::Draw(cx + 16, textY + 36, TR(card.benefit.c_str()), GetColor(80, 255, 120), FontSize::Small);

        // Dòng đánh đổi (chữ đỏ cam, ẩn đi nếu không có đánh đổi)
        if (!card.drawback.empty())
        {
            FontManager::Draw(cx + 16, textY + 62, TR(card.drawback.c_str()), GetColor(255, 100, 90), FontSize::Small);
        }

        // Hiệu ứng phủ (vệt sáng, ảnh khung PNG nếu có)
        CardFrame::DrawFront(cx, cardY, cardW, cardH, card.rarity, isSelected, frameTime + i * 0.7f);

        // Con trỏ chọn bên dưới thẻ
        if (isSelected)
        {
            FontManager::Draw(cx + cardW / 2 - 76, cardY + cardH + 16, TR(L"> 選択中 (ENTER) <"), GetColor(255, 230, 80), FontSize::Medium);
        }
    }

    // 4. Danh sách các thẻ đã có ở góc trên bên phải
    if (!activeCards_.empty())
    {
        int listX = sw - 320;
        int listY = 30;
        FontManager::Draw(listX, listY, TR(L"=== 所持カード ==="), GetColor(255, 220, 80), FontSize::Body);
        listY += 24;
        for (const auto& c : activeCards_)
        {
            int col = static_cast<int>(CardFrame::GetStyle(c.rarity).edge); // cùng màu với khung thẻ
            FontManager::DrawFormat(listX, listY, col, FontSize::Small, L"- %s", TR(c.name.c_str()));
            listY += 20;
        }
    }
}

#if defined(_DEBUG) || !defined(NDEBUG)
void CardManager::RunSelfTest()
{
    const auto& allCards = CardDatabase::GetAllCards();
    if (allCards.size() != 8)
    {
        AppLogAdd(L"[CARD_TEST] FAILED: CardDatabase size is %zu, expected 8!\n", allCards.size());
        return;
    }

    int totalTests = 0;
    int passedTests = 0;

    // Rút 100 lần ở mỗi mức số thẻ đã lấy (0 đến 4)
    for (int ownedCount = 0; ownedCount <= 4; ++ownedCount)
    {
        for (int run = 0; run < 100; ++run)
        {
            activeCards_.clear();
            std::vector<int> indices = { 0, 1, 2, 3, 4, 5, 6, 7 };
            for (int s = 0; s < ownedCount; ++s)
            {
                int swapIdx = s + (rand() % (8 - s));
                std::swap(indices[s], indices[swapIdx]);
                activeCards_.push_back(allCards[indices[s]]);
            }

            int wave = 1 + (run % 5);
            Roll3Cards(wave);
            totalTests++;

            // Xác nhận không bao giờ trùng thẻ trong một lượt và không thẻ rỗng
            bool dup01 = (currentOffer_[0].id == currentOffer_[1].id);
            bool dup12 = (currentOffer_[1].id == currentOffer_[2].id);
            bool dup02 = (currentOffer_[0].id == currentOffer_[2].id);
            bool hasEmpty = currentOffer_[0].id.empty() || currentOffer_[1].id.empty() || currentOffer_[2].id.empty();
            bool hasOwned = HasCard(currentOffer_[0].id) || HasCard(currentOffer_[1].id) || HasCard(currentOffer_[2].id);

            if (dup01 || dup12 || dup02 || hasEmpty || hasOwned)
            {
                AppLogAdd(L"[CARD_TEST] FAILED at ownedCount=%d run=%d: [%s, %s, %s] (dup: %d,%d,%d, empty: %d, owned: %d)\n",
                    ownedCount, run,
                    std::wstring(currentOffer_[0].id.begin(), currentOffer_[0].id.end()).c_str(),
                    std::wstring(currentOffer_[1].id.begin(), currentOffer_[1].id.end()).c_str(),
                    std::wstring(currentOffer_[2].id.begin(), currentOffer_[2].id.end()).c_str(),
                    dup01, dup12, dup02, hasEmpty, hasOwned);
                activeCards_.clear();
                return;
            }
            passedTests++;
        }
    }

    activeCards_.clear();
    selectedIndex_ = 0;
    AppLogAdd(L"[CARD_TEST] PASSED: All %d self-test rolls (0..4 owned cards, 100 runs each) verified unique, non-empty, and valid pool!\n", passedTests);
}
#endif
