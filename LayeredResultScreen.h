#pragma once

struct ResultStats;

// Images and layout only. ResultScreen owns input, timing, scoring and actions.
class LayeredResultScreen
{
public:
    void Init();
    void Shutdown();
    void Enter();
    bool Ready(bool victory, bool japanese) const;
    int ButtonAt(int x, int y, int selected, bool held, int pressed, bool japanese) const;
    bool Draw(const ResultStats& stats, float timer, int selected, bool held, int pressed, int cursor) const;

private:
    struct Rect { int x, y, w, h; };
    struct Sprite { int handle = -1; Rect source = { 0, 0, 0, 0 }; };
    struct ValueLayout {
        int x = 1000, width = 600;
        int rows[5] = { 326, 460, 590, 725, 857 };
    };
    void Load(Sprite& sprite, const wchar_t* name);
    void ReadLayout();
    static Rect Fit(const Sprite& sprite, Rect box);
    static Rect Scale(Rect box, float scale);
    static bool Contains(Rect box, int x, int y);
    static void DrawPart(const Sprite& sprite, Rect box);
    Rect ButtonRect(int choice, int selected, bool held, int pressed, bool japanese) const;
    int scenery_ = -1;
    Sprite fish_[2];
    Sprite headers_[2][2]; // [loss/victory][locale]
    Sprite boards_[2];
    Sprite buttons_[2][2]; // [retry/title][locale]
    Rect headerBox_ = { 80, 45, 710, 235 };
    Rect fishBox_ = { 130, 330, 620, 440 };
    Rect boardBox_ = { 835, 95, 715, 520 };
    Rect buttonBoxes_[2] = { { 875, 640, 660, 115 }, { 875, 770, 660, 115 } };
    ValueLayout values_[2];
    int modeX_ = 440, modeY_ = 300;
    int notesX_ = 440, notesY_ = 810;
    int selectedLift_ = 4;
    float selectedScale_ = 1.025f, pressScale_ = 0.97f;
    float numberScale_ = 1.5f, scoreScale_ = 1.75f;
};
