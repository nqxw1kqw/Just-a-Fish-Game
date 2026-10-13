#pragma once

// Trạng thái bàn phím dùng chung cho toàn game.
// Chụp toàn bộ bàn phím đúng 1 lần mỗi tick logic (gọi Update() ở đầu GamePrototype::Update),
// nhờ vậy mọi module so sánh "vừa nhấn" trên cùng một cặp prev/current, thay cho các biến
// static bool xxxPrev rải rác (vốn bị "đóng băng" khi nhánh code không chạy).
class Input
{
public:
    static Input& Instance();

    // Chụp trạng thái phím của tick hiện tại (prev <- current, current <- bàn phím)
    void Update();

    // Đồng bộ prev = current: phím đang giữ sẽ không bị coi là "vừa nhấn" ở tick sau
    void Flush();

    // Phím đang được giữ
    bool IsDown(int key) const;
    // Phím vừa được nhấn ở tick này (cạnh lên)
    bool IsPressed(int key) const;

private:
    Input() = default;

    static constexpr int KEY_COUNT = 256;
    char cur_[KEY_COUNT] = {};
    char prev_[KEY_COUNT] = {};
};
