//// =============================
//// Scenes/Base/Scene.h
//// =============================
//#pragma once
//#include "DxPlus/DxPlus.h"
//
//class Scene
//{
//public:
//    // 明示コンストラクタ（抽象クラスだが、暗黙変換を防ぐ意味で残す）
//    explicit Scene(class GameContext* context) : gameContext(context) {}
//    virtual ~Scene() = default;
//
//    // ライフサイクル
//    virtual void Init() = 0;            // 初期化（純粋仮想関数）
//    virtual void Update(float) {}            // 毎フレーム更新（派生で実装）
//    virtual void Render() const {}      // 毎フレーム描画（派生で実装）
//    virtual void End() {}
//
//    // 駆動（メインループから毎フレーム呼ぶ）
//    void Drive(float deltaTime);
//
//    // 遷移
//    virtual bool IsFinished() const { return finished; }
//    virtual Scene* GetNextScene() { return nextScene; }
//    void SetNextScene(Scene* scene);
//
//    // コンテキスト差し替え（常駐運用・テスト用）
//    void SetGameContext(class GameContext* context) { gameContext = context; }
//    void DrawFadeOverlay() const;
//
//protected:
//    // フェード（今は使わない）
//    void UpdateFadeController();
//    void StartFadeIn(float duration = 1.0f);
//    void StartFadeOut(float duration = 1.0f);
//
//protected:
//    GameContext* gameContext = nullptr;
//    Scene* nextScene = nullptr;
//    bool finished = false;
//
//private:
//    DxPlus::FadeController fade;
//};
