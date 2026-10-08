#pragma once

class GamePrototype;
class WhaleBoss;

class UIRenderer
{
public:
    void DrawHUD(const GamePrototype& game) const;
    void DrawBossHealthBar(const WhaleBoss& boss) const;
    void DrawBossIntroScreen() const;
    void DrawTitleScreen(const GamePrototype& game) const;
    void DrawVictoryScreen(const GamePrototype& game) const;
    void DrawGameOverScreen(const GamePrototype& game) const;
};
