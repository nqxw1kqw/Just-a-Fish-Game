#include "Title.h"
#include "DxPlus/DxPlus.h"
#include "SceneManager.h"

bool Title::IsKeyPressed(int key)
{
	return keyState[key] == 1 && oldKeyState[key] == 0;
}

Title::Title()
{
}

void Title::Init()
{
	DxLib::SetBackgroundColor(16, 28, 38);

}

void Title::Update()
{

	memcpy(oldKeyState, keyState, 256);
	GetHitKeyStateAll(keyState);

	//if (IsKeyPressed(KEY_INPUT_RETURN))
	//{
	//	finished = true;
	//	if (currentMenu == TitleMenu::GamePlay)
	//	{
	//		goSelect = true;

	//	}

	//}

	//Select

	if ((IsKeyPressed(KEY_INPUT_W) || IsKeyPressed(KEY_INPUT_UP)) && (selectCount > 0))
	{
		selectCount--;
	}
	else if ((IsKeyPressed(KEY_INPUT_W) || IsKeyPressed(KEY_INPUT_UP)) && (selectCount == 0))
	{
		selectCount = 3;
	}


	if ((IsKeyPressed(KEY_INPUT_S) || IsKeyPressed(KEY_INPUT_DOWN)) && (selectCount < 3))
	{
		selectCount++;
	}
	else if ((IsKeyPressed(KEY_INPUT_S) || IsKeyPressed(KEY_INPUT_DOWN)) && (selectCount == 3))
	{
		selectCount = 0;
	}

	Game_Color = None_Color;
	Tutorial_Color = None_Color;
	Option_Color = None_Color;
	Credit_Color = None_Color;

	switch (selectCount)
	{
	case 0:
		Game_Color = Select_Color;
		break;
	case 1:
		Tutorial_Color = Select_Color;
		break;
	case 2:
		Option_Color = Select_Color;
		break;
	case 3:
		Credit_Color = Select_Color;
		break;


	}


	



	//Scene
	if (selectCount == 0 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		goSelect = true;
	}
	else if (selectCount == 1 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		goTutorial = true;
	}
	else if (selectCount == 2 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		goOption = true;
	}
	else if (selectCount == 3 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		goCredit = true;
	}

}

void Title::Reset()
{
	finished = false;
	goSelect = false;
	goOption = false;
	goCredit = false;
	goTutorial = false;
}

void Title::Draw()
{
	DrawString(10, 10, L"Scene:Title", GetColor(255, 255, 255));

	DrawString(200, 100, L"GameStart", GetColor(Game_Color.x,Game_Color.y,Game_Color.z));
	DrawString(200, 150, L"Tutorial", GetColor(Tutorial_Color.x, Tutorial_Color.y, Tutorial_Color.z));
	DrawString(200, 200, L"Option", GetColor(Option_Color.x, Option_Color.y, Option_Color.z));
	DrawString(200, 250, L"Credit", GetColor(Credit_Color.x, Credit_Color.y, Credit_Color.z));
	
}


bool Title::IsFinished()
{
	return finished;
}

void Title::StopBGM()
{
}




