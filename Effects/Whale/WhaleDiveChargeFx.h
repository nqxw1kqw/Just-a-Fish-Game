#pragma once
#include "WhaleFxBase.h"

// 5. Dive Charge: dorsal fin cuts the water leaving a straight foam trail, bow wave while lunging, splash on wall hit.
// Call Start() at lunge start, SetBoss() every frame with the boss position/forward, HitWall() on impact.
class WhaleDiveChargeFx : public WhaleFx::FxBase<4096> {
public:
    void Start(VECTOR bossPos, VECTOR forward, float speed = 600.0f, float finHeight = 40.0f) {
        pos = lastTrail = bossPos; fwd = VNorm(VGet(forward.x, 0, forward.z)); spd = speed; fin = finHeight; time = 0; running = true;
    }
    void SetBoss(VECTOR bossPos, VECTOR forward) { pos = bossPos; fwd = VNorm(VGet(forward.x, 0, forward.z)); }
    void HitWall(VECTOR hitPos, VECTOR wallNormal) {
        using namespace WhaleFx;
        running = false;
        VECTOR n = VNorm(VGet(wallNormal.x, 0, wallNormal.z));
        VECTOR side = VGet(-n.z, 0, n.x);
        for (int i = 0; i < 420; ++i) {
            float s = rng.Range(-1, 1), up = rng.Range(250, 800), out = rng.Range(60, 320);
            VECTOR v = VAdd(VAdd(VScale(n, out), VScale(side, s * 360)), VGet(0, up, 0));
            if (i % 3 == 0) Foam(VAdd(hitPos, VGet(0, rng.Range(0, 40), 0)), v, rng.Range(0.6f, 1.1f), rng.Range(12, 20), rng.Range(24, 40), 0.9f, 980);
            else Drop(VAdd(hitPos, VGet(0, rng.Range(0, 40), 0)), v, rng.Range(0.7f, 1.3f), rng.Range(8, 16), rng.Range(12, 22), 0.85f, 980);
        }
        for (int i = 0; i < 30; ++i)
            if (auto* m = Foam(VAdd(hitPos, VGet(rng.Range(-60, 60), rng.Range(40, 200), rng.Range(-60, 60))), VAdd(VScale(n, rng.Range(20, 80)), VGet(0, 60, 0)), rng.Range(1.0f, 1.6f), 70, 160, 0.25f, 0)) m->drag = 1.5f;
    }
    void Update(float dt) {
        using namespace WhaleFx;
        time += dt;
        if (running) {
            VECTOR side = VGet(-fwd.z, 0, fwd.x);
            // Straight foam trail: fixed-spaced, stationary foam blobs behind the fin.
            float moved = std::sqrt(VSquareSize(VSub(pos, lastTrail)));
            while (moved >= 10) {
                lastTrail = VAdd(lastTrail, VScale(VNorm(VSub(pos, lastTrail)), 10));
                moved -= 10;
                for (int k = 0; k < 3; ++k) {
                    auto* f = Foam(VAdd(VGet(lastTrail.x, floorY + 3, lastTrail.z), VScale(side, rng.Range(-10, 10))), VScale(side, rng.Range(-25, 25)), rng.Range(1.0f, 1.6f), rng.Range(10, 16), rng.Range(26, 40), 0.75f, 0);
                    if (f) f->drag = 2;
                }
            }
            // Fin spray + bow wave (V-shaped side sheets ahead of the head).
            int n = static_cast<int>(dt * 900 + rng.Next());
            for (int i = 0; i < n; ++i) {
                float sgn = rng.Next() < 0.5f ? -1.0f : 1.0f;
                VECTOR bow = VAdd(VGet(pos.x, floorY + 5, pos.z), VScale(fwd, rng.Range(30, 60)));
                VECTOR v = VAdd(VAdd(VScale(side, sgn * rng.Range(220, 400)), VScale(fwd, spd * rng.Range(0.3f, 0.6f))), VGet(0, rng.Range(120, 300), 0));
                if (i % 2) Foam(VAdd(bow, VScale(side, sgn * 20)), v, rng.Range(0.35f, 0.6f), rng.Range(9, 15), rng.Range(18, 28), 0.85f, 900);
                else Drop(VAdd(VGet(pos.x, floorY + fin * rng.Range(0.3f, 1.0f), pos.z), VScale(fwd, 10)), VAdd(VScale(side, sgn * rng.Range(60, 160)), VGet(0, rng.Range(80, 220), 0)), rng.Range(0.3f, 0.5f), 6, 9, 0.8f, 900);
            }
        }
        pool.Update(dt, floorY);
    }
private:
    VECTOR pos{}, lastTrail{}, fwd{1, 0, 0};
    float spd = 600, fin = 40;
};
