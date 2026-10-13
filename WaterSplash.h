#pragma once
#include <DxLib.h>
#include <array>
#include <cstddef>
#include "Vector3.h"

// =======================================================================
// WaterSplash.h
// Hệ thống hiệu ứng nước tung tóe (Water Splash & Ripples Particle System)
// Chuyển hóa và tối ưu hóa từ 3D Particle System Demo sang thế giới game 3D.
// Đảm bảo ZERO DYNAMIC ALLOCATION trong vòng lặp 120Hz (Fixed Object Pool).
// =======================================================================

// Hạt giọt nước 3D văng parabol
struct WaterDropParticle
{
    Vec3 pos{};
    Vec3 vel{};
    float age = 0.0f;
    float lifetime = 0.70f;
    float sizeStart = 5.0f;
    float sizeEnd = 2.0f;
    float gravity = 280.0f;
    float surfaceY = 0.1f;
    int r = 160, g = 225, b = 255;
    bool active = false;
};

// Vòng sóng nước nở lan tỏa trên mặt sàn XZ
struct WaterRipple
{
    Vec3 center{};
    float age = 0.0f;
    float lifetime = 0.85f;
    float startRadius = 5.0f;
    float endRadius = 65.0f;
    float opacity = 0.80f;
    bool active = false;
};

// Vành nước 3D hình vương miện dựng đứng (Splash Crown)
struct WaterCrown
{
    Vec3 center{};
    float age = 0.0f;
    float lifetime = 0.38f;
    float startRadius = 8.0f;
    float endRadius = 55.0f;
    float maxHeight = 24.0f;
    float strength = 1.0f;
    bool active = false;
};

// Hạt bọt nước dạt ngang mặt sàn
struct WaterFoamParticle
{
    Vec3 pos{};
    Vec3 vel{};
    float age = 0.0f;
    float lifetime = 0.60f;
    float sizeStart = 4.5f;
    float sizeEnd = 1.0f;
    int r = 210, g = 242, b = 255;
    bool active = false;
};

class WaterSplashSystem
{
public:
    WaterSplashSystem() = default;
    ~WaterSplashSystem();

    // Khởi tạo texture giọt nước procedurally và object pool
    void Init();
    // Đặt lại toàn bộ hạt về trạng thái tắt
    void Reset();
    // Giải phóng texture thủ công
    void Shutdown();

    // Kích hoạt cú tiếp đất va chạm mặt nước (cá đập đất / slam)
    void EmitImpact(const Vec3& origin, float radius, float strength = 1.0f);

    // Cập nhật vật lý hạt mỗi tick logic
    void Update(float dt);

    // Kết xuất đồ họa 3D
    void Draw() const;

private:
    static constexpr std::size_t MAX_DROPS = 180;
    static constexpr std::size_t MAX_RIPPLES = 48;
    static constexpr std::size_t MAX_CROWNS = 8;
    static constexpr std::size_t MAX_FOAMS = 64;

    std::array<WaterDropParticle, MAX_DROPS> drops_{};
    std::array<WaterRipple, MAX_RIPPLES> ripples_{};
    std::array<WaterCrown, MAX_CROWNS> crowns_{};
    std::array<WaterFoamParticle, MAX_FOAMS> foams_{};

    int dropTexture_ = -1;
    int glowTexture_ = -1;

    void SpawnSecondaryRipple(const Vec3& pos);
};
