#pragma once

// Shared image renderer and pointer interaction for the two illustrated choices.
// Keyboard navigation and scene transitions remain in GamePrototype.
class IllustratedChoiceMenu
{
public:
    enum class Layout { Language, Mode };
    void Init(const wchar_t* englishPath, const wchar_t* japanesePath, Layout layout);
    void Shutdown();
    void ResetPointer();
    bool UpdatePointer(int& selected, bool japanese);
    bool Draw(int selected, bool japanese) const;

private:
    struct Rect { int x, y, w, h; };
    struct Sprite { int handle = -1; Rect source = { 0, 0, 0, 0 }; };
    void LoadLayers();
    void LoadPart(Sprite& sprite, const wchar_t* name);
    void ReadLayout();
    bool HasLayers(bool japanese) const;
    bool DrawLayers(int selected, bool japanese) const;
    static Rect Fit(const Sprite& sprite, Rect box);
    static Rect Scale(Rect box, float scale);
    static bool Contains(Rect box, int x, int y);
    static void DrawPart(const Sprite& sprite, Rect box);
    Rect CardRect(int choice, int selected, bool japanese) const;
    void DrawCursor(int cx, int cy) const;
    int Background(bool japanese) const;
    int PointerTarget(int x, int y, int selected, bool japanese) const;
    int scenery_ = -1;
    Sprite headers_[2];
    Sprite cards_[2][2]; // [locale][left / right]
    Sprite confirm_;
    Rect headerBox_ = { 400, 12, 800, 195 };
    Rect cardBoxes_[2] = { { 110, 235, 600, 540 }, { 890, 235, 600, 540 } };
    Rect confirmBox_ = { 520, 790, 560, 110 };
    int selectedLift_ = 10;
    float selectedScale_ = 1.03f, pressScale_ = 0.97f;
    int backgrounds_[2] = { -1, -1 };
    int cursor_ = -1;
    bool bilingual_ = false;
    Layout layout_ = Layout::Language;
    int mouseX_ = -1, mouseY_ = -1;
    bool mousePrev_ = true, mouseReady_ = false;
    int pressedTarget_ = -1;
};
