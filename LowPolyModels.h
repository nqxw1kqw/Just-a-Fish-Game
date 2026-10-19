#pragma once

// Mô hình low-poly (Blender, màu phẳng) cho quái và Boss Cá Voi.
// Các file nằm ở Data/Model/LowPoly/*.x (gốc tọa độ = giữa đáy, hướng +Z, 1 đơn vị = 1 đơn vị game).
// Luôn dùng mô hình (không còn vẽ khối hộp). Nạp lỗi -> ghi log + printfDx, đối tượng đó bị bỏ qua khi vẽ
// (quái thường còn dự phòng mô hình cũ PreModelTest).

enum class LowPolyId
{
    Minion = 0,
    Hopper,
    Tanker,
    Float,
    Charge,
    WhaleBoss,
    SharkBoss,      // Boss Cá Mập (Endless) - thân
    SharkBossTail,  // Đuôi tách riêng để vẫy (gốc = khớp đuôi)
    SquidBoss,      // Boss #3: Giant Squid
    Count
};

namespace LowPolyModels
{
    void Load();     // Nạp một lần (gọi sau DxLib_Init)
    void Release();  // Giải phóng (gọi trước DxLib_End)

    // Trả về handle dùng chung (-1 nếu chưa nạp được)
    int GetHandle(LowPolyId id);

    // Boss Cá Voi: đổi màu vật liệu theo phase (phase 2: thân đỏ, mắt vàng)
    void ApplyWhalePhaseColors(bool phase2);
}
