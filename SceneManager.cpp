#include "SceneManager.h"

SceneManager::SceneManager()
{
	current = SceneType::Title;
	title.Init();
	title.Reset();
}

void SceneManager::Update()
{

    switch (current)

    {
    case SceneType::Title:
        title.Update();

        if (title.IsFinished())
        {
            if (title.GoSelect())
            {

            }

        }
        break;


    }

}

void SceneManager::Draw()
{
}
