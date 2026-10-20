#pragma once
#include "BattleHud.h"
#include "ModeSelectScreen.h"
#include "PauseMenuScreen.h"

class GamePrototype;
class WhaleBoss;
class TurtleBoss;
class SwordfishBoss;

class UIRenderer
{
public:
    void Init();
    void Shutdown();
    void EnterModeSelect() { modeSelectScreen_.Enter(); }
    void EnterPause() { pauseMenuScreen_.Enter(); }
    void LeavePause() { pauseMenuScreen_.Leave(); }
    PauseMenuScreen::PointerResult UpdatePausePointer(int& selected) { return pauseMenuScreen_.UpdatePointer(selected); }
    bool UpdateModePointer(int& selected) { return modeSelectScreen_.UpdatePointer(selected); }
    void DrawHUD(const GamePrototype& game) const;
    void DrawBossHealthBar(const WhaleBoss& boss) const;
    void DrawBossHealthBar(const SharkBoss& shark) const;
    void DrawBossHealthBar(const SquidBoss& squid) const;
    void DrawBossHealthBar(const TurtleBoss& turtle) const;
    void DrawBossHealthBar(const SwordfishBoss& swordfish) const;
    void DrawBossIntroScreen(int bossKind = 0) const; // 0 Whale, 1 Shark, 2 Squid, 3 Turtle, 4 Swordfish
    void DrawTitleScreen(const GamePrototype& game) const;
    void DrawVictoryScreen(const GamePrototype& game) const;
    void DrawGameOverScreen(const GamePrototype& game) const;
    void DrawPauseScreen(const GamePrototype& game) const;
    void DrawModeSelectScreen(const GamePrototype& game) const;
    void DrawFlowOverlay(const GamePrototype& game) const; // gợi ý tutorial + thẻ Hell

private:
    int titleArtwork_ = -1;
    ModeSelectScreen modeSelectScreen_;
    PauseMenuScreen pauseMenuScreen_;
    BattleHud battleHud_; // 戦闘HUD
};
