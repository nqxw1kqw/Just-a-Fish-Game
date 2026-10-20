#include "PauseMenuScreen.h"
#include "Consts.h"
#include "Localization.h"
#include "MenuStyle.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    const wchar_t* const kRoot = L"Data/UI/Pause/Layered-v1/";
    void Path(const wchar_t* file, wchar_t (&out)[MAX_PATH])
    {
        wchar_t relative[MAX_PATH]; swprintf_s(relative, L"%s%s", kRoot, file);
        GetFullPathNameW(relative, MAX_PATH, out, nullptr);
    }
}

void PauseMenuScreen::Load(Sprite& sprite, const wchar_t* name, const wchar_t* sharedPath)
{
    wchar_t path[MAX_PATH]; swprintf_s(path, L"%s%s.png", kRoot, name);
    sprite.handle = LoadGraph(sharedPath ? sharedPath : path);
    if (sprite.handle == -1) { AppLogAdd(L"[PAUSE_UI] Missing %s; using previous menu.\n", sharedPath ? sharedPath : path); return; }
    int w = 0, h = 0; GetGraphSize(sprite.handle, &w, &h);
    sprite.src = { 0, 0, w, h };
    // Shared art uses the bounds stored beside its own PNG.
    wchar_t bounds[MAX_PATH]; Path(L"asset-bounds.ini", bounds);
    if (sharedPath)
        GetFullPathNameW(L"Data/UI/Results/Layered-v1/asset-bounds.ini", MAX_PATH, bounds, nullptr);
    if (static_cast<int>(GetPrivateProfileIntW(name, L"Width", w, bounds)) != w ||
        static_cast<int>(GetPrivateProfileIntW(name, L"Height", h, bounds)) != h) return;
    Rect src = { static_cast<int>(GetPrivateProfileIntW(name, L"X", 0, bounds)),
                 static_cast<int>(GetPrivateProfileIntW(name, L"Y", 0, bounds)),
                 static_cast<int>(GetPrivateProfileIntW(name, L"W", w, bounds)),
                 static_cast<int>(GetPrivateProfileIntW(name, L"H", h, bounds)) };
    if (src.x >= 0 && src.y >= 0 && src.w > 0 && src.h > 0 && src.x + src.w <= w && src.y + src.h <= h)
        sprite.src = src;
}

void PauseMenuScreen::Init()
{
    Shutdown();
    Load(panel_, L"panel"); Load(headers_[0], L"header-en"); Load(headers_[1], L"header-ja");
    Load(resume_[0], L"resume-en"); Load(resume_[1], L"resume-ja");
    Load(title_[0], L"button-title-en", L"Data/UI/Results/Layered-v1/button-title-en.png");
    Load(title_[1], L"button-title-ja", L"Data/UI/Results/Layered-v1/button-title-ja.png");
    Load(rows_[0], L"row-bgm"); Load(rows_[1], L"row-sfx"); Load(rows_[2], L"row-display");
    Load(modes_[0][0], L"window-en"); Load(modes_[0][1], L"fullscreen-en");
    Load(modes_[1][0], L"window-ja"); Load(modes_[1][1], L"fullscreen-ja");
    Load(track_, L"slider-track"); Load(knob_, L"slider-knob");
    Load(cursor_, L"selection-arrow", L"Data/UI/Results/Illustrated-v1/selection-arrow.png");
    ReadLayout();
}

void PauseMenuScreen::Shutdown()
{
    Leave();
    const auto free = [](Sprite& s) { if (s.handle != -1) DeleteGraph(s.handle); s = Sprite{}; };
    free(panel_); for (auto& s : headers_) free(s); for (auto& s : resume_) free(s);
    for (auto& s : title_) free(s); for (auto& s : rows_) free(s);
    for (auto& locale : modes_) for (auto& s : locale) free(s);
    free(track_); free(knob_); free(cursor_);
}

void PauseMenuScreen::ReadLayout()
{
    wchar_t path[MAX_PATH]; Path(L"layout.ini", path);
    const auto value = [&](const wchar_t* key, int fallback) {
        return static_cast<int>(GetPrivateProfileIntW(L"Pause", key, fallback, path));
    };
    const auto rect = [&](const wchar_t* prefix, Rect r) {
        const wchar_t* suffix[] = { L"X", L"Y", L"Width", L"Height" };
        int* fields[] = { &r.x, &r.y, &r.w, &r.h };
        for (int i = 0; i < 4; ++i) {
            wchar_t key[48]; swprintf_s(key, L"%s%s", prefix, suffix[i]); *fields[i] = value(key, *fields[i]);
        }
        r.w = std::clamp(r.w, 32, GameConsts::SCREEN_WIDTH); r.h = std::clamp(r.h, 32, GameConsts::SCREEN_HEIGHT);
        r.x = std::clamp(r.x, 0, GameConsts::SCREEN_WIDTH - r.w); r.y = std::clamp(r.y, 0, GameConsts::SCREEN_HEIGHT - r.h);
        return r;
    };
    panelBox_ = rect(L"Panel", { 350, 25, 900, 850 }); headerBox_ = rect(L"Header", { 510, 65, 580, 115 });
    const wchar_t* names[] = { L"Resume", L"Bgm", L"Sfx", L"Display", L"Title" };
    const Rect defaults[] = { {480,210,640,100}, {470,335,660,95}, {470,445,660,95}, {470,555,660,95}, {480,675,640,100} };
    for (int i = 0; i < 5; ++i) itemBoxes_[i] = rect(names[i], defaults[i]);
    helpY_ = std::clamp(value(L"HelpY", 790), 0, 860);
    selectedLift_ = std::clamp(value(L"SelectedLift", 3), 0, 15);
    selectedScale_ = std::clamp(value(L"SelectedScalePercent", 103), 100, 110) / 100.0f;
    pressScale_ = std::clamp(value(L"PressScalePercent", 97), 90, 100) / 100.0f;
    trackStart_ = std::clamp(value(L"TrackStartPercent", 43), 30, 60);
    trackEnd_ = std::clamp(value(L"TrackEndPercent", 78), trackStart_ + 10, 85);
    valueX_ = std::clamp(value(L"ValueCenterPercent", 89), 86, 95);
}

void PauseMenuScreen::Enter()
{
    if (!ownsCursor_) previousCursorVisible_ = GetMouseDispFlag();
    ownsCursor_ = true;
    SetMouseDispFlag(TRUE);
    ReadLayout(); GetMousePoint(&mouseX_, &mouseY_);
    mousePrev_ = true; mouseReady_ = false; pressed_ = dragging_ = -1; enteredAt_ = GetNowCount();
}

void PauseMenuScreen::Leave()
{
    if (ownsCursor_) SetMouseDispFlag(previousCursorVisible_);
    ownsCursor_ = false;
    pressed_ = dragging_ = -1;
}

bool PauseMenuScreen::Ready() const
{
    const int locale = L10N::Get() == Language::Japanese ? 1 : 0;
    return panel_.handle != -1 && headers_[locale].handle != -1 && resume_[locale].handle != -1 &&
        title_[locale].handle != -1 && rows_[0].handle != -1 && rows_[1].handle != -1 && rows_[2].handle != -1 &&
        modes_[locale][0].handle != -1 && modes_[locale][1].handle != -1 && track_.handle != -1 && knob_.handle != -1;
}

PauseMenuScreen::Rect PauseMenuScreen::Fit(const Sprite& s, Rect box)
{
    const double scale = (std::min)(static_cast<double>(box.w) / s.src.w, static_cast<double>(box.h) / s.src.h);
    const int w = (std::max)(1, static_cast<int>(std::lround(s.src.w * scale)));
    const int h = (std::max)(1, static_cast<int>(std::lround(s.src.h * scale)));
    return { box.x + (box.w - w) / 2, box.y + (box.h - h) / 2, w, h };
}
PauseMenuScreen::Rect PauseMenuScreen::Scale(Rect r, float scale)
{
    const int w = static_cast<int>(std::lround(r.w * scale)), h = static_cast<int>(std::lround(r.h * scale));
    return { r.x + (r.w - w) / 2, r.y + (r.h - h) / 2, w, h };
}
bool PauseMenuScreen::Contains(Rect r, int x, int y) { return x >= r.x && y >= r.y && x < r.x + r.w && y < r.y + r.h; }
void PauseMenuScreen::Part(const Sprite& s, Rect r)
{
    DrawRectExtendGraph(r.x, r.y, r.x + r.w, r.y + r.h, s.src.x, s.src.y, s.src.w, s.src.h, s.handle, TRUE);
}
const PauseMenuScreen::Sprite& PauseMenuScreen::Item(int i) const
{
    const int locale = L10N::Get() == Language::Japanese ? 1 : 0;
    return i == 0 ? resume_[locale] : i == 4 ? title_[locale] : rows_[i - 1];
}
PauseMenuScreen::Rect PauseMenuScreen::ItemRect(int i, int selected) const
{
    Rect box = itemBoxes_[i];
    if (i == selected) { box = Scale(box, selectedScale_); box.y -= selectedLift_; }
    if (mousePrev_ && pressed_ == i && dragging_ < 0) box = Scale(box, pressScale_);
    return Fit(Item(i), box);
}
PauseMenuScreen::Rect PauseMenuScreen::TrackRect(Rect row) const
{
    Rect box = { row.x + row.w * trackStart_ / 100, row.y + row.h / 2 - 10, row.w * (trackEnd_ - trackStart_) / 100, 20 };
    return Fit(track_, box);
}
PauseMenuScreen::Rect PauseMenuScreen::SliderHitRect(Rect row) const
{
    const Rect track = TrackRect(row);
    return { track.x - 20, row.y, track.w + 40, row.h };
}
int PauseMenuScreen::Hit(int x, int y, int selected) const
{
    for (int i = 0; i < 5; ++i) if (Contains(ItemRect(i, selected), x, y)) return i;
    return -1;
}

PauseMenuScreen::PointerResult PauseMenuScreen::UpdatePointer(int& selected)
{
    PointerResult result;
    if (ownsCursor_) SetMouseDispFlag(TRUE);
    if (!Ready()) return result;
    int x = mouseX_, y = mouseY_; GetMousePoint(&x, &y);
    const bool down = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    const bool moved = x != mouseX_ || y != mouseY_;
    const bool pressed = down && !mousePrev_, released = !down && mousePrev_;
    mouseX_ = x; mouseY_ = y;
    if (!mouseReady_) {
        mousePrev_ = down;
        if (!down) mouseReady_ = true;
        return result;
    }
    // Hit testing uses the visual transform from the preceding frame.
    const int hovered = Hit(x, y, selected);
    if (dragging_ >= 0) selected = dragging_;
    else if (moved && hovered >= 0) selected = hovered;
    if (pressed) {
        pressed_ = hovered;
        if (hovered >= 0) selected = hovered;
        if ((hovered == 1 || hovered == 2) && Contains(SliderHitRect(ItemRect(hovered, selected)), x, y)) dragging_ = hovered;
    }
    if (dragging_ >= 0 && (down || released)) {
        const Rect track = TrackRect(ItemRect(dragging_, selected));
        result.volumeItem = dragging_;
        result.volume = std::clamp(static_cast<int>(std::lround((x - track.x) * 100.0 / (std::max)(1, track.w))), 0, 100);
        result.volumeReleased = released;
    }
    if (released) {
        if (dragging_ < 0 && pressed_ >= 0 && hovered == pressed_ && (pressed_ == 0 || pressed_ == 3 || pressed_ == 4)) result.activated = pressed_;
        pressed_ = dragging_ = -1;
    }
    mousePrev_ = down;
    return result;
}

bool PauseMenuScreen::Draw(int selected, int bgm, int sfx, bool windowed) const
{
    if (!Ready()) return false;
    selected = std::clamp(selected, 0, 4);
    const MenuStyle::DrawState drawState;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(0, 0, GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, GetColor(6, 14, 26), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    Part(panel_, Fit(panel_, panelBox_));
    const int locale = L10N::Get() == Language::Japanese ? 1 : 0;
    Part(headers_[locale], Fit(headers_[locale], headerBox_));
    const unsigned int ink = GetColor(15, 35, 64);
    for (int i = 0; i < 5; ++i) {
        const Rect row = ItemRect(i, selected);
        Part(Item(i), row);
        if (i == 1 || i == 2) {
            const int volume = std::clamp(i == 1 ? bgm : sfx, 0, 100);
            const Rect track = TrackRect(row);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 60); Part(track_, track);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            const int srcW = track_.src.w * volume / 100, dstW = track.w * volume / 100;
            if (srcW > 0 && dstW > 0)
                DrawRectExtendGraph(track.x, track.y, track.x + dstW, track.y + track.h,
                    track_.src.x, track_.src.y, srcW, track_.src.h, track_.handle, TRUE);
            const int knobSize = (std::max)(28, row.h * 48 / 100);
            const Rect knobBox = { track.x + track.w * volume / 100 - knobSize / 2, row.y + row.h / 2 - knobSize / 2, knobSize, knobSize };
            Part(knob_, Fit(knob_, knobBox));
            wchar_t number[16]; swprintf_s(number, L"%d", volume);
            const float numberScale = 1.05f * row.h / 95.0f;
            const int numberHeight = static_cast<int>(GetFontSizeToHandle(MenuStyle::Font(FontSize::Header)) * numberScale);
            MenuStyle::Center(row.x + row.w * valueX_ / 100, row.y + (row.h - numberHeight) / 2,
                number, ink, FontSize::Header, numberScale, row.w * 12 / 100);
        }
        if (i == 3) {
            const Rect box = { row.x + row.w * 35 / 100, row.y + row.h * 28 / 100, row.w * 56 / 100, row.h * 44 / 100 };
            const Sprite& mode = modes_[locale][windowed ? 0 : 1]; Part(mode, Fit(mode, box));
        }
    }
    if (cursor_.handle != -1) {
        const Rect row = ItemRect(selected, selected);
        const int bob = static_cast<int>(std::sin((GetNowCount() - enteredAt_) * 0.005f) * 3.0f);
        Part(cursor_, { row.x - 135 + bob, row.y + row.h / 2 - 65, 130, 130 });
    }
    MenuStyle::Center(800, helpY_, TR(L"W/S or UP/DOWN: Select   ENTER/SPACE: Confirm"), ink, FontSize::Body, 0.9f, 690);
    MenuStyle::Center(800, helpY_ + 24, TR(L"A/D: Adjust   ESC/P: Resume"), ink, FontSize::Body, 0.9f, 690);
    return true;
}
