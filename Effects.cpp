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
#include <cwchar>

namespace
{
    float RandomFloat(float minVal, float maxVal)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + r * (maxVal - minVal);
    }
}

void EffectSystem::Init()
{
    Reset();
}

void EffectSystem::Reset()
{
    for (auto& s : shockwaves_) s.active = false;
    for (auto& p : particles_) p.active = false;
    for (auto& e : expOrbs_) e.active = false;
    for (auto& dp : damagePopups_) dp.active = false;
    trauma_ = 0.0f;
    shakeTime_ = 0.0f;
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

void EffectSystem::SpawnHitParticles(const Vec3& pos, int count, int color)
{
    int spawned = 0;
    for (auto& p : particles_)
    {
        if (!p.active)
        {
            p.pos = pos;
            p.vel = Vec3{
                RandomFloat(-180.0f, 180.0f),
                RandomFloat(80.0f, 260.0f),
                RandomFloat(-180.0f, 180.0f)
            };
            float sz = RandomFloat(4.0f, 9.0f);
            p.size = Vec3{ sz, sz, sz };
            p.color = color;
            p.life = 0.0f;
            p.maxLife = RandomFloat(0.35f, 0.6f);
            p.active = true;

            spawned++;
            if (spawned >= count) break;
        }
    }
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
    shakeTime_ += dt;
    if (trauma_ > 0.0f)
    {
        trauma_ = std::max(0.0f, trauma_ - dt * 2.5f); // Tiêu giảm chấn động
    }

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

    // Cập nhật hạt vụn
    for (auto& p : particles_)
    {
        if (!p.active) continue;
        p.life += dt;
        if (p.life >= p.maxLife)
        {
            p.active = false;
            continue;
        }

        p.vel.y -= 600.0f * dt; // Trọng lực
        p.pos += p.vel * dt;

        if (p.pos.y < p.size.y * 0.5f)
        {
            p.pos.y = p.size.y * 0.5f;
            p.vel.y = -p.vel.y * 0.35f; // Nảy nhẹ
            p.vel.x *= 0.7f;
            p.vel.z *= 0.7f;
        }
    }

    // Cập nhật hạt exp
    for (auto& e : expOrbs_)
    {
        if (!e.active) continue;
        e.life += dt;

        // Trọng lực ban đầu nếu đang văng
        if (e.pos.y > 6.0f)
        {
            e.vel.y -= 450.0f * dt;
            e.pos += e.vel * dt;
            if (e.pos.y <= 6.0f)
            {
                e.pos.y = 6.0f;
                e.vel = { 0, 0, 0 };
            }
        }

        // Tự động bay về phía người chơi khi ở gần (nam châm hút exp)
        Vec3 toPlayer = playerPos - e.pos;
        float distSq = toPlayer.LengthSq();
        if (distSq < 220.0f * 220.0f)
        {
            float dist = std::sqrt(distSq);
            if (dist > 1.0f)
            {
                Vec3 dir = toPlayer / dist;
                float magnetSpeed = 480.0f;
                e.pos += dir * magnetSpeed * dt;
            }
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
        Vec3 diff = playerPos - e.pos;
        diff.y = 0; // Xét trên mặt phẳng
        if (diff.LengthSq() <= radSq)
        {
            e.active = false;
            collected += static_cast<int>(e.value);
        }
    }
    return collected;
}

void EffectSystem::Draw() const
{
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

    // Vẽ hạt vụn 3D (Particles)
    for (const auto& p : particles_)
    {
        if (!p.active) continue;
        VECTOR minP = DxConv::ToVECTOR(p.pos - p.size * 0.5f);
        VECTOR maxP = DxConv::ToVECTOR(p.pos + p.size * 0.5f);
        Render3D::DrawBox3D(minP, maxP, p.color, TRUE);
    }

    // Vẽ hạt EXP (Khối ngọc xanh ngọc bích phát sáng xoay)
    int expColor = GetColor(50, 255, 160);
    int expEdge = GetColor(20, 180, 100);
    for (const auto& e : expOrbs_)
    {
        if (!e.active) continue;
        float hoverY = e.pos.y + std::sin(e.life * 8.0f) * 2.5f;
        Vec3 orbPos{ e.pos.x, hoverY, e.pos.z };
        Vec3 sz{ 8.0f, 8.0f, 8.0f };
        VECTOR minP = DxConv::ToVECTOR(orbPos - sz * 0.5f);
        VECTOR maxP = DxConv::ToVECTOR(orbPos + sz * 0.5f);
        Render3D::DrawBox3D(minP, maxP, expColor, TRUE);
        Render3D::DrawBox3D(minP, maxP, expEdge, FALSE);
    }
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
