#include "CreditMenu.h"
#include "DxLib.h"

CreditMenu::CreditMenu()
{
}

void CreditMenu::Init()
{
	DxLib::SetBackgroundColor(0, 255, 224);

}

void CreditMenu::Update()
{
	memcpy(oldKeyState, keyState, 256);
	GetHitKeyStateAll(keyState);

	if (IsKeyPressed(KEY_INPUT_BACK))
	{
		finish = true;
		goTitle = true;
	}


}

void CreditMenu::Reset()
{	
	finish = false;
	goTitle = false;
}

void CreditMenu::Draw()
{
	DrawString(10, 10, L"Scene:Credit", GetColor(255, 255, 255));

}

bool CreditMenu::IsFinished()
{
	return finish;
}

void CreditMenu::StopBGM()
{
}

bool CreditMenu::IsKeyPressed(int key)
{
	return keyState[key] == 1 && oldKeyState[key] == 0;
}
