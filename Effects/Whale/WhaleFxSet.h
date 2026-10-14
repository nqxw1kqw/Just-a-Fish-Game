#pragma once
// Just-a-Fish: gom toàn bộ hiệu ứng hạt của Cá voi (của Fuyuu) vào 1 khối cấp phát trên heap.
// Mỗi pool ~100KB / 1024 hạt -> KHÔNG đặt trên stack / làm thành viên trực tiếp.
#include "WhaleBreachSlamFx.h"
#include "WhaleTailSlapFx.h"
#include "WhaleBubbleNetFx.h"
#include "WhaleInhaleFx.h"
#include "WhaleDiveChargeFx.h"
#include "WhaleCallPodFx.h"
#include "WhaleStatusFx.h"

struct WhaleFxSet
{
    WhaleBreachSlamFx breach;
    WhaleTailSlapFx tail;
    WhaleBubbleNetFx net;
    WhaleInhaleFx inhale;
    WhaleDiveChargeFx dive;
    WhaleCallPodFx pod;
    WhaleStunFx stun;
    PlayerSlowFx slow;
    int glowTex = -1;
    int bubbleTex = -1;

    // Chấm sáng mềm cho giọt nước / bọt (Fuyuu chỉ cung cấp texture bọt)
    static int CreateGlowTexture(int size = 32)
    {
        int soft = MakeARGB8ColorSoftImage(size, size);
        if (soft < 0) return -1;
        float c = (size - 1) * 0.5f;
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                float dx = (x - c) / c, dy = (y - c) / c;
                float d2 = dx * dx + dy * dy;
                float a = (d2 < 1.0f) ? std::exp(-d2 * 3.0f) * (1.0f - d2) : 0.0f;
                DrawPixelSoftImage(soft, x, y, 255, 255, 255, static_cast<int>(a * 255.0f));
            }
        }
        int g = CreateGraphFromSoftImage(soft);
        DeleteSoftImage(soft);
        return g;
    }

    void Init()
    {
        if (glowTex < 0) glowTex = CreateGlowTexture();
        if (bubbleTex < 0) bubbleTex = WhaleFx::CreateBubbleTexture();
        breach.SetTextures(glowTex, bubbleTex);
        tail.SetTextures(glowTex, bubbleTex);
        net.SetTextures(glowTex, bubbleTex);
        inhale.SetTextures(glowTex, bubbleTex);
        dive.SetTextures(glowTex, bubbleTex);
        pod.SetTextures(glowTex, bubbleTex);
        stun.SetTextures(glowTex, bubbleTex);
        slow.SetTextures(glowTex, bubbleTex);
    }

    void Release()
    {
        if (glowTex >= 0) { DeleteGraph(glowTex); glowTex = -1; }
        if (bubbleTex >= 0) { DeleteGraph(bubbleTex); bubbleTex = -1; }
    }

    void ClearAll()
    {
        breach.Clear(); tail.Clear(); net.Clear(); inhale.Clear();
        dive.Clear(); pod.Clear(); stun.Clear(); slow.Clear();
    }

    void Update(float dt)
    {
        if (breach.IsActive()) breach.Update(dt);
        if (tail.IsActive()) tail.Update(dt);
        if (net.IsActive()) net.Update(dt);
        if (inhale.IsActive()) inhale.Update(dt);
        if (dive.IsActive()) dive.Update(dt);
        if (pod.IsActive()) pod.Update(dt);
        if (stun.IsActive()) stun.Update(dt);
        if (slow.IsActive()) slow.Update(dt);
    }

    void Draw() const
    {
        if (breach.IsActive()) breach.Draw();
        if (tail.IsActive()) tail.Draw();
        if (net.IsActive()) net.Draw();
        if (inhale.IsActive()) inhale.Draw();
        if (dive.IsActive()) dive.Draw();
        if (pod.IsActive()) pod.Draw();
        if (stun.IsActive()) stun.Draw();
        if (slow.IsActive()) slow.Draw();
        SetUseLighting(TRUE); // Pool::Draw tắt lighting mà không bật lại
    }
};
