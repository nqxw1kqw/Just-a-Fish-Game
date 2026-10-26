#include "Title.h"
#include "DxPlus/DxPlus.h"
#include "SceneManager.h"
#include "SoundManager.h"

bool Title::IsKeyPressed(int key)
{
	return keyState[key] == 1 && oldKeyState[key] == 0;
}

Title::Title()
{

	titleBackModelHandle = -1;
	titleBackHandle = -1;
	titleLogoHandle = -1;
	titlePlayerModelHandle = -1;

	titleBGMSoundHandle = -1;
	titleSelectSESoundHandle = -1;
	titleEnterSESoundHandle = -1;



}

void Title::Init()
{
	//Camera/Light
	VECTOR camPos = VGet(0.0f, 50.0f, -120.0f);
	VECTOR camTarget = VGet(0.0f, 20.0f, 0.0f);

	SetCameraPositionAndTarget_UpVecY(camPos, camTarget);


	SetLightEnable(TRUE);
	SetLightAmbColor(GetColorF(1.0f, 1.0f, 1.0f, 0.0f));
	SetLightDirection(VGet(0.0f, -0.1f, -0.1f));


	//

	if(titleBackModelHandle < 0)
	{
		titleBackModelHandle = MV1LoadModel(L"Data/Model/Title/TitleBack.mv1");
	}

	MV1SetPosition(titleBackModelHandle, VGet(0, -30, 0));
	//MV1SetRotationXYZ(titleBackModelHandle, VGet(0, 0.5f, 0));
	MV1SetScale(titleBackModelHandle, VGet(0.5f, 0.5f, 0.5f));

	//Sound 一度だけ再生
	if (titleBGMSoundHandle < 0)
	{
		titleBGMSoundHandle = LoadSoundMem(L"Data/Sound/BGM/TitleBGM.mp3");
	}

	if (CheckSoundMem(titleBGMSoundHandle) == 0)
	{
		PlaySoundMem(titleBGMSoundHandle, DX_PLAYTYPE_LOOP);
	}

	//SE
	if (titleSelectSESoundHandle < 0)
	{
		titleSelectSESoundHandle = LoadSoundMem(L"Data/Sound/SE/Select_SE.mp3");
	}

	if (titleEnterSESoundHandle < 0)
	{
		titleEnterSESoundHandle = LoadSoundMem(L"Data/Sound/SE/Enter_SE.mp3");
	}

	//UI
	if (titleStartUiHandle < 0)
	{
		titleStartUiHandle = LoadGraph(L"Data/Image/StartUI.png");
	}
	if (titleOptionUiHandle < 0)
	{
		titleOptionUiHandle = LoadGraph(L"Data/Image/OptionUI.png");
	}


	titleBackHandle = LoadGraph(L"Data/Image/TitleBack.png");
	titleLogoHandle = LoadGraph(L"Data/Image/Title_Logo2.png");

		
	
	DxLib::SetBackgroundColor(16, 28, 38);

	titleFishMove.Init();

	//====================================

	effectManager.Load();

	int seVolume255 = (int)(SoundManager::seVolume * 2.55f);

	ChangeVolumeSoundMem(seVolume255, titleSelectSESoundHandle);
	ChangeVolumeSoundMem(seVolume255, titleEnterSESoundHandle);


	//int bgmVolume255 = (int)((SoundManager::bgmVolume - 0) /(100 - 0) * 255.0f);
	int bgmVolume255 = (int)(SoundManager::bgmVolume * 2.55f);

	ChangeVolumeSoundMem(bgmVolume255, titleBGMSoundHandle);
	//ChangeVolumeSoundMem(SoundManager::bgmVolume, titleSESoundHandle);
	// 
	//================================================================

	Selectscale = 0.32f;
	Nonescale = 0.29f;

	

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
		PlaySoundMem(titleSelectSESoundHandle, DX_PLAYTYPE_NORMAL);

	}
	else if ((IsKeyPressed(KEY_INPUT_W) || IsKeyPressed(KEY_INPUT_UP)) && (selectCount == 0))
	{
		selectCount = 1;
		PlaySoundMem(titleSelectSESoundHandle, DX_PLAYTYPE_NORMAL);
	}


	if ((IsKeyPressed(KEY_INPUT_S) || IsKeyPressed(KEY_INPUT_DOWN)) && (selectCount < 1))
	{
		selectCount++;
		PlaySoundMem(titleSelectSESoundHandle, DX_PLAYTYPE_NORMAL);

	}
	else if ((IsKeyPressed(KEY_INPUT_S) || IsKeyPressed(KEY_INPUT_DOWN)) && (selectCount == 1))
	{
		selectCount = 0;
		PlaySoundMem(titleSelectSESoundHandle, DX_PLAYTYPE_NORMAL);

	}





	GetMousePoint(&mousePosX, &mousePosY);

	bool isClick = GetMouseInput() == MOUSE_INPUT_LEFT;

	hoverStart =
		mousePosX >= startX &&
		mousePosX <= startX + startW &&
		mousePosY >= startY &&
		mousePosY <= startY + startH;

	hoverOption =
		mousePosX >= optionX &&
		mousePosX <= optionX + optionW &&
		mousePosY >= optionY &&
		mousePosY <= optionY + optionH;

	if (hoverOption)
	{
		selectCount = 1;   
	}

	if (hoverStart)
	{
		selectCount = 0;  
	}

	if (hoverStart && isClick) {
		PlaySoundMem(titleEnterSESoundHandle, DX_PLAYTYPE_NORMAL);
		finished = true;
		goSelect = true;   // ゲーム開始
	}

	if (hoverOption && isClick) {
		PlaySoundMem(titleEnterSESoundHandle, DX_PLAYTYPE_NORMAL);
		finished = true;
		goOption = true;   // オプション
	}



	//=====================================

	Sscale = Nonescale;
	Oscale = Nonescale;



	switch (selectCount)
	{
	case 0:
		Sscale = Selectscale;
		
		break;
	case 1:
		Oscale = Selectscale;
		

		break;
	default:
		break;
	}


	



	//Scene
	if (selectCount == 0 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		PlaySoundMem(titleEnterSESoundHandle, DX_PLAYTYPE_NORMAL);

		finished = true;
		goSelect = true;
	}
	else if (selectCount == 1 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		PlaySoundMem(titleEnterSESoundHandle, DX_PLAYTYPE_NORMAL);

		finished = true;
		goOption = true;
	}
	/*else if (selectCount == 2 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		goCredit = true;
	}*/
	
	//titleFishMove.Update();

	Vec3 EfPosition{ 0,0,0 };

	//effectManager.PlayHit(EfPosition, 0);

	//effectManager.Update();

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

	//画像
	DrawExtendGraph(
		0, 0,
		1600,
		900,
		titleBackHandle,
		FALSE
	);

	DrawExtendGraph(
		-50, -50,
		800,500,
		titleLogoHandle,
		TRUE
	);

	//UI
	/*DrawExtendGraph(
		1000, 50,
		1500, 200,
		titleStartUiHandle,
		TRUE
	);*/
	DxPlus::Sprite::Draw(titleStartUiHandle, { 930,50 }, { Sscale,Sscale });
	DxPlus::Sprite::Draw(titleOptionUiHandle, { 930,220 }, { Oscale,Oscale });


	





	/*if (titleBackModelHandle >= 0)
	{
		MV1DrawModel(titleBackModelHandle);
	}*/

	/*MV1SetPosition(titlePlayerModelHandle, VGet(0, 0, 0));
	MV1SetScale(titlePlayerModelHandle, VGet(4.5f, 4.5f, 4.5f));

	
	if (titlePlayerModelHandle >= 0)
	{
		MV1DrawModel(titlePlayerModelHandle);
	}*/



	//DrawString(10, 10, L"Scene:Title", GetColor(255, 255, 255));

	//DrawString(200, 100, L"GameStart", GetColor(Game_Color.x,Game_Color.y,Game_Color.z));
	//DrawString(200, 150, L"Option", GetColor(Option_Color.x, Option_Color.y, Option_Color.z));
	//DrawString(200, 200, L"Credit", GetColor(Credit_Color.x, Credit_Color.y, Credit_Color.z));
	//DrawString(200, 150, L"Tutorial", GetColor(Tutorial_Color.x, Tutorial_Color.y, Tutorial_Color.z));
	
	//titleFishMove.Draw();

	//effectManager.Draw();
}


bool Title::IsFinished()
{
	return finished;
}

void Title::StopBGM()
{
	StopSoundMem(titleBGMSoundHandle);
}




