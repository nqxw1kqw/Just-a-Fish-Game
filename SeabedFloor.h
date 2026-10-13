#pragma once

// =======================================================================
// SeabedFloor.h
// 海底の砂テクスチャ: vẽ mặt sàn bằng texture cát lặp (UV wrap).
// Ảnh: Data/Texture/Stage/sand_floor.png (tileable). Nếu thiếu file, Draw()
// trả về false để GamePrototype vẽ sàn cũ (hộp màu + lưới).
// =======================================================================
namespace SeabedFloor
{
    // Vẽ sàn [-half, half] x [-half, half] tại y = 0. Trả về false nếu không có texture.
    bool Draw(float arenaHalfSize);
    void Release();
}
