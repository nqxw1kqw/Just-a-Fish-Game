#pragma once
#include "Vector3.h"
#include "DxLib.h"
#include <vector>
#include <array>
#include <memory>

// Vòng sóng nước nở trên mặt sàn
struct Shockwave
{
    Vec3 center;
    float currentRadius = 0.0f;
    float maxRadius = 130.0f;
    float life = 0.0f;
    float maxLife = 0.35f;
    bool active = false;
};


// Hạt ngọc kinh nghiệm / tiền
struct ExpOrb
{
    Vec3 pos;
    Vec3 vel;
    float value = 1.0f;
    float life = 0.0f;
    float spin = 0.0f;        // pha xoay riêng (không xoay đồng loạt)
    float magnetT = -1.0f;    // < 0: chưa bị hút; >= 0: thời gian đã bị hút (tăng tốc dần)
    float curveSign = 1.0f;   // hướng cong khi bay vào người chơi
    float absorbT = -1.0f;    // >= 0: đang bị hấp thụ (thu nhỏ + chớp) rồi biến mất
    bool landed = false;
    bool active = false;
};

// Số sát thương nổi 3D chiếu lên màn hình 2D
struct DamagePopup
{
    Vec3 pos;
    Vec3 vel;
    float damage = 0.0f;
    float life = 0.0f;
    float maxLife = 0.70f;
    bool isCrit = false;
    bool active = false;
};

#include "WaterSplash.h"
#include "BubbleSystem.h"
#include "NamedFx.h"

struct HitFxSet; // Effects/Hit/HitFeedback.h

class EffectSystem
{
public:
    EffectSystem();
    ~EffectSystem();
    EffectSystem(const EffectSystem&) = delete;
    EffectSystem& operator=(const EffectSystem&) = delete;
    // Phản hồi trúng đòn (Fuyuu): tia lửa + bọt phụt tại điểm trúng; nổ bọt/vảy khi cá chết
    void SpawnHitFeedback(const Vec3& pos, const Vec3& dir, bool crit);
    void SpawnFishDeath(const Vec3& pos, float size);
    void SpawnPlayerHurt(const Vec3& pos, const Vec3& dir, float amount); // người chơi trúng đòn: máu (Fuyuu), amount 0.25–3
    void Init();
    void Reset();
    void Shutdown();
    void Update(float dt, const Vec3& playerPos);
    void Draw() const;
    void DrawDamagePopups() const;

    void SpawnShockwave(const Vec3& pos, float maxRadius = 130.0f);
    void SpawnWaterSplash(const Vec3& pos, float radius, float strength = 1.0f);
    void SpawnBubbles(const Vec3& center, int count, float spread, float sizeMin = 2.5f, float sizeMax = 6.0f);
    void SpawnSwimWake(const Vec3& fishPos, const Vec3& moveDir);
    void SpawnFishSwim(const Vec3& pos, const Vec3& dir, float speed, float sizeMul = 1.0f, bool isDashing = false);
    void SpawnTurtlePaddle(const Vec3& pos, const Vec3& dir, float radius);
    void SpawnFrogHop(const Vec3& pos, const Vec3& dir, float radius);
    void SpawnFrogLand(const Vec3& pos, float radius);
    void SpawnCrabScuttle(const Vec3& pos, const Vec3& moveDir, const Vec3& facingDir, float radius);
    void DrawShallowOcean(float arenaHalfSize) const;
    // Hiệu ứng theo tên (thay hạt khối lập phương cũ). No-op nếu hiệu ứng chưa được đăng ký.
    void PlayFx(FxId id, const Vec3& pos, const Vec3& dir = Vec3{ 0, 1, 0 }, float scale = 1.0f);
    void RegisterFx(FxId id, NamedFx::Handler handler);
    bool HasFx(FxId id) const;
    void SetParticleScale(float s) { particleScale_ = s; } // Hell: giảm hạt khi đông quái
    float particleScale_ = 1.0f;
    void SpawnExpOrb(const Vec3& pos, float val = 1.0f);
    void SpawnDamagePopup(const Vec3& pos, float damage, bool isCrit = false);
    void AddTrauma(float amount);

    Vec3 GetCameraShakeOffset(float shakeScale = 1.0f) const;
    int CollectExpOrbs(const Vec3& playerPos, float pickupRadius);

private:
    std::unique_ptr<HitFxSet> hitFx_;
    static constexpr size_t MAX_DAMAGE_POPUPS = 128;

    std::array<Shockwave, 32> shockwaves_{};
    void RegisterNamedFx(); // nơi đăng ký hiệu ứng Fuyuu theo tên
    std::array<NamedFx::Handler, static_cast<size_t>(FxId::Count)> namedFx_{};
    mutable std::array<bool, static_cast<size_t>(FxId::Count)> namedFxWarned_{};
    std::array<ExpOrb, 300> expOrbs_{};
    void UpdateExpOrbs(float dt, const Vec3& playerPos);
    void DrawExpOrbs() const;
    Vec3 expTarget_{};             // vị trí người chơi (điểm hấp thụ)
    mutable int expGlowTex_ = -1;  // texture quầng sáng mềm (tạo lười)
    std::array<DamagePopup, MAX_DAMAGE_POPUPS> damagePopups_{};

    WaterSplashSystem waterSplash_;
    BubbleSystem bubbles_; // #17 泡パーティクル: bọt nổi từ sàn

    float trauma_ = 0.0f; // Chấn động camera [0, 1]
    float shakeTime_ = 0.0f;
};
