#pragma once
#include "SquidFxCommon.h"

// SquidGrabTentacle: a tentacle reaching from the boss toward the player.
// Particle-chain approximation: every frame, beads are spawned along a quadratic
// Bezier (from -> raised control -> tip) with a slight curl near the tip; beads live
// ~0.18 s, so the chain is a segmented, tapering tentacle. Phases: reach (duration),
// hold 0.35 s, retract 0.35 s, then idle. Stop()/Clear() remove it immediately.
class SquidGrabTentacleEffect : public SquidFx::FxType<512> {
public:
    void Start(VECTOR fromPos, VECTOR targetPos, float duration) {
        from = fromPos; target = targetPos;
        dur = (std::max)(0.05f, duration); t = 0; phase = 0; splashed = false; active = true;
    }
    void SetTarget(VECTOR targetPos) { target = targetPos; }            // follow a moving player while reaching
    void Stop() { active = false; phase = 0; pool.Clear(); }
    bool IsReaching() const { return active; }
    void Update(float dt) {
        using namespace SquidFx;
        if (active) {
            t += dt;
            float reach = 1.0f;
            if (phase == 0) { reach = EaseOut(t / dur); if (t >= dur) { phase = 1; t = 0; } }
            else if (phase == 1) { reach = 1.0f; if (t >= 0.35f) { phase = 2; t = 0; } }
            else { reach = 1.0f - EaseOut(t / 0.35f); if (t >= 0.35f) { active = false; phase = 0; reach = 0; } }
            if (active && reach > 0.02f) {
                EmitChain(reach);
                if (phase >= 1 && !splashed) { splashed = true; SplashTip(); }
            }
        }
        running = active;
        pool.Update(dt);
    }
    void Clear() { pool.Clear(); active = false; phase = 0; running = false; }
private:
    static float EaseOut(float x) { x = std::clamp(x, 0.0f, 1.0f); return 1 - (1 - x) * (1 - x); }
    VECTOR PointOn(float s, float reach, float curlPh) const {
        using namespace SquidFx;
        VECTOR tip = VAdd(from, VScale(VSub(target, from), reach));
        VECTOR mid = VAdd(VScale(VAdd(from, tip), 0.5f), VGet(0, VSize(VSub(tip, from)) * 0.22f + 10, 0));
        float u = 1 - s;
        VECTOR p = VAdd(VAdd(VScale(from, u * u), VScale(mid, 2 * u * s)), VScale(tip, s * s));
        VECTOR side = Flat(VSub(tip, from), VGet(1, 0, 0));
        VECTOR perp = VGet(side.z, 0, -side.x);
        float curl = std::sin(s * 6.0f + curlPh) * 9.0f * s * s;     // slight curl, stronger near the tip
        return VAdd(p, VAdd(VScale(perp, curl), VGet(0, std::sin(s * Pi) * 4.0f, 0)));
    }
    void EmitChain(float reach) {
        using namespace SquidFx;
        curlPhase += 0.12f;
        const int beads = 11;
        for (int i = 0; i < beads; ++i) {
            float s = beads == 1 ? 1.0f : static_cast<float>(i) / (beads - 1);
            bool sucker = (i % 3) == 2;
            SquidFx::Particle* p = Add(PointOn(s, reach, curlPhase), VGet(0, 0, 0), 0.18f,
                (11.0f - 6.0f * s) * (sucker ? 0.45f : 1.0f), (11.0f - 6.0f * s) * (sucker ? 0.45f : 1.0f), sucker ? 0.95f : 0.9f,
                sucker ? 235 : 148, sucker ? 190 : 66, sucker ? 205 : 118, false);
            if (!p) return;
        }
    }
    void SplashTip() {
        for (int i = 0; i < 10; ++i) {
            SquidFx::Particle* p = Add(VAdd(target, VGet(0, 4, 0)),
                VScale(SquidFx::ConeDir(rng, VGet(0, 1, 0), 0.8f), rng.Range(70, 160)),
                rng.Range(0.25f, 0.35f), rng.Range(3, 5), 1.5f, 0.85f, 215, 235, 245, false);
            if (p) p->gravity = 500;
        }
    }
    VECTOR from{}, target{};
    float dur = 0.8f, t = 0, curlPhase = 0;
    int phase = 0;
    bool active = false, splashed = true;
};
