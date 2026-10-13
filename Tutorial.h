#pragma once
class Tutorial
{
public:
	Tutorial();
	void Init();
	void Update();
	void Reset();
	void Draw();

	bool IsFinished();
	void StopBGM();

	//Scene
	bool GoTitle() const { return goTitle; };
	

private:
	bool finish{ false };

	bool goTitle{ false };
	







};

