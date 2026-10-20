#pragma once
#include "Slider.h"
#include "SoundManager.h"


class OptionMenu
{
public:
	OptionMenu();
	void Init();
	void Update();
	void Reset();
	void Draw();

	bool IsFinished();
	void StopBGM();

	//Scene
	bool GoTitle() const { return goTitle; }


	

private:
	int OptionBgmHandle = -1;
	int OptionStageModelHandle = -1;
	int OptionBackModelHandle = -1;
	int OptionSelectModelHandle = -1;


	bool finish{false};

	bool goTitle{ false };

	bool IsKeyPressed(int key);


	char keyState[256] = { 0 };
	char oldKeyState[256] = { 0 };

	//Slider slider;
	Slider sliders[2];



	int mousePosX;
	int mousePosY;

	float bgmV = 100;
	float seV = 100;

	//Test

	VECTOR camPos = VGet(0.0f, 10.0f, -220.0f);
	VECTOR camTarget = VGet(0.0f, 20.0f, 0.0f);

	float deg = 0.0f; 
	float rad = deg * DX_PI_F / 180.0f;

	float tx;
	float ty;
	float tz;

};

