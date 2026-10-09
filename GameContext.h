// =============================
// Core/GameContext.h
// =============================
#pragma once
#include "DxPlus/DxPlus.h"
#include "Vector3.h"

class GameContext
{
public:
    GameContext() = default;
    ~GameContext() = default;

    // ---- accessors ----

    // Colors
    int GetBgRed() const { return bgRed; }
    void SetBgRed(int r) { bgRed = r; }

    int GetBgGreen() const { return bgGreen; }
    void SetBgGreen(int g) { bgGreen = g; }

    int GetBgBlue() const { return bgBlue; }
    void SetBgBlue(int b) { bgBlue = b; }

    // Grid
    Grid& GetGrid() { return grid; }
    const Grid& GetGrid() const { return grid; }

    // Orbit Camera
    OrbitCamera& GetOrbitCamera() { return orbitCamera; }
    const OrbitCamera& GetOrbitCamera() const { return orbitCamera; }

    // ---- lifecycle ----
    void Init();
    void Reset();
    void Update(float deltaTime);
    void Draw() const;

private:
    // Colors
    int bgRed{ 24 };
    int bgGreen{ 160 };
    int bgBlue{ 224 };
    int stageModelsHandle{ -1 };

    Vec3 eye{ 400,400,-400 };
    Vec3 target{ 0.0f,0.0f,0.0f };
    Vec3 tpsUp{ 0.0f,1.0f,0.0f };
    float targetHeight{ 120.0f };
    float tpsDistance{ 500.0f };
    float tpsHeight{ 200.0f };


    Grid grid;
    OrbitCamera orbitCamera;

    bool wasOrbitControl{ false };

    Player player;

};
