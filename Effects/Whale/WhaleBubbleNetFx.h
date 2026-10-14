#pragma once
#include "WhaleFxBase.h"

// 3. Bubble Net: circular wall of rising bubbles with a safe gap, then a water burst when the boss surfaces at the center.
class WhaleBubbleNetFx : public WhaleFx::FxBase<4096> {
public:
    // gapYaw: direction (radians, atan2(z,x)) of the gap center. gapDeg 60 (P1) / 40 (P2).
    // autoSurface=false: call Surface() yourself when the boss comes up.
    void Start(VECTOR c, float radius = 260.0f, float gapDeg = 60.0f, float gapYawRad = 0.0f, float duration = 3.0f, float burstRadius = 160.0f, bool autoSurface = true) {
        center = c; ringR = radius; gapHalf = gapDeg * WhaleFx::Pi / 360; gapYaw = gapYawRad; dur = duration; burstR = burstRadius;
        surfaceAt = autoSurface ? duration : -1.0f; time = 0; acc = 0; surfaced = false; running = true;
    }
    void Surface() {
        if (surfaced) return;
        surfaced = true;
        using namespace WhaleFx;
        for (int i = 0; i < 380; ++i) {
            float a = rng.Range(-Pi, Pi), life = rng.Range(0.55f, 0.9f);
            float sp = burstR * rng.Range(0.6f, 1.15f) / life;
            VECTOR d = Dir(a);
            if (i % 2) Foam(VAdd(center, VScale(d, rng.Range(10, 50))), VGet(d.x * sp, rng.Range(150, 420), d.z * sp), life, rng.Range(10, 18), rng.Range(22, 36), 0.9f, 850);
            else Drop(VAdd(center, VGet(0, rng.Range(0, 40), 0)), VGet(d.x * sp * 0.6f, rng.Range(400, 750), d.z * sp * 0.6f), rng.Range(0.7f, 1.2f), rng.Range(9, 16), rng.Range(14, 24), 0.85f, 980);
        }
        for (int i = 0; i < 120; ++i) {
            float a = rng.Range(-Pi, Pi), r = rng.Range(0, burstR * 0.6f);
            Bubble(VAdd(center, VGet(std::cos(a) * r, 0, std::sin(a) * r)), VGet(0, rng.Range(150, 300), 0), rng.Range(0.4f, 0.8f), 10, 18, 0.8f);
        }
    }
    // Just-a-Fish: cột bọt đơn lẻ (lưới của game là các cột trên vòng khép dần 260 -> 130)
    void EmitPillar(VECTOR p, float dt, float rate = 40.0f) {
        using namespace WhaleFx;
        float n = dt * rate + rng.Next();
        for (int i = 0; i < static_cast<int>(n); ++i) {
            float a = rng.Range(-Pi, Pi), r = rng.Range(0, 14), rise = rng.Range(70, 170);
            Bubble(VAdd(p, VGet(std::cos(a) * r, rng.Range(0, 10), std::sin(a) * r)), VGet(0, rise, 0), rng.Range(0.7f, 1.5f), rng.Range(6, 12), rng.Range(12, 22), 0.75f);
            if (rng.Next() < 0.08f) Foam(VAdd(p, VGet(0, rise * 1.2f, 0)), VGet(0, 60, 0), 0.4f, 8, 16, 0.5f, 300);
        }
    }
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running && time < dur) {
            acc += dt * 650;
            while (acc >= 1) {
                acc -= 1;
                float a = rng.Range(-Pi, Pi);
                if (std::abs(WrapAngle(a - gapYaw)) < gapHalf) continue;   // keep the gap clean
                float r = ringR + rng.Range(-14, 14);
                float rise = rng.Range(70, 170);
                Bubble(VAdd(center, VGet(std::cos(a) * r, rng.Range(0, 10), std::sin(a) * r)), VGet(0, rise, 0), rng.Range(0.7f, 1.5f), rng.Range(6, 12), rng.Range(12, 22), 0.75f);
                if (rng.Next() < 0.08f) Foam(VAdd(center, VGet(std::cos(a) * r, rise * 1.2f, std::sin(a) * r)), VGet(0, 60, 0), 0.4f, 8, 16, 0.5f, 300);
            }
        }
        if (running && surfaceAt >= 0 && time >= surfaceAt) Surface();
        if (time >= dur && (surfaced || surfaceAt < 0)) running = false;
        pool.Update(dt, floorY);
    }
private:
    VECTOR center{};
    float ringR = 260, gapHalf = 0.52f, gapYaw = 0, dur = 3, burstR = 160, surfaceAt = 3, acc = 0;
    bool surfaced = false;
};
