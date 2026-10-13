#include "OptionMenu.h"
#include "DxPlus/DxPlus.h"
#include "SceneManager.h"




OptionMenu::OptionMenu()
{
}

void OptionMenu::Init()
{
	DxLib::SetBackgroundColor(0, 255, 224);

}

void OptionMenu::Update()
{
	memcpy(oldKeyState, keyState, 256);
	GetHitKeyStateAll(keyState);

	if (IsKeyPressed(KEY_INPUT_BACK))
	{
		finish = true;
		goTitle = true;
	}


}

void OptionMenu::Reset()
{
	finish = false;
	goTitle = false;
}

void OptionMenu::Draw()
{
	DrawString(10, 10, L"Scene:Option", GetColor(255, 255, 255));
}

bool OptionMenu::IsFinished()
{
	return finish;
}

void OptionMenu::StopBGM()
{
}

bool OptionMenu::IsKeyPressed(int key)
{
	return keyState[key] == 1 && oldKeyState[key] == 0;
}
