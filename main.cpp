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

// Trạng thái vẽ 3D/2D. ChangeWindowMode() làm DxLib reset hệ thống đồ hoạ: ảnh/model/font tạo từ file
// được DxLib tự khôi phục, nhưng các thiết lập vẽ có thể về mặc định, nên phải áp dụng lại ở đây.
static void ApplyRenderSettings(int& dirLightHandle)
{
    SetDrawScreen(DX_SCREEN_BACK);
    SetBackgroundColor(16, 24, 38);

    // Bật Z-buffer cho đồ họa 3D theo mục 3.10 của fish game.md
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    SetZBufferBitDepth(24);

    // Cấu hình ánh sáng toàn cục
    SetUseLighting(TRUE);
    SetGlobalAmbientLight(GetColorF(0.65f, 0.65f, 0.72f, 1.0f));
    if (dirLightHandle != -1)
    {
        DeleteLightHandle(dirLightHandle);
    }
    dirLightHandle = CreateDirLightHandle(VGet(0.5f, -0.8f, 0.4f));
}

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

    // Thiết lập trạng thái vẽ (gọi lại sau khi đổi Window/Fullscreen vì DxLib reset hệ thống đồ hoạ)
    int dirLightHandle = -1;
    ApplyRenderSettings(dirLightHandle);

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

    // ESC: tạm dừng khi đang chơi, thoát game ở các màn khác (xử lý trong GamePrototype)
    while (ProcessMessage() == 0 && !game.IsQuitRequested())
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

        // Đổi Window/Fullscreen từ màn hình tạm dừng (Task #13)
        if (game.ConsumeWindowModeToggleRequest())
        {
            BOOL toWindow = (GetWindowModeFlag() == TRUE) ? FALSE : TRUE;
            if (ChangeWindowMode(toWindow) != DX_CHANGESCREEN_OK)
            {
                AppLogAdd(L"[SCREEN] ChangeWindowMode thất bại\n");
            }
            ApplyRenderSettings(dirLightHandle);
            // Bỏ qua thời gian chờ đổi chế độ màn hình để bộ tích luỹ 120Hz không bị dồn
            prev = GetNowHiPerformanceCount();
            acc = 0.0f;
        }

        // Kết xuất hình ảnh
        // Render interpolation (#26): alpha = phần dư của bộ tích luỹ giữa 2 tick logic
        ClearDrawScreen();
        game.BeginRenderInterp(acc / kStep);
        game.Draw();
        game.EndRenderInterp();
        ScreenFlip();
    }

    game.Shutdown();
    SoundManager::Instance().Shutdown();
    FontManager::Shutdown();
    DxLib_End();
    return 0;
}
