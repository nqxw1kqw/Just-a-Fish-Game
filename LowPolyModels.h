#pragma once

// Mô hình low-poly (Blender, màu phẳng) cho quái và Boss Cá Voi.
// Các file nằm ở Data/Model/LowPoly/*.x (gốc tọa độ = giữa đáy, hướng +Z, 1 đơn vị = 1 đơn vị game).
// Mô hình cũ (PreModelTest) và cách vẽ khối hộp cũ vẫn được giữ nguyên làm phương án dự phòng.
// Phím F11: bật/tắt mô hình low-poly trong lúc chơi.

enum class LowPolyId
{
    Minion = 0,
    Hopper,
    Tanker,
    Float,
    Charge,
    WhaleBoss,
    Count
};

namespace LowPolyModels
{
    // Đặt false để mặc định dùng lại mô hình/cách vẽ cũ
    constexpr bool ENABLED_BY_DEFAULT = true;

    void Load();     // Nạp một lần (gọi sau DxLib_Init)
    void Release();  // Giải phóng (gọi trước DxLib_End)

    // Trả về handle dùng chung (-1 nếu chưa nạp được hoặc đang tắt)
    int GetHandle(LowPolyId id);

    bool IsEnabled();
    void SetEnabled(bool enable);
    void Toggle();

    // Boss Cá Voi: đổi màu vật liệu theo phase (phase 2: thân đỏ, mắt vàng)
    void ApplyWhalePhaseColors(bool phase2);
}
