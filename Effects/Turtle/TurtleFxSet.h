#pragma once
// Boss #4 Ancient Turtle: gom hiệu ứng của Fuyuu (Effects/Turtle) vào 1 khối cấp phát trên heap.
// Mỗi pool 256-512 hạt -> không đặt trên stack.
#include "TurtleShellBreakFx.h"
#include "TurtleRollTrailFx.h"
#include "TurtleRollImpactFx.h"
#include "TurtleWithdrawDustFx.h"
#include "TurtleBiteSnapFx.h"
#include "TurtleStaggerStarsFx.h"

struct TurtleFxSet
{
    TurtleShellBreakEffect shellBreak;
    TurtleRollTrailEffect rollTrail;
    TurtleRollImpactEffect rollImpact;
    TurtleWithdrawDustEffect withdrawDust;
    TurtleBiteSnapEffect biteSnap;
    TurtleStaggerStarsEffect staggerStars;

    void Update(float dt) { shellBreak.Update(dt); rollTrail.Update(dt); rollImpact.Update(dt); withdrawDust.Update(dt); biteSnap.Update(dt); staggerStars.Update(dt); }
    void Draw() const
    {
        rollTrail.Draw(); withdrawDust.Draw(); shellBreak.Draw(); rollImpact.Draw(); biteSnap.Draw(); staggerStars.Draw();
        SetUseLighting(TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    void ClearAll() { shellBreak.Clear(); rollTrail.Clear(); rollImpact.Clear(); withdrawDust.Clear(); biteSnap.Clear(); staggerStars.Clear(); }
};