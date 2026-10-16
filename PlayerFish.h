#pragma once
#include "Vector3.h"
#include "Consts.h"
#include "Stats.h"
#include <algorithm>

class EffectSystem;

class PlayerFish
{
public:
    void Init();
    void Reset();
    void Release();
    ~PlayerFish() { Release(); }
    void Update(float dt, EffectSystem& effects);
    void Draw() const;

    // Quản lý mô hình 3D Cá Chính (fish.mv1)
    void SetFishModelScale(float s) { fishModelScale_ = s; }
    float GetFishModelScale() const { return fishModelScale_; }
    void SetFishModelRotYDeg(float deg) { fishModelRotYDeg_ = deg; }
    float GetFishModelRotYDeg() const { return fishModelRotYDeg_; }
    void SetFishModelOffsetY(float y) { fishModelOffsetY_ = y; }
    float GetFishModelOffsetY() const { return fishModelOffsetY_; }

    // Áp dụng chỉ số từ hệ thống Stats
    void ApplyStats(const PlayerStats& stats);
    void SetHooks(bool hasPufferfish, bool hasBulletRain, bool hasContinuousBounce);

    // Va chạm & Nhận sát thương (J4: Bất tử khi ở trên không)
    // sourcePos: nguồn sát thương (đẩy lùi ra xa). nullptr = không đẩy lùi thêm
    bool TakeDamage(float dmg, EffectSystem& effects, const Vec3* sourcePos = nullptr);
    void Heal(float amount) { hp_ = (hp_ + amount > maxHp_) ? maxHp_ : (hp_ + amount); }
    void ApplyKnockback(const Vec3& force) { knockback_ += force; }
    // Boss rework PR C: làm chậm (lấy hệ số nhỏ nhất, gia hạn thời gian) và lực kéo ngoài (áp 1 khung rồi xoá)
    void ApplySlow(float factor, float duration)
    {
        if (slowTimer_ <= 0.0f || factor < slowFactor_) slowFactor_ = factor;
        slowTimer_ = (std::max)(slowTimer_, duration);
    }
    bool IsSlowed() const { return slowTimer_ > 0.0f; }
    void AddExternalPull(const Vec3& velocity) { externalPull_ += velocity; }
    // Shark Hunt: bị ngoạm (bỏ qua i-frame chỉ cho đòn này; nhảy thì vẫn né được), khoá input, rồi bị ném
    bool Grab(float dmg, EffectSystem& effects);
    void SetGrabPose(const Vec3& p, float yaw) { grabPos_ = p; grabYaw_ = yaw; }
    void ReleaseThrow(const Vec3& vel, float iframes);
    bool IsGrabbed() const { return grabbed_; }
    bool IsInvulnerable() const { return invulnerableTimer_ > 0.0f; }
    float GetDamageTakenThisWave() const { return damageTakenThisWave_; }
    void ResetDamageTakenThisWave() { damageTakenThisWave_ = 0.0f; }

    // Hỗ trợ Debug & Quy tắc công bằng (Phần D & Phần A5)
    void SetGodMode(bool enable) { isGodMode_ = enable; }
    bool IsGodMode() const { return isGodMode_; }
    void SetDamageTakenMul(float mul) { damageTakenMul_ = mul; } // Hell: x1.5
    void ResetJumpCooldown() { jumpCooldownTimer_ = 0.0f; }
    void FullHealAndResetJumpCooldown()
    {
        hp_ = maxHp_;
        jumpCooldownTimer_ = 0.0f;
        landRecoveryTimer_ = 0.0f;
    }
    void Kill() { hp_ = 0.0f; }

    // Thu thập EXP
    void AddExp(int amount);
    int ConsumeLevelUps() { int n = pendingLevelUps_; pendingLevelUps_ = 0; return n; }

    // Getters & Trạng thái Lõi nhảy
    const Vec3& GetPosition() const { return pos_; }
    float GetGroundHeight() const { return 0.0f; }
    float GetHeightY() const { return pos_.y; }

    // Render interpolation (#26): lưu transform tick trước, đổi sang transform nội suy khi vẽ
    void SavePrevTransform() { prevPos_ = pos_; prevYaw_ = yaw_; }
    void BeginRenderInterp(float alpha);
    void EndRenderInterp() { pos_ = savedPos_; yaw_ = savedYaw_; }
    float GetBouncePhase() const { return jumpPhase_; }
    float GetJumpPhase() const { return jumpPhase_; }
    bool IsAirborne() const { return isJumping_ || pos_.y > 0.01f; }
    bool IsApexInvincible() const { return IsAirborne(); } // J4: Bất tử toàn bộ thời gian trên không
    bool IsBouncing() const { return isJumping_; }
    bool IsJumping() const { return isJumping_; }
    bool CanJump() const { return !isJumping_ && jumpCooldownTimer_ <= 0.0f; }
    float GetJumpCooldownTimer() const { return jumpCooldownTimer_; }
    float GetJumpCooldownMax() const { return jumpCooldown_; }
    float GetLandRecoveryTimer() const { return landRecoveryTimer_; }
    const Vec3& GetPredictedLandPos() const { return predictedLandPos_; }
    float GetYaw() const { return yaw_; }

    float GetHp() const { return hp_; }
    float GetMaxHp() const { return maxHp_; }
    int GetExp() const { return exp_; }
    int GetExpNext() const { return expNext_; }
    int GetLevel() const { return level_; }

    float GetHitboxRadius() const { return 18.0f; }

    bool ConsumeLandEvent()
    {
        bool landed = justLanded_;
        justLanded_ = false;
        return landed;
    }

    bool ConsumeBulletRainTrigger()
    {
        bool b = triggerBulletRain_;
        triggerBulletRain_ = false;
        return b;
    }

    bool ConsumePufferfishTrigger()
    {
        bool p = triggerPufferfishSpikes_;
        triggerPufferfishSpikes_ = false;
        return p;
    }

    bool IsComboBuffActive() const { return bounceComboBuffTimer_ > 0.0f; }
    int GetComboCount() const { return bounceComboCount_; }

    // Sát thương tiếp đất (Slam Attack)
    float GetShockwaveRadius() const { return slamRadius_; }
    float GetShockwaveDamage() const { return slamDamage_ * (IsComboBuffActive() ? 1.5f : 1.0f); }
    float GetSlamRadius() const { return slamRadius_; }
    float GetSlamDamage() const { return slamDamage_ * (IsComboBuffActive() ? 1.5f : 1.0f); }
    float GetSlamKnockback() const { return slamKnockback_; }

private:
    void UpdateCore(float dt, EffectSystem& effects);
    void StartJump(const Vec3& inputDir);
    void OnLand(EffectSystem& effects);

    Vec3 pos_{ 0.0f, 0.0f, 0.0f };
    Vec3 moveDir_{ 0.0f, 0.0f, 1.0f };
    Vec3 airVel_{ 0.0f, 0.0f, 0.0f };
    Vec3 knockback_{ 0.0f, 0.0f, 0.0f };
    bool grabbed_ = false;
    Vec3 grabPos_{ 0.0f, 0.0f, 0.0f };
    float grabYaw_ = 0.0f;
    Vec3 predictedLandPos_{ 0.0f, 0.0f, 0.0f };
    float yaw_ = 0.0f;
    Vec3 prevPos_{ 0.0f, 0.0f, 0.0f };
    float prevYaw_ = 0.0f;
    Vec3 savedPos_{ 0.0f, 0.0f, 0.0f };
    float savedYaw_ = 0.0f;
    float swimTimer_ = 0.0f;
    float invulnerableTimer_ = 0.0f;
    bool isGodMode_ = false;
    float damageTakenMul_ = 1.0f;
    float damageTakenThisWave_ = 0.0f;

    // Cơ chế nhảy & tấn công (J1 -> J5)
    bool isJumping_ = false;
    bool justLanded_ = false;
    float jumpPhase_ = 0.0f; // t từ 0.0f đến 1.0f
    float jumpDuration_ = GameConsts::JUMP_DURATION;
    float jumpHeight_ = GameConsts::JUMP_HEIGHT;
    float jumpCooldown_ = GameConsts::JUMP_COOLDOWN;
    float jumpCooldownTimer_ = 0.0f;
    float landRecoveryDuration_ = GameConsts::LAND_RECOVERY;
    float landRecoveryTimer_ = 0.0f;
    float speed_ = GameConsts::FISH_SPEED;
    float airSpeedMul_ = GameConsts::AIR_SPEED_MUL;
    int extraJumps_ = 0;
    int remainingJumps_ = 0;

    // Sát thương tiếp đất (Slam)
    float slamDamage_ = GameConsts::SLAM_DAMAGE;
    float slamRadius_ = GameConsts::SLAM_RADIUS;
    float slamKnockback_ = GameConsts::SLAM_KNOCKBACK;

    // Hooks & Buffs
    bool hasPufferfishHook_ = false;
    bool hasBulletRainHook_ = false;
    bool hasContinuousBounceHook_ = false;
    bool triggerPufferfishSpikes_ = false;
    bool triggerBulletRain_ = false;
    int bounceComboCount_ = 0;
    float bounceComboBuffTimer_ = 0.0f;

    // Chỉ số RPG
    float hp_ = 100.0f;
    float maxHp_ = 100.0f;
    int exp_ = 0;
    int expNext_ = 30; // 20 + 10 * 1
    int level_ = 1;
    int pendingLevelUps_ = 0;

    // Hiệu ứng
    float hitFlashTimer_ = 0.0f;
    float slowTimer_ = 0.0f;
    float slowFactor_ = 1.0f;
    Vec3 externalPull_{ 0.0f, 0.0f, 0.0f };
    float swimSoundTimer_ = 0.0f;
    float swimWakeTimer_ = 0.0f;

    // Mô hình 3D Cá Chính
    int fishModelHandle_ = -1;
    int fishTextureHandle_ = -1;
    int fishAnimAttachIndex_ = -1;
    int currentAnimIndex_ = 0;
    float fishAnimTime_ = 0.0f;
    bool fishModelHasTexture_ = false;
    float fishModelScale_ = GameConsts::PLAYER_MODEL_SCALE;
    float fishModelRotYDeg_ = GameConsts::PLAYER_MODEL_ROT_Y_DEG;
    float fishModelOffsetY_ = GameConsts::PLAYER_MODEL_OFFSET_Y;
};
