#pragma once
#include "Vector3.h"

class EffectsManager
{
public:
	void Load();
	void Update();
	void Draw()const;
	void Finalize();
	void PlayHit(const Vec3& position, float yaw);

private:
	int fireBallHandle = -1;

};

