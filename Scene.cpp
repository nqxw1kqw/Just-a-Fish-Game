// =============================
// Scenes/Base/Scene.cpp
// =============================
#include "Scene.h"
#include "DxPlus/DxPlus.h"

void Scene::Drive(float deltaTime)
{
#if 0   // フェードを有効にする場合は#if 0に変更すること
    Update(deltaTime);
#else
    UpdateFadeController();
    if (fade.IsStable())
    {
        Update(deltaTime);
    }
#endif    
}

void Scene::SetNextScene(Scene* scene)
{
    nextScene = scene;
    //finished = (scene != nullptr);  // フェードイン/アウトさせたい場合はこの行をコメントアウト
}

void Scene::UpdateFadeController()
{
    fade.Update();
    if (!fade.IsStable())
    {
        if (fade.IsFadeOutDone())
        {
            finished = true;
        }
    }
}

void Scene::DrawFadeOverlay() const
{
    fade.Draw();
}

void Scene::StartFadeIn(float duration)
{
    finished = false;
    fade.StartFadeIn(duration);
}

void Scene::StartFadeOut(float duration)
{
    fade.StartFadeOut(duration);
}
