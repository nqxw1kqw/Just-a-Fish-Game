#include "Input.h"
#include "DxLib.h"
#include <cstring>

Input& Input::Instance()
{
    static Input instance;
    return instance;
}

void Input::Update()
{
    std::memcpy(prev_, cur_, sizeof(cur_));
    GetHitKeyStateAll(cur_);
}

void Input::Flush()
{
    std::memcpy(prev_, cur_, sizeof(cur_));
}

bool Input::IsDown(int key) const
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return cur_[key] != 0;
}

bool Input::IsPressed(int key) const
{
    if (key < 0 || key >= KEY_COUNT) return false;
    return cur_[key] != 0 && prev_[key] == 0;
}
