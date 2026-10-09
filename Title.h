#pragma once
#include "Scene.h"

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



};

