#pragma once
#include "DxLib.h"
#include "Vector3.h"

class TitleFishMove
{
public:
    TitleFishMove();
    ~TitleFishMove();

    void Init();
    void Update();
    void Draw();

private:
    int modelHandle = -1;
    float t = 0.0f;
    Vec3 pos;
    float yaw = 0.0f;
};
