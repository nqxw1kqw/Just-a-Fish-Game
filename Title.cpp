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
	if(titleBackModelHandle < 0)
	{
		titleBackModelHandle = MV1LoadModel(L"Data/Model/Title/TitleBack.mv1");
	}

	titleBackHandle = LoadGraph(L"Data/Image/TitleBack.png");

	/*if (titlePlayerModelHandle < 0)
	{
		titlePlayerModelHandle = MV1LoadModel(L"Data/Model/Title/Title_Fish.mv1");
	}*/
	
	
	DxLib::SetBackgroundColor(16, 28, 38);

	titleFishMove.Init();

	//====================================

	effectManager.Load();

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
		selectCount = 2;
	}


	if ((IsKeyPressed(KEY_INPUT_S) || IsKeyPressed(KEY_INPUT_DOWN)) && (selectCount < 2))
	{
		selectCount++;
	}
	else if ((IsKeyPressed(KEY_INPUT_S) || IsKeyPressed(KEY_INPUT_DOWN)) && (selectCount == 2))
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
		Option_Color = Select_Color;
		break;
	case 2:
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
		goOption = true;
	}
	else if (selectCount == 2 && IsKeyPressed(KEY_INPUT_RETURN))
	{
		finished = true;
		goCredit = true;
	}
	
	titleFishMove.Update();

	Vec3 EfPosition{ 0,0,0 };

	effectManager.PlayHit(EfPosition, 0);

	effectManager.Update();

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
	DrawExtendGraph(
		0, 0,
		1600,
		900,
		titleBackHandle,
		FALSE
	);
	VECTOR camPos = VGet(0.0f, 50.0f, -120.0f);
	VECTOR camTarget = VGet(0.0f, 20.0f, 0.0f);

	SetCameraPositionAndTarget_UpVecY(camPos, camTarget);


	SetLightEnable(TRUE);
	SetLightAmbColor(GetColorF(1.0f, 1.0f, 1.0f,0.0f)); 
	SetLightDirection(VGet(0.0f, -0.1f, -0.1f)); 

	MV1SetPosition(titleBackModelHandle, VGet(0, -30, 0));
	//MV1SetRotationXYZ(titleBackModelHandle, VGet(0, 0.5f, 0));
	MV1SetScale(titleBackModelHandle, VGet(0.5f, 0.5f, 0.5f));


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



	DrawString(10, 10, L"Scene:Title", GetColor(255, 255, 255));

	DrawString(200, 100, L"GameStart", GetColor(Game_Color.x,Game_Color.y,Game_Color.z));
	DrawString(200, 150, L"Option", GetColor(Option_Color.x, Option_Color.y, Option_Color.z));
	DrawString(200, 200, L"Credit", GetColor(Credit_Color.x, Credit_Color.y, Credit_Color.z));
	//DrawString(200, 150, L"Tutorial", GetColor(Tutorial_Color.x, Tutorial_Color.y, Tutorial_Color.z));
	
	//titleFishMove.Draw();

	effectManager.Draw();
}


bool Title::IsFinished()
{
	return finished;
}

void Title::StopBGM()
{
}




