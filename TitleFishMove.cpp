#include "TitleFishMove.h"



TitleFishMove::TitleFishMove()
{
}

TitleFishMove::~TitleFishMove()
{
    if (modelHandle >= 0)
    {
        MV1DeleteModel(modelHandle);
    }
}

void TitleFishMove::Init()
{
    modelHandle = MV1LoadModel(L"Data/Model/Title/Title_Fish.mv1");
    pos = Vec3(0, 20, 0);
    yaw = 0.0f;
    MV1SetScale(modelHandle, VGet(4.5f, 4.5f, 4.5f));

}

void TitleFishMove::Update()
{
    t += 0.02f;

    pos.y = 20.0f + sinf(t) * 5.0f;

    pos.x = cosf(t * 0.7f) * 10.0f;

    yaw = sinf(t * 0.5f) * 0.3f;

    MV1SetPosition(modelHandle, VGet(pos.x, pos.y, pos.z));
    //MV1SetPosition(modelHandle, VGet(0,0,0));
    MV1SetRotationXYZ(modelHandle, VGet(0, yaw, 0));

}

void TitleFishMove::Draw()
{

    if (modelHandle >= 0)
    {
        MV1DrawModel(modelHandle);
    }
}
