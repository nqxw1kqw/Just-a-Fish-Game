#pragma once
// Just-a-Fish: gom hiệu ứng hạt của Boss Cá Mập (Fuyuu, namespace SharkFx) vào 1 khối cấp phát trên heap.
// Mỗi pool 2048 hạt (Rage 1024) -> không đặt trên stack.
#include "SharkBiteLungeFx.h"
#include "SharkCircleFinFx.h"
#include "SharkVortexFx.h"
#include "SharkRageFx.h"
#include "../SharkSand/SharkSandFx.h"

struct SharkFxSet
{
    SharkBiteLungeFx bite;
    SharkCircleFinFx fin;
    SharkVortexFx vortex;
    SharkRageFx rage;
    SharkSandEffect sand; // Fuyuu: cát khi Hunt (lặn, vệt vây, phun, bụi vòng khoá)

    void Update(float dt) { bite.Update(dt); fin.Update(dt); vortex.Update(dt); rage.Update(dt); sand.Update(dt); }
    void Draw() const
    {
        vortex.Draw(); sand.Draw(); fin.Draw(); bite.Draw(); rage.Draw();
        SetUseLighting(TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    void ClearAll() { bite.Clear(); fin.Clear(); vortex.Clear(); rage.Clear(); sand.Clear(); }
};
