#pragma once
// Just-a-Fish: phản hồi trúng đòn (tia lửa, bọt phụt, nổ khi chết) — hạt của Fuyuu (Effects/Hit)
// + chớp trắng mô hình (tự viết). Mỗi loại 1 pool 1024 hạt -> gom trong khối cấp phát heap.
#include "DxLib.h"
#include "HitSparkFx.h"
#include "HitBubblePuffFx.h"
#include "FishDeathFx.h"

struct HitFxSet
{
    HitSparkFx spark;
    HitBubblePuffFx puff;
    FishDeathFx death;

    void Update(float dt) { spark.Update(dt); puff.Update(dt); death.Update(dt); }
    void Draw() const { puff.Draw(); death.Draw(); spark.Draw(); }
    void Clear() { spark.Clear(); puff.Clear(); death.Clear(); }
};

namespace HitFlash
{
    // Vẽ MV1 với chớp trắng: cộng emissive trắng (k = 0..1) vào mọi material rồi trả lại như cũ.
    // MV1SetDifColorScale chỉ nhân màu diffuse nên texture tối không thành trắng; emissive thì có.
    inline void DrawModel(int handle, float k)
    {
        if (k <= 0.0f)
        {
            MV1DrawModel(handle);
            return;
        }
        if (k > 1.0f) k = 1.0f;
        constexpr int MAX_MATS = 32;
        COLOR_F saved[MAX_MATS];
        int n = MV1GetMaterialNum(handle);
        if (n > MAX_MATS) n = MAX_MATS;
        for (int i = 0; i < n; ++i)
        {
            saved[i] = MV1GetMaterialEmiColor(handle, i);
            COLOR_F c = saved[i];
            c.r = (c.r + k > 1.0f) ? 1.0f : c.r + k;
            c.g = (c.g + k > 1.0f) ? 1.0f : c.g + k;
            c.b = (c.b + k > 1.0f) ? 1.0f : c.b + k;
            MV1SetMaterialEmiColor(handle, i, c);
        }
        MV1DrawModel(handle);
        for (int i = 0; i < n; ++i) MV1SetMaterialEmiColor(handle, i, saved[i]);
    }
}
