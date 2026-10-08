#pragma once
#include "Card.h"
#include "DxLib.h"
#include <vector>
#include <string>

class CardDatabase
{
public:
    static std::vector<Card>& GetAllCards()
    {
        static std::vector<Card> cards = {
            // 1. Deep Trench (Common, modifier)
            {
                "vucsau",
                L"Deep Trench",
                L"+ Slam Radius +25%",
                L"",
                CardRarity::Common,
                false,
                3, // maxStack = 3 (Common)
                {
                    { "slamRadius", ModifierType::Percent, 0.25f }
                },
                HookType::None
            },
            // 2. Heavy Slam (Common, modifier)
            {
                "cudam",
                L"Heavy Slam",
                L"+ Slam Damage +40%",
                L"- Jump Cooldown +15%",
                CardRarity::Common,
                false,
                3, // maxStack = 3 (Common)
                {
                    { "slamDamage", ModifierType::Percent, 0.40f },
                    { "jumpCooldown", ModifierType::Percent, 0.15f }
                },
                HookType::None
            },
            // 3. Nimble Leap (Common, modifier)
            {
                "nhaygon",
                L"Nimble Leap",
                L"+ Jump Cooldown -20%",
                L"- Slam Damage -10%",
                CardRarity::Common,
                false,
                3, // maxStack = 3 (Common)
                {
                    { "jumpCooldown", ModifierType::Percent, -0.20f },
                    { "slamDamage", ModifierType::Percent, -0.10f }
                },
                HookType::None
            },
            // 4. Swift Fins (Common, modifier)
            {
                "vaynhanh",
                L"Swift Fins",
                L"+ Swim Speed +15%",
                L"",
                CardRarity::Common,
                false,
                3, // maxStack = 3 (Common)
                {
                    { "moveSpeed", ModifierType::Percent, 0.15f }
                },
                HookType::None
            },
            // 5. Thick Scales (Common, modifier)
            {
                "daday",
                L"Thick Scales",
                L"+ Max HP +30%",
                L"- Swim Speed -8%",
                CardRarity::Common,
                false,
                3, // maxStack = 3 (Common)
                {
                    { "maxHp", ModifierType::Percent, 0.30f },
                    { "moveSpeed", ModifierType::Percent, -0.08f }
                },
                HookType::None
            },
            // 6. Echo Wave (Rare, modifier)
            {
                "songdoi",
                L"Echo Wave",
                L"+ Slam Radius +35%, Push +30%",
                L"- Slam Damage -10%",
                CardRarity::Rare,
                false,
                2, // maxStack = 2 (Rare)
                {
                    { "slamRadius", ModifierType::Percent, 0.35f },
                    { "slamKnockback", ModifierType::Percent, 0.30f },
                    { "slamDamage", ModifierType::Percent, -0.10f }
                },
                HookType::None
            },
            // 7. Puffer Needles (Rare, hook OnHit taken)
            {
                "canoc",
                L"Puffer Needles",
                L"+ Counter 8 Spikes on Hit",
                L"- Max HP -15%",
                CardRarity::Rare,
                false,
                1, // maxStack = 1 (Hook bool chỉ 1 lần)
                {
                    { "maxHp", ModifierType::Percent, -0.15f }
                },
                HookType::OnHitTaken
            },
            // 8. Crushing Impact (Epic, modifier)
            {
                "cudapnang",
                L"Crushing Impact",
                L"+ Knockback +100%, DMG +30%",
                L"- Jump Cooldown +20%",
                CardRarity::Epic,
                false,
                1, // maxStack = 1 (Epic chỉ 1 lần)
                {
                    { "slamKnockback", ModifierType::Percent, 1.00f },
                    { "slamDamage", ModifierType::Percent, 0.30f },
                    { "jumpCooldown", ModifierType::Percent, 0.20f }
                },
                HookType::None
            }
        };
        return cards;
    }

    // Nạp toàn bộ ảnh thẻ đúng một lần lúc khởi tạo (T2)
    static void LoadImages()
    {
        auto& cards = GetAllCards();
        for (auto& card : cards)
        {
            if (card.imageHandle != -1) continue;

            // 1. Thử load đường dẫn tương đối chuẩn <id>.png
            std::wstring pathId = L"Data/UI/Card/" + std::wstring(card.id.begin(), card.id.end()) + L".png";
            card.imageHandle = LoadGraph(pathId.c_str());

            // 2. Nếu chưa được, thử fallback sang tên file tiếng Anh gốc
            if (card.imageHandle == -1)
            {
                const wchar_t* fallbackPath = nullptr;
                if (card.id == "vucsau") fallbackPath = L"Data/UI/Card/Deep Trench.png";
                else if (card.id == "cudam") fallbackPath = L"Data/UI/Card/Heavy Slam.png";
                else if (card.id == "nhaygon") fallbackPath = L"Data/UI/Card/Nimble Leap.png";
                else if (card.id == "vaynhanh") fallbackPath = L"Data/UI/Card/Swift Fins.png";
                else if (card.id == "daday") fallbackPath = L"Data/UI/Card/Thick Scales.png";
                else if (card.id == "songdoi") fallbackPath = L"Data/UI/Card/Crushing Impact.png";
                else if (card.id == "canoc") fallbackPath = L"Data/UI/Card/Puffer Needles.png";
                else if (card.id == "cudapnang") fallbackPath = L"Data/UI/Card/Waterfall Carp.png";

                if (fallbackPath != nullptr)
                {
                    card.imageHandle = LoadGraph(fallbackPath);
                }
            }

            // 3. Fallback: Nếu không tìm thấy, in cảnh báo ra Log, KHÔNG crash!
            if (card.imageHandle == -1)
            {
                AppLogAdd(L"[CARD_IMAGE] Cảnh báo: Không tìm thấy ảnh thẻ: %s (sẽ dùng ô màu thay thế)\n",
                    std::wstring(card.id.begin(), card.id.end()).c_str());
            }
            else
            {
                AppLogAdd(L"[CARD_IMAGE] Nạp thành công ảnh thẻ: %s (handle=%d)\n",
                    std::wstring(card.id.begin(), card.id.end()).c_str(), card.imageHandle);
            }
        }
    }

    // Giải phóng toàn bộ ảnh thẻ khi thoát game (T2)
    static void ReleaseImages()
    {
        auto& cards = GetAllCards();
        for (auto& card : cards)
        {
            if (card.imageHandle != -1)
            {
                DeleteGraph(card.imageHandle);
                card.imageHandle = -1;
            }
        }
        AppLogAdd(L"[CARD_IMAGE] Đã giải phóng toàn bộ ảnh thẻ thành công.\n");
    }
};
