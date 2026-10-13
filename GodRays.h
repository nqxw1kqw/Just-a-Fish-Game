#pragma once
#include <DxLib.h>

// =======================================================================
// GodRays.h
// 光の筋: tia sáng từ mặt nước chiếu xuống giữa sàn đấu (additive, lắc lư chậm)
// + vệt caustics mờ trên sàn. Vẽ bằng DrawPrimitive3D với buffer tĩnh (rẻ).
// Phím G: bật/tắt (debug). Thời gian lấy từ đồng hồ hệ thống nên không
// phụ thuộc vào vòng lặp logic.
// =======================================================================
class GodRays
{
public:
    // Vẽ sau các vật thể đục (z-test bật, không ghi z)
    void Draw() const;
    bool IsEnabled() const { return enabled_; }

private:
    void PollToggle() const;

    mutable bool enabled_ = true;
    mutable bool keyPrev_ = false;
    mutable long long startCount_ = 0;
};
