#pragma once
#include "Title.h"
#include "GamePrototype.h"


enum class SceneType
{
    Title,
    Option,
    Game,
    Tutorial,
    Pouse
};

class SceneManager
{
private:
    SceneType current = SceneType::Title;

    Title title;
    GamePrototype game;
    

    bool firstUpdate = true;


public:
    SceneManager();
    void Update();
    void Draw();
};
