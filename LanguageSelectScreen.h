#pragma once
#include "IllustratedChoiceMenu.h"

class LanguageSelectScreen
{
public:
    void Init();
    void Shutdown();
    void Draw(int selected) const;
    void Enter() { illustratedMenu_.ResetPointer(); }
    bool UpdatePointer(int& selected) { return illustratedMenu_.UpdatePointer(selected, false); }

private:
    IllustratedChoiceMenu illustratedMenu_;
    int artwork_[2] = { -1, -1 };
};
