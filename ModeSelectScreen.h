#pragma once
#include "IllustratedChoiceMenu.h"

// Artwork and layout for the post-tutorial choice; gameplay stays in GamePrototype.
class ModeSelectScreen
{
public:
    void Init();
    void Shutdown();
    void Draw(int selected) const;
    void Enter() { illustratedMenu_.ResetPointer(); }
    bool UpdatePointer(int& selected);

private:
    IllustratedChoiceMenu illustratedMenu_;
};
