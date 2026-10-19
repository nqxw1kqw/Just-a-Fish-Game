#pragma once

// Artwork and layout for the post-tutorial choice; gameplay stays in GamePrototype.
class ModeSelectScreen
{
public:
    void Init();
    void Shutdown();
    void Draw(int selected) const;

private:
    int artwork_[2] = { -1, -1 };
};
