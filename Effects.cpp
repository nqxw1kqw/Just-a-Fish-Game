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
    if (expGlowTex_ >= 0) { DeleteGraph(expGlowTex_); expGlowTex_ = -1; }
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
    // Hiệu ứng của Fuyuu (Effects/Heal, Effects/Dodge, Effects/EnemyTrail) - pool nằm trong HitFxSet (heap)
    HitFxSet* fx = hitFx_.get();
    RegisterFx(FxId::HealWaveEnd,    [fx](VECTOR p, VECTOR, float) { fx->heal.HealWaveEnd(p); });
    RegisterFx(FxId::HealCardScale,  [fx](VECTOR p, VECTOR, float) { fx->heal.HealCardScale(p); });
    RegisterFx(FxId::HealPoolBonus,  [fx](VECTOR p, VECTOR, float) { fx->heal.HealPoolBonus(p); });
    RegisterFx(FxId::HealBossReward, [fx](VECTOR p, VECTOR, float) { fx->heal.HealBossReward(p); });
    RegisterFx(FxId::HealOnKill,     [fx](VECTOR p, VECTOR, float) { fx->heal.HealOnKill(p); });
    RegisterFx(FxId::TsunamiDodge,   [fx](VECTOR p, VECTOR d, float) { fx->dodge.Emit(p, d); });
    // Vệt lao gọi mỗi frame: Hell đông quái (scale < 1) -> bỏ bớt frame theo xác suất
    RegisterFx(FxId::ChargeDashTrail, [fx](VECTOR p, VECTOR d, float k)
    {
        if (k < 1.0f && static_cast<float>(rand()) / static_cast<float>(RAND_MAX) > k) return;
        fx->dashTrail.Emit(p, d);
    });
}

void EffectSystem::SpawnExpOrb(const Vec3& pos, float val)
{
    for (auto& e : expOrbs_)
    {
        if (!e.active)
        {
            // Bật ra theo vòng cung: văng ngang ngẫu nhiên + lên cao, rơi xuống nảy 1 lần
            float a = RandomFloat(0.0f, 6.2831853f);
            float sp = RandomFloat(GameConsts::EXP_ORB_POP_SPEED_MIN, GameConsts::EXP_ORB_POP_SPEED_MAX);
            e.pos = Vec3{ pos.x, GameConsts::EXP_ORB_HOVER_Y + 4.0f, pos.z };
            e.vel = Vec3{ std::cos(a) * sp, RandomFloat(GameConsts::EXP_ORB_POP_UP_MIN, GameConsts::EXP_ORB_POP_UP_MAX), std::sin(a) * sp };
            e.value = val;
            e.life = 0.0f;
            e.spin = RandomFloat(0.0f, 6.2831853f);
            e.magnetT = -1.0f;
            e.curveSign = (RandomFloat(0.0f, 1.0f) < 0.5f) ? -1.0f : 1.0f;
            e.absorbT = -1.0f;
            e.landed = false;
            e.active = true;
            break;
        }
    }
}

void EffectSystem::UpdateExpOrbs(float dt, const Vec3& playerPos)
{
    expTarget_ = playerPos + Vec3{ 0.0f, 12.0f, 0.0f };
    const float magR2 = GameConsts::EXP_MAGNET_RADIUS * GameConsts::EXP_MAGNET_RADIUS;
    for (auto& e : expOrbs_)
    {
        if (!e.active) continue;
        e.life += dt;
        e.spin += GameConsts::EXP_ORB_SPIN_SPEED * (e.magnetT >= 0.0f ? 3.0f : 1.0f) * dt;

        // Đang hấp thụ: trượt vào tâm người chơi rồi biến mất
        if (e.absorbT >= 0.0f)
        {
            e.absorbT += dt;
            float k = (std::min)(1.0f, dt * 30.0f);
            e.pos += (expTarget_ - e.pos) * k;
            if (e.absorbT >= GameConsts::EXP_ORB_ABSORB_TIME) e.active = false;
            continue;
        }

        Vec3 toPlayer = expTarget_ - e.pos;
        toPlayer.y = 0.0f;
        float distSq = toPlayer.LengthSq();
        if (e.magnetT < 0.0f && distSq < magR2) e.magnetT = 0.0f; // vào vùng hút: bị hút tới khi nhặt

        if (e.magnetT >= 0.0f)
        {
            // Nam châm: ease-in (tăng tốc dần) + cong nhẹ lúc đầu, bay lên ngang thân cá
            e.magnetT += dt;
            float dist = std::sqrt(distSq);
            if (dist > 0.5f)
            {
                Vec3 dir = toPlayer / dist;
                float speed = (std::min)(GameConsts::EXP_MAGNET_SPEED,
                                         GameConsts::EXP_MAGNET_SPEED_START + GameConsts::EXP_MAGNET_ACCEL * e.magnetT);
                float curveK = 1.0f - (std::min)(1.0f, e.magnetT / GameConsts::EXP_MAGNET_CURVE_TIME);
                Vec3 side{ -dir.z, 0.0f, dir.x };
                Vec3 step = dir * (speed * dt) + side * (GameConsts::EXP_MAGNET_CURVE * curveK * e.curveSign * dt);
                if (step.LengthSq() > distSq) step = toPlayer; // không bay vượt qua người chơi
                e.pos += step;
            }
            e.pos.y += (expTarget_.y - e.pos.y) * (std::min)(1.0f, dt * 6.0f);
            continue;
        }

        // Rơi theo vòng cung, nảy 1 lần rồi nằm lơ lửng
        if (!e.landed)
        {
            e.vel.y -= GameConsts::EXP_ORB_GRAVITY * dt;
            e.pos += e.vel * dt;
            if (e.pos.y <= GameConsts::EXP_ORB_HOVER_Y)
            {
                e.pos.y = GameConsts::EXP_ORB_HOVER_Y;
                if (e.vel.y < -60.0f)
                {
                    e.vel.y = -e.vel.y * GameConsts::EXP_ORB_BOUNCE;
                    e.vel.x *= 0.45f; e.vel.z *= 0.45f;
                }
                else
                {
                    e.vel = Vec3{ 0, 0, 0 };
                    e.landed = true;
                }
            }
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
    UpdateExpOrbs(dt, playerPos);
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
        if (!e.active || e.absorbT >= 0.0f) continue;
        if (Collision::DistSqXZ(playerPos, e.pos) <= radSq) // Xét trên mặt phẳng
        {
            e.absorbT = 0.0f; // EXP cộng ngay; viên ngọc thu nhỏ + chớp rồi biến mất
            collected += static_cast<int>(e.value);
        }
    }
    return collected;
}

void EffectSystem::DrawExpOrbs() const
{
    // Ngọc EXP: bát diện pha lê trắng/xanh băng (8 mặt, tô theo mặt) + quầng sáng additive.
    // Gom tất cả vào 1 lần DrawPolygon3D để nhẹ khi Hell đông quái.
    static VERTEX3D verts[GameConsts::MAX_EXP_ORBS * 24];
    static const float kShade[8] = { 1.00f, 0.80f, 0.92f, 0.72f, 0.86f, 0.66f, 0.78f, 0.60f };
    int nTri = 0, nOrb = 0;
    for (const auto& e : expOrbs_)
    {
        if (!e.active) continue;
        ++nOrb;
        if (nTri + 8 > GameConsts::MAX_EXP_ORBS * 8) break;
        float scale = 1.0f;
        if (e.absorbT >= 0.0f) scale = 1.0f - 0.8f * (std::min)(1.0f, e.absorbT / GameConsts::EXP_ORB_ABSORB_TIME);
        else if (e.life < 0.15f) scale = 0.4f + 0.6f * (e.life / 0.15f); // nở ra khi vừa rơi
        float bob = (e.landed && e.magnetT < 0.0f) ? std::sin(e.life * GameConsts::EXP_ORB_BOB_SPEED + e.spin * 0.3f) * GameConsts::EXP_ORB_BOB_AMP : 0.0f;
        float r = GameConsts::EXP_ORB_SIZE * scale;
        float h = r * 1.55f;
        VECTOR c = VGet(e.pos.x, e.pos.y + bob, e.pos.z);
        float cs = std::cos(e.spin), sn = std::sin(e.spin);
        VECTOR ring[4];
        for (int i = 0; i < 4; ++i)
        {
            float ax = (i == 0) ? 1.0f : (i == 2) ? -1.0f : 0.0f;
            float az = (i == 1) ? 1.0f : (i == 3) ? -1.0f : 0.0f;
            ring[i] = VGet(c.x + (ax * cs - az * sn) * r, c.y, c.z + (ax * sn + az * cs) * r);
        }
        VECTOR top = VGet(c.x, c.y + h, c.z), bot = VGet(c.x, c.y - h * 0.8f, c.z);
        bool flash = (e.absorbT >= 0.0f);
        for (int f = 0; f < 8; ++f)
        {
            int i0 = f % 4, i1 = (f + 1) % 4;
            VECTOR tip = (f < 4) ? top : bot;
            float k = flash ? 1.0f : kShade[f];
            // Trắng hơi xanh băng: mặt trên sáng, mặt dưới ngả cyan
            int R = static_cast<int>(235 * k + 20), G = static_cast<int>(245 * k + 10), B = 255;
            if (f >= 4) { R = static_cast<int>(R * 0.82f); G = static_cast<int>(G * 0.93f); }
            COLOR_U8 col = GetColorU8((std::min)(255, R), (std::min)(255, G), B, 255);
            VECTOR p[3] = { tip, ring[i0], ring[i1] };
            if (f >= 4) { p[1] = ring[i1]; p[2] = ring[i0]; }
            for (int v = 0; v < 3; ++v)
            {
                VERTEX3D& vx = verts[nTri * 3 + v];
                vx.pos = p[v];
                vx.norm = VGet(0.0f, 1.0f, 0.0f);
                vx.dif = col;
                vx.spc = GetColorU8(0, 0, 0, 0);
                vx.u = vx.v = vx.su = vx.sv = 0.0f;
            }
            ++nTri;
        }
    }
    if (nTri == 0) return;

    SetUseLighting(FALSE);
    SetUseBackCulling(FALSE);
    DrawPolygon3D(verts, nTri, DX_NONE_GRAPH, FALSE);

    // Quầng sáng mềm (additive, không ghi Z). Đông quá thì bỏ để giảm lag.
    if (nOrb <= GameConsts::EXP_ORB_HALO_MAX)
    {
        if (expGlowTex_ < 0)
        {
            const int S = 32;
            int soft = MakeARGB8ColorSoftImage(S, S);
            for (int y = 0; y < S; ++y)
                for (int x = 0; x < S; ++x)
                {
                    float dx = (x + 0.5f) / S * 2.0f - 1.0f, dy = (y + 0.5f) / S * 2.0f - 1.0f;
                    float d2 = dx * dx + dy * dy;
                    float a = (d2 >= 1.0f) ? 0.0f : std::exp(-d2 * 4.0f) * (1.0f - d2);
                    DrawPixelSoftImage(soft, x, y, 255, 255, 255, static_cast<int>(a * 255.0f));
                }
            expGlowTex_ = CreateGraphFromSoftImage(soft);
            DeleteSoftImage(soft);
        }
        SetWriteZBuffer3D(FALSE);
        SetDrawBright(200, 235, 255);
        for (const auto& e : expOrbs_)
        {
            if (!e.active) continue;
            bool flash = (e.absorbT >= 0.0f);
            float pulse = 0.85f + 0.15f * std::sin(e.life * 5.0f + e.spin);
            float size = GameConsts::EXP_ORB_HALO_SIZE * (flash ? 1.6f : pulse);
            int alpha = flash ? 230 : GameConsts::EXP_ORB_HALO_ALPHA;
            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawBillboard3D(VGet(e.pos.x, e.pos.y + 2.0f, e.pos.z), 0.5f, 0.5f, size, 0.0f, expGlowTex_, TRUE);
        }
        SetDrawBright(255, 255, 255);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        SetWriteZBuffer3D(TRUE);
    }
    SetUseLighting(TRUE);
    SetUseBackCulling(TRUE);
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

    // Ngọc EXP (bát diện pha lê + quầng sáng)
    DrawExpOrbs();

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
