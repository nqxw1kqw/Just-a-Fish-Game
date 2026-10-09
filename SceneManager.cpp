#include "SceneManager.h"

SceneManager::SceneManager()
{
	current = SceneType::Title;
	title.Init();
	title.Reset();
}

void SceneManager::Update(float dt)
{

    switch (current)

    {
    case SceneType::Title:
        title.Update();

        if (title.IsFinished())
        {
            if (title.GoSelect())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()



                //current = SceneType::Select;
                //Debug
                current = SceneType::Game;
            }
            //kari


            else if (title.GoOption())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()

                current = SceneType::Option;
            }
            else if (title.GoCredit())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()

                current = SceneType::Credit;
            }
            else if (title.GoTutorial())
            {
                title.StopBGM();
                //Ç±Ç±Ç…ÅZÅZ.Reset()Ç∆Init()

                current = SceneType::Tutorial;
            }

    case SceneType::Game:
        game.Update(dt);
        if (game.GetState() == GameState::Title)
        {
            current = SceneType::Title;
            title.Reset();
            title.Init();
        }
        break;

        }

    }


}

void SceneManager::Draw()
{
    switch (current)
    {
    case SceneType::Title:
        title.Draw();
        break;
    case SceneType::Game:
        game.Draw();
        break;

    /*case SceneType::Option:
        option.Draw();
        break;

    case SceneType::Tutorial:
        tutorial.Draw();
        break;*/
    }
}
