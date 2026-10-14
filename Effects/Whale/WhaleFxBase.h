#pragma once
#include "WhaleFxCommon.h"

namespace WhaleFx {
// Common shell: textures, pool, floor height. Each effect adds Start()/Update().
template <int N>
class FxBase {
public:
    void SetTextures(int glow, int bubble) { glowTex = glow; bubbleTex = bubble; }
    void SetFloorY(float y) { floorY = y; }
    void Draw() const { pool.Draw(glowTex, bubbleTex); }
    void Clear() { pool.Clear(); running = false; }
    int ParticleCount() const { return pool.Count(); }
    bool IsActive() const { return running || pool.Count() > 0; }
    void Stop() { running = false; }   // Just-a-Fish: dừng phát hạt, hạt cũ tàn dần
protected:
    Particle* Drop(VECTOR pos, VECTOR vel, float life, float size0, float size1, float alpha, float gravity) {
        Particle* p = pool.Spawn();
        if (!p) return nullptr;
        p->pos = pos; p->vel = vel; p->life = life; p->size0 = size0; p->size1 = size1; p->alpha = alpha; p->gravity = gravity;
        p->r = WaterR; p->g = WaterG; p->b = WaterB;
        return p;
    }
    Particle* Foam(VECTOR pos, VECTOR vel, float life, float size0, float size1, float alpha, float gravity) {
        Particle* p = Drop(pos, vel, life, size0, size1, alpha, gravity);
        if (p) { p->r = FoamR; p->g = FoamG; p->b = FoamB; }
        return p;
    }
    Particle* Bubble(VECTOR pos, VECTOR vel, float life, float size0, float size1, float alpha) {
        Particle* p = Drop(pos, vel, life, size0, size1, alpha, -60.0f);
        if (p) { p->bubble = true; p->r = 200; p->g = 240; p->b = 255; p->wobble = size0 * 0.25f; p->drag = 0.6f; }
        return p;
    }
    Pool<N> pool;
    Rng rng;
    int glowTex = -1, bubbleTex = -1;
    float floorY = 0, time = 0;
    bool running = false;
};
}
