#pragma once

// Artwork and pointer interaction; GamePrototype applies actions and settings.
class PauseMenuScreen
{
public:
    struct PointerResult {
        int activated = -1;
        int volumeItem = -1;
        int volume = 0;
        bool volumeReleased = false;
    };
    void Init();
    void Shutdown();
    void Enter();
    void Leave();
    PointerResult UpdatePointer(int& selected);
    bool Draw(int selected, int bgm, int sfx, bool windowed) const;

private:
    struct Rect { int x, y, w, h; };
    struct Sprite { int handle = -1; Rect src = { 0, 0, 0, 0 }; };
    void Load(Sprite& sprite, const wchar_t* name, const wchar_t* sharedPath = nullptr);
    void ReadLayout();
    bool Ready() const;
    static Rect Fit(const Sprite& sprite, Rect box);
    static Rect Scale(Rect box, float scale);
    static bool Contains(Rect box, int x, int y);
    static void Part(const Sprite& sprite, Rect box);
    const Sprite& Item(int index) const;
    Rect ItemRect(int index, int selected) const;
    Rect TrackRect(Rect row) const;
    Rect SliderHitRect(Rect row) const;
    int Hit(int x, int y, int selected) const;
    Sprite panel_, headers_[2], resume_[2], title_[2], rows_[3];
    Sprite modes_[2][2]; // [English/Japanese][window/fullscreen]
    Sprite track_, knob_, cursor_;
    Rect panelBox_ = { 350, 25, 900, 850 };
    Rect headerBox_ = { 510, 65, 580, 115 };
    Rect itemBoxes_[5] = {
        { 480, 210, 640, 100 }, { 470, 335, 660, 95 }, { 470, 445, 660, 95 },
        { 470, 555, 660, 95 }, { 480, 675, 640, 100 }
    };
    int helpY_ = 790, selectedLift_ = 3;
    float selectedScale_ = 1.025f, pressScale_ = 0.97f;
    int trackStart_ = 43, trackEnd_ = 78, valueX_ = 89;
    int mouseX_ = -1, mouseY_ = -1;
    bool mousePrev_ = true, mouseReady_ = false;
    int pressed_ = -1, dragging_ = -1;
    int enteredAt_ = 0;
    int previousCursorVisible_ = 1;
    bool ownsCursor_ = false;
};
