#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "Effects.h"
#include "DxConv.h"
#include "Render3DUtil.h"
#include "FontManager.h"
#include "Consts.h"
#include "CollisionUtil.h"
#include "SoundManager.h"
#include "Effects/Hit/HitFeedback.h"
#include <cwchar>
#include <utility>

namespace
{
    float RandomFloat(float minVal, float maxVal)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + r * (maxVal - minVal);
    }
}

EffectSystem::EffectSystem() : hitFx_(std::make_unique<HitFxSet>()) { RegisterNamedFx(); }
EffectSystem::~EffectSystem() = default;

void EffectSystem::SpawnHitFeedback(const Vec3& pos, const Vec3& dir, bool crit)
{
    if (!hitFx_) return;
    VECTOR p = DxConv::ToVECTOR(pos);
    VECTOR d = DxConv::ToVECTOR(dir);
    // Quái/Boss trúng đòn: EnemyHitEffect (Fuyuu); tia lửa streak chỉ thêm cho đòn crit để không rối
    hitFx_->enemyHit.Emit(p, d, crit);
    if (crit) hitFx_->spark.Emit(p, d, true);
    // Âm thanh trúng đòn (SoundManager tự giới hạn tần suất khi hạ hàng loạt)
    SoundManager::Instance().PlaySFX(crit ? SfxId::EnemyHitCrit : SfxId::EnemyHit);
}

void EffectSystem::SpawnPlayerHurt(const Vec3& pos, const Vec3& dir, float amount)
{
    if (hitFx_) hitFx_->playerHurt.Emit(DxConv::ToVECTOR(pos), DxConv::ToVECTOR(dir), amount);
}

void EffectSystem::SpawnFishDeath(const Vec3& pos, float size)
{
    if (hitFx_) hitFx_->death.Emit(DxConv::ToVECTOR(pos), size);
}

void EffectSystem::Init()
{
    waterSplash_.Init();
    bubbles_.Init(GameConsts::ARENA_HALF_SIZE);
    Reset();
}

void EffectSystem::Reset()
{
    for (auto& s : shockwaves_) s.active = false;
    for (auto& e : expOrbs_) e.active = false;
    for (auto& dp : damagePopups_) dp.active = false;
    waterSplash_.Reset();
    bubbles_.Reset();
    if (hitFx_) hitFx_->Clear();
    trauma_ = 0.0f;
    shakeTime_ = 0.0f;
}

void EffectSystem::Shutdown()
{
    waterSplash_.Shutdown();
    bubbles_.Shutdown();
}

void EffectSystem::SpawnWaterSplash(const Vec3& pos, float radius, float strength)
{
    waterSplash_.EmitImpact(pos, radius, strength);
}

void EffectSystem::SpawnSwimWake(const Vec3& fishPos, const Vec3& moveDir)
{
    waterSplash_.EmitSwimWake(fishPos, moveDir);
}

void EffectSystem::SpawnFishSwim(const Vec3& pos, const Vec3& dir, float speed, float sizeMul, bool isDashing)
{
    waterSplash_.EmitFishSwim(pos, dir, speed, sizeMul, isDashing);
}

void EffectSystem::SpawnTurtlePaddle(const Vec3& pos, const Vec3& dir, float radius)
{
    waterSplash_.EmitTurtlePaddle(pos, dir, radius);
}

void EffectSystem::SpawnFrogHop(const Vec3& pos, const Vec3& dir, float radius)
{
    waterSplash_.EmitFrogHop(pos, dir, radius);
}

void EffectSystem::SpawnFrogLand(const Vec3& pos, float radius)
{
    waterSplash_.EmitFrogLand(pos, radius);
}

void EffectSystem::SpawnCrabScuttle(const Vec3& pos, const Vec3& moveDir, const Vec3& facingDir, float radius)
{
    waterSplash_.EmitCrabScuttle(pos, moveDir, facingDir, radius);
}

void EffectSystem::DrawShallowOcean(float arenaHalfSize) const
{
    waterSplash_.DrawShallowOcean(arenaHalfSize);
}

void EffectSystem::SpawnDamagePopup(const Vec3& pos, float damage, bool isCrit)
{
    if (damage <= 0.01f) return;

    for (auto& dp : damagePopups_)
    {
        if (!dp.active)
        {
            // Tọa độ sinh: trên vị trí mục tiêu một chút, tản nhẹ ngẫu nhiên
            dp.pos = pos + Vec3{
                RandomFloat(-5.0f, 5.0f),
                RandomFloat(4.0f, 12.0f),
                RandomFloat(-5.0f, 5.0f)
            };
            // Vận tốc bay: vọt lên cao và trôi nhẹ sang 2 bên
            dp.vel = Vec3{
                RandomFloat(-16.0f, 16.0f),
                isCrit ? RandomFloat(75.0f, 95.0f) : RandomFloat(52.0f, 68.0f),
                RandomFloat(-16.0f, 16.0f)
            };
            dp.damage = damage;
            dp.isCrit = isCrit;
            dp.life = 0.0f;
            dp.maxLife = isCrit ? 0.85f : 0.70f;
            dp.active = true;
            break;
        }
    }
}

void EffectSystem::SpawnShockwave(const Vec3& pos, float maxRadius)
{
    for (auto& s : shockwaves_)
    {
        if (!s.active)
        {
            s.center = pos;
            s.center.y = 1.0f; // Sát mặt đất
            s.currentRadius = 15.0f;
            s.maxRadius = maxRadius;
            s.life = 0.0f;
            s.maxLife = 0.32f;
            s.active = true;
            break;
        }
    }
}

void EffectSystem::RegisterFx(FxId id, NamedFx::Handler handler)
{
    size_t i = static_cast<size_t>(id);
    if (i < namedFx_.size()) namedFx_[i] = std::move(handler);
}

bool EffectSystem::HasFx(FxId id) const
{
    size_t i = static_cast<size_t>(id);
    return i < namedFx_.size() && static_cast<bool>(namedFx_[i]);
}

void EffectSystem::PlayFx(FxId id, const Vec3& pos, const Vec3& dir, float scale)
{
    size_t i = static_cast<size_t>(id);
    if (i >= namedFx_.size()) return;
    if (!namedFx_[i])
    {
        // Chưa có hiệu ứng (đang chờ Fuyuu): bỏ qua, chỉ log 1 lần
        if (!namedFxWarned_[i])
        {
            namedFxWarned_[i] = true;
            wchar_t wname[64] = {};
            const char* n = NamedFx::Name(id);
            for (int k = 0; k < 63 && n[k]; ++k) wname[k] = static_cast<wchar_t>(n[k]);
            AppLogAdd(L"[FX] '%s' chưa đăng ký -> bỏ qua\n", wname);
        }
        return;
    }
    namedFx_[i](DxConv::ToVECTOR(pos), DxConv::ToVECTOR(dir), scale * particleScale_);
}

void EffectSystem::RegisterNamedFx()
{
    // Khi Fuyuu giao hiệu ứng (vd. Effects/Heal/HealFx.h với lớp HealWaveEndFx có Emit/Update/Draw/Clear):
    //   1) thêm thành viên vào HitFxSet (Effects/Hit/HitFeedback.h) + Update/Draw/Clear
    //   2) đăng ký ở đây:
    //      RegisterFx(FxId::HealWaveEnd, [this](VECTOR p, VECTOR d, float k) { hitFx_->healWaveEnd.Emit(p, k); });
    // Hiện chưa có hiệu ứng nào -> mọi PlayFx là no-op.
}

void EffectSystem::SpawnExpOrb(const Vec3& pos, float val)
{
    for (auto& e : expOrbs_)
    {
        if (!e.active)
        {
            e.pos = pos;
            e.pos.y = 6.0f; // Nổi nhẹ trên mặt đất
            e.vel = Vec3{
                RandomFloat(-40.0f, 40.0f),
                RandomFloat(30.0f, 80.0f),
                RandomFloat(-40.0f, 40.0f)
            };
            e.value = val;
            e.life = 0.0f;
            e.active = true;
            break;
        }
    }
}

void EffectSystem::AddTrauma(float amount)
{
    trauma_ = std::min(1.0f, trauma_ + amount);
}

#include "Consts.h"

Vec3 EffectSystem::GetCameraShakeOffset(float shakeScale) const
{
    if (trauma_ <= 0.001f || shakeScale <= 0.001f) return Vec3{ 0, 0, 0 };

    float shake = (trauma_ * trauma_) * shakeScale;
    float maxOffset = GameConsts::MAX_CAM_SHAKE_OFFSET * shake;

    // C5: Rung 2 trục tần số khác nhau kết hợp sóng hài (noise-like)
    float offsetX = std::sin(shakeTime_ * 41.3f) * std::cos(shakeTime_ * 17.9f) * maxOffset;
    float offsetY = std::sin(shakeTime_ * 29.7f) * (maxOffset * 0.45f);
    float offsetZ = std::cos(shakeTime_ * 53.1f) * (maxOffset * 0.45f);

    return Vec3{ offsetX, offsetY, offsetZ };
}

void EffectSystem::Update(float dt, const Vec3& playerPos)
{
    if (hitFx_) hitFx_->Update(dt);
    (void)playerPos;
    shakeTime_ += dt;
    if (trauma_ > 0.0f)
    {
        trauma_ = std::max(0.0f, trauma_ - dt * 2.5f); // Tiêu giảm chấn động
    }

    // Cập nhật hệ thống nước tung tóe (Water Splash Particle System)
    waterSplash_.Update(dt);
    bubbles_.Update(dt);

    // Cập nhật sóng nước
    for (auto& s : shockwaves_)
    {
        if (!s.active) continue;
        s.life += dt;
        float progress = s.life / s.maxLife;
        if (progress >= 1.0f)
        {
            s.active = false;
        }
        else
        {
            s.currentRadius = 15.0f + (s.maxRadius - 15.0f) * progress;
        }
    }


    // 4. Cập nhật số sát thương nổi (Floating Damage Numbers)
    for (auto& dp : damagePopups_)
    {
        if (!dp.active) continue;

        dp.life += dt;
        if (dp.life >= dp.maxLife)
        {
            dp.active = false;
            continue;
        }

        dp.pos += dp.vel * dt;
        dp.vel.y -= 110.0f * dt;
        dp.vel.x *= (1.0f - 2.5f * dt);
        dp.vel.z *= (1.0f - 2.5f * dt);
    }
}

int EffectSystem::CollectExpOrbs(const Vec3& playerPos, float pickupRadius)
{
    int collected = 0;
    float radSq = pickupRadius * pickupRadius;
    for (auto& e : expOrbs_)
    {
        if (!e.active) continue;
        if (Collision::DistSqXZ(playerPos, e.pos) <= radSq) // Xét trên mặt phẳng
        {
            e.active = false;
            collected += static_cast<int>(e.value);
        }
    }
    return collected;
}

void EffectSystem::Draw() const
{
    // Vẽ hiệu ứng nước tung tóe (Water Splash & Ripples Particle System)
    waterSplash_.Draw();
    bubbles_.Draw();

    // Vẽ sóng nước (Shockwave rings)
    for (const auto& s : shockwaves_)
    {
        if (!s.active) continue;
        float progress = s.life / s.maxLife;
        int alpha = static_cast<int>((1.0f - progress) * 220.0f);
        if (alpha <= 0) continue;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        int ringColor = GetColor(100, 220, 255);
        int innerColor = GetColor(60, 160, 255);

        // Vẽ 2 vòng tròn đồng tâm tạo độ dày cho sóng nước
        Render3D::DrawCircle3D(s.center, s.currentRadius, 32, ringColor, false);
        Render3D::DrawCircle3D(s.center, s.currentRadius * 0.85f, 32, innerColor, false);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // Vẽ hạt EXP (Khối ngọc xanh ngọc bích phát sáng xoay)
    int expColor = GetColor(250, 250, 255); // EXP orb trắng
    int expEdge = GetColor(170, 190, 220);
    for (const auto& e : expOrbs_)
    {
        if (!e.active) continue;
        float hoverY = e.pos.y + std::sin(e.life * 8.0f) * 2.5f;
        Vec3 orbPos{ e.pos.x, hoverY, e.pos.z };
        Vec3 sz{ GameConsts::EXP_ORB_SIZE, GameConsts::EXP_ORB_SIZE, GameConsts::EXP_ORB_SIZE };
        VECTOR minP = DxConv::ToVECTOR(orbPos - sz * 0.5f);
        VECTOR maxP = DxConv::ToVECTOR(orbPos + sz * 0.5f);
        Render3D::DrawBox3D(minP, maxP, expColor, TRUE);
        Render3D::DrawBox3D(minP, maxP, expEdge, FALSE);
    }

    // Phản hồi trúng đòn: vẽ sau cùng (trong suốt, không ghi Z). Camera đã đặt trước Effects::Draw
    if (hitFx_) hitFx_->Draw();
}

void EffectSystem::DrawDamagePopups() const
{
    for (const auto& dp : damagePopups_)
    {
        if (!dp.active) continue;

        VECTOR wPos = DxConv::ToVECTOR(dp.pos);
        VECTOR sPos = ConvWorldPosToScreenPos(wPos);

        // Chỉ vẽ nếu điểm nằm phía trước camera (khoảng z [0, 1])
        if (sPos.z < 0.0f || sPos.z > 1.0f) continue;

        int sx = static_cast<int>(sPos.x);
        int sy = static_cast<int>(sPos.y);

        // Loại bỏ nếu vượt ngoài màn hình quá xa
        if (sx < -100 || sx > GameConsts::SCREEN_WIDTH + 100 ||
            sy < -50 || sy > GameConsts::SCREEN_HEIGHT + 50) continue;

        // Tính alpha mờ dần ở 45% thời gian cuối
        float progress = dp.life / dp.maxLife;
        int alpha = 255;
        if (progress > 0.55f)
        {
            alpha = static_cast<int>(255.0f * (1.0f - (progress - 0.55f) / 0.45f));
            alpha = (std::clamp)(alpha, 0, 255);
        }
        if (alpha <= 0) continue;

        int dmgInt = static_cast<int>(std::round(dp.damage));
        if (dmgInt < 1) dmgInt = 1;

        wchar_t textBuf[32];
        if (dp.isCrit)
        {
            swprintf_s(textBuf, L"%d!", dmgInt);
        }
        else
        {
            swprintf_s(textBuf, L"%d", dmgInt);
        }

        FontSize size = dp.isCrit ? FontSize::Medium : FontSize::Body;
        int fontH = FontManager::GetHandle(size);
        int strW = (fontH != -1) ? GetDrawStringWidthToHandle(textBuf, static_cast<int>(wcslen(textBuf)), fontH)
                                 : GetDrawStringWidth(textBuf, static_cast<int>(wcslen(textBuf)));

        int drawX = sx - strW / 2;
        int drawY = sy - 10;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // Viền đen 4 hướng tạo tương phản sắc nét
        unsigned int shadowCol = GetColor(10, 10, 15);
        FontManager::Draw(drawX - 1, drawY, textBuf, shadowCol, size);
        FontManager::Draw(drawX + 1, drawY, textBuf, shadowCol, size);
        FontManager::Draw(drawX, drawY - 1, textBuf, shadowCol, size);
        FontManager::Draw(drawX, drawY + 1, textBuf, shadowCol, size);
        FontManager::Draw(drawX + 1, drawY + 2, textBuf, shadowCol, size);

        // Màu chữ: Vàng kim rực rỡ nếu Crit / Nổ to, Trắng ngà tươi sáng nếu đòn thường
        unsigned int textColor = dp.isCrit ? GetColor(255, 215, 40) : GetColor(255, 250, 230);
        FontManager::Draw(drawX, drawY, textBuf, textColor, size);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void EffectSystem::SpawnBubbles(const Vec3& center, int count, float spread, float sizeMin, float sizeMax)
{
    bubbles_.EmitAt(center, count, spread, sizeMin, sizeMax);
}
