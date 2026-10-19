#pragma once
// Boss #3 Giant Squid: gom hiệu ứng của Fuyuu (Effects/Squid) vào 1 khối cấp phát trên heap.
// Mỗi pool 256-512 hạt -> không đặt trên stack.
#include "SquidTentacleSlamFx.h"
#include "SquidInkSprayFx.h"
#include "SquidInkJetFx.h"
#include "SquidGrabTentacleFx.h"
#include "SquidEnrageAuraFx.h"

struct SquidFxSet
{
    SquidTentacleSlamEffect slam;
    SquidInkSprayEffect inkSpray;
    SquidInkJetEffect inkJet;
    SquidGrabTentacleEffect grab;
    SquidEnrageAuraEffect enrageAura;

    void Update(float dt) { slam.Update(dt); inkSpray.Update(dt); inkJet.Update(dt); grab.Update(dt); enrageAura.Update(dt); }
    void Draw() const
    {
        inkJet.Draw(); inkSpray.Draw(); slam.Draw(); grab.Draw(); enrageAura.Draw();
        SetUseLighting(TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    void ClearAll() { slam.Clear(); inkSpray.Clear(); inkJet.Clear(); grab.Clear(); enrageAura.Clear(); }
};