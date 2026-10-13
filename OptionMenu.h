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

};

