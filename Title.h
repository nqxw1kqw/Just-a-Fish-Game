#pragma once
#include "Vector3.h"
#include "Scene.h"
#include "TitleFishMove.h"
#include "EffectsManager.h"

enum TitleMenu
{
	GamePlay,
	Tutorial,
	Option,
	Credit
};

class Title
{
public:
	Title();
	void Init();
	void Update();
	void Reset();
	void Draw();
	
	bool IsFinished();
	void StopBGM();

	//Scene
	bool GoTutorial() const { return goTutorial; }
	bool GoOption() const { return goOption; };
	bool GoCredit() const { return goCredit; };
	bool GoSelect() const { return goSelect; };



private:
	TitleMenu currentMenu = TitleMenu::GamePlay;

	TitleFishMove titleFishMove;
	EffectsManager effectManager;


	bool finished{ false };

	bool goSelect{ false };
	bool goOption{ false };
	bool goCredit{ false };
	bool goTutorial{ false };

	bool IsKeyPressed(int key);


	char keyState[256] = { 0 };
	char oldKeyState[256] = { 0 };

	//Handle
	int Title_Bg_Handle = -1;;
	int Title_BGM_Handle = -1;;

	//Color
	Vec3Int None_Color{ 255,255,255 };
	Vec3Int Select_Color{ 255,255,0 };

	Vec3Int Game_Color{ Select_Color };
	Vec3Int Tutorial_Color{ None_Color };
	Vec3Int Option_Color{ None_Color };
	Vec3Int Credit_Color{ None_Color };

	//Count
	int selectCount = 0;

	int titleBackModelHandle = -1;
	int titleBackHandle = -1;
	int titlePlayerModelHandle = -1;

};

