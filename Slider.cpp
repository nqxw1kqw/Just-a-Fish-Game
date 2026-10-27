#include "Slider.h"
#include "DxPlus//DxPlus.h"
#include "Vector3.h"

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

    if (boxHandle == -1)
    {
        boxHandle = LoadGraph(L"./Data/Image/Box.png");
    }
    
    if (boxFrameHandle == -1)
    {
        boxFrameHandle = LoadGraph(L"./Data/Image/Frame.png");
    }
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

//void Slider::Update(int mousePosX, int mousePosY, bool isClick)
//{
//	if (PixelAndBoxHit(
//		mousePosX, mousePosY,
//		handlePosition.x - handleSize.x,
//		handlePosition.y - handleSize.y,
//		handleSize.x * 2,
//		handleSize.y * 2
//	))
//	{
//		isHandleMove = true;
//	}
//
//	else if (PixelAndBoxHit(
//		mousePosX, mousePosY,
//		handlePosition.x - handleSize.x,
//		handlePosition.y - handleSize.y,
//		handleSize.x * 2,
//		handleSize.y * 2
//	))
//	{
//		isHandleMove = false;
//	}
//
//	if (isHandleMove == true) {
//
//		handlePosition.x = mousePosX;
//
//		if (handlePosition.x > position.x + backSize.x) handlePosition.x = position.x + backSize.x;
//		else if (handlePosition.x < position.x - backSize.x) handlePosition.x = position.x - backSize.x;
//
//
//		value = (handlePosition.x - (position.x - backSize.x)) / (backSize.x * 2.0f) * (maxValue - minValue) + minValue;
//
//
//		if (value < minValue) value = minValue;
//		else if (value > maxValue) value = maxValue;
//
//	}
//	else {
//
//		handlePosition = {
//				(position.x - backSize.x) + (value - minValue) / (maxValue - minValue) * (backSize.x * 2),
//				position.y,
//		};
//	}
//}

void Slider::Update(int mousePosX, int mousePosY, bool isClick)
{
    // ハンドルの当たり判定
    bool hitHandle = PixelAndBoxHit(
        mousePosX, mousePosY,
        handlePosition.x - handleSize.x,
        handlePosition.y - handleSize.y,
        handleSize.x * 2,
        handleSize.y * 2
    );

    // バー全体の当たり判定
    bool hitBar = PixelAndBoxHit(
        mousePosX, mousePosY,
        position.x - backSize.x,
        position.y - backSize.y,
        backSize.x * 2,
        backSize.y * 2
    );

    // ハンドルを掴んだ瞬間
    if (hitHandle && isClick)
    {
        isHandleMove = true;
    }
    // クリックを離した or バーの外に出たら解除
    else if (!isClick || !hitBar)
    {
        isHandleMove = false;
    }

    // ハンドル移動中
    if (isHandleMove)
    {
        handlePosition.x = mousePosX;

        // 範囲制限
        if (handlePosition.x > position.x + backSize.x) handlePosition.x = position.x + backSize.x;
        if (handlePosition.x < position.x - backSize.x) handlePosition.x = position.x - backSize.x;

        // 値計算
        value = (handlePosition.x - (position.x - backSize.x))
            / (backSize.x * 2.0f) * (maxValue - minValue) + minValue;

        if (value < minValue) value = minValue;
        if (value > maxValue) value = maxValue;
    }
    else
    {
        // 値からハンドル位置を逆算
        handlePosition.x =
            (position.x - backSize.x)
            + (value - minValue) / (maxValue - minValue) * (backSize.x * 2);

        handlePosition.y = position.y;
    }


    if (hitHandle)
    {
        isSelected = true;
    }
    else 
    {
        isSelected = false;
    }


    wasHovered = isHovered;
    isHovered = hitBar;   // バーにカーソルが乗っているか?

}



void Slider::Draw()
{
	/*DrawBox(
		position.x - backSize.x, position.y - backSize.y,
		position.x + backSize.x, position.y + backSize.y,
		backColor, true);


	DrawBox(
		position.x - backSize.x + 1, position.y - backSize.y + 1, 
		handlePosition.x - 1, position.y + backSize.y - 1,
		fillColor, true);*/


    DrawExtendGraph(position.x - backSize.x + 1, position.y - backSize.y + 1,
        handlePosition.x - 1, position.y + backSize.y - 1,
        boxHandle, true);

    DrawExtendGraph(position.x - backSize.x, position.y - backSize.y,
        position.x + backSize.x, position.y + backSize.y, boxFrameHandle, TRUE);



	if (GetHandleEnabled()) {

		/*DrawBox(
			handlePosition.x - handleSize.x, handlePosition.y - handleSize.y,
			handlePosition.x + handleSize.x, handlePosition.y + handleSize.y,
			handleColor, true);*/
        

        int radius = (int)handleSize.x;

        DrawCircle(
            (int)handlePosition.x,
            (int)handlePosition.y,
            radius,
            handleColor,
            true
        );
	}
}
