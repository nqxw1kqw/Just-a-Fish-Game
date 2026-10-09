#include <Windows.h>
#include <crtdbg.h>
#include <atomic>
#include <new>
#include "DxLib.h"
#include "Consts.h"
#include "GamePrototype.h"
#include "FontManager.h"
#include "SoundManager.h"

#ifdef _DEBUG
std::atomic<size_t> g_gameplayAllocCount{ 0 };
bool g_trackGameplayAlloc = false;

void* operator new(size_t size)
{
    if (g_trackGameplayAlloc)
    {
        g_gameplayAllocCount++;
    }
    void* p = malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete(void* p) noexcept
{
    free(p);
}

void* operator new[](size_t size)
{
    if (g_trackGameplayAlloc)
    {
        g_gameplayAllocCount++;
    }
    void* p = malloc(size);
    if (!p) throw std::bad_alloc();
    return p;
}

void operator delete[](void* p) noexcept
{
    free(p);
}
#endif

int WINAPI WinMain(
    _In_ HINSTANCE,
    _In_opt_ HINSTANCE,
    _In_ LPSTR,
    _In_ int)
{
    // Kiểm tra rò rỉ bộ nhớ CRT trong môi trường Debug
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Cấu hình DxLib
    ChangeWindowMode(TRUE);
    SetMainWindowText(L"Odoru - Just a Fish Game");
    SetGraphMode(GameConsts::SCREEN_WIDTH, GameConsts::SCREEN_HEIGHT, 32);
    SetBackgroundColor(16, 24, 38);

    if (DxLib_Init() == -1)
    {
        return -1;
    }

    // Khởi tạo Font tùy chỉnh (Yabikoma)
    FontManager::Init();

    // Khởi tạo Module Âm thanh (SoundManager)
    SoundManager::Instance().Init();

    SetDrawScreen(DX_SCREEN_BACK);

    // Bật Z-buffer cho đồ họa 3D theo mục 3.10 của fish game.md
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetZBufferBitDepth(24);

    // Cấu hình ánh sáng toàn cục
    SetUseLighting(TRUE);
    SetGlobalAmbientLight(GetColorF(0.65f, 0.65f, 0.72f, 1.0f));
    CreateDirLightHandle(VGet(0.5f, -0.8f, 0.4f));

    // Khởi tạo Prototype Game
    GamePrototype game;
    game.Init();

#ifdef _DEBUG
    // Bắt đầu theo dõi cấp phát bộ nhớ động trong vòng lặp game loop (B3)
    g_trackGameplayAlloc = true;
#endif

    // Vòng lặp chính: Fixed Timestep 120Hz theo mục 3.2 của fish game.md
    constexpr float kStep = GameConsts::FIXED_TIMESTEP;
    float acc = 0.0f;
    LONGLONG prev = GetNowHiPerformanceCount();
    float timeScale = 1.0f;

    while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
    {
        LONGLONG now = GetNowHiPerformanceCount();
        float frame = static_cast<float>(now - prev) / 1000000.0f;
        prev = now;

        // Chặn spiral of death khi bị lag/delay
        if (frame > 0.1f) frame = 0.1f;
        acc += frame * timeScale;

        // Cập nhật logic với bước thời gian cố định
        while (acc >= kStep)
        {
            game.Update(kStep);
            acc -= kStep;
        }

        // Kết xuất hình ảnh
        ClearDrawScreen();
        game.Draw();
        ScreenFlip();
    }

    game.Shutdown();
    SoundManager::Instance().Shutdown();
    FontManager::Shutdown();
    DxLib_End();
    return 0;
}
