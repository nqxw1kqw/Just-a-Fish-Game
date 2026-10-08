#pragma once
#include <string>

class GamePrototype;

class DebugController
{
public:
    void Reset();
    void Update(float dt, GamePrototype& game);
    void DrawHUD(const GamePrototype& game) const;

    bool IsDebugHUDVisible() const { return showDebugHUD_; }
    void SetDebugHUDVisible(bool visible) { showDebugHUD_ = visible; }
    bool WasDebugUsed() const { return debugUsed_; }
    void MarkDebugUsed() { debugUsed_ = true; }
    int GetDebugEnemyTypeIdx() const { return debugEnemyTypeIdx_; }

private:
    bool showDebugHUD_ = true;
    bool debugUsed_ = false;
    int debugEnemyTypeIdx_ = 1;
};
