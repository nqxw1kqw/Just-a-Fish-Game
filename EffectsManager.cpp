#include "EffectsManager.h"
#include "EffekseerForDXLib.h"
#include "DxPlus/DxPlus.h"
//#include "ResourceKeys.h"
//#include "ResourceManager.h"

void EffectsManager::Load()
{
	//fireBallHandle = LoadEffekseerEffect(L"Data/Effect/FireBall.efkefc", 30.0f);
}


void EffectsManager::Update()
{
	UpdateEffekseer3D();
}

void EffectsManager::Draw() const
{
	Effekseer_Sync3DSetting();
	DrawEffekseer3D();
}

void EffectsManager::Finalize()
{
	if (fireBallHandle != -1)
	{
		DeleteEffekseerEffect(fireBallHandle);
		fireBallHandle = -1;
	}
}

void EffectsManager::PlayHit(const Vec3& position, float yaw)
{
	if (fireBallHandle == -1)return;

	int playingHandle = PlayEffekseer3DEffect(fireBallHandle);

	if (playingHandle == -1)return;

	SetPosPlayingEffekseer3DEffect(playingHandle, position.x, position.y, position.z);

	SetRotationPlayingEffekseer3DEffect(playingHandle, 0.0f, yaw + DxPlus::PI, 0.0f);
}
