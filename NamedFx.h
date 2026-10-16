#pragma once
#include "DxLib.h"
#include <functional>

// Hook hiệu ứng theo TÊN (thay cho hạt khối lập phương cũ).
// Quy ước giống hiệu ứng của Fuyuu: mỗi hiệu ứng là 1 lớp header-only trong Effects/<Nhóm>/ (Emit/Update/Draw/Clear),
// gom vào 1 Set và đăng ký ở EffectSystem::RegisterNamedFx(). Chưa đăng ký -> PlayFx là no-op (ghi log 1 lần).

enum class FxId
{
    // Hồi máu / phần thưởng (quanh người chơi)
    HealWaveEnd,      // Hết stage: hồi máu cuối wave
    HealCardScale,    // Thẻ hook OnWaveEnd (Vảy hồi phục): hồi thêm
    HealPoolBonus,    // Hết pool thẻ: thưởng hồi 20% máu
    HealBossReward,   // Hạ Boss: hồi 50% máu
    HealOnKill,       // Thẻ Cá mập đói: giết quái hồi 5 máu
    // Né / phản hồi kỹ năng
    TsunamiDodge,     // Nhảy né sóng thần thành công
    // Quái
    ChargeDashTrail,  // Quái Charge (cá mập nhỏ) lao: vệt sau lưng (gọi mỗi frame)
    Count
};

namespace NamedFx
{
    // Tên dùng khi giao việc / đặt tên lớp hiệu ứng (vd. Effects/Heal/HealWaveEndFx.h)
    inline const char* Name(FxId id)
    {
        static const char* kNames[static_cast<int>(FxId::Count)] = {
            "HealWaveEnd", "HealCardScale", "HealPoolBonus", "HealBossReward", "HealOnKill",
            "TsunamiDodge", "ChargeDashTrail" };
        int i = static_cast<int>(id);
        return (i >= 0 && i < static_cast<int>(FxId::Count)) ? kNames[i] : "?";
    }

    // pos: vị trí; dir: hướng (chuẩn hoá, mặc định hướng lên); scale: 1 = bình thường (Hell đông quái < 1)
    using Handler = std::function<void(VECTOR pos, VECTOR dir, float scale)>;
}
