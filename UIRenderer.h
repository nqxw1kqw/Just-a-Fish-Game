#pragma once
#include "BattleHud.h"
#include "ModeSelectScreen.h"

class GamePrototype;
class WhaleBoss;
class TurtleBoss;
class SwordfishBoss;

class UIRenderer
{
public:
    void Init();
    void Shutdown();
    void DrawHUD(const GamePrototype& game) const;
    void DrawBossHealthBar(const WhaleBoss& boss) const;
    void DrawBossHealthBar(const SharkBoss& shark) const;
    void DrawBossHealthBar(const SquidBoss& squid) const;
    void DrawBossHealthBar(const TurtleBoss& turtle) const;
    void DrawBossHealthBar(const SwordfishBoss& swordfish) const;
    void DrawBossIntroScreen(int bossKind = 0) const; // 0 Whale, 1 Shark, 2 Squid, 3 Turtle, 4 Swordfish
    void DrawLanguageSelectScreen(const GamePrototype& game) const;
    void DrawTitleScreen(const GamePrototype& game) const;
    void DrawVictoryScreen(const GamePrototype& game) const;
    void DrawGameOverScreen(const GamePrototype& game) const;
    void DrawPauseScreen(const GamePrototype& game) const;
    void DrawModeSelectScreen(const GamePrototype& game) const;
    void DrawFlowOverlay(const GamePrototype& game) const; // gợi ý tutorial + thẻ Hell

private:
    ModeSelectScreen modeSelectScreen_;
    BattleHud battleHud_; // 戦闘HUD
};
