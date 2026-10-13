#include "SceneManager.h"

SceneManager::SceneManager()
{
	current = SceneType::Title_Scene;
	title.Init();
	title.Reset();
}

void SceneManager::Update(float dt)
{

    switch (current)

    {
    case SceneType::Title_Scene:
        title.Update();

        if (title.IsFinished())
        {
            if (title.GoSelect())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()



                //current = SceneType::Select;
                //Debug
                current = SceneType::Game_Scene;
            }
            //kari


            else if (title.GoOption())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()
                
                option.Init();
                option.Reset();
                current = SceneType::Option_Scene;
            }
            else if (title.GoCredit())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()

                credit.Init();
                credit.Reset();

                current = SceneType::Credit_Scene;
            }
            else if (title.GoTutorial())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()

                current = SceneType::Tutorial_Scene;
            }
            break;
        }
    case SceneType::Game_Scene:
        game.Update(dt);
        if (game.GetState() == GameState::Title)
        {
            current = SceneType::Title_Scene;
            title.Init();
            title.Reset();
        }
        break;


    case SceneType::Option_Scene:
        option.Update();
        if (option.IsFinished())
        {
            option.Reset();
            current = SceneType::Title_Scene;
            title.Init();
            title.Reset();
        }
        break;
    case SceneType::Credit_Scene:
        credit.Update();
        if (credit.IsFinished())
        {
            credit.Reset();
            current = SceneType::Title_Scene;
            title.Init();
            title.Reset();
        }
        break;
    }


}

void SceneManager::Draw()
{
    switch (current)
    {
    case SceneType::Title_Scene:
        title.Draw();
        break;
    case SceneType::Game_Scene:
        game.Draw();
        break;

    case SceneType::Option_Scene:
        option.Draw();
        break;

    case SceneType::Credit_Scene:
        credit.Draw();
        break;

  /*  case SceneType::Tutorial:
        tutorial.Draw();
        break;*/
    }
}
