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
    constexpr float WEAPON_FIRE_COOLDOWN = 0.13f; // Balance pass: 0.16 -> 0.13
    constexpr float BULLET_SPEED = 850.0f;
    constexpr float BULLET_DAMAGE = 22.0f;
    constexpr float BULLET_LIFETIME = 1.2f;

    // Lõi nhảy & Tấn công tiếp đất (Jump & Slam Mechanics - J1 -> J9)
    constexpr float JUMP_DURATION = 0.6f;      // kJumpDuration (0.6 giây)
    constexpr float JUMP_HEIGHT = 85.0f;       // kJumpHeight (80 - 90)
    constexpr float JUMP_COOLDOWN = 0.85f;     // Balance pass: 1.2 -> 0.85 (tính từ khi tiếp đất)
    constexpr float LAND_RECOVERY = 0.2f;      // kLandRecovery (khựng 0.2 giây khi vừa chạm đất)
    constexpr float AIR_SPEED_MUL = 1.25f;     // kAirSpeedMul (nhân tốc độ ngang trên không)
    constexpr float SLAM_RADIUS = 120.0f;      // slamRadius (110 - 130)
    constexpr float SLAM_DAMAGE = 40.0f;       // slamDamage (quái nhỏ 1 hit, rùa 3-4 hit)
    constexpr float SLAM_KNOCKBACK = 320.0f;   // slamKnockback đẩy lùi quái

    // Giới hạn an toàn chỉ số (Phần C3)
    constexpr float MIN_JUMP_COOLDOWN = 0.35f; // Balance pass: 0.4 -> 0.35 (sàn khi cộng dồn thẻ)
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

    // ---- Boss rework PR A: Breach Slam / Tail Slap / AI / Stun ----
    // ---- Phản hồi trúng đòn ----
    constexpr float HIT_FLASH_TIME = 0.1f;              // Chớp trắng mô hình cá khi trúng đòn
    constexpr float PLAYER_HIT_FLASH_TIME = 0.16f;      // Chớp trắng cá người chơi (giữ như cũ)
    constexpr float HIT_CRIT_DAMAGE = 50.0f;            // Đòn >= mức này dùng tia lửa crit
    constexpr float HIT_FX_HEIGHT = 10.0f;              // Độ cao điểm phát hạt trên thân cá
    // Âm thanh: chống chồng tiếng khi hạ hàng loạt (ms tối thiểu giữa 2 lần phát cùng 1 SFX)
    constexpr int SFX_HIT_MIN_INTERVAL_MS = 45;
    constexpr int SFX_DEATH_MIN_INTERVAL_MS = 70;
    constexpr int SFX_PLAYER_HURT_BASE_VOL = 80;        // Lớp tiếng mới (synth) chồng lên sfx_player_hit cũ
    constexpr int SFX_SHARK_STRAFE_MIN_INTERVAL_MS = 280; // Vòng tròn: chống chồng whoosh
    constexpr int SFX_SHARK_IMPACT_MIN_INTERVAL_MS = 90;  // Erupt / dash: chống chồng khi Frenzy chain

    // Người chơi trúng đòn
    constexpr float PLAYER_HURT_KNOCKBACK = 760.0f;     // Vận tốc đẩy lùi khỏi nguồn sát thương (giảm 8.5/s -> trượt ~90 đơn vị)
    constexpr float PLAYER_HURT_FX_DMG_UNIT = 20.0f;    // amount hạt máu = sát thương / 20 (kẹp 0.25–3)
    constexpr float PLAYER_HURT_FX_MIN = 0.25f;
    constexpr float PLAYER_HURT_FX_MAX = 3.0f;
    constexpr float PLAYER_IFRAME_TIME = 1.2f;          // Bất tử sau khi trúng đòn (cửa sổ an toàn để đổi vị trí)
    constexpr float PLAYER_IFRAME_BLINK_HZ = 12.0f;     // Nhấp nháy ẩn/hiện mô hình trong lúc bất tử
    constexpr float PLAYER_IFRAME_RING_RADIUS = 34.0f;  // Vòng an toàn dưới chân khi đang bất tử
    // Quái chết: thu nhỏ + nảy văng ra rồi mới biến mất
    constexpr float ENEMY_DEATH_TIME = 0.55f;
    constexpr float ENEMY_DEATH_HOP_SPEED = 220.0f;     // Văng ngang ra xa kẻ giết
    constexpr float ENEMY_DEATH_HOP_UP = 260.0f;        // Vận tốc nảy lên
    constexpr float ENEMY_DEATH_GRAVITY = 1100.0f;
    constexpr float ENEMY_DEATH_BOUNCE = 0.45f;         // Hệ số nảy khi chạm sàn
    constexpr int   DEATH_WIPE_MAX_BURSTS = 60;         // KillAll: tối đa số vụ nổ chết mỗi lần quét (~44 hạt/vụ, pool 4096)

    constexpr float BOSS_TURN_RATE = 1.8f;               // rad/s: boss xoay dần về người chơi (để có thể vòng ra sau lưng)
    constexpr float BREACH_TELEGRAPH_P1 = 1.0f;          // Lặn xuống + bọt sủi ở điểm đáp
    constexpr float BREACH_TELEGRAPH_P2 = 0.7f;
    constexpr float BREACH_SINK_DEPTH = 20.0f;           // Boss chìm xuống khi gồng
    constexpr float BREACH_AIR_TIME = 0.65f;             // Thời gian bay
    constexpr float BREACH_JUMP_HEIGHT = 130.0f;
    constexpr float BREACH_LOCK_TIME = 0.25f;            // Điểm đáp bám theo người chơi tới 0.25s trước khi đáp
    constexpr float BREACH_INNER_RADIUS = 120.0f;
    constexpr float BREACH_OUTER_RADIUS = 200.0f;
    constexpr float BREACH_INNER_DAMAGE = 35.0f;
    constexpr float BREACH_OUTER_DAMAGE = 20.0f;
    constexpr float BOSS_DODGE_HEIGHT = 28.0f;           // Cao hơn mức này = né được đòn đập/quật
    constexpr float TAIL_TRIGGER_RANGE = 180.0f;         // Người chơi trong tầm này + sau lưng/bên hông
    constexpr float TAIL_TRIGGER_ANGLE_DEG = 60.0f;      // Lệch khỏi hướng mặt boss tối thiểu
    constexpr float TAIL_TELEGRAPH_P1 = 0.6f;
    constexpr float TAIL_TELEGRAPH_P2 = 0.45f;
    constexpr float TAIL_CONE_HALF_DEG = 70.0f;          // Nón 140 độ phía sau
    constexpr float TAIL_RANGE = 220.0f;
    constexpr float TAIL_DAMAGE = 20.0f;
    constexpr float TAIL_KNOCKBACK = 300.0f;
    constexpr float TAIL_RECOVERY = 0.3f;
    constexpr float TAIL_COOLDOWN = 4.0f;
    constexpr float BOSS_NEAR_RANGE = 180.0f;            // AI: gần -> ưu tiên Tail/Breach
    constexpr float BOSS_FAR_RANGE = 450.0f;             // AI: xa -> ưu tiên Tsunami
    constexpr float BOSS_HUG_TIME = 2.5f;                // Bám sát lâu hơn -> ép Tail Slap
    constexpr float BOSS_STUN_DAMAGE_MUL = 1.5f;         // Hệ số sát thương nhận khi choáng (mặc định)

    // ---- Boss rework PR B: Tsunami canh nhịp nhảy + Sóng đôi P2 + Quật ba nhịp P2 ----
    constexpr float TSUNAMI_DODGE_HEIGHT = 40.0f;        // Chỉ né được sóng khi đang cao hơn mức này lúc sóng đi qua (canh nhịp)
    constexpr float TSUNAMI_DOUBLE_DELAY = 0.8f;         // Sóng đôi: vòng 2 (có khe) phóng sau vòng 1 (không khe)
    constexpr float TSUNAMI_REPEAT_DELAY = 2.3f;         // Biến thể cũ: 2 vòng không khe cách nhau
    constexpr int   TRIPLE_SLAM_COUNT = 3;
    constexpr float TRIPLE_TELEGRAPH_FIRST = 0.6f;
    constexpr float TRIPLE_TELEGRAPH_NEXT = 0.45f;
    constexpr float TRIPLE_AIR_TIME = 0.5f;
    constexpr float TRIPLE_JUMP_HEIGHT = 110.0f;
    constexpr float TRIPLE_DAMAGE = 25.0f;
    constexpr float TRIPLE_FINAL_RADIUS = 240.0f;        // Cú thứ 3 to nhất

    // ---- Boss rework PR C: Lưới bọt (Bubble Net) + Hút nước (Inhale) + hiệu ứng làm chậm ----
    constexpr float BUBBLENET_TELEGRAPH = 0.5f;
    constexpr float BUBBLENET_SWIM_TIME = 2.0f;          // Thời gian boss bơi trọn 1 vòng
    constexpr float BUBBLENET_START_RADIUS = 260.0f;
    constexpr float BUBBLENET_END_RADIUS = 130.0f;       // Vòng lưới khép dần
    constexpr int   BUBBLENET_PILLARS = 18;              // Số cột bọt trong 1 vòng (mỗi 20°)
    constexpr float BUBBLENET_PILLAR_LIFE = 4.0f;
    constexpr float BUBBLENET_PILLAR_RADIUS = 22.0f;
    constexpr float BUBBLENET_PILLAR_DAMAGE = 5.0f;
    constexpr float BUBBLENET_SLOW_FACTOR = 0.6f;        // Chạm cột bọt: còn 60% tốc độ
    constexpr float BUBBLENET_SLOW_TIME = 1.5f;
    constexpr float BUBBLENET_HIT_INTERVAL = 0.5f;
    constexpr float BUBBLENET_FINISH_TELEGRAPH = 0.6f;   // Sau vòng lưới: Breach ngay giữa lưới
    constexpr float BUBBLENET_COOLDOWN = 12.0f;

    constexpr float INHALE_TELEGRAPH = 0.6f;
    constexpr float INHALE_DURATION = 2.0f;
    constexpr float INHALE_RANGE = 420.0f;
    constexpr float INHALE_CONE_HALF_DEG = 60.0f;
    constexpr float INHALE_PULL_SPEED = 140.0f;          // Vận tốc kéo (đơn vị/giây), nửa giá trị khi đang nhảy
    constexpr float INHALE_MOUTH_RADIUS = 70.0f;
    constexpr float INHALE_BITE_DAMAGE = 25.0f;
    constexpr float INHALE_BITE_KNOCKBACK = 400.0f;
    constexpr float INHALE_RECOVERY_STUN = 0.8f;         // Hút xong: boss choáng ngắn (cửa sổ phản công)
    constexpr float INHALE_COOLDOWN = 10.0f;

    // ---- Boss rework PR D: Gọi bầy (Call the Pack) + Lao ngầm (Dive Charge) + Combo P2 + Sóng chuyển phase + Cuồng nộ ----
    constexpr float PACK_TELEGRAPH = 0.85f;              // Bọt sủi ở điểm xuất hiện trước khi cá con trồi lên
    constexpr float PACK_SPAWN_RADIUS = 260.0f;          // Bao vây quanh NGƯỜI CHƠI (không phải quanh boss)
    constexpr int   PACK_P1_MINIONS = 6;
    constexpr int   PACK_P2_MINIONS = 5;
    constexpr int   PACK_P2_HOPPERS = 3;

    constexpr float DIVE_TELEGRAPH_P1 = 0.9f;
    constexpr float DIVE_TELEGRAPH_P2 = 0.7f;
    constexpr float DIVE_SPEED = 700.0f;
    constexpr float DIVE_MAX_TIME = 0.9f;
    constexpr float DIVE_HIT_RADIUS = 60.0f;
    constexpr float DIVE_DAMAGE = 25.0f;
    constexpr float DIVE_KNOCKBACK = 380.0f;
    constexpr float DIVE_WALL_STUN = 1.5f;               // Đâm vào thành đấu trường -> choáng
    constexpr float DIVE_COOLDOWN = 7.0f;

    constexpr float COMBO_DELAY = 0.25f;                 // P2: chiêu nối tiếp ra sau khoảng nghỉ ngắn
    constexpr float COMBO_TAIL_BREACH_CHANCE = 0.5f;     // P2: Quật đuôi -> Breach
    constexpr float COMBO_DIVE_TSUNAMI_CHANCE = 0.6f;    // P2: Lao ngầm -> Sóng thần

    constexpr float RAGE_HP_RATIO = 0.2f;                // < 20% HP: cuồng nộ
    constexpr float RAGE_IDLE = 0.65f;
    constexpr float RAGE_COOLDOWN_MUL = 1.5f;            // Hồi chiêu nhanh hơn 1.5 lần

    // ---- Boss AI pacing: chống lặp chiêu + ngắm đón đầu + nhịp tấn công/nghỉ ----
    constexpr float AI_REPEAT_WEIGHT_LAST = 0.25f;       // Trọng số chiêu vừa dùng
    constexpr float AI_REPEAT_WEIGHT_PREV = 0.6f;        // Trọng số chiêu trước đó
    // (Không bao giờ chọn 1 chiêu 3 lần liên tiếp)

    constexpr float AIM_LEAD_SCALE = 0.7f;               // Chỉ đón đầu 70% -> đổi hướng vẫn né được
    constexpr float AIM_BREACH_LEAD = 0.35f;             // Breach đón đầu ~0.35s (bám tới BREACH_LOCK_TIME trước khi đáp)
    constexpr float AIM_VEL_SMOOTHING = 0.25f;           // EMA vận tốc người chơi (tính từ chênh lệch khung)
    constexpr float AIM_MAX_PLAYER_SPEED = 600.0f;       // Bỏ qua đột biến (knockback / dịch chuyển)
    constexpr float AIM_ARENA_MARGIN = 40.0f;            // Điểm ngắm luôn kẹp trong đấu trường

    constexpr int   BURST_MIN_P1 = 2;                    // P1: mỗi đợt 2–3 chiêu
    constexpr int   BURST_MAX_P1 = 3;
    constexpr int   BURST_SIZE_P2 = 3;                   // P2 / cuồng nộ: 3 chiêu
    constexpr float BURST_GAP_P1 = 0.7f;                 // Nghỉ ngắn giữa các chiêu trong 1 đợt
    constexpr float BURST_GAP_P2 = 0.5f;
    constexpr float BURST_GAP_RAGE = 0.4f;
    constexpr float REST_TIME_P1 = 2.0f;                 // Cửa sổ nghỉ dễ đọc sau mỗi đợt
    constexpr float REST_TIME_P2 = 1.4f;
    constexpr float REST_TIME_RAGE = 1.0f;
    constexpr float REST_DRIFT_SPEED = 45.0f;            // Lượn vòng chậm quanh người chơi khi nghỉ
    constexpr float REST_PREFERRED_DIST = 320.0f;

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
    constexpr float ENDLESS_WAVE_DURATION = 20.0f; // Balance pass: 35 -> 20s mỗi stage Endless

    // ===== Balance pass: luồng Tutorial -> Endless =====
    constexpr int   TUTORIAL_WAVES = 3;              // Tutorial (Cá Voi): 5 -> 3 wave, rồi Boss Cá Voi, rồi tự vào Endless
    constexpr int   ENDLESS_BOSS_EVERY = 5;          // Stage 5, 10, 15... là stage Boss (ngẫu nhiên Cá Voi / Cá Mập)
    constexpr int   ENDLESS_MAX_STAGE = 30;          // Endless tối đa 30 stage -> màn CLEAR
    // Phase 1 flow: Tutorial 15s/stage -> Whale -> chọn chế độ Normal / Hell
    constexpr float TUTORIAL_WAVE_DURATION = 15.0f;  // Mỗi stage tutorial 15 giây
    constexpr float HELL_WAVE_DURATION = 20.0f;      // Hell: 20 giây mỗi stage
    constexpr float HELL_DAMAGE_TAKEN_MUL = 1.5f;    // Hell: sát thương nhận vào x1.5
    constexpr int   HELL_MAX_ALIVE = 13;             // Hell: trần số quái sống ở stage 1 (không dọn quái giữa stage)
    // Hell: mỗi stage tăng số quái (giữ trần để tránh lag)
    constexpr int   HELL_MAX_ALIVE_STAGES_PER_ADD = 2; // Cứ 2 stage: trần quái sống +1
    constexpr int   HELL_MAX_ALIVE_CAP = 24;         // Trần cứng tuyệt đối (perf)
    constexpr int   HELL_SPAWN_PER_STAGE_ADD = 1;    // Mỗi nhóm sinh thêm +1 con ...
    constexpr int   HELL_SPAWN_STAGES_PER_ADD = 4;   // ... cứ mỗi 4 stage
    constexpr int   HELL_SPAWN_GROUP_ADD_MAX = 3;    // Tối đa +3 con / nhóm
    constexpr float HELL_COST_CAP_PER_STAGE = 0.04f; // Ngân sách quái sống +4% / stage
    constexpr float HELL_COST_CAP_MUL_MAX = 2.2f;    // Trần hệ số ngân sách
    // Phase 2 - Hell: sinh quái riêng
    constexpr float HELL_SPAWN_INTERVAL_MUL = 0.75f; // Khoảng nghỉ sinh quái x0.75 (sinh nhanh hơn)
    constexpr float HELL_COST_CAP_MUL = 1.25f;       // Trần ngân sách quái sống x1.25 (vẫn bị HELL_MAX_ALIVE chặn)
    constexpr float HELL_ENEMY_HP_MUL = 1.20f;       // HP quái x1.2 so với Normal
    constexpr int   HELL_TIER_OFFSET = 1;            // Bảng tỉ lệ quái: dịch sớm 1 bậc (Tanker/Charge sớm hơn)
    constexpr float HELL_WAVE_HEAL_PERCENT = 0.20f;
    constexpr float HELL_PLAYER_DMG_MUL = 1.5f;      // Hell: quái + Boss nhận x1.5 sát thương từ người chơi
    // Hell: hạt hiệu ứng nhẹ hơn khi đông quái (giảm lag)
    constexpr int   HELL_FX_LIGHT_THRESHOLD = 8;     // Từ 8 quái sống trở lên
    constexpr float HELL_FX_LIGHT_SCALE = 0.4f;      // Số hạt x0.4
    // Normal + Hell: mỗi lần lên cấp (EXP) -> +1 điểm ngẫu nhiên vào 5 chỉ số (Tutorial không áp dụng)
    constexpr float LV_ATK_PCT = 0.05f;              // Attack: +5% sát thương Slam + đạn / điểm
    constexpr float LV_DEF_PCT = 0.04f;              // Def: -4% sát thương nhận / điểm
    constexpr float LV_DEF_MAX = 0.50f;              // Def: giảm tối đa 50%
    constexpr float LV_SPD_PCT = 0.04f;              // Speed: +4% tốc độ bơi / điểm
    constexpr float LV_CD_PCT = -0.04f;              // Cooldown: -4% hồi nhảy / điểm
    constexpr int   LV_LUCK_RARITY_SHIFT = 4;        // Luck: +4 vào roll độ hiếm (0..99) / điểm -> thẻ hiếm hơn
    constexpr float LEVEL_UP_BANNER_TIME = 2.0f;     // Thời gian hiện bảng LEVEL UP
    // EXP orb (Normal + Hell): mọi quái chết đều rơi 1 viên trắng
    constexpr float EXP_ORB_VALUE = 5.0f;            // EXP / viên (lên cấp đầu cần 30, x1.4 mỗi cấp)
    constexpr float EXP_ORB_SIZE = 9.0f;             // Bán kính ngọc (bát diện) - chiều cao = 2x
    constexpr float EXP_ORB_HOVER_Y = 9.0f;          // Độ cao lơ lửng khi nằm yên
    constexpr float EXP_ORB_BOB_AMP = 2.5f;          // Biên độ nhấp nhô
    constexpr float EXP_ORB_BOB_SPEED = 4.0f;        // Tốc độ nhấp nhô (rad/s)
    constexpr float EXP_ORB_SPIN_SPEED = 2.2f;       // Tốc độ xoay (rad/s), x3 khi bị hút
    constexpr float EXP_ORB_POP_SPEED_MIN = 60.0f;   // Văng ra khi rơi (ngang)
    constexpr float EXP_ORB_POP_SPEED_MAX = 130.0f;
    constexpr float EXP_ORB_POP_UP_MIN = 150.0f;     // Văng lên
    constexpr float EXP_ORB_POP_UP_MAX = 220.0f;
    constexpr float EXP_ORB_GRAVITY = 650.0f;
    constexpr float EXP_ORB_BOUNCE = 0.30f;          // Nảy 1 lần khi chạm đất
    constexpr float EXP_ORB_HALO_SIZE = 34.0f;       // Quầng sáng billboard (additive)
    constexpr int   EXP_ORB_HALO_ALPHA = 110;
    constexpr int   EXP_ORB_HALO_MAX = 120;          // Quá số này: bỏ quầng sáng (Hell đông, giảm lag)
    constexpr float EXP_ORB_ABSORB_TIME = 0.12f;     // Hấp thụ: thu nhỏ + chớp
    constexpr float EXP_MAGNET_RADIUS = 260.0f;      // Bán kính nam châm (vào vùng này là bị hút đến khi nhặt)
    constexpr float EXP_MAGNET_SPEED = 520.0f;       // Tốc độ hút tối đa
    constexpr float EXP_MAGNET_SPEED_START = 60.0f;  // Tốc độ lúc mới bị hút (ease-in)
    constexpr float EXP_MAGNET_ACCEL = 1600.0f;      // Gia tốc hút (đơn vị/s^2)
    constexpr float EXP_MAGNET_CURVE = 140.0f;       // Lực cong ngang lúc đầu (giảm dần về 0)
    constexpr float EXP_MAGNET_CURVE_TIME = 0.35f;   // Thời gian cong
    constexpr float EXP_PICKUP_RADIUS = 45.0f;       // Bán kính nhặt
    // Hell: thẻ bị giảm mạnh (~1/10): +10% -> +1%. Thẻ vẫn cộng dồn. (bulletCount / extraJumps giữ nguyên)
    constexpr float HELL_CARD_BONUS_SCALE = 0.10f;
    constexpr float HELL_ROULETTE_FADE_TIME = 0.5f;  // Mờ dần sau khi hiện thẻ trúng
    // Phase 2 - Hell: vòng quay thẻ (không dừng game)
    constexpr float HELL_ROULETTE_SPIN_TIME = 1.6f;  // Thời gian quay
    constexpr float HELL_ROULETTE_HOLD_TIME = 1.2f;  // Hiện thẻ trúng sau khi dừng
    constexpr float HELL_ROULETTE_STEP_START = 0.05f;// Khoảng đổi ô lúc đầu (giây)
    constexpr float HELL_ROULETTE_STEP_END = 0.28f;  // Khoảng đổi ô lúc cuối (chậm dần)
    // Phase 2 - Hell: Boss thông minh hơn (AI giữ nguyên, chỉ đổi nhịp/trọng số)
    constexpr float HELL_BOSS_COOLDOWN_SPEED = 1.4f; // Hồi chiêu nhanh x1.4
    constexpr float HELL_BOSS_REST_MUL = 0.6f;       // Thời gian nghỉ giữa chuỗi chiêu x0.6
    constexpr float HELL_WHALE_STRONG_WEIGHT_MUL = 1.5f; // Cá Voi: DoubleSlam/Tsunami/BubbleNet/Dive x1.5
    constexpr float HELL_WHALE_MINION_WEIGHT_MUL = 0.5f; // Cá Voi: ít triệu hồi hơn (sàn đã đông)
    constexpr float HELL_WHALE_P1_DOUBLESLAM_WEIGHT = 1.5f; // Cá Voi: DoubleSlam có từ Phase 1
    constexpr float HELL_SHARK_HUNT_FIRST_DELAY = 3.0f;  // Cá Mập: Hunt lần đầu sau 3s (Normal 8s)
    constexpr float HELL_SHARK_HUNT_COOLDOWN = 8.0f;     // Cá Mập: hồi Hunt 8s (Normal 14s)
    constexpr float HELL_SHARK_HUNT_WEIGHT_MUL = 2.0f;   // Cá Mập: trọng số Hunt x2
    constexpr float HELL_SHARK_VORTEX_WEIGHT_MUL = 1.5f; // Cá Mập: trọng số Vortex x1.5
    constexpr float HELL_CARD_TOAST_TIME = 2.5f;     // Hell: thời gian hiện tên thẻ tự chọn
    // Gợi ý tutorial (mỗi gợi ý hiện 1 lần / phiên, [TAB] bỏ qua)
    constexpr float TIP_SOFT_PAUSE_TIME = 0.8f;      // Thời gian làm chậm khi gợi ý vừa hiện
    constexpr float TIP_SOFT_PAUSE_SCALE = 0.35f;    // Tốc độ game khi làm chậm
    constexpr float TIP_MAX_TIME = 8.0f;             // Gợi ý tự tắt sau thời gian này
    constexpr float TIP_BOSS_SHOW_TIME = 4.0f;       // Gợi ý Boss hiện trong thời gian này
    constexpr float TIP_MOVE_DISTANCE = 120.0f;      // Bơi đủ quãng này = hoàn thành gợi ý di chuyển
    constexpr float TIP_START_DELAY = 0.4f;          // Trễ trước gợi ý đầu tiên của stage 1
    // Tutorial dễ hơn (chỉ áp dụng trong Tutorial, Normal/Hell không đổi)
    constexpr float TUTORIAL_PLAYER_HP_BONUS = 0.50f;        // +50% HP tối đa (modifier Percent)
    constexpr float TUTORIAL_DMG_MUL_VS_ENEMY = 1.50f;       // x1.5 sát thương lên quái thường (nhân thêm vào PLAYER_DMG_MUL_VS_ENEMY)
    constexpr float TUTORIAL_DAMAGE_TAKEN_MUL = 0.75f;       // Stage tutorial: nhận x0.75 sát thương
    constexpr float TUTORIAL_BOSS_DAMAGE_TAKEN_MUL = 0.40f;  // Trận Cá Voi tutorial: nhận x0.40 sát thương
    constexpr float TUTORIAL_DMG_MUL_VS_BOSS = 3.0f;         // Trận Cá Voi tutorial: người chơi gây x3 sát thương lên Boss
    constexpr float TUTORIAL_BOSS_CLEAR_HEAL = 0.50f;// Hồi máu khi hạ Cá Voi tutorial trước khi vào Endless
    // Sát thương của cá chính lên quái thường (không áp dụng cho Boss)
    constexpr float PLAYER_DMG_MUL_VS_ENEMY = 1.35f;
    // HP quái Endless: mul = min(MAX, 1 + LIN*(s-1) + QUAD*(s-1)^2)
    constexpr float ENDLESS_ENEMY_HP_LIN = 0.07f;
    constexpr float ENDLESS_ENEMY_HP_QUAD = 0.0015f;
    constexpr float ENDLESS_ENEMY_HP_MAX = 4.5f;
    // HP Boss Endless: mul = 1 + PER_STAGE*(s-5)  (stage 5 = x1.0, stage 30 = x2.25)
    constexpr float ENDLESS_BOSS_HP_PER_STAGE = 0.05f;
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
    constexpr float PLAYER_MODEL_SCALE = 3.6f;     // Tỉ lệ scale mặc định của mô hình cá (tăng lên 3.6 theo yêu cầu)
    constexpr float PLAYER_MODEL_ROT_Y_DEG = 90.0f;// Xoay đảo 180 độ (-90 + 180 = 90) để đầu cá bơi hướng về phía trước
    constexpr float PLAYER_MODEL_OFFSET_Y = 9.8f;  // Độ dời trục Y để bụng cá chạm mặt sàn (Ymin = -2.82 * 3.6)

    // Camera & Game Feel (P0)
    constexpr float CAM_PITCH_DEG = 50.0f;         // Góc nghiêng mặc định 50 độ
    constexpr float CAM_DISTANCE = 580.0f;         // Khoảng cách camera tới mục tiêu
    constexpr float CAM_FOLLOW_SPEED = 6.5f;       // Tốc độ bám mượt kCamFollow (1 - exp(-k*dt))
    constexpr float CAM_DEAD_ZONE = 35.0f;         // Vùng chết (dead zone)
    constexpr float MAX_CAM_SHAKE_OFFSET = 12.0f;  // Giới hạn biên độ rung tối đa
    constexpr float LAND_TRAUMA_AMOUNT = 0.05f;    // Chấn động tiếp đất cơ bản

    // Sương mù dưới nước (#15) - camera cách mục tiêu ~580, sàn 1400x1400
    constexpr float FOG_START = 700.0f;            // Bắt đầu mờ ngay sau vùng quanh cá
    constexpr float FOG_END = 2000.0f;             // Mờ hoàn toàn ở góc sàn xa nhất
    constexpr int FOG_COLOR_R = 10;                // Màu xanh biển sâu
    constexpr int FOG_COLOR_G = 40;
    constexpr int FOG_COLOR_B = 80;

    // Luồng game & Hồi máu (G3)
    constexpr float WAVE_HEAL_PERCENT = 0.30f;     // Hồi 30% máu tối đa sau mỗi wave (G3)

    // Bố cục Giao diện Thẻ Roguelite (Task T3)
    constexpr int CARD_WIDTH = 260;                 // Chiều rộng khung thẻ (~240-260)
    constexpr int CARD_HEIGHT = 410;                // Chiều cao khung thẻ (~340-410)
    constexpr int CARD_IMAGE_SIZE = 220;            // Kích thước ô ảnh vuông 220 x 220
    constexpr int CARD_SPACING = 60;                // Khoảng cách ngang giữa các thẻ

    // ================= BOSS CÁ MẬP (Endless) =================
    constexpr float SHARK_MAX_HP = 800.0f;               // x (1 + 0.5 * (tier - 1)) như cá voi
    constexpr float SHARK_HITBOX_RADIUS = 45.0f;
    constexpr float SHARK_SWIM_SPEED = 115.0f;           // Lượn khi Idle (~68% bản đầu 170; người chơi bơi 280)
    constexpr float SHARK_TURN_RATE = 4.0f;              // rad/s
    constexpr float SHARK_PREFERRED_DIST = 300.0f;
    constexpr float SHARK_MOUTH_FWD = 95.0f;             // Mõm cách tâm (dọc thân)
    constexpr float SHARK_TAIL_PIVOT_Y = 30.5f;          // Khớp đuôi (khớp với Tools/LowPolyModels/build_shark_boss.py)
    constexpr float SHARK_TAIL_PIVOT_Z = -74.8f;
    constexpr float SHARK_TAIL_SWAY = 0.25f;             // rad, biên độ vẫy đuôi (giảm để khớp đuôi không hở)
    constexpr float SHARK_BODY_COUNTER_SWAY = 0.04f;     // Thân lắc ngược nhẹ theo đuôi
    constexpr float SHARK_TAIL_FREQ = 2.2f;              // Hz khi bơi thường
    constexpr int   SHARK_ENDLESS_BOSS_PARITY = 1;       // Endless: bossIdx lẻ (wave 5, 15, 25...) = cá mập, chẵn = cá voi
    // 1. Bite Lunge: khựng + vệt đỏ chỉ hướng -> lao ngắn rất nhanh (nhanh hơn Dive 700 u/s, ngắn hơn)
    constexpr float SHARK_BITE_TELEGRAPH = 0.6f;
    constexpr float SHARK_BITE_TELEGRAPH_FRENZY = 0.45f;
    constexpr float SHARK_BITE_LOCK_TIME = 0.2f;         // Ngừng bám mục tiêu 0.2s trước khi lao
    constexpr float SHARK_BITE_SPEED = 720.0f;           // ~65% bản đầu 1100 (vẫn nhanh hơn Dive 700 một chút)
    constexpr float SHARK_BITE_RANGE = 360.0f;
    constexpr float SHARK_BITE_HIT_RADIUS = 45.0f;
    constexpr float SHARK_BITE_DAMAGE = 20.0f;
    constexpr float SHARK_BITE_RECOVER = 0.55f;
    constexpr float SHARK_WALL_STUN = 1.2f;              // Đâm thành đấu trường -> choáng
    constexpr float SHARK_STUN_DAMAGE_MUL = 1.5f;
    // 2. Circle Strafe: vây lưng nhô lên, lượn vòng quanh người chơi rồi ngoặt vào cắn
    constexpr float SHARK_STRAFE_RADIUS = 260.0f;
    constexpr float SHARK_STRAFE_ANG_SPEED = 1.25f;      // rad/s -> 325 u/s trên vòng r=260 (< tốc độ nhảy lướt 350 của người chơi)
    constexpr float SHARK_STRAFE_ANG_SPEED_FRENZY = 1.35f; // 351 u/s
    constexpr float SHARK_STRAFE_LAPS = 1.0f;            // chậm hơn -> 1 vòng (~5s) cho đỡ dài
    constexpr float SHARK_STRAFE_BITE_TELEGRAPH = 0.4f;  // Ngoặt vào: telegraph ngắn hơn
    constexpr float SHARK_FIN_DAMAGE = 10.0f;            // Chạm thân khi đang lượn
    constexpr float SHARK_FIN_HIT_COOLDOWN = 0.8f;
    // 3. Tail Vortex: xoáy nước kéo nhẹ người chơi về tâm (AddExternalPull)
    constexpr float SHARK_VORTEX_CAST_TIME = 0.7f;
    constexpr float SHARK_VORTEX_RADIUS = 240.0f;
    constexpr float SHARK_VORTEX_DURATION = 3.5f;
    constexpr float SHARK_VORTEX_PULL = 130.0f;          // u/s ở tâm, giảm dần ra mép (nhảy = không bị kéo)
    constexpr float SHARK_VORTEX_CORE_RADIUS = 55.0f;
    constexpr float SHARK_VORTEX_CORE_DAMAGE = 8.0f;
    constexpr float SHARK_VORTEX_TICK = 0.6f;
    constexpr float SHARK_VORTEX_COOLDOWN = 7.0f;
    constexpr float SHARK_VORTEX_BACK = 120.0f;          // Tâm xoáy sau đuôi
    // 4. Frenzy < 30% HP: chuỗi 2–3 cú cắn
    constexpr float SHARK_FRENZY_HP_RATIO = 0.3f;
    constexpr int   SHARK_FRENZY_BITES_MIN = 2;
    constexpr int   SHARK_FRENZY_BITES_MAX = 3;
    constexpr float SHARK_FRENZY_CHAIN_TELEGRAPH = 0.3f;
    // Nhịp đòn (dùng chung ý tưởng burst/rest của cá voi)
    constexpr int   SHARK_BURST_SIZE = 2;
    constexpr int   SHARK_BURST_SIZE_FRENZY = 3;
    constexpr float SHARK_BURST_GAP = 0.6f;
    constexpr float SHARK_BURST_GAP_FRENZY = 0.35f;
    constexpr float SHARK_REST_TIME = 1.8f;
    constexpr float SHARK_REST_TIME_FRENZY = 1.0f;
    constexpr float SHARK_REST_DRIFT_SPEED = 60.0f;
    constexpr float SHARK_WEIGHT_BITE = 1.0f;
    constexpr float SHARK_WEIGHT_STRAFE = 0.8f;
    constexpr float SHARK_WEIGHT_VORTEX = 0.6f;
    constexpr float SHARK_WEIGHT_BITE_FRENZY = 1.5f;
    constexpr float SHARK_WEIGHT_HUNT = 0.5f;
    // 5. Hunt (săn mồi): nhảy lên -> lặn vào cát (chỉ thấy vây) -> bám theo -> khoá vị trí -> phóng lên cắn, ngoạm & kéo -> ném
    constexpr float SHARK_HUNT_COOLDOWN = 14.0f;
    constexpr float SHARK_HUNT_FIRST_DELAY = 8.0f;       // Không dùng ngay đầu trận
    constexpr float SHARK_HUNT_LEAP_TIME = 0.6f;
    constexpr float SHARK_HUNT_LEAP_HEIGHT = 140.0f;
    constexpr float SHARK_HUNT_LEAP_FWD_SPEED = 160.0f;
    constexpr float SHARK_HUNT_BURROW_DEPTH = 72.0f;     // Thân chìm dưới cát, chỉ vây lưng (~27u) nhô lên
    constexpr float SHARK_HUNT_TRACK_TIME = 1.1f;        // Bám ngắn rồi khoá vị trí người chơi từ xa
    constexpr float SHARK_HUNT_LOCK_MIN_DIST = 160.0f;   // Vòng đỏ không sinh sát thân: lùi vây ra ít nhất mức này
    constexpr float SHARK_HUNT_DASH_TIME = 0.35f;        // Vây lao ngầm tới vòng trong thời gian cố định
    constexpr float SHARK_HUNT_DASH_MIN_SPEED = 1400.0f; // ...nhưng không chậm hơn mức này
    constexpr float SHARK_HUNT_TRACK_SPEED = 190.0f;     // < người chơi 280 -> chạy thoát được
    constexpr float SHARK_HUNT_TRACK_TURN = 2.5f;
    constexpr float SHARK_HUNT_LOCK_TIME = 0.5f;         // Vòng đỏ hiện ngay + vây khựng lại
    constexpr float SHARK_HUNT_BURST_TIME = 0.08f;       // Phóng thẳng lên gần như tức thì
    constexpr float SHARK_HUNT_BITE_RADIUS = 75.0f;
    constexpr float SHARK_HUNT_BITE_DAMAGE = 25.0f;
    constexpr float SHARK_HUNT_SWING_RADIUS = 300.0f;    // Kéo người chơi quay vòng nhanh quanh tâm sân
    constexpr float SHARK_HUNT_SWING_ANG_SPEED = 5.0f;   // rad/s (~1500 u/s trên vòng)
    constexpr float SHARK_HUNT_SWING_LAPS = 1.25f;       // -> ~1.57s
    constexpr float SHARK_HUNT_SWING_RADIUS_LERP = 6.0f; // Hội tụ bán kính vòng quay (1/s)
    constexpr float SHARK_HUNT_JAW_Y = 26.0f;            // Ổ hàm (local thân cá mập)
    constexpr float SHARK_HUNT_JAW_Z = 92.0f;
    constexpr float SHARK_HUNT_HELD_Y_OFFSET = -12.0f;   // Hạ cá người chơi để nằm giữa hai hàm
    constexpr float SHARK_HUNT_FAILSAFE_TIME = 12.0f;    // Hunt quá lâu -> tự kết thúc (thả người chơi)
    constexpr float SHARK_HUNT_THROW_SPEED = 2400.0f;    // Knockback ban đầu (giảm 8.5/s -> trượt ~280u)
    constexpr float SHARK_HUNT_THROW_IFRAMES = 2.2f;     // Bất tử sau khi ném (bao cả lúc trượt + ~1.5s)
    constexpr float SHARK_HUNT_MISS_RECOVER = 1.5f;      // Cắn hụt -> đứng phơi, nhận x1.5 sát thương
    constexpr float SHARK_HUNT_BURST_POSE_TIME = 0.35f;  // Ngóc đầu khi phóng lên
    // Camera cận cảnh khi bị ngoạm
    constexpr float CAM_CINE_DISTANCE = 300.0f;
    constexpr float CAM_CINE_PITCH_DEG = 34.0f;
    constexpr float CAM_CINE_BLEND_SPEED = 4.0f;         // 1 - exp(-k dt)


    // ================= BOSS #3: GIANT SQUID =================
    constexpr float SQUID_MAX_HP = 850.0f;
    constexpr float SQUID_HITBOX_RADIUS = 50.0f;
    constexpr float SQUID_SPAWN_DIST = 380.0f;
    constexpr float SQUID_HOVER_HEIGHT = 48.0f;
    constexpr float SQUID_HOVER_BOB_AMP = 8.0f;
    constexpr float SQUID_HOVER_BOB_SPEED = 1.6f;
    constexpr float SQUID_MOVE_SPEED = 150.0f;
    constexpr float SQUID_TURN_RATE = 3.5f;
    constexpr float SQUID_PREFERRED_RANGE = 330.0f;
    constexpr float SQUID_RANGE_TOLERANCE = 60.0f;
    constexpr float SQUID_STRAFE_SPEED = 110.0f;
    constexpr float SQUID_STRAFE_SWITCH_TIME = 3.5f;
    constexpr float SQUID_WEAK_LOCAL_Y = 35.0f;
    constexpr float SQUID_WEAK_LOCAL_Z = 0.0f;
    constexpr float SQUID_WEAK_RADIUS = 95.0f;
    constexpr float SQUID_WEAK_DAMAGE_MUL = 1.5f;
    constexpr float SQUID_RECOVER_DAMAGE_MUL = 1.25f;
    constexpr float SQUID_SLAM_TELEGRAPH = 0.85f;
    constexpr float SQUID_SLAM_TELEGRAPH_ENRAGED = 0.60f;
    constexpr float SQUID_SLAM_LOCK_TIME = 0.20f;
    constexpr float SQUID_SLAM_COLUMN_RADIUS = 70.0f;
    constexpr float SQUID_SLAM_DAMAGE = 22.0f;
    constexpr float SQUID_SLAM_STRIKE_TIME = 0.18f;
    constexpr float SQUID_SLAM_RECOVER = 0.90f;
    constexpr float SQUID_SLAM_RECOVER_ENRAGED = 0.60f;
    constexpr float SQUID_INK_CAST_TIME = 0.70f;
    constexpr float SQUID_INK_DARK_TIME = 2.20f;
    constexpr int   SQUID_INK_DARK_ALPHA = 165;
    constexpr float SQUID_INK_SLOW_FACTOR = 0.65f;
    constexpr float SQUID_INK_SLOW_TIME = 1.20f;
    constexpr float SQUID_INK_COOLDOWN = 9.0f;
    constexpr int   SQUID_INKJET_COUNT = 3;
    constexpr int   SQUID_INKJET_COUNT_ENRAGED = 5;
    constexpr float SQUID_INKJET_INTERVAL = 0.28f;
    constexpr float SQUID_INKJET_SPEED = 420.0f;
    constexpr float SQUID_INKJET_DAMAGE = 12.0f;
    constexpr float SQUID_INKJET_RADIUS = 14.0f;
    constexpr float SQUID_INKJET_LIFE = 3.0f;
    constexpr int   SQUID_INK_MAX_SHOTS = 24;
    constexpr float SQUID_GRAB_RANGE = 260.0f;
    constexpr float SQUID_GRAB_LOW_HP_RATIO = 0.30f;
    constexpr float SQUID_GRAB_TELEGRAPH = 0.60f;
    constexpr float SQUID_GRAB_PULL_TIME = 0.80f;
    constexpr float SQUID_GRAB_PULL_SPEED = 380.0f;
    constexpr float SQUID_GRAB_HIT_RANGE = 130.0f;
    constexpr float SQUID_GRAB_DAMAGE = 15.0f;
    constexpr float SQUID_GRAB_RECOVER = 0.80f;
    constexpr float SQUID_GRAB_COOLDOWN = 11.0f;
    constexpr float SQUID_GRAB_FIRST_DELAY = 6.0f;
    constexpr float SQUID_ENRAGE_HP_RATIO = 0.30f;
    constexpr int   SQUID_BURST_SIZE = 2;
    constexpr int   SQUID_BURST_SIZE_ENRAGED = 3;
    constexpr float SQUID_BURST_GAP = 0.55f;
    constexpr float SQUID_BURST_GAP_ENRAGED = 0.30f;
    constexpr float SQUID_REST_TIME = 1.60f;
    constexpr float SQUID_REST_TIME_ENRAGED = 0.90f;
    constexpr float SQUID_WEIGHT_SLAM = 1.0f;
    constexpr float SQUID_WEIGHT_SLAM_ENRAGED = 1.4f;
    constexpr float SQUID_WEIGHT_INK_SPRAY = 0.7f;
    constexpr float SQUID_WEIGHT_INKJET = 0.9f;
    constexpr float SQUID_WEIGHT_GRAB = 1.1f;

    // UI: bảng thông tin & ngôn ngữ (Localization)
    constexpr bool INFO_PANELS_DEFAULT_VISIBLE = true;   // F11 bật/tắt bảng stats/thẻ/debug/hướng dẫn
    constexpr bool DEBUG_HUD_DEFAULT_VISIBLE = false;    // F1 Debug HUD (cheat keys) mặc định tắt
    constexpr int KEY_TOGGLE_PANELS = 0x57;              // KEY_INPUT_F11
    constexpr int KEY_LANGUAGE_SELECT = 0x26;            // KEY_INPUT_L (ở màn Title)
    constexpr const wchar_t* FONT_LATIN_PATH = L"Data/Font/Yabikoma-e9W16.otf";
    constexpr const wchar_t* FONT_LATIN_NAME = L"Yabikoma";
    constexpr const wchar_t* FONT_JP_PATH = L"Data/Font/MPLUSRounded1c-Bold.ttf"; // SIL OFL 1.1 (Data/Font/OFL.txt)
    constexpr const wchar_t* FONT_JP_NAME = L"Rounded Mplus 1c Bold";
    constexpr const wchar_t* FONT_JP_FALLBACK_NAME = L"Meiryo";
}
