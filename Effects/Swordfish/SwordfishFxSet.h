#pragma once
// Boss #5 Swordfish: gom hiệu ứng của Fuyuu (Effects/Swordfish) vào 1 khối cấp phát trên heap.
// Mỗi pool 384-512 hạt -> không đặt trên stack.
#include "SwordfishDashStreakFx.h"
#include "SwordfishSpinVortexFx.h"
#include "SwordfishLeapArcFx.h"

struct SwordfishFxSet
{
    SwordfishDashStreakEffect dashStreak;
    SwordfishSpinVortexEffect spinVortex;
    SwordfishLeapArcEffect leapArc;

    void Update(float dt) { dashStreak.Update(dt); spinVortex.Update(dt); leapArc.Update(dt); }
    void Draw() const
    {
        dashStreak.Draw(); spinVortex.Draw(); leapArc.Draw();
        SetUseLighting(TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    void ClearAll() { dashStreak.Clear(); spinVortex.Clear(); leapArc.Clear(); }
};