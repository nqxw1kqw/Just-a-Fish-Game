#pragma once
#include <DxLib.h>
#include <array>
#include <cstddef>
#include "Vector3.h"

// =======================================================================
// TsunamiWaterEffect.h
// Hiệu ứng nước cho kỹ năng Radial Tsunami của WhaleBoss.
// Chuyển từ RadialOceanWaveEffect (3D-Particle-System-Demo, scale game/60)
// về đơn vị game theo GameConsts. Chỉ là hình ảnh: va chạm / damage / timing
// vẫn do WhaleBoss quản lý. Pool hạt riêng, không cấp phát động trong frame.
// =======================================================================

// Trạng thái 1 vòng sóng (ánh xạ từ TsunamiRing của boss)
struct TsunamiWaveState
{
    bool  active = false;
    Vec3  center{ 0.0f, 0.0f, 0.0f };
    float radius = 0.0f;      // bán kính tâm vành sóng
    float thickness = 0.0f;
    float speed = 0.0f;
    float gapCenterRad = 0.0f; // hệ góc atan2(x, z)
    float gapHalfRad = 0.0f;   // 0 = không có khe
};

struct TsunamiParticle
{
    Vec3 pos{};
    Vec3 vel{};
    float age = 0.0f;
    float lifetime = 0.5f;
    float sizeStart = 5.0f;
    float sizeEnd = 5.0f;
    float gravity = 0.0f;
    float opacity = 0.8f;
    float fadeIn = 0.0f;
    int r = 230, g = 245, b = 255;
    bool active = false;
};

class TsunamiWaterEffect
{
public:
    static constexpr int MAX_WAVES = 2;

    TsunamiWaterEffect() = default;
    ~TsunamiWaterEffect();

    void Reset();
    void Shutdown();

    // Gọi mỗi tick logic. charging = boss đang gồng sóng thần.
    void Update(float dt, bool charging, const Vec3& bossPos,
                const std::array<TsunamiWaveState, MAX_WAVES>& waves);
    // Gọi đúng lúc phóng vòng sóng (72 tia nước)
    void OnLaunch(const TsunamiWaveState& wave);

    void Draw(bool charging, float chargeRatio, const Vec3& bossPos,
              const std::array<TsunamiWaveState, MAX_WAVES>& waves) const;

    std::size_t ActiveParticleCount() const;

private:
    static constexpr std::size_t MAX_PARTICLES = 2048;
    std::array<TsunamiParticle, MAX_PARTICLES> particles_{};
    std::size_t nextSlot_ = 0;
    std::array<float, MAX_WAVES> sprayTimer_{};
    float suckTimer_ = 0.0f;
    float chargeAge_ = 0.0f;
    float moundAge_ = 10.0f;   // thời gian từ lúc phóng (mound xẹp xuống)
    Vec3 moundCenter_{ 0.0f, 0.0f, 0.0f };
    int glowTexture_ = -1;

    void EnsureTexture();
    void Add(const TsunamiParticle& p);
};
