#include "Slider.h"

Slider::Slider() {

	SetValue(0);
	SetPosition(0, 0);
	SetHandleSize(0, 0);
	SetHandleColor(GetColor(255, 255, 255));
	SetHandleEnabled(true);
	SetBackSize(0, 0);
	SetBackColor(GetColor(200, 200, 200));
	SetFillColor(GetColor(0, 225, 225));
}

void Slider::SetValue(float setValue)
{
	value = setValue;

	if (value < minValue) value = minValue;
	else if (value > maxValue) value = maxValue;
}

void Slider::SetMaxValue(float setMaxValue)
{
	if (setMaxValue <= minValue)setMaxValue = minValue + 1;
	maxValue = setMaxValue;
}

void Slider::SetMinValue(float setMinValue)
{
	if (setMinValue >= maxValue) setMinValue = maxValue - 1;

	minValue = setMinValue;
}

void Slider::SetPosition(float setPositionX, float setPositionY)
{
	position = { setPositionX,setPositionY };
}

void Slider::SetHandleSize(float setHandleWidth, float setHandleHeight)
{
	handleSize = { setHandleWidth,setHandleHeight };
}

void Slider::SetHandleColor(int setHandleColor)
{
	handleColor = setHandleColor;
}

void Slider::SetHandleEnabled(bool setHandleEnabled)
{
	handleEnabled = setHandleEnabled;
}

void Slider::SetBackSize(float setBackWidth, float setBackHeight)
{
	backSize = { setBackWidth,setBackHeight };
}

void Slider::SetBackColor(int setBackColor)
{
	backColor = setBackColor;
}

void Slider::SetFillColor(int setFillColor)
{
	fillColor = setFillColor;
}

void Slider::Update(int mousePosX, int mousePosY, bool icClick)
{
	if (PixelAndBoxHit(mousePosX, mousePosY, handlePosition.x, handlePosition.y, handleSize.x, handleSize.y) == true && isClick == true) {

		isHandleMove = true;
	}
	else if (PixelAndBoxHit(mousePosX, mousePosY, position.x, position.y, backSize.x + 100, backSize.y + 50) == false || isClick == false) { //Sliderよりも大きく指定することで、外側でもある程度の距離を対応できる

		isHandleMove = false;
	}

	if (isHandleMove == true) {

		handlePosition.x = mousePosX;

		if (handlePosition.x > position.x + backSize.x) handlePosition.x = position.x + backSize.x;
		else if (handlePosition.x < position.x - backSize.x) handlePosition.x = position.x - backSize.x;


		value = (handlePosition.x - (position.x - backSize.x)) / (backSize.x * 2.0f) * (maxValue - minValue) + minValue;


		if (value < minValue) value = minValue;
		else if (value > maxValue) value = maxValue;

	}
	else {

		handlePosition = {
				(position.x - backSize.x) + (value - minValue) / (maxValue - minValue) * (backSize.x * 2),
				position.y,
		};
	}
}

void Slider::Draw()
{
	DrawBox(
		position.x - backSize.x, position.y - backSize.y,
		position.x + backSize.x, position.y + backSize.y,
		backColor, true);


	DrawBox(
		position.x - backSize.x + 1, position.y - backSize.y + 1, 
		handlePosition.x - 1, position.y + backSize.y - 1,
		fillColor, true);


	if (GetHandleEnabled()) {

		DrawBox(
			handlePosition.x - handleSize.x, handlePosition.y - handleSize.y,
			handlePosition.x + handleSize.x, handlePosition.y + handleSize.y,
			handleColor, true);
	}
}
