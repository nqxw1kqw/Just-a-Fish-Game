#pragma once
#include "DxLib.h"


class Slider
{
public:
	Slider();
	void SetValue(float setValue);
	void SetMaxValue(float setMaxValue);
	void SetMinValue(float setMinValue);
	float GetValue() { return value; }
	float GetMaxValue() { return maxValue; }
	float GetMinValue() { return minValue; }

	bool GetIsHandleMoveSlider() { return isHandleMove; }
	

	void SetPosition(float setPositionX, float setPositionY);
	void SetHandleSize(float setHandleWidth, float setHandleHeight);
	void SetHandleColor(int setHandleColor);
	void SetHandleEnabled(bool setHandleEnabled);
	bool GetHandleEnabled() { return handleEnabled; }
	void SetBackSize(float setBackWidth, float setBackHeight);
	void SetBackColor(int setBackColor);
	void SetFillColor(int setFillColor);

	void Update(int mousePosX, int mousePosY, bool icClick);
	void Draw();

	/*bool PixelAndBoxHit(float pixelX, float pixelY, float boxPosX, float boxPosY, float boxWidth, float boxHeight) {

		float vx = boxPosX - pixelX;
		float vy = boxPosY - pixelY;

		if (vx < 0) vx *= -1;
		if (vy < 0) vy *= -1;

		if (vx < boxWidth && vy < boxHeight) {
			return true;
		}

		return false;
	}*/

	bool PixelAndBoxHit(float pixelX, float pixelY,
		float boxPosX, float boxPosY,
		float boxWidth, float boxHeight)
	{
		float vx = boxPosX - pixelX;
		float vy = boxPosY - pixelY;

		if (vx < 0) vx *= -1;
		if (vy < 0) vy *= -1;

		return (vx < boxWidth && vy < boxHeight);
	}

	bool isSelected = false;

	bool isHovered = false;
	bool wasHovered = false;

private:

	int handleColor;
	int backColor;
	int fillColor;

	bool handleEnabled;

	VECTOR position;
	VECTOR handleSize;
	VECTOR backSize;
	VECTOR handlePosition = {};

	float value;
	float maxValue = 1;
	float minValue = 0;

	bool isHandleMove = false;




};

