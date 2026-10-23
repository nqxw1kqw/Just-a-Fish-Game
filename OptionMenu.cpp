#include "OptionMenu.h"
#include "DxPlus/DxPlus.h"
#include "SceneManager.h"


OptionMenu::OptionMenu()
{
	OptionBGMSooundHandle = -1;

}

void OptionMenu::Init()
{
	//カメラ・ライト
	camPos = VGet(0.0f, 10.0f, -220.0f);
	camTarget = VGet(0.0f, 25.0f, 0.0f);

	SetCameraPositionAndTarget_UpVecY(camPos, camTarget);

	tlig = 0.0f;

	SetLightDirection(VGet(-1.0f, 1.0f, 1.0f)); // 斜め上
	SetGlobalAmbientLight(GetColorF(tlig, tlig, tlig,0.0f));//light設定
	//SetLightDifColor(GetColorF(2.0f, 2.0f, 2.0f,1.0f)); 

	SetLightEnable(TRUE);

	DxLib::SetBackgroundColor(0, 30, 200);

	//------------

	//モデル読みこみ
	if (OptionStageModelHandle < 0)
	{
		OptionStageModelHandle = MV1LoadModel(L"Data/Model/Option/TitleBack.mv1");
	}
	if (OptionBackModelHandle < 0)
	{
		OptionBackModelHandle = MV1LoadModel(L"Data/Model/Option/Option_Back.mv1");
	}
	if (OptionSelectModelHandle < 0)
	{
		OptionSelectModelHandle = MV1LoadModel(L"Data/Model/Option/kanisann.mv1");
	}

	//Sound読み込み　一度だけ再生
	if (OptionBGMSooundHandle < 0)
	{
		OptionBGMSooundHandle = LoadSoundMem(L"Data/Sound/BGM/OptionBGM.mp3");
	}

	if (CheckSoundMem(OptionBGMSooundHandle) == 0)
	{
		PlaySoundMem(OptionBGMSooundHandle, DX_PLAYTYPE_LOOP);
	}

	//その他初期化
	rad = 1.55;
	camPos = VGet(0.0f, 50.0f, -120.0f);
	camTarget = VGet(0.0f, 20.0f, 0.0f);

	degK = 170.0f;
	tsc = 0.36;


	sliders[0].SetPosition(730, 260);
	sliders[0].SetHandleSize(25, 25);
	sliders[0].SetBackSize(200, 20);
	sliders[0].SetFillColor(GetColor(0, 255, 255));

	sliders[0].SetMinValue(0);
	sliders[0].SetMaxValue(100);
	sliders[0].SetValue(SoundManager::bgmVolume);

	sliders[1].SetPosition(730, 460);
	sliders[1].SetHandleSize(25, 25);
	sliders[1].SetBackSize(200, 20);
	sliders[1].SetFillColor(GetColor(0, 255, 255));

	sliders[1].SetMinValue(0);
	sliders[1].SetMaxValue(100);
	sliders[1].SetValue(SoundManager::seVolume);
}

void OptionMenu::Update()
{
	//単体キー取得
	memcpy(oldKeyState, keyState, 256);
	GetHitKeyStateAll(keyState);

	//タイトルに戻る
	if (IsKeyPressed(KEY_INPUT_BACK))
	{
		finish = true;
		goTitle = true;
	}



	//スライダーのためのマウスポイント取得
	GetMousePoint(&mousePosX, &mousePosY);

	bool isClick = GetMouseInput() == MOUSE_INPUT_LEFT;

	for (auto& s : sliders)
	{
		s.Update(mousePosX, mousePosY, isClick);
	}

	//if ((!sliders[0].wasHovered && sliders[0].isHovered) ||
	//	(!sliders[1].wasHovered && sliders[1].isHovered))
	//{
	//	degK = 170.0f;
	//}

	//if (sliders[0].isHovered || sliders[1].isHovered)
	//{
	//	degK += 1.0f;   
	//}

	//サウンド設定
	SoundManager::bgmVolume = sliders[0].GetValue();
	SoundManager::seVolume = sliders[1].GetValue();

	float bgmValue = sliders[0].GetValue();

	int bgmVolume255 = (int)(
		(bgmValue - sliders[0].GetMinValue()) /
		(sliders[0].GetMaxValue() - sliders[0].GetMinValue()) * 255.0f
		);

	ChangeVolumeSoundMem(bgmVolume255, OptionBGMSooundHandle);

	float seValue = sliders[1].GetValue();

	int seVolume255 = (int)(
		(seValue - sliders[1].GetMinValue()) /
		(sliders[1].GetMaxValue() - sliders[1].GetMinValue()) * 255.0f
		);
	ChangeVolumeSoundMem(bgmVolume255, OptionSESooundHandle);

	//ChangeVolumeSoundMem(seVolume255, seHandle);


	//Debug--------------------------------------------------------------------------

	float speed = 10.0f;

	//デバッグカメラ移動
	// 前後
	//if (CheckHitKey(KEY_INPUT_W)) camPos.z += speed;
	//if (CheckHitKey(KEY_INPUT_S)) camPos.z -= speed;

	//// 左右
	//if (CheckHitKey(KEY_INPUT_A)) camPos.x -= speed;
	//if (CheckHitKey(KEY_INPUT_D)) camPos.x += speed;

	//// 上下
	//if (CheckHitKey(KEY_INPUT_Q)) camPos.y += speed;
	//if (CheckHitKey(KEY_INPUT_E)) camPos.y -= speed;

	//// カメラ回転（簡易）
	//if (CheckHitKey(KEY_INPUT_LEFT)) camTarget.x -= speed;
	//if (CheckHitKey(KEY_INPUT_RIGHT)) camTarget.x += speed;
	//if (CheckHitKey(KEY_INPUT_UP)) camTarget.y += speed;
	//if (CheckHitKey(KEY_INPUT_DOWN)) camTarget.y -= speed;

	//kakudotyousei
	//if (CheckHitKey(KEY_INPUT_M)) tlig += 1.01f;
	//if (CheckHitKey(KEY_INPUT_N)) tlig -= 1.01f;

	//// itityousei
	if (CheckHitKey(KEY_INPUT_0))  tx += 0.01f;
	if (CheckHitKey(KEY_INPUT_9))  ty += 0.01f;
	if (CheckHitKey(KEY_INPUT_8))  tz += 0.01f;

	if (CheckHitKey(KEY_INPUT_1)) tx -= 0.01f;
	if (CheckHitKey(KEY_INPUT_2)) ty -= 0.01f;
	if (CheckHitKey(KEY_INPUT_3)) tz -= 0.01f;

	//if (CheckHitKey(KEY_INPUT_6)) tsc += 0.01f;
	//if (CheckHitKey(KEY_INPUT_7)) tsc -= 0.01f;


	SetCameraPositionAndTarget_UpVecY(camPos, camTarget);
	radK = degK * DX_PI_F / 180.0f;


	degK++;
	if (degK >= 530)degK = 170;


}

void OptionMenu::Reset()
{
	finish = false;
	goTitle = false;

	camPos = VGet(0.0f, 50.0f, -120.0f);
	camTarget = VGet(0.0f, 30.0f, 0.0f);

	degK = 170.0f;


	//=========================================================//
	//Slider

	
}

void OptionMenu::Draw()
{
	


	//-----------------------------------------

	//背景
	//MV1SetPosition(OptionBackModelHandle, VGet(-46, -195, 115));
	MV1SetPosition(OptionBackModelHandle, VGet(-60, -219, 134));
	MV1SetRotationXYZ(OptionBackModelHandle, VGet(0.0f, rad, 0.0f));
	MV1SetScale(OptionBackModelHandle, VGet(tsc, tsc, tsc));

	if (OptionBackModelHandle >= 0)
	{
		MV1DrawModel(OptionBackModelHandle);
	}

	//水槽
	//MV1SetPosition(OptionStageModelHandle, VGet(0, -30, 0));
	MV1SetPosition(OptionStageModelHandle, VGet(1, -29, 24));
	MV1SetScale(OptionStageModelHandle, VGet(0.5f, 0.5f, 0.5f));

	if (OptionStageModelHandle >= 0)
		{
			MV1DrawModel(OptionStageModelHandle);
		}

	
	





	//カニ
	
	MV1SetScale(OptionSelectModelHandle, VGet(0.06f, 0.06f, 0.06f));

	

	if (sliders[0].isSelected)
	{
		MV1SetPosition(OptionSelectModelHandle, VGet(-43, 53, -24));
		//MV1SetPosition(OptionSelectModelHandle, VGet(tx, ty, tz));
		MV1SetRotationXYZ(OptionSelectModelHandle, VGet(0.0f, radK, 0.0f));

		if (OptionSelectModelHandle >= 0)
		{
			MV1DrawModel(OptionSelectModelHandle);
		}
	}

	if (sliders[1].isSelected)
	{
		MV1SetPosition(OptionSelectModelHandle, VGet(-44, 28, -24));
		MV1SetRotationXYZ(OptionSelectModelHandle, VGet(0.0f, radK, 0.0f));
		if (OptionSelectModelHandle >= 0)
		{
			MV1DrawModel(OptionSelectModelHandle);
		}
	}


	





	//===============================================================

	DrawString(10, 10, L"Scene:Option", GetColor(255, 255, 255));

	for (auto& s : sliders)
	{
		s.Draw();
	}

	DxLib::DrawString(540, 200, L"BGM : %d", GetColor(255, 255, 255));
	DxLib::DrawString(540, 400, L"SE : %d", GetColor(255, 255, 255));


	//==============================================
	//Debug
	float value = camPos.x;

	std::wstring text = L"camPos.x = " + std::to_wstring(value);
	DrawString(0, 0, text.c_str(), GetColor(255, 255, 255));
	
	wchar_t buf[256];

	sprintfDx(buf, L"tx = %.2f  ty = %.2f  tz = %.2f", tx, ty, tz);
	DrawString(10, 60, buf, GetColor(255, 255, 255));

	sprintfDx(buf, L"tlig = %.2f", tlig);
	DrawString(10, 90, buf, GetColor(255, 255, 255));

	sprintfDx(buf, L"tsc = %.2f", tsc);
	DrawString(10, 120, buf, GetColor(255, 255, 255));



}


bool OptionMenu::IsFinished()
{
	return finish;
}

void OptionMenu::StopBGM()
{
	
	StopSoundMem(OptionBGMSooundHandle);
	
}

bool OptionMenu::IsKeyPressed(int key)
{
	return keyState[key] == 1 && oldKeyState[key] == 0;
}
