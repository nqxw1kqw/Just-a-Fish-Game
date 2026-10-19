#pragma once

class PlayerFish;
class WhaleBoss;
class SharkBoss;
class SquidBoss;

// =======================================================================
// BattleHud.h
// 戦闘HUD: khung HP người chơi (thanh máu trễ), EXP/Lv, vòng hồi chiêu Slam,
// combo, thanh máu Boss (tên + pha + mốc 50%) và chớp đỏ khi bị trúng đòn.
// Vẽ hoàn toàn bằng DxLib (không cần file ảnh). Trạng thái hoạt ảnh là mutable
// vì UIRenderer::Draw* là const; dt lấy từ đồng hồ hiệu năng cao.
// =======================================================================
class BattleHud
{
public:
    void Reset() const;
    void DrawPlayerPanel(const PlayerFish& player) const;
    void DrawBossBar(const WhaleBoss& boss) const;
    void DrawBossBar(const SharkBoss& shark) const;
    void DrawBossBar(const SquidBoss& squid) const;
    void DrawBossBarCore(bool active, float hp, float maxHp, bool p2, const wchar_t* name, const wchar_t* phaseLabel, float markRatio) const;
    void DrawDamageFlash() const;
    // Y (pixel) ngay dưới khung người chơi, để UI khác xếp phía dưới
    static int PanelBottomY();

private:
    float FrameDt() const;

    mutable long long lastCount_ = 0;
    mutable float dt_ = 0.0f;
    mutable float lastPlayerHp_ = -1.0f;
    mutable float trailPlayerHp_ = -1.0f;
    mutable float flashTimer_ = 0.0f;
    mutable float lastBossHp_ = -1.0f;
    mutable float trailBossHp_ = -1.0f;
    mutable float bossShake_ = 0.0f;
    mutable float pulse_ = 0.0f;
};
