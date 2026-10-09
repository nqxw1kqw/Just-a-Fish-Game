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

}

void Title::Update()
{

	memcpy(oldKeyState, keyState, 256);
	GetHitKeyStateAll(keyState);

	if (IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		if (currentMenu == TitleMenu::GamePlay)
		{
			goSelect = true;

		}

	}


	


}

void Title::Reset()
{
}

void Title::Draw()
{
	DrawString(10, 10, L"Scene:Title", GetColor(255, 255, 255));

	DrawString(200, 100, L"GameStart", GetColor(255, 255, 255));
	DrawString(200, 150, L"Tutorial", GetColor(255, 255, 255));
	DrawString(200, 200, L"Option", GetColor(255, 255, 255));
	DrawString(200, 250, L"Credit", GetColor(255, 255, 255));
	
}


bool Title::IsFinished()
{
	return finished;
}

void Title::StopBGM()
{
}




