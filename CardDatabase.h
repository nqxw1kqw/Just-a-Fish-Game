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
                L"深海の溝",
                L"+ スラム範囲 +25%",
                L"",
                CardRarity::Common,
                false,
                6, // maxStack (Endless), trần MAX_SLAM_RADIUS
                {
                    { "slamRadius", ModifierType::Percent, 0.25f }
                },
                HookType::None
            },
            // 2. Heavy Slam (Common, modifier)
            {
                "cudam",
                L"ヘビースラム",
                L"+ スラムダメージ +40%",
                L"",
                CardRarity::Common,
                false,
                6, // maxStack (Endless)
                {
                    { "slamDamage", ModifierType::Percent, 0.40f }
                },
                HookType::None
            },
            // 3. Nimble Leap (Common, modifier)
            {
                "nhaygon",
                L"身軽なジャンプ",
                L"+ ジャンプ待ち時間 x0.8",
                L"",
                CardRarity::Common,
                false,
                6, // maxStack (Endless) - nhân x0.8 mỗi lần, sàn MIN_JUMP_COOLDOWN
                {
                    { "jumpCooldown", ModifierType::More, 0.80f }
                },
                HookType::None
            },
            // 4. Swift Fins (Common, modifier)
            {
                "vaynhanh",
                L"疾風のヒレ",
                L"+ 泳ぐ速さ +15%",
                L"",
                CardRarity::Common,
                false,
                6, // maxStack (Endless), trần MAX_MOVE_SPEED
                {
                    { "moveSpeed", ModifierType::Percent, 0.15f }
                },
                HookType::None
            },
            // 5. Thick Scales (Common, modifier)
            {
                "daday",
                L"分厚いウロコ",
                L"+ 最大体力 +30%",
                L"",
                CardRarity::Common,
                false,
                6, // maxStack (Endless), trần MAX_PLAYER_HP
                {
                    { "maxHp", ModifierType::Percent, 0.30f }
                },
                HookType::None
            },
            // 6. Echo Wave (Rare, modifier)
            {
                "songdoi",
                L"エコーウェーブ",
                L"+ スラム範囲 +35%、押し出し +30%",
                L"",
                CardRarity::Rare,
                false,
                4, // maxStack (Endless), trần MAX_SLAM_RADIUS / MAX_SLAM_KNOCKBACK
                {
                    { "slamRadius", ModifierType::Percent, 0.35f },
                    { "slamKnockback", ModifierType::Percent, 0.30f }
                },
                HookType::None
            },
            // 7. Puffer Needles (Rare, hook OnHit taken)
            {
                "canoc",
                L"フグの針",
                L"+ 被弾時にトゲ8本で反撃",
                L"",
                CardRarity::Rare,
                false,
                1, // maxStack = 1 (Hook bool chỉ 1 lần)
                {
                },
                HookType::OnHitTaken
            },
            // 8. Crushing Impact (Epic, modifier)
            {
                "cudapnang",
                L"粉砕インパクト",
                L"+ ノックバック +100%、ダメージ +30%",
                L"",
                CardRarity::Epic,
                false,
                3, // maxStack (Endless), trần MAX_SLAM_DAMAGE / MAX_SLAM_KNOCKBACK
                {
                    { "slamKnockback", ModifierType::Percent, 1.00f },
                    { "slamDamage", ModifierType::Percent, 0.30f }
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
