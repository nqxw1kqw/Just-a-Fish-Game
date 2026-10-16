#pragma once
#include "BattleHud.h"

class GamePrototype;
class WhaleBoss;

class UIRenderer
{
public:
    void DrawHUD(const GamePrototype& game) const;
    void DrawBossHealthBar(const WhaleBoss& boss) const;
    void DrawBossHealthBar(const SharkBoss& shark) const;
    void DrawBossIntroScreen(bool shark = false) const;
    void DrawTitleScreen(const GamePrototype& game) const;
    void DrawVictoryScreen(const GamePrototype& game) const;
    void DrawGameOverScreen(const GamePrototype& game) const;
    void DrawPauseScreen(const GamePrototype& game) const;
    void DrawModeSelectScreen(const GamePrototype& game) const;
    void DrawFlowOverlay(const GamePrototype& game) const; // gợi ý tutorial + thẻ Hell

private:
    BattleHud battleHud_; // 戦闘HUD
};
