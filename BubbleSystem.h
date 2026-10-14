#pragma once
#include <DxLib.h>
#include <array>
#include <cstddef>
#include "Vector3.h"

// =======================================================================
// BubbleSystem.h
// Task #17 泡パーティクル: bọt khí nổi lên từ sàn đấu (môi trường dưới nước).
// - Các "khe" cố định trên sàn phun từng chùm bọt + bọt lẻ rải rác khắp sàn.
// - Bọt nổi lên, lắc lư nhẹ, phình dần; vỡ khi chạm mặt nước hoặc hết tuổi.
// Pool cố định, không cấp phát động trong vòng lặp.
// =======================================================================

struct Bubble
{
    Vec3 base{};          // vị trí gốc (chưa cộng lắc lư)
    float riseSpeed = 40.0f;
    float age = 0.0f;
    float lifetime = 5.0f;
    float size = 4.0f;
    float wobbleAmp = 4.0f;
    float wobbleFreq = 2.0f;
    float wobblePhase = 0.0f;
    float popTimer = -1.0f; // >= 0: đang vỡ
    bool active = false;
};

class BubbleSystem
{
public:
    BubbleSystem() = default;
    ~BubbleSystem();

    void Init(float arenaHalfSize);
    void Reset();
    void Shutdown();
    void Update(float dt);
    void Draw() const;

    // Phát bọt tại vị trí bất kỳ (telegraph của boss...)
    void EmitAt(const Vec3& center, int count, float spread, float sizeMin = 2.5f, float sizeMax = 6.0f);

    std::size_t ActiveCount() const;

private:
    static constexpr std::size_t MAX_BUBBLES = 384;
    static constexpr int VENT_COUNT = 10;
    static constexpr float SURFACE_Y = 320.0f;  // độ cao "mặt nước": bọt vỡ ở đây
    static constexpr float POP_DURATION = 0.12f;

    struct Vent
    {
        Vec3 pos{};
        float timer = 0.0f;   // đếm ngược tới chùm kế tiếp
        int pending = 0;      // số bọt còn lại trong chùm
        float streamTimer = 0.0f;
    };

    void Spawn(const Vec3& pos, float sizeMin, float sizeMax);

    std::array<Bubble, MAX_BUBBLES> bubbles_{};
    std::array<Vent, VENT_COUNT> vents_{};
    std::size_t nextSlot_ = 0;
    float ambientTimer_ = 0.0f;
    float arenaHalf_ = 700.0f;
    int texture_ = -1;
};
