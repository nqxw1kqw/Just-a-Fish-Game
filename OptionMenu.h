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
	int OptionStageModelHandle = -1;
	int OptionBackModelHandle = -1;
	int OptionSelectModelHandle = -1;
	int OptionUiHandle = -1;

	int optionEnterSESoundHandle = -1;


	int OptionBGMSooundHandle = -1;
	int OptionSESooundHandle = -1;

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

	float rotationTime;

	//Test

	VECTOR camPos = VGet(0.0f, 10.0f, -220.0f);
	VECTOR camTarget = VGet(0.0f, 20.0f, 0.0f);

	float deg = 0.0f; 
	float rad = deg * DX_PI_F / 180.0f;
	float degK = 0.0f; 
	float radK = deg * DX_PI_F / 180.0f;

	float tx =-44;
	float ty = 53;
	float tz = -24;

	float tsc = 1.0f;
	float tlig = 1.0f;

	//======================
	int titleX = 800;
	int titleY = 600;
	int titleW = 600;
	int titleH = 150;

	bool hovertitle;
	bool select;
	float Uiscale = 0.3f;

	float Nscale = 0.3f;
	float Sscale = 0.34f;
};

