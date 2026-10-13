#include "BubbleSystem.h"
#include "DxConv.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    float Rnd(float a, float b)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return a + r * (b - a);
    }

    // Texture bọt: viền sáng + điểm highlight, giữa gần trong suốt
    int MakeBubbleTexture()
    {
        BASEIMAGE image{};
        if (CreateARGB8ColorBaseImage(64, 64, &image) < 0) return -1;
        for (int y = 0; y < 64; ++y)
        {
            for (int x = 0; x < 64; ++x)
            {
                float dx = (x - 31.5f) / 31.5f, dy = (y - 31.5f) / 31.5f;
                float r = std::sqrt(dx * dx + dy * dy);
                float rim = std::exp(-std::pow((r - 0.82f) / 0.09f, 2.0f));
                float fill = (r < 0.9f) ? 0.12f : 0.0f;
                float hx = dx + 0.35f, hy = dy + 0.35f;
                float highlight = std::exp(-(hx * hx + hy * hy) / 0.012f);
                float a = std::clamp(rim * 0.9f + fill + highlight, 0.0f, 1.0f);
                if (r > 1.0f) a = 0.0f;
                SetPixelBaseImage(&image, x, y, 255, 255, 255, static_cast<int>(a * 255.0f));
            }
        }
        int handle = CreateGraphFromBaseImage(&image);
        ReleaseBaseImage(&image);
        return handle;
    }
}

BubbleSystem::~BubbleSystem()
{
    Shutdown();
}

void BubbleSystem::Init(float arenaHalfSize)
{
    arenaHalf_ = arenaHalfSize;
    if (texture_ < 0) texture_ = MakeBubbleTexture();
    Reset();
}

void BubbleSystem::Reset()
{
    for (auto& b : bubbles_) b.active = false;
    float lim = arenaHalf_ * 0.9f;
    for (auto& v : vents_)
    {
        v.pos = Vec3{ Rnd(-lim, lim), 0.0f, Rnd(-lim, lim) };
        v.timer = Rnd(0.0f, 3.0f);
        v.pending = 0;
        v.streamTimer = 0.0f;
    }
    ambientTimer_ = 0.0f;
}

void BubbleSystem::Shutdown()
{
    if (texture_ >= 0)
    {
        DeleteGraph(texture_);
        texture_ = -1;
    }
}

void BubbleSystem::Spawn(const Vec3& pos, float sizeMin, float sizeMax)
{
    std::size_t idx = nextSlot_;
    for (std::size_t k = 0; k < MAX_BUBBLES; ++k)
    {
        std::size_t i = (nextSlot_ + k) % MAX_BUBBLES;
        if (!bubbles_[i].active) { idx = i; break; }
    }
    nextSlot_ = (idx + 1) % MAX_BUBBLES;

    Bubble& b = bubbles_[idx];
    b = Bubble{};
    b.active = true;
    b.base = pos;
    b.size = Rnd(sizeMin, sizeMax);
    b.riseSpeed = 30.0f + b.size * 6.0f + Rnd(-5.0f, 5.0f); // bọt to nổi nhanh hơn
    b.lifetime = Rnd(5.0f, 9.0f);
    b.wobbleAmp = Rnd(2.0f, 6.0f);
    b.wobbleFreq = Rnd(1.5f, 3.2f);
    b.wobblePhase = Rnd(0.0f, 6.2831853f);
}

void BubbleSystem::Update(float dt)
{
    // 1. Khe phun: định kỳ phun một chùm 4-9 bọt nối đuôi nhau
    for (auto& v : vents_)
    {
        if (v.pending > 0)
        {
            v.streamTimer -= dt;
            if (v.streamTimer <= 0.0f)
            {
                Spawn(v.pos + Vec3{ Rnd(-4.0f, 4.0f), 2.0f, Rnd(-4.0f, 4.0f) }, 2.5f, 6.5f);
                --v.pending;
                v.streamTimer = Rnd(0.08f, 0.2f);
            }
        }
        else
        {
            v.timer -= dt;
            if (v.timer <= 0.0f)
            {
                v.pending = 4 + rand() % 6;
                v.streamTimer = 0.0f;
                v.timer = Rnd(1.5f, 4.0f);
            }
        }
    }

    // 2. Bọt lẻ rải rác khắp sàn
    ambientTimer_ -= dt;
    while (ambientTimer_ <= 0.0f)
    {
        ambientTimer_ += 0.12f;
        float lim = arenaHalf_ * 0.95f;
        Spawn(Vec3{ Rnd(-lim, lim), 1.0f, Rnd(-lim, lim) }, 1.5f, 3.5f);
    }

    // 3. Nổi lên + lắc lư; vỡ ở mặt nước hoặc khi hết tuổi
    for (auto& b : bubbles_)
    {
        if (!b.active) continue;
        if (b.popTimer >= 0.0f)
        {
            b.popTimer += dt;
            if (b.popTimer >= POP_DURATION) b.active = false;
            continue;
        }
        b.age += dt;
        b.base.y += b.riseSpeed * dt;
        b.size += dt * 0.25f; // giãn nở nhẹ khi lên cao
        if (b.base.y >= SURFACE_Y || b.age >= b.lifetime) b.popTimer = 0.0f;
    }
}

void BubbleSystem::Draw() const
{
    if (texture_ < 0) return;
    SetUseLighting(FALSE);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(FALSE);
    SetDrawBright(200, 235, 255);

    for (const auto& b : bubbles_)
    {
        if (!b.active) continue;
        float w = b.age * b.wobbleFreq + b.wobblePhase;
        Vec3 p = b.base + Vec3{ std::sin(w) * b.wobbleAmp, 0.0f, std::cos(w * 0.8f) * b.wobbleAmp * 0.6f };
        float alpha = (std::min)(1.0f, b.age / 0.3f) * 0.75f;  // hiện dần khi vừa sinh
        float size = b.size;
        if (b.popTimer >= 0.0f)
        {
            float t = b.popTimer / POP_DURATION;  // vỡ: phình to + mờ nhanh
            size *= 1.0f + 0.8f * t;
            alpha *= 1.0f - t;
        }
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(alpha * 255.0f));
        DrawBillboard3D(DxConv::ToVECTOR(p), 0.5f, 0.5f, size * 2.0f, 0.0f, texture_, TRUE);
    }

    SetDrawBright(255, 255, 255);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    SetWriteZBuffer3D(TRUE);
    SetUseLighting(TRUE);
}

std::size_t BubbleSystem::ActiveCount() const
{
    std::size_t n = 0;
    for (const auto& b : bubbles_) if (b.active) ++n;
    return n;
}
