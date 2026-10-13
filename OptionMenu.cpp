#include "OptionMenu.h"
#include "DxPlus/DxPlus.h"
#include "SceneManager.h"


OptionMenu::OptionMenu()
{
}

void OptionMenu::Init()
{
	DxLib::SetBackgroundColor(0, 0, 0);

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



	
	GetMousePoint(&mousePosX, &mousePosY);

	bool isClick = GetMouseInput() == MOUSE_INPUT_LEFT;

	for (auto& s : sliders)
	{
		s.Update(mousePosX, mousePosY, isClick);
	}




	SoundManager::bgmVolume = sliders[0].GetValue();
	SoundManager::seVolume = sliders[1].GetValue();

	float bgmValue = sliders[0].GetValue();

	int bgmVolume255 = (int)(
		(bgmValue - sliders[0].GetMinValue()) /
		(sliders[0].GetMaxValue() - sliders[0].GetMinValue()) * 255.0f
		);

	ChangeVolumeSoundMem(bgmVolume255, OptionBgmHandle);

	float seValue = sliders[1].GetValue();

	int seVolume255 = (int)(
		(seValue - sliders[1].GetMinValue()) /
		(sliders[1].GetMaxValue() - sliders[1].GetMinValue()) * 255.0f
		);

	//ChangeVolumeSoundMem(seVolume255, seHandle);



}

void OptionMenu::Reset()
{
	finish = false;
	goTitle = false;

	

	//=========================================================//
	sliders[0].SetPosition(DxPlus::CLIENT_WIDTH * 0.5, 260);
	sliders[0].SetHandleSize(25, 25);
	sliders[0].SetBackSize(200, 20);
	sliders[0].SetFillColor(GetColor(0, 255, 255));
		 
	sliders[0].SetMinValue(0); 
	sliders[0].SetMaxValue(100);
	sliders[0].SetValue(100);

	sliders[1].SetPosition(DxPlus::CLIENT_WIDTH * 0.5, 460);
	sliders[1].SetHandleSize(25, 25);
	sliders[1].SetBackSize(200, 20);
	sliders[1].SetFillColor(GetColor(0, 255, 255));
		   
	sliders[1].SetMinValue(0);
	sliders[1].SetMaxValue(100);
	sliders[1].SetValue(100);
}

void OptionMenu::Draw()
{
	DrawString(10, 10, L"Scene:Option", GetColor(255, 255, 255));

	for (auto& s : sliders)
	{
		s.Draw();
	}


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
