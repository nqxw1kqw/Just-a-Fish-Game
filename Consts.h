#pragma once
#include "Vector3.h"

namespace GameConsts
{
    // Cửa sổ & Đồ họa
    constexpr int SCREEN_WIDTH = 1600;
    constexpr int SCREEN_HEIGHT = 900;
    constexpr float FIXED_TIMESTEP = 1.0f / 120.0f; // 120Hz logic loop

    // Sàn đấu (Arena)
    constexpr float ARENA_SIZE = 1400.0f;
    constexpr float ARENA_HALF_SIZE = ARENA_SIZE * 0.5f;

    // Chế độ Debug (Phần D)
#ifdef _DEBUG
    constexpr bool ALLOW_DEBUG_KEYS = true;  // kAllowDebugKeys
    constexpr bool kAllowDebugKeys = true;
#else
    constexpr bool ALLOW_DEBUG_KEYS = false;
    constexpr bool kAllowDebugKeys = false;
#endif

    // Cá chính (Player Fish)
    constexpr float FISH_SIZE_X = 22.0f; // Bề ngang
    constexpr float FISH_SIZE_Y = 18.0f; // Độ dày thân
    constexpr float FISH_SIZE_Z = 38.0f; // Chiều dài
    constexpr float FISH_BOUNCE_HEIGHT = 65.0f; // Độ cao nảy tối đa (H)
    constexpr float FISH_BOUNCE_DURATION = 0.38f; // Chu kỳ mỗi cú nảy (T)
    constexpr float FISH_SPEED = 280.0f; // Tốc độ di chuyển XZ
    constexpr float APEX_THRESHOLD = 0.58f; // Tỉ lệ H bắt đầu kích hoạt bất tử Apex (k * H)

    // Sóng nước khi tiếp đất (Land Shockwave)
    constexpr float SHOCKWAVE_RADIUS = 130.0f;
    constexpr float SHOCKWAVE_DAMAGE = 35.0f;
    constexpr float SHOCKWAVE_KNOCKBACK = 320.0f;

    // Súng & Đạn (Tắt súng tự bắn cho bản Nhảy để tấn công - J8)
    constexpr bool ENABLE_AUTO_GUN = false; // kEnableAutoGun = false
    constexpr float WEAPON_RANGE = 480.0f;
    constexpr float WEAPON_FIRE_COOLDOWN = 0.16f;
    constexpr float BULLET_SPEED = 850.0f;
    constexpr float BULLET_DAMAGE = 22.0f;
    constexpr float BULLET_LIFETIME = 1.2f;

    // Lõi nhảy & Tấn công tiếp đất (Jump & Slam Mechanics - J1 -> J9)
    constexpr float JUMP_DURATION = 0.6f;      // kJumpDuration (0.6 giây)
    constexpr float JUMP_HEIGHT = 85.0f;       // kJumpHeight (80 - 90)
    constexpr float JUMP_COOLDOWN = 1.2f;      // kJumpCooldown (1.2 giây tính từ khi tiếp đất)
    constexpr float LAND_RECOVERY = 0.2f;      // kLandRecovery (khựng 0.2 giây khi vừa chạm đất)
    constexpr float AIR_SPEED_MUL = 1.25f;     // kAirSpeedMul (nhân tốc độ ngang trên không)
    constexpr float SLAM_RADIUS = 120.0f;      // slamRadius (110 - 130)
    constexpr float SLAM_DAMAGE = 40.0f;       // slamDamage (quái nhỏ 1 hit, rùa 3-4 hit)
    constexpr float SLAM_KNOCKBACK = 320.0f;   // slamKnockback đẩy lùi quái

    // Giới hạn an toàn chỉ số (Phần C3)
    constexpr float MIN_JUMP_COOLDOWN = 0.4f;  // Giới hạn dưới hồi chiêu nhảy >= 0.4s
    constexpr float MAX_SLAM_RADIUS = 240.0f;  // Mức tối đa bán kính dậm đất
    constexpr float MAX_MOVE_SPEED = 340.0f;   // Mức tối đa tốc độ bơi

    // Kỹ năng mới Boss: Sóng thần tỏa tròn (Radial Tsunami - Phần A1)
    constexpr float TSUNAMI_TELEGRAPH_P1 = 1.1f;   // kTsunamiTelegraph P1 (1.1s)
    constexpr float TSUNAMI_TELEGRAPH_P2 = 0.9f;   // kTsunamiTelegraph P2 (0.9s)
    constexpr float TSUNAMI_SPEED_P1 = 260.0f;     // kTsunamiSpeed P1 (260 unit/s)
    constexpr float TSUNAMI_SPEED_P2 = 320.0f;     // kTsunamiSpeed P2 (320 unit/s)
    constexpr float TSUNAMI_THICKNESS = 90.0f;     // kTsunamiThickness (90 unit)
    constexpr float TSUNAMI_START_RADIUS = 55.0f;  // kTsunamiStartRadius (55 - bán kính boss)
    constexpr float TSUNAMI_HEIGHT = 40.0f;        // kTsunamiHeight (40 - chiều cao vẽ)
    constexpr float TSUNAMI_DAMAGE = 25.0f;        // kTsunamiDamage (25)
    constexpr float TSUNAMI_KNOCKBACK = 220.0f;    // kTsunamiKnockback (220 đẩy hướng ra ngoài)
    constexpr float TSUNAMI_GAP_DEG_P1 = 0.0f;     // kTsunamiGapDeg P1 (0 độ - không khe hở)
    constexpr float TSUNAMI_GAP_DEG_P2 = 50.0f;    // kTsunamiGapDeg P2 (50 độ - khe hở an toàn)
    constexpr int   TSUNAMI_MAX_ACTIVE = 2;        // kTsunamiMaxActive (2 vòng tối đa)
    constexpr float BOSS_MIN_JUMP_GAP = 2.0f;      // kBossMinJumpGap (2.0s giãn cách an toàn giữa các đòn bắt buộc nhảy)

    // Cấu hình Boss AI & Kỹ năng (Task 10 T3)
    constexpr float BOSS_MAX_HP = 1000.0f;
    constexpr float BOSS_DOUBLE_SLAM_TELEGRAPH2 = 0.45f; // Thời gian báo hiệu cú đập thứ 2 (3b)
    constexpr float BOSS_TSUNAMI_COOLDOWN = 6.0f;        // Hồi chiêu tối thiểu giữa 2 lần chọn chiêu Sóng thần (3a)
    constexpr float BOSS_WALK_SPEED = 50.0f;             // Tốc độ bò chậm trong lúc Idle (3e)

    // Kẻ địch
    constexpr int MAX_ENEMIES = 300;
    constexpr int MAX_BULLETS = 600;
    constexpr int MAX_PARTICLES = 500;
    constexpr int MAX_EXP_ORBS = 300;

    // Chi phí quái (Enemy Cost - Task 10 T1 & T2)
    constexpr float ENEMY_COST_MINION = 1.0f;
    constexpr float ENEMY_COST_HOPPER = 1.5f;
    constexpr float ENEMY_COST_FLOAT  = 1.5f;
    constexpr float ENEMY_COST_CHARGE = 2.0f;
    constexpr float ENEMY_COST_TANKER = 3.0f;

    // Trần chi phí quái sống theo Wave (Task 10 T2)
    constexpr float WAVE_COST_CAP_W1 = 14.0f;
    constexpr float WAVE_COST_CAP_W2 = 18.0f;
    constexpr float WAVE_COST_CAP_W3 = 22.0f;
    constexpr float WAVE_COST_CAP_W4 = 26.0f;
    constexpr float WAVE_COST_CAP_W5 = 30.0f;

    // Khoảng cách tối thiểu không sinh sát người chơi
    constexpr float SPAWN_MIN_DIST_FROM_PLAYER = 260.0f;

    // Giới hạn quái sống tối đa khi Boss triệu hồi
    constexpr int BOSS_MINION_ALIVE_CAP = 12;

    // Chế độ Vô tận (Endless Mode - Task 10 T4)
    constexpr float ENDLESS_WAVE_DURATION = 35.0f; // Thời lượng mỗi wave Endless cố định 35s (4b)
    constexpr float ENDLESS_COST_CAP_MAX = 48.0f;  // Trần chi phí quái sống tối đa trong Endless (4b)
    constexpr float ENDLESS_EMPTY_POOL_HEAL = 0.20f;// Tỉ lệ hồi máu khi hết sạch pool thẻ trong Endless (4d)

    // Giới hạn an toàn chỉ số bổ sung cho Endless cộng dồn thẻ (Task 10 T4d)
    constexpr float MAX_SLAM_DAMAGE = 200.0f;
    constexpr float MAX_SLAM_KNOCKBACK = 700.0f;
    constexpr float MAX_PLAYER_HP = 350.0f;

    // Cấu hình mô hình 3D Cua (EnemyType::Hopper) cũ / fallback
    constexpr float CRAB_MODEL_SCALE = 0.3f;       // Tỉ lệ scale mặc định của mô hình cua theo yêu cầu Shin (0.5f)
    constexpr float CRAB_MODEL_ROT_Y_DEG = 0.0f;   // Góc xoay bù hướng mặt cua (độ)
    constexpr float CRAB_MODEL_OFFSET_Y = 0.0f;    // Độ dời trục Y để khớp mặt đất

    // Cấu hình mô hình 3D Enemy PreTest (Minion, Hopper, Tanker, Float, Charge)
    constexpr float ENEMY_MODEL_SCALE_MINION = 0.20f;
    constexpr float ENEMY_MODEL_ROT_Y_MINION = 180.0f;
    constexpr float ENEMY_MODEL_OFFSET_Y_MINION = 0.0f;

    constexpr float ENEMY_MODEL_SCALE_HOPPER = 0.16f;
    constexpr float ENEMY_MODEL_ROT_Y_HOPPER = 180.0f;
    constexpr float ENEMY_MODEL_OFFSET_Y_HOPPER = 0.0f;

    constexpr float ENEMY_MODEL_SCALE_TANKER = 0.32f;
    constexpr float ENEMY_MODEL_ROT_Y_TANKER = 180.0f;
    constexpr float ENEMY_MODEL_OFFSET_Y_TANKER = 0.0f;

    constexpr float ENEMY_MODEL_SCALE_FLOAT = 0.22f;
    constexpr float ENEMY_MODEL_ROT_Y_FLOAT = 180.0f;
    constexpr float ENEMY_MODEL_OFFSET_Y_FLOAT = 0.0f;

    constexpr float ENEMY_MODEL_SCALE_CHARGE = 0.26f;
    constexpr float ENEMY_MODEL_ROT_Y_CHARGE = 180.0f;
    constexpr float ENEMY_MODEL_OFFSET_Y_CHARGE = 0.0f;

    // Cấu hình mô hình 3D Cá Chính (PlayerFish)
    constexpr float PLAYER_MODEL_SCALE = 2.2f;     // Tỉ lệ scale mặc định của mô hình cá (khớp chiều dài ~29, hitbox R=18)
    constexpr float PLAYER_MODEL_ROT_Y_DEG = -90.0f;// Góc xoay bù hướng mặt cá (độ)
    constexpr float PLAYER_MODEL_OFFSET_Y = 6.0f;  // Độ dời trục Y để bụng cá chạm mặt sàn (Ymin = -2.85 * 2.2)

    // Camera & Game Feel (P0)
    constexpr float CAM_PITCH_DEG = 50.0f;         // Góc nghiêng mặc định 50 độ
    constexpr float CAM_DISTANCE = 580.0f;         // Khoảng cách camera tới mục tiêu
    constexpr float CAM_FOLLOW_SPEED = 6.5f;       // Tốc độ bám mượt kCamFollow (1 - exp(-k*dt))
    constexpr float CAM_DEAD_ZONE = 35.0f;         // Vùng chết (dead zone)
    constexpr float MAX_CAM_SHAKE_OFFSET = 12.0f;  // Giới hạn biên độ rung tối đa
    constexpr float LAND_TRAUMA_AMOUNT = 0.05f;    // Chấn động tiếp đất cơ bản

    // Luồng game & Hồi máu (G3)
    constexpr float WAVE_HEAL_PERCENT = 0.30f;     // Hồi 30% máu tối đa sau mỗi wave (G3)

    // Bố cục Giao diện Thẻ Roguelite (Task T3)
    constexpr int CARD_WIDTH = 260;                 // Chiều rộng khung thẻ (~240-260)
    constexpr int CARD_HEIGHT = 410;                // Chiều cao khung thẻ (~340-410)
    constexpr int CARD_IMAGE_SIZE = 220;            // Kích thước ô ảnh vuông 220 x 220
    constexpr int CARD_SPACING = 60;                // Khoảng cách ngang giữa các thẻ
}
