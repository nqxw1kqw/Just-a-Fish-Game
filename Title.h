#pragma once

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
	bool finished{ false };

	bool goSelect{ false };
	bool goOption{ false };
	bool goCredit{ false };
	bool goTutorial{ false };


};

