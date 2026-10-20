#include "IllustratedChoiceMenu.h"
#include "MenuStyle.h"
#include "Consts.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    // Original canvases: 1672 x 941. Hit regions share the draw conversion.
    int X(int x) { return x * GameConsts::SCREEN_WIDTH / 1672; }
    int Y(int y) { return y * GameConsts::SCREEN_HEIGHT / 941; }
    const wchar_t* const kLayerRoot = L"Data/UI/ChoiceMenus/Layered-v1/";
    void ConfigPath(const wchar_t* file, wchar_t (&path)[MAX_PATH])
    {
        wchar_t relative[MAX_PATH];
        swprintf_s(relative, L"%s%s", kLayerRoot, file);
        GetFullPathNameW(relative, MAX_PATH, path, nullptr);
    }
}

void IllustratedChoiceMenu::LoadPart(Sprite& sprite, const wchar_t* name)
{
    wchar_t path[MAX_PATH];
    swprintf_s(path, L"%s%s.png", kLayerRoot, name);
    sprite.handle = LoadGraph(path);
    if (sprite.handle == -1)
    {
        AppLogAdd(L"[CHOICE_UI] Missing layer %s; using the previous menu.\n", path);
        return;
    }
    int width, height;
    GetGraphSize(sprite.handle, &width, &height);
    sprite.source = { 0, 0, width, height };
    wchar_t boundsPath[MAX_PATH];
    ConfigPath(L"asset-bounds.ini", boundsPath);
    if (static_cast<int>(GetPrivateProfileIntW(name, L"Width", width, boundsPath)) != width ||
        static_cast<int>(GetPrivateProfileIntW(name, L"Height", height, boundsPath)) != height) return;
    Rect crop = {
        static_cast<int>(GetPrivateProfileIntW(name, L"X", 0, boundsPath)),
        static_cast<int>(GetPrivateProfileIntW(name, L"Y", 0, boundsPath)),
        static_cast<int>(GetPrivateProfileIntW(name, L"W", width, boundsPath)),
        static_cast<int>(GetPrivateProfileIntW(name, L"H", height, boundsPath))
    };
    // Trim only transparent padding during rendering; preserve the PNG files.
    if (crop.x >= 0 && crop.y >= 0 && crop.w > 0 && crop.h > 0 &&
        crop.x + crop.w <= width && crop.y + crop.h <= height) sprite.source = crop;
}

void IllustratedChoiceMenu::LoadLayers()
{
    scenery_ = LoadGraph(L"Data/UI/ChoiceMenus/Layered-v1/background.png");
    LoadPart(confirm_, L"confirm-ja");
    LoadPart(header_, L"header-mode-ja");
    LoadPart(cards_[0], L"mode-normal-ja");
    LoadPart(cards_[1], L"mode-hard-ja");
}

void IllustratedChoiceMenu::ReadLayout()
{
    wchar_t path[MAX_PATH];
    ConfigPath(L"layout.ini", path);
    const wchar_t* section = L"Mode";
    const auto read = [&](const wchar_t* prefix, Rect defaults) {
        Rect value = defaults;
        const wchar_t* suffix[4] = { L"X", L"Y", L"Width", L"Height" };
        int* fields[4] = { &value.x, &value.y, &value.w, &value.h };
        for (int i = 0; i < 4; ++i)
        {
            wchar_t key[48];
            swprintf_s(key, L"%s%s", prefix, suffix[i]);
            *fields[i] = static_cast<int>(GetPrivateProfileIntW(section, key, *fields[i], path));
        }
        value.w = std::clamp(value.w, 32, GameConsts::SCREEN_WIDTH);
        value.h = std::clamp(value.h, 32, GameConsts::SCREEN_HEIGHT);
        value.x = std::clamp(value.x, 0, GameConsts::SCREEN_WIDTH - value.w);
        value.y = std::clamp(value.y, 0, GameConsts::SCREEN_HEIGHT - value.h);
        return value;
    };
    headerBox_ = read(L"Header", { 400, 12, 800, 195 });
    cardBoxes_[0] = read(L"Left", { 110, 235, 600, 540 });
    cardBoxes_[1] = read(L"Right", { 890, 235, 600, 540 });
    confirmBox_ = read(L"Confirm", { 520, 790, 560, 110 });
    selectedLift_ = std::clamp(static_cast<int>(GetPrivateProfileIntW(section, L"SelectedLift", 10, path)), 0, 40);
    selectedScale_ = std::clamp(static_cast<int>(GetPrivateProfileIntW(section, L"SelectedScalePercent", 103, path)), 100, 110) / 100.0f;
    pressScale_ = std::clamp(static_cast<int>(GetPrivateProfileIntW(section, L"PressScalePercent", 97, path)), 90, 100) / 100.0f;
}

bool IllustratedChoiceMenu::HasLayers() const
{
    return scenery_ != -1 && confirm_.handle != -1 && header_.handle != -1 &&
           cards_[0].handle != -1 && cards_[1].handle != -1;
}

IllustratedChoiceMenu::Rect IllustratedChoiceMenu::Fit(const Sprite& sprite, Rect box)
{
    const double scale = (std::min)(static_cast<double>(box.w) / sprite.source.w,
                                    static_cast<double>(box.h) / sprite.source.h);
    const int width = (std::max)(1, static_cast<int>(std::lround(sprite.source.w * scale)));
    const int height = (std::max)(1, static_cast<int>(std::lround(sprite.source.h * scale)));
    return { box.x + (box.w - width) / 2, box.y + (box.h - height) / 2, width, height };
}

IllustratedChoiceMenu::Rect IllustratedChoiceMenu::Scale(Rect box, float scale)
{
    const int width = (std::max)(1, static_cast<int>(std::lround(box.w * scale)));
    const int height = (std::max)(1, static_cast<int>(std::lround(box.h * scale)));
    return { box.x + (box.w - width) / 2, box.y + (box.h - height) / 2, width, height };
}

bool IllustratedChoiceMenu::Contains(Rect box, int x, int y)
{
    return x >= box.x && y >= box.y && x < box.x + box.w && y < box.y + box.h;
}

void IllustratedChoiceMenu::DrawPart(const Sprite& sprite, Rect box)
{
    const Rect src = sprite.source;
    DrawRectExtendGraph(box.x, box.y, box.x + box.w, box.y + box.h, src.x, src.y, src.w, src.h, sprite.handle, TRUE);
}

IllustratedChoiceMenu::Rect IllustratedChoiceMenu::CardRect(int choice, int selected) const
{
    Rect box = cardBoxes_[choice];
    if (choice == selected)
    {
        box = Scale(box, selectedScale_);
        const int bob = static_cast<int>(std::sin(static_cast<double>(GetNowCount() % 10000) * 0.004) * 2.0);
        box.y -= selectedLift_ + bob;
    }
    if (mousePrev_ && pressedTarget_ == choice) box = Scale(box, pressScale_);
    return Fit(cards_[choice], box);
}

void IllustratedChoiceMenu::DrawCursor(int cx, int cy) const
{
    if (cursor_ == -1) return;
    int width, height;
    GetGraphSize(cursor_, &width, &height);
    const double scale = 144.0 / (std::max)(1, (std::max)(width, height));
    DrawRotaGraph3(cx, cy, width / 2, height / 2, scale, scale, -1.5707963267948966, cursor_, TRUE);
}

bool IllustratedChoiceMenu::DrawLayers(int selected) const
{
    if (!HasLayers()) return false;
    const MenuStyle::DrawState drawState;
    selected = selected == 1 ? 1 : 0;
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawExtendGraph(0, 0, GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, scenery_, FALSE);
    DrawPart(header_, Fit(header_, headerBox_));
    for (int i : { 1 - selected, selected })
        DrawPart(cards_[i], CardRect(i, selected));
    Rect button = confirmBox_;
    if (mousePrev_ && pressedTarget_ == 2) button = Scale(button, pressScale_);
    DrawPart(confirm_, Fit(confirm_, button));
    const Rect active = CardRect(selected, selected);
    DrawCursor(active.x + active.w / 2, active.y + active.h + 60);
    if (cursor_ == -1)
    {
        // Optional cursor missing: keep the selected sprite visibly brighter.
        SetDrawBlendMode(DX_BLENDMODE_ADD, 35);
        DrawPart(cards_[selected], active);
    }
    return true;
}

void IllustratedChoiceMenu::Init(const wchar_t* japanesePath)
{
    Shutdown();
    background_ = LoadGraph(japanesePath);
    cursor_ = LoadGraph(L"Data/UI/Results/Illustrated-v1/selection-arrow.png");
    LoadLayers();
    if (background_ == -1) AppLogAdd(L"[CHOICE_UI] Missing %s; using the previous menu.\n", japanesePath);
    ResetPointer();
}

void IllustratedChoiceMenu::Shutdown()
{
    if (scenery_ != -1) DeleteGraph(scenery_);
    scenery_ = -1;
    const auto release = [](Sprite& sprite) {
        if (sprite.handle != -1) DeleteGraph(sprite.handle);
        sprite = Sprite{};
    };
    release(header_);
    for (Sprite& card : cards_) release(card);
    release(confirm_);
    if (background_ != -1) DeleteGraph(background_);
    background_ = -1;
    if (cursor_ != -1) DeleteGraph(cursor_);
    cursor_ = -1;
}

int IllustratedChoiceMenu::Background() const
{
    return background_;
}

void IllustratedChoiceMenu::ResetPointer()
{
    ReadLayout();
    GetMousePoint(&mouseX_, &mouseY_);
    mousePrev_ = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    mouseReady_ = false;
    pressedTarget_ = -1;
}

int IllustratedChoiceMenu::PointerTarget(int x, int y, int selected) const
{
    if (HasLayers())
    {
        for (int i = 0; i < 2; ++i)
            if (Contains(CardRect(i, selected), x, y)) return i;
        if (Contains(Fit(confirm_, confirmBox_), x, y)) return 2;
        return -1;
    }
    const int cardBottom = 790;
    if (y >= Y(215) && y <= Y(cardBottom))
    {
        if (x >= X(165) && x <= X(810)) return 0;
        if (x >= X(865) && x <= X(1505)) return 1;
    }
    const int confirmTop = 795;
    if (x >= X(520) && x <= X(1150) && y >= Y(confirmTop) && y <= Y(915)) return 2;
    return -1;
}

bool IllustratedChoiceMenu::UpdatePointer(int& selected)
{
    if (!HasLayers() && Background() == -1) return false;
    int mouseX = mouseX_, mouseY = mouseY_;
    GetMousePoint(&mouseX, &mouseY);
    const bool down = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;
    const bool pressed = down && !mousePrev_;
    const bool released = !down && mousePrev_;
    const bool moved = mouseX != mouseX_ || mouseY != mouseY_;
    mouseX_ = mouseX; mouseY_ = mouseY; mousePrev_ = down;
    const int target = PointerTarget(mouseX, mouseY, selected);
    // An idle mouse must not override a choice made with the keyboard.
    if (moved && target >= 0 && target < 2) selected = target;
    if (!mouseReady_)
    {
        if (!down) mouseReady_ = true;
        return false;
    }
    if (pressed)
    {
        pressedTarget_ = target;
        if (target >= 0 && target < 2) selected = target;
    }
    if (released)
    {
        const int original = pressedTarget_;
        pressedTarget_ = -1;
        if (target >= 0 && target == original)
        {
            if (target < 2) selected = target;
            return true;
        }
    }
    return false;
}

bool IllustratedChoiceMenu::Draw(int selected) const
{
    if (DrawLayers(selected)) return true;
    const int background = Background();
    if (background == -1) return false;
    const MenuStyle::DrawState drawState;
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawExtendGraph(0, 0, GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, background, FALSE);
    selected = selected == 1 ? 1 : 0;
    if (cursor_ != -1)
    {
        int width = 0, height = 0;
        GetGraphSize(cursor_, &width, &height);
        const double scale = 144.0 / (std::max)(1, (std::max)(width, height));
        const int bob = static_cast<int>(std::sin(static_cast<double>(GetNowCount() % 10000) * 0.005) * 4.0);
        // Rotate the existing shell arrow upwards toward the chosen picture card.
        DrawRotaGraph3(X(selected == 0 ? 380 : 1295), Y(865) + bob,
                      width / 2, height / 2, scale, scale, -1.5707963267948966, cursor_, TRUE);
    }
    else
    {
        const int left = selected == 0 ? 165 : 865;
        const int right = selected == 0 ? 810 : 1505;
        const int bottom = 790;
        SetDrawBlendMode(DX_BLENDMODE_ADD, 45);
        DrawRectExtendGraph(X(left), Y(215), X(right), Y(bottom), left, 215, right - left, bottom - 215, background, FALSE);
    }
    return true;
}
