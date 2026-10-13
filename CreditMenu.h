#pragma once
class CreditMenu
{
public:
	CreditMenu();
	void Init();
	void Update();
	void Reset();
	void Draw();

	bool IsFinished();
	void StopBGM();

	//Scene
	bool GoTitle() const { return goTitle; }

private:
	bool finish{ false };

	bool goTitle{ false };

	bool IsKeyPressed(int key);


	char keyState[256] = { 0 };
	char oldKeyState[256] = { 0 };




};

