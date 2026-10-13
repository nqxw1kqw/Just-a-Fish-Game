#pragma once
#include "Title.h"
#include "GamePrototype.h"
#include "OptionMenu.h"
#include "Tutorial.h"
#include "CreditMenu.h"


enum class SceneType
{
    Title_Scene,
    Select_Scene,
    Credit_Scene,
    Option_Scene,
    Game_Scene,
    Tutorial_Scene,
    Pouse_Scene
};

class SceneManager
{
private:
    SceneType current = SceneType::Title_Scene;

    Title title;
    GamePrototype game;
    OptionMenu option;
    CreditMenu credit;
    

    

    bool firstUpdate = true;


public:
    SceneManager();
    void Update(float dt);
    void Draw();
};
