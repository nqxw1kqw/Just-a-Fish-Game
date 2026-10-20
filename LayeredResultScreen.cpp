#include "LayeredResultScreen.h"
#include "ResultScreen.h"
#include "Consts.h"
#include "Localization.h"
#include "MenuStyle.h"
#include <algorithm>
#include <cmath>
#include <cwchar>

namespace
{
    const wchar_t* const kRoot = L"Data/UI/Results/Layered-v1/";
    void ConfigPath(const wchar_t* file, wchar_t (&path)[MAX_PATH])
    {
        wchar_t relative[MAX_PATH];
        swprintf_s(relative, L"%s%s", kRoot, file);
        GetFullPathNameW(relative, MAX_PATH, path, nullptr);
    }
    float Ease(float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
    }
    void Note(int cx, int y, const wchar_t* text, unsigned int color)
    {
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (dx || dy) MenuStyle::Center(cx + dx, y + dy, text, GetColor(255, 248, 221), FontSize::Body, 1.0f, 740);
        MenuStyle::Center(cx, y, text, color, FontSize::Body, 1.0f, 740);
    }
}

void LayeredResultScreen::Load(Sprite& sprite, const wchar_t* name)
{
    wchar_t path[MAX_PATH];
    swprintf_s(path, L"%s%s.png", kRoot, name);
    sprite.handle = LoadGraph(path);
    if (sprite.handle == -1)
    {
        AppLogAdd(L"[RESULT_UI] Missing layer %s; using the previous result screen.\n", path);
        return;
    }
    int width = 0, height = 0;
    GetGraphSize(sprite.handle, &width, &height);
    sprite.source = { 0, 0, width, height };
    wchar_t bounds[MAX_PATH];
    ConfigPath(L"asset-bounds.ini", bounds);
    if (static_cast<int>(GetPrivateProfileIntW(name, L"Width", width, bounds)) != width ||
        static_cast<int>(GetPrivateProfileIntW(name, L"Height", height, bounds)) != height) return;
    Rect crop = {
        static_cast<int>(GetPrivateProfileIntW(name, L"X", 0, bounds)),
        static_cast<int>(GetPrivateProfileIntW(name, L"Y", 0, bounds)),
        static_cast<int>(GetPrivateProfileIntW(name, L"W", width, bounds)),
        static_cast<int>(GetPrivateProfileIntW(name, L"H", height, bounds))
    };
    if (crop.x >= 0 && crop.y >= 0 && crop.w > 0 && crop.h > 0 &&
        crop.x + crop.w <= width && crop.y + crop.h <= height) sprite.source = crop;
}

void LayeredResultScreen::Init()
{
    Shutdown();
    scenery_ = LoadGraph(L"Data/UI/ChoiceMenus/Layered-v1/background.png");
    Load(fish_[0], L"fish-lose"); Load(fish_[1], L"fish-win");
    Load(headers_[0][1], L"header-lose-ja");
    Load(headers_[1][1], L"header-win-ja");
    Load(boards_[1], L"board-stage-ja");
    Load(buttons_[0][1], L"button-retry-ja");
    Load(buttons_[1][1], L"button-title-ja");
    ReadLayout();
}

void LayeredResultScreen::Shutdown()
{
    if (scenery_ != -1) DeleteGraph(scenery_);
    scenery_ = -1;
    const auto release = [](Sprite& sprite) {
        if (sprite.handle != -1) DeleteGraph(sprite.handle);
        sprite = Sprite{};
    };
    for (Sprite& fish : fish_) release(fish);
    for (auto& outcome : headers_) for (Sprite& header : outcome) release(header);
    for (Sprite& board : boards_) release(board);
    for (auto& action : buttons_) for (Sprite& button : action) release(button);
}

void LayeredResultScreen::Enter() { ReadLayout(); }

void LayeredResultScreen::ReadLayout()
{
    wchar_t path[MAX_PATH];
    ConfigPath(L"layout.ini", path);
    const auto value = [&](const wchar_t* section, const wchar_t* key, int fallback) {
        return static_cast<int>(GetPrivateProfileIntW(section, key, fallback, path));
    };
    const auto rect = [&](const wchar_t* prefix, Rect fallback) {
        const wchar_t* suffix[4] = { L"X", L"Y", L"Width", L"Height" };
        int* fields[4] = { &fallback.x, &fallback.y, &fallback.w, &fallback.h };
        for (int i = 0; i < 4; ++i)
        {
            wchar_t key[48]; swprintf_s(key, L"%s%s", prefix, suffix[i]);
            *fields[i] = value(L"Result", key, *fields[i]);
        }
        fallback.w = std::clamp(fallback.w, 32, GameConsts::SCREEN_WIDTH);
        fallback.h = std::clamp(fallback.h, 32, GameConsts::SCREEN_HEIGHT);
        fallback.x = std::clamp(fallback.x, 0, GameConsts::SCREEN_WIDTH - fallback.w);
        fallback.y = std::clamp(fallback.y, 0, GameConsts::SCREEN_HEIGHT - fallback.h);
        return fallback;
    };
    headerBox_ = rect(L"Header", { 80, 45, 710, 235 });
    fishBox_ = rect(L"Fish", { 130, 330, 620, 440 });
    boardBox_ = rect(L"Board", { 835, 95, 715, 520 });
    buttonBoxes_[0] = rect(L"Retry", { 875, 640, 660, 115 });
    buttonBoxes_[1] = rect(L"Title", { 875, 770, 660, 115 });
    selectedLift_ = std::clamp(value(L"Result", L"SelectedLift", 4), 0, 20);
    selectedScale_ = std::clamp(value(L"Result", L"SelectedScalePercent", 102), 100, 110) / 100.0f;
    pressScale_ = std::clamp(value(L"Result", L"PressScalePercent", 97), 90, 100) / 100.0f;
    numberScale_ = std::clamp(value(L"Result", L"NumberScalePercent", 150), 50, 300) / 100.0f;
    scoreScale_ = std::clamp(value(L"Result", L"ScoreScalePercent", 175), 50, 300) / 100.0f;
    modeX_ = std::clamp(value(L"Result", L"ModeX", 440), 0, GameConsts::SCREEN_WIDTH);
    modeY_ = std::clamp(value(L"Result", L"ModeY", 300), 0, GameConsts::SCREEN_HEIGHT - 20);
    notesX_ = std::clamp(value(L"Result", L"NotesX", 440), 0, GameConsts::SCREEN_WIDTH);
    notesY_ = std::clamp(value(L"Result", L"NotesY", 810), 30, GameConsts::SCREEN_HEIGHT - 80);
    for (int locale = 1; locale < 2; ++locale)
    {
        const wchar_t* section = L"ValuesJP";
        values_[locale] = ValueLayout{};
        values_[locale].x = value(section, L"X", 1000);
        values_[locale].width = std::clamp(value(section, L"Width", 600), 32, 2000);
        for (int row = 0; row < 5; ++row)
        {
            wchar_t key[20]; swprintf_s(key, L"Row%d", row);
            values_[locale].rows[row] = value(section, key, values_[locale].rows[row]);
        }
    }
}

bool LayeredResultScreen::Ready(bool victory, bool japanese) const
{
    const int outcome = victory ? 1 : 0, locale = japanese ? 1 : 0;
    return scenery_ != -1 && fish_[outcome].handle != -1 && headers_[outcome][locale].handle != -1 &&
           boards_[locale].handle != -1 && buttons_[0][locale].handle != -1 && buttons_[1][locale].handle != -1;
}

LayeredResultScreen::Rect LayeredResultScreen::Fit(const Sprite& sprite, Rect box)
{
    const double scale = (std::min)(static_cast<double>(box.w) / sprite.source.w,
                                    static_cast<double>(box.h) / sprite.source.h);
    const int w = (std::max)(1, static_cast<int>(std::lround(sprite.source.w * scale)));
    const int h = (std::max)(1, static_cast<int>(std::lround(sprite.source.h * scale)));
    return { box.x + (box.w - w) / 2, box.y + (box.h - h) / 2, w, h };
}

LayeredResultScreen::Rect LayeredResultScreen::Scale(Rect box, float scale)
{
    const int w = (std::max)(1, static_cast<int>(std::lround(box.w * scale)));
    const int h = (std::max)(1, static_cast<int>(std::lround(box.h * scale)));
    return { box.x + (box.w - w) / 2, box.y + (box.h - h) / 2, w, h };
}

bool LayeredResultScreen::Contains(Rect box, int x, int y)
{
    return x >= box.x && y >= box.y && x < box.x + box.w && y < box.y + box.h;
}

void LayeredResultScreen::DrawPart(const Sprite& sprite, Rect box)
{
    const Rect src = sprite.source;
    DrawRectExtendGraph(box.x, box.y, box.x + box.w, box.y + box.h, src.x, src.y, src.w, src.h, sprite.handle, TRUE);
}

LayeredResultScreen::Rect LayeredResultScreen::ButtonRect(int choice, int selected, bool held, int pressed, bool japanese) const
{
    Rect box = buttonBoxes_[choice];
    if (choice == selected) { box = Scale(box, selectedScale_); box.y -= selectedLift_; }
    if (held && pressed == choice) box = Scale(box, pressScale_);
    return Fit(buttons_[choice][japanese ? 1 : 0], box);
}

int LayeredResultScreen::ButtonAt(int x, int y, int selected, bool held, int pressed, bool japanese) const
{
    for (int i = 0; i < 2; ++i)
        if (Contains(ButtonRect(i, selected, held, pressed, japanese), x, y)) return i;
    return -1;
}

bool LayeredResultScreen::Draw(const ResultStats& s, float timer, int selected, bool held, int pressed, int cursor) const
{
    const bool japanese = true;
    if (!Ready(s.victory, japanese)) return false;
    const MenuStyle::DrawState drawState;
    const int locale = japanese ? 1 : 0, outcome = s.victory ? 1 : 0;
    const unsigned int ink = GetColor(15, 35, 64);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    DrawExtendGraph(0, 0, GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, scenery_, FALSE);
    DrawPart(headers_[outcome][locale], Fit(headers_[outcome][locale], headerBox_));
    Rect fish = Fit(fish_[outcome], fishBox_);
    if (s.victory) fish.y += static_cast<int>(std::sin(timer * 3.0f) * 3.0f);
    DrawPart(fish_[outcome], fish);
    const Sprite& board = boards_[locale];
    const Rect boardRect = Fit(board, boardBox_);
    DrawPart(board, boardRect);

    const float count = Ease((timer - 0.45f) / 0.85f);
    wchar_t values[5][64] = {};
    swprintf_s(values[0], L"%d", static_cast<int>(ResultScreen::ComputeScore(s) * count + 0.5f));
    const int seconds = static_cast<int>(s.time);
    swprintf_s(values[1], L"%02d:%02d", seconds / 60, seconds % 60);
    swprintf_s(values[2], L"%d", static_cast<int>(s.kills * count + 0.5f));
    if (s.campaign) swprintf_s(values[3], L"%d / 5", s.stage);
    else swprintf_s(values[3], L"練習");
    swprintf_s(values[4], L"%d / %d", s.wave, s.campaign ? GameConsts::CAMPAIGN_WAVES : GameConsts::TUTORIAL_WAVES);
    const float sx = static_cast<float>(boardRect.w) / board.source.w;
    const float sy = static_cast<float>(boardRect.h) / board.source.h;
    const int valueX = boardRect.x + static_cast<int>((values_[locale].x - board.source.x) * sx);
    for (int row = 0; row < 5; ++row)
    {
        const float scale = (row == 0 ? scoreScale_ : numberScale_) * boardRect.w / 715.0f;
        const int centerY = boardRect.y + static_cast<int>((values_[locale].rows[row] - board.source.y) * sy);
        const int height = static_cast<int>(GetFontSizeToHandle(MenuStyle::Font(FontSize::Header)) * scale);
        MenuStyle::Center(valueX, centerY - height / 2, values[row], ink, FontSize::Header, scale,
                          (std::max)(32, static_cast<int>(values_[locale].width * sx)));
    }

    const wchar_t* mode = s.campaign ? TR(L"冒険") : TR(L"チュートリアル");
    wchar_t journey[80];
    swprintf_s(journey, L"%s　ボス撃破 %d / %d", mode, s.bossesDefeated, GameConsts::CAMPAIGN_BOSSES);
    Note(modeX_, modeY_, journey, ink);
    if (s.retryTutorial) Note(notesX_, notesY_, TR(L"チュートリアルから再挑戦"), ink);
    if (s.campaign)
    {
        wchar_t best[128];
        const int bestSeconds = static_cast<int>(s.bestTime);
        swprintf_s(best, TR(L"ベスト: ステージ %d  |  撃破 %d  |  %02d:%02d  |  スコア %d"),
                   s.bestWave, s.bestKills, bestSeconds / 60, bestSeconds % 60, s.bestScore);
        Note(notesX_, notesY_ + 30, best, ink);
    }
    Note(notesX_, notesY_ + 60, TR(L"R: リトライ   T: タイトルへ"), ink);
    if (s.debugUsed) Note(notesX_, notesY_ - 30, s.campaign ? TR(L"[デバッグ使用 ― 記録は保存されません]") : TR(L"[デバッグ使用]"), GetColor(151, 39, 44));

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * Ease((timer - 0.6f) / 0.2f)));
    for (int i : { 1 - selected, selected }) DrawPart(buttons_[i][locale], ButtonRect(i, selected, held, pressed, japanese));
    const Rect active = ButtonRect(selected, selected, held, pressed, japanese);
    if (cursor != -1)
    {
        const int bob = static_cast<int>(std::sin(timer * 5.0f) * 4.0f);
        DrawExtendGraph(active.x - 145 + bob, active.y + active.h / 2 - 72,
                        active.x - 1 + bob, active.y + active.h / 2 + 72, cursor, TRUE);
    }
    else
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 45);
        DrawPart(buttons_[selected][locale], active);
    }
    const float fade = Ease(timer / 0.45f);
    if (fade < 1.0f)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, static_cast<int>(255 * (1.0f - fade)));
        DrawBox(0, 0, GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, GetColor(7, 19, 31), TRUE);
    }
    return true;
}
