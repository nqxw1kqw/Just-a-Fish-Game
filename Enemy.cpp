#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "Enemy.h"
#include "PlayerFish.h"
#include "Effects.h"
#include "Render3DUtil.h"
#include "DxLib.h"

namespace
{
    float RandomRange(float minVal, float maxVal)
    {
        float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        return minVal + r * (maxVal - minVal);
    }
}

void EnemyManager::Init()
{
    // Nạp texture bảng màu colormap.png dùng chung cho pack PreModelTest
    if (sharedTextureHandle_ < 0)
    {
        sharedTextureHandle_ = MV1LoadTexture(L"Data/Model/PreModelTest/colormap.png");
        if (sharedTextureHandle_ < 0)
        {
            sharedTextureHandle_ = LoadGraph(L"Data/Model/PreModelTest/colormap.png");
        }
        if (sharedTextureHandle_ < 0)
        {
            sharedTextureHandle_ = MV1LoadTexture(L"Data/Model/PreModelTest/DiffuseColor_Texture.png");
        }
        AppLogAdd(L"[ENEMY_MODEL] Shared texture colormap.png handle=%d\n", sharedTextureHandle_);
    }

    struct ModelLoadInfo
    {
        EnemyType type;
        const wchar_t* path;
        const wchar_t* name;
        float defaultScale;
        float defaultRotY;
        float defaultOffsetY;
    };

    const ModelLoadInfo loadList[ENEMY_TYPE_COUNT] = {
        { EnemyType::Minion, L"Data/Model/PreModelTest/Pre_Minion.mv1",    L"Minion", GameConsts::ENEMY_MODEL_SCALE_MINION, GameConsts::ENEMY_MODEL_ROT_Y_MINION, GameConsts::ENEMY_MODEL_OFFSET_Y_MINION },
        { EnemyType::Hopper, L"Data/Model/PreModelTest/Pre_Carb.mv1",      L"Hopper", GameConsts::ENEMY_MODEL_SCALE_HOPPER, GameConsts::ENEMY_MODEL_ROT_Y_HOPPER, GameConsts::ENEMY_MODEL_OFFSET_Y_HOPPER },
        { EnemyType::Tanker, L"Data/Model/PreModelTest/Pre_Turtle.mv1",    L"Tanker", GameConsts::ENEMY_MODEL_SCALE_TANKER, GameConsts::ENEMY_MODEL_ROT_Y_TANKER, GameConsts::ENEMY_MODEL_OFFSET_Y_TANKER },
        { EnemyType::Float,  L"Data/Model/PreModelTest/Pre_Jellyfish.mv1", L"Float",  GameConsts::ENEMY_MODEL_SCALE_FLOAT,  GameConsts::ENEMY_MODEL_ROT_Y_FLOAT,  GameConsts::ENEMY_MODEL_OFFSET_Y_FLOAT },
        { EnemyType::Charge, L"Data/Model/PreModelTest/Pre_Shark.mv1",     L"Charge", GameConsts::ENEMY_MODEL_SCALE_CHARGE, GameConsts::ENEMY_MODEL_ROT_Y_CHARGE, GameConsts::ENEMY_MODEL_OFFSET_Y_CHARGE }
    };

    for (const auto& item : loadList)
    {
        size_t idx = static_cast<size_t>(item.type);
        auto& cfg = modelConfigs_[idx];
        if (cfg.handle < 0)
        {
            cfg.handle = MV1LoadModel(item.path);
            if (cfg.handle < 0 && item.type == EnemyType::Hopper)
            {
                // Fallback mô hình Cua cũ nếu Pre_Carb không nạp được
                cfg.handle = MV1LoadModel(L"Data/Model/Kani/kanisan3D.mv1");
                if (cfg.handle < 0)
                {
                    cfg.handle = MV1LoadModel(L"Data/Model/kanisan3D.mv1");
                }
                if (cfg.handle >= 0)
                {
                    fallbackCrabHandle_ = cfg.handle;
                    if (fallbackCrabTexHandle_ < 0)
                    {
                        fallbackCrabTexHandle_ = LoadGraph(L"Data/Model/Kani/kani.png");
                        if (fallbackCrabTexHandle_ < 0)
                        {
                            fallbackCrabTexHandle_ = LoadGraph(L"Data/Model/Kani/kanisan3D.png");
                        }
                    }
                    int texNum = MV1GetTextureNum(cfg.handle);
                    for (int t = 0; t < texNum; ++t)
                    {
                        if (fallbackCrabTexHandle_ >= 0)
                        {
                            MV1SetTextureGraphHandle(cfg.handle, t, fallbackCrabTexHandle_, FALSE);
                        }
                    }
                    cfg.scale = GameConsts::CRAB_MODEL_SCALE;
                    cfg.rotYDeg = GameConsts::CRAB_MODEL_ROT_Y_DEG;
                    cfg.offsetY = GameConsts::CRAB_MODEL_OFFSET_Y;
                    AppLogAdd(L"[ENEMY_MODEL] Fallback Hopper to kanisan3D SUCCESS, handle=%d\n", cfg.handle);
                    continue;
                }
            }

            if (cfg.handle >= 0)
            {
                AppLogAdd(L"[ENEMY_MODEL] Load %s (%s) SUCCESS, handle=%d\n", item.name, item.path, cfg.handle);

                int meshNum = MV1GetMeshNum(cfg.handle);
                VECTOR minP = VGet(1e9f, 1e9f, 1e9f);
                VECTOR maxP = VGet(-1e9f, -1e9f, -1e9f);
                for (int m = 0; m < meshNum; ++m)
                {
                    VECTOR mi = MV1GetMeshMinPosition(cfg.handle, m);
                    VECTOR ma = MV1GetMeshMaxPosition(cfg.handle, m);
                    if (mi.x < minP.x) minP.x = mi.x;
                    if (mi.y < minP.y) minP.y = mi.y;
                    if (mi.z < minP.z) minP.z = mi.z;
                    if (ma.x > maxP.x) maxP.x = ma.x;
                    if (ma.y > maxP.y) maxP.y = ma.y;
                    if (ma.z > maxP.z) maxP.z = ma.z;
                }
                AppLogAdd(L"[ENEMY_MODEL] %s Meshes=%d, Min=(%.2f, %.2f, %.2f), Max=(%.2f, %.2f, %.2f)\n",
                    item.name, meshNum, minP.x, minP.y, minP.z, maxP.x, maxP.y, maxP.z);
                AppLogAdd(L"[ENEMY_MODEL] %s Size=(%.2f, %.2f, %.2f), Center=(%.2f, %.2f, %.2f)\n",
                    item.name, maxP.x - minP.x, maxP.y - minP.y, maxP.z - minP.z,
                    (minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f);

                int animNum = MV1GetAnimNum(cfg.handle);
                int matNum = MV1GetMaterialNum(cfg.handle);
                int texNum = MV1GetTextureNum(cfg.handle);

                // Gắn texture colormap.png
                bool boundTex = false;
                for (int t = 0; t < texNum; ++t)
                {
                    const TCHAR* tName = MV1GetTextureName(cfg.handle, t);
                    int gh = MV1GetTextureGraphHandle(cfg.handle, t);
                    AppLogAdd(L"[ENEMY_MODEL] %s Tex[%d]: '%s', gh=%d\n", item.name, t, tName ? tName : L"null", gh);
                    if (sharedTextureHandle_ >= 0)
                    {
                        MV1SetTextureGraphHandle(cfg.handle, t, sharedTextureHandle_, FALSE);
                        MV1SetTextureColorFilePath(cfg.handle, t, L"Data/Model/PreModelTest/colormap.png");
                        boundTex = true;
                    }
                    else if (gh > 0)
                    {
                        boundTex = true;
                    }
                }
                cfg.hasTexture = boundTex;

                // Cấu hình vật liệu
                if (cfg.hasTexture)
                {
                    for (int m = 0; m < matNum; ++m)
                    {
                        MV1SetMaterialDifMapTexture(cfg.handle, m, 0);
                        MV1SetMaterialDifColor(cfg.handle, m, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
                        MV1SetMaterialAmbColor(cfg.handle, m, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                        MV1SetMaterialSpcColor(cfg.handle, m, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                        MV1SetMaterialEmiColor(cfg.handle, m, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                    }
                }

                // Gắn hoạt ảnh (nếu có: anim 2 = walk, hoặc anim 0)
                if (animNum > 2)
                {
                    cfg.animAttachIndex = MV1AttachAnim(cfg.handle, 2);
                    cfg.animTotalTime = MV1GetAnimTotalTime(cfg.handle, 2);
                    AppLogAdd(L"[ENEMY_MODEL] %s Attached anim 2 (walk), TotalTime=%.2f, attachIndex=%d\n",
                        item.name, cfg.animTotalTime, cfg.animAttachIndex);
                }
                else if (animNum > 0)
                {
                    cfg.animAttachIndex = MV1AttachAnim(cfg.handle, 0);
                    cfg.animTotalTime = MV1GetAnimTotalTime(cfg.handle, 0);
                    AppLogAdd(L"[ENEMY_MODEL] %s Attached anim 0, TotalTime=%.2f, attachIndex=%d\n",
                        item.name, cfg.animTotalTime, cfg.animAttachIndex);
                }

                cfg.scale = item.defaultScale;
                cfg.rotYDeg = item.defaultRotY;
                if (minP.y < -0.1f)
                {
                    cfg.offsetY = -minP.y * cfg.scale;
                }
                else
                {
                    cfg.offsetY = item.defaultOffsetY;
                }

                AppLogAdd(L"[ENEMY_MODEL] %s Final Config: Scale=%.2f, RotY=%.1f, OffsetY=%.2f\n",
                    item.name, cfg.scale, cfg.rotYDeg, cfg.offsetY);
            }
            else
            {
                AppLogAdd(L"[ENEMY_MODEL] WARNING: Failed to load %s (%s), fallback to 3D box.\n", item.name, item.path);
            }
        }
    }

    Reset();
}

void EnemyManager::Release()
{
    for (size_t i = 0; i < ENEMY_TYPE_COUNT; ++i)
    {
        auto& cfg = modelConfigs_[i];
        if (cfg.handle >= 0)
        {
            if (cfg.animAttachIndex >= 0)
            {
                MV1DetachAnim(cfg.handle, cfg.animAttachIndex);
                cfg.animAttachIndex = -1;
            }
            MV1DeleteModel(cfg.handle);
            cfg.handle = -1;
        }
    }

    if (fallbackCrabHandle_ >= 0)
    {
        MV1DeleteModel(fallbackCrabHandle_);
        fallbackCrabHandle_ = -1;
    }
    if (fallbackCrabTexHandle_ >= 0)
    {
        DeleteGraph(fallbackCrabTexHandle_);
        fallbackCrabTexHandle_ = -1;
    }
    if (sharedTextureHandle_ >= 0)
    {
        DeleteGraph(sharedTextureHandle_);
        sharedTextureHandle_ = -1;
    }
}

int EnemyManager::GetModelHandle(EnemyType type) const
{
    size_t idx = static_cast<size_t>(type);
    return (idx < ENEMY_TYPE_COUNT) ? modelConfigs_[idx].handle : -1;
}

float EnemyManager::GetModelScale(EnemyType type) const
{
    size_t idx = static_cast<size_t>(type);
    return (idx < ENEMY_TYPE_COUNT) ? modelConfigs_[idx].scale : 1.0f;
}

void EnemyManager::SetModelScale(EnemyType type, float s)
{
    size_t idx = static_cast<size_t>(type);
    if (idx < ENEMY_TYPE_COUNT) modelConfigs_[idx].scale = s;
}

float EnemyManager::GetModelRotYDeg(EnemyType type) const
{
    size_t idx = static_cast<size_t>(type);
    return (idx < ENEMY_TYPE_COUNT) ? modelConfigs_[idx].rotYDeg : 0.0f;
}

void EnemyManager::SetModelRotYDeg(EnemyType type, float deg)
{
    size_t idx = static_cast<size_t>(type);
    if (idx < ENEMY_TYPE_COUNT) modelConfigs_[idx].rotYDeg = deg;
}

float EnemyManager::GetModelOffsetY(EnemyType type) const
{
    size_t idx = static_cast<size_t>(type);
    return (idx < ENEMY_TYPE_COUNT) ? modelConfigs_[idx].offsetY : 0.0f;
}

void EnemyManager::SetModelOffsetY(EnemyType type, float y)
{
    size_t idx = static_cast<size_t>(type);
    if (idx < ENEMY_TYPE_COUNT) modelConfigs_[idx].offsetY = y;
}

void EnemyManager::Reset()
{
    for (auto& e : enemies_) e.active = false;
    spawnTimer_ = 0.0f;
    waveTimer_ = 0.0f;
    currentWave_ = 1;
    autoSpawn_ = true;
    spawnedThisWave_ = 0;
    killedThisWave_ = 0;
    totalKills_ = 0;
}

void EnemyManager::SetWave(int wave, bool isEndless)
{
    currentWave_ = wave;
    isEndless_ = isEndless;
    waveTimer_ = 0.0f;
    spawnTimer_ = 0.0f;
    spawnedThisWave_ = 0;
    killedThisWave_ = 0;

    if (isEndless_)
    {
        // Công thức scaling quái Endless có trần (Task 10 T4b)
        hpMultiplier_ = (std::min)(2.5f, 1.0f + 0.06f * (wave - 1));
        dmgMultiplier_ = (std::min)(1.6f, 1.0f + 0.03f * (wave - 1));
        spdMultiplier_ = (std::min)(1.15f, 1.0f + 0.005f * (wave - 1));
    }
    else
    {
        hpMultiplier_ = 1.0f;
        dmgMultiplier_ = 1.0f;
        spdMultiplier_ = 1.0f;
    }
}

float EnemyManager::GetAliveCost() const
{
    float total = 0.0f;
    for (const auto& e : enemies_)
    {
        if (!e.active) continue;
        switch (e.type)
        {
        case EnemyType::Minion: total += GameConsts::ENEMY_COST_MINION; break;
        case EnemyType::Hopper: total += GameConsts::ENEMY_COST_HOPPER; break;
        case EnemyType::Float:  total += GameConsts::ENEMY_COST_FLOAT; break;
        case EnemyType::Charge: total += GameConsts::ENEMY_COST_CHARGE; break;
        case EnemyType::Tanker: total += GameConsts::ENEMY_COST_TANKER; break;
        }
    }
    return total;
}

float EnemyManager::GetWaveCostCap(int wave) const
{
    if (isEndless_)
    {
        // Trần chi phí quái sống trong Endless: min(CAP_MAX, 14 + 2.5 * n) (Task 10 T4b)
        return (std::min)(GameConsts::ENDLESS_COST_CAP_MAX, 14.0f + 2.5f * wave);
    }

    switch (wave)
    {
    case 1: return GameConsts::WAVE_COST_CAP_W1;
    case 2: return GameConsts::WAVE_COST_CAP_W2;
    case 3: return GameConsts::WAVE_COST_CAP_W3;
    case 4: return GameConsts::WAVE_COST_CAP_W4;
    default: return GameConsts::WAVE_COST_CAP_W5;
    }
}

float EnemyManager::GetEnemyCost(EnemyType type)
{
    switch (type)
    {
    case EnemyType::Minion: return GameConsts::ENEMY_COST_MINION;
    case EnemyType::Hopper: return GameConsts::ENEMY_COST_HOPPER;
    case EnemyType::Float:  return GameConsts::ENEMY_COST_FLOAT;
    case EnemyType::Charge: return GameConsts::ENEMY_COST_CHARGE;
    case EnemyType::Tanker: return GameConsts::ENEMY_COST_TANKER;
    default: return 1.0f;
    }
}

bool EnemyManager::SpawnEnemy(EnemyType type, const Vec3& pos)
{
    for (auto& e : enemies_)
    {
        if (!e.active)
        {
            e.type = type;
            e.pos = pos;
            e.knockback = Vec3{ 0, 0, 0 };
            e.hitFlashTimer = 0.0f;
            e.animTime = 0.0f;
            e.active = true;
            spawnedThisWave_++;

            switch (type)
            {
            case EnemyType::Minion:
                e.hp = e.maxHp = 26.0f * hpMultiplier_;
                e.speed = 110.0f * spdMultiplier_;
                e.radius = 13.0f;
                e.damage = 5.0f * dmgMultiplier_;
                e.pos.y = 0.0f;
                break;
            case EnemyType::Hopper:
                e.hp = e.maxHp = 38.0f * hpMultiplier_;
                e.speed = 135.0f * spdMultiplier_;
                e.radius = 15.0f;
                e.damage = 7.0f * dmgMultiplier_;
                e.bouncePhase = RandomRange(0.0f, 1.0f);
                e.bounceHeight = 28.0f;
                e.bounceDuration = 0.36f;
                break;
            case EnemyType::Tanker:
                e.hp = e.maxHp = 130.0f * hpMultiplier_;
                e.speed = 52.0f * spdMultiplier_;
                e.radius = 24.0f;
                e.damage = 12.0f * dmgMultiplier_;
                e.pos.y = 0.0f;
                break;
            case EnemyType::Float:
                e.hp = e.maxHp = 32.0f * hpMultiplier_;
                e.speed = 60.0f * spdMultiplier_;
                e.radius = 16.0f;
                e.damage = 7.0f * dmgMultiplier_;
                e.floatPhase = RandomRange(0.0f, 6.28f);
                e.pos.y = 16.0f;
                break;
            case EnemyType::Charge:
                e.hp = e.maxHp = 45.0f * hpMultiplier_;
                e.speed = 92.0f * spdMultiplier_;
                e.radius = 17.0f;
                e.damage = 12.0f * dmgMultiplier_;
                e.chargeState = 0;
                e.chargeTimer = 0.0f;
                e.chargeDir = Vec3{ 0, 0, 1 };
                e.pos.y = 0.0f;
                break;
            }
            return true;
        }
    }
    return false; // Mảng MAX_ENEMIES đã đầy
}

bool EnemyManager::SpawnRandomEdge(EnemyType type, const Vec3& playerPos)
{
    float half = GameConsts::ARENA_HALF_SIZE - 20.0f;
    float minDistSq = GameConsts::SPAWN_MIN_DIST_FROM_PLAYER * GameConsts::SPAWN_MIN_DIST_FROM_PLAYER;

    for (int attempt = 0; attempt < 4; ++attempt)
    {
        int side = rand() % 4;
        Vec3 spawnPos{ 0, 0, 0 };

        switch (side)
        {
        case 0: // Cạnh Bắc
            spawnPos = Vec3{ RandomRange(-half, half), 0, half };
            break;
        case 1: // Cạnh Nam
            spawnPos = Vec3{ RandomRange(-half, half), 0, -half };
            break;
        case 2: // Cạnh Đông
            spawnPos = Vec3{ half, 0, RandomRange(-half, half) };
            break;
        case 3: // Cạnh Tây
            spawnPos = Vec3{ -half, 0, RandomRange(-half, half) };
            break;
        }

        // Kiểm tra khoảng cách tối thiểu với người chơi (Task 10 T2: không sinh sát mặt player)
        Vec3 diff = spawnPos - playerPos;
        diff.y = 0.0f;
        if (diff.LengthSq() >= minDistSq)
        {
            return SpawnEnemy(type, spawnPos);
        }
    }

    return false; // Hết 4 lượt thử đều quá gần người chơi thì bỏ qua
}

void EnemyManager::Update(float dt, PlayerFish& player, EffectSystem& effects)
{
    waveTimer_ += dt;
    spawnTimer_ += dt;

    // Tự động sinh quái theo đợt có ngân sách và nhóm nhỏ (Task 10 T2)
    if (autoSpawn_)
    {
        float baseInterval = 1.25f - (currentWave_ - 1) * 0.16f;
        float minInterval = 0.40f - (currentWave_ - 1) * 0.02f;
        float spawnInterval = (std::max)(minInterval, baseInterval - waveTimer_ * 0.010f);
        if (spawnTimer_ >= spawnInterval)
        {
            // Sinh theo nhóm nhỏ: W1-2 tối đa 2 con, W3+ tối đa 3 con
            int maxGroup = (currentWave_ <= 2) ? 2 : 3;
            int groupSize = 1 + (rand() % maxGroup);
            // Nhân khoảng nghỉ với số con trong nhóm
            spawnTimer_ = -spawnInterval * (groupSize - 1);

            for (int g = 0; g < groupSize; ++g)
            {
                // Giữ nguyên tỉ lệ phân bố quái vật của từng wave
                int roll = rand() % 100;
                EnemyType typeToSpawn = EnemyType::Minion;

                if (isEndless_)
                {
                    // Endless Mode: Dịch dần sang quái Tanker/Charge theo bậc 5 wave (Task 10 T4b)
                    int tier = (currentWave_ - 1) / 5;
                    if (tier == 0) // Wave 1..5
                    {
                        if (roll < 45) typeToSpawn = EnemyType::Minion;
                        else if (roll < 70) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 85) typeToSpawn = EnemyType::Tanker;
                        else if (roll < 93) typeToSpawn = EnemyType::Float;
                        else typeToSpawn = EnemyType::Charge;
                    }
                    else if (tier == 1) // Wave 6..10
                    {
                        if (roll < 30) typeToSpawn = EnemyType::Minion;
                        else if (roll < 55) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 75) typeToSpawn = EnemyType::Tanker;
                        else if (roll < 88) typeToSpawn = EnemyType::Float;
                        else typeToSpawn = EnemyType::Charge;
                    }
                    else if (tier == 2) // Wave 11..15
                    {
                        if (roll < 20) typeToSpawn = EnemyType::Minion;
                        else if (roll < 40) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 65) typeToSpawn = EnemyType::Tanker;
                        else if (roll < 82) typeToSpawn = EnemyType::Float;
                        else typeToSpawn = EnemyType::Charge;
                    }
                    else // Wave 16+
                    {
                        if (roll < 15) typeToSpawn = EnemyType::Minion;
                        else if (roll < 30) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 55) typeToSpawn = EnemyType::Tanker;
                        else if (roll < 78) typeToSpawn = EnemyType::Float;
                        else typeToSpawn = EnemyType::Charge;
                    }
                }
                else
                {
                    // Chế độ Classic: Giữ nguyên 100% tỉ lệ phân bố của 5 wave gốc
                    if (currentWave_ <= 1)
                    {
                        if (roll < 75) typeToSpawn = EnemyType::Minion;
                        else typeToSpawn = EnemyType::Hopper;
                    }
                    else if (currentWave_ == 2)
                    {
                        if (roll < 70) typeToSpawn = EnemyType::Minion;
                        else typeToSpawn = EnemyType::Hopper;
                    }
                    else if (currentWave_ == 3)
                    {
                        if (roll < 45) typeToSpawn = EnemyType::Minion;
                        else if (roll < 75) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 90) typeToSpawn = EnemyType::Tanker;
                        else typeToSpawn = EnemyType::Float;
                    }
                    else if (currentWave_ == 4)
                    {
                        if (roll < 30) typeToSpawn = EnemyType::Minion;
                        else if (roll < 55) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 75) typeToSpawn = EnemyType::Tanker;
                        else if (roll < 88) typeToSpawn = EnemyType::Float;
                        else typeToSpawn = EnemyType::Charge;
                    }
                    else // Wave 5: Tổng lực hỗn hợp mọi chủng loại quái
                    {
                        if (roll < 20) typeToSpawn = EnemyType::Minion;
                        else if (roll < 40) typeToSpawn = EnemyType::Hopper;
                        else if (roll < 60) typeToSpawn = EnemyType::Tanker;
                        else if (roll < 80) typeToSpawn = EnemyType::Float;
                        else typeToSpawn = EnemyType::Charge;
                    }
                }

                // Kiểm tra trần chi phí quái sống của wave: chỉ sinh nếu không vượt trần
                float cost = GetEnemyCost(typeToSpawn);
                if (GetAliveCost() + cost <= GetWaveCostCap(currentWave_))
                {
                    SpawnRandomEdge(typeToSpawn, player.GetPosition());
                }
                else
                {
                    break; // Đã chạm trần ngân sách quái sống
                }
            }
        }
    }

    const Vec3 playerPos = player.GetPosition();
    const float playerHitRadius = player.GetHitboxRadius();

    // 1. Cập nhật vị trí và hành vi quái
    for (size_t i = 0; i < enemies_.size(); ++i)
    {
        auto& e = enemies_[i];
        if (!e.active) continue;

        if (e.hitFlashTimer > 0.0f) e.hitFlashTimer -= dt;

        // Giảm dần lực đẩy knockback
        e.pos += e.knockback * dt;
        e.knockback = e.knockback * (std::max)(0.0f, 1.0f - dt * 7.5f);

        // Xử lý bị choáng (J6)
        if (e.stunTimer > 0.0f)
        {
            e.stunTimer -= dt;
            // Khi bị choáng thì đứng khựng lại, không rượt cá
        }
        else
        {
            Vec3 toPlayer = playerPos - e.pos;
            toPlayer.y = 0.0f;
            float distToPlayer = toPlayer.Length();

            if (e.type == EnemyType::Charge)
            {
                // Hành vi quái cá mập Charge (G4)
                if (e.chargeState == 0) // Rình rập / áp sát
                {
                    if (distToPlayer > 0.1f)
                    {
                        Vec3 moveDir = toPlayer / distToPlayer;
                        e.yaw = std::atan2(moveDir.x, moveDir.z);
                        e.pos.x += moveDir.x * e.speed * dt;
                        e.pos.z += moveDir.z * e.speed * dt;
                    }
                    if (distToPlayer < 280.0f)
                    {
                        e.chargeState = 1;
                        e.chargeTimer = 0.65f; // Khựng báo hiệu 0.65s
                        e.chargeDir = (distToPlayer > 0.1f) ? (toPlayer / distToPlayer) : Vec3{ 0, 0, 1 };
                        e.yaw = std::atan2(e.chargeDir.x, e.chargeDir.z);
                    }
                }
                else if (e.chargeState == 1) // Khựng nạp lực báo hiệu
                {
                    e.chargeTimer -= dt;
                    if (e.chargeTimer <= 0.0f)
                    {
                        e.chargeState = 2;
                        e.chargeTimer = 0.65f; // Lao tới trong 0.65s
                    }
                }
                else if (e.chargeState == 2) // Lao thẳng tốc độ cực nhanh
                {
                    e.chargeTimer -= dt;
                    e.pos.x += e.chargeDir.x * (e.speed * 3.2f) * dt;
                    e.pos.z += e.chargeDir.z * (e.speed * 3.2f) * dt;
                    effects.SpawnHitParticles(e.pos + Vec3{ 0, 4, 0 }, 1, GetColor(255, 120, 60));
                    if (e.chargeTimer <= 0.0f)
                    {
                        e.chargeState = 3;
                        e.chargeTimer = 0.9f; // Hồi phục
                    }
                }
                else if (e.chargeState == 3) // Hồi phục sau cú lao
                {
                    e.chargeTimer -= dt;
                    e.pos.x += e.chargeDir.x * (e.speed * 0.25f) * dt;
                    e.pos.z += e.chargeDir.z * (e.speed * 0.25f) * dt;
                    if (e.chargeTimer <= 0.0f)
                    {
                        e.chargeState = 0;
                    }
                }
            }
            else
            {
                // Quái thường Minion, Hopper, Tanker, Float
                if (distToPlayer > 0.1f)
                {
                    Vec3 moveDir = toPlayer / distToPlayer;
                    e.yaw = std::atan2(moveDir.x, moveDir.z);

                    // Di chuyển
                    e.pos.x += moveDir.x * e.speed * dt;
                    e.pos.z += moveDir.z * e.speed * dt;
                }
            }
        }

        // Quản lý cao độ Y
        if (e.type == EnemyType::Hopper)
        {
            e.bouncePhase += dt / e.bounceDuration;
            if (e.bouncePhase >= 1.0f) e.bouncePhase -= 1.0f;
            float t = e.bouncePhase;
            e.pos.y = 4.0f * e.bounceHeight * t * (1.0f - t);
        }
        else if (e.type == EnemyType::Float)
        {
            e.floatPhase += dt * 3.5f;
            e.pos.y = 16.0f + 5.0f * std::sin(e.floatPhase); // Sứa bay lơ lửng
        }
        else
        {
            e.pos.y = 0.0f;
        }

        // Giới hạn trong sàn đấu
        float limit = GameConsts::ARENA_HALF_SIZE - e.radius;
        e.pos.x = std::clamp(e.pos.x, -limit, limit);
        e.pos.z = std::clamp(e.pos.z, -limit, limit);

        // Đẩy nhẹ các quái khác để tránh chồng chéo (Separation)
        for (size_t j = i + 1; j < enemies_.size(); ++j)
        {
            auto& other = enemies_[j];
            if (!other.active) continue;

            Vec3 diff = e.pos - other.pos;
            diff.y = 0.0f;
            float dSq = diff.LengthSq();
            float minD = e.radius + other.radius;
            if (dSq < minD * minD && dSq > 0.001f)
            {
                float d = std::sqrt(dSq);
                Vec3 push = (diff / d) * (minD - d) * 0.5f;
                e.pos.x += push.x;
                e.pos.z += push.z;
                other.pos.x -= push.x;
                other.pos.z -= push.z;
            }
        }

        // 2. Va chạm với cá (Gây sát thương nếu cá không ở trên không - J4)
        Vec3 toPlayer = playerPos - e.pos;
        toPlayer.y = 0.0f;
        float distToPlayer = toPlayer.Length();
        float totalRad = e.radius + playerHitRadius;
        if (distToPlayer <= totalRad)
        {
            Vec3 pushDir = (distToPlayer > 0.001f) ? (toPlayer / distToPlayer) : Vec3{ 0.0f, 0.0f, 1.0f };

            if (player.TakeDamage(e.damage, effects))
            {
                // Knockback nhẹ cho cả hai:
                // - Quái bị nảy lùi về phía sau (ngược hướng lao tới)
                e.knockback = -pushDir * 180.0f;

                // - Cá chính bị nảy lùi nhẹ theo hướng bị húc
                player.ApplyKnockback(pushDir * 160.0f);
            }
            else if (!player.IsAirborne())
            {
                // Khi cá ở dưới đất nhưng đang trong thời gian I-Frame: đẩy nhẹ tách hitbox để không lồng vào nhau
                float overlap = (totalRad - distToPlayer) * 0.5f;
                e.pos -= pushDir * overlap;
            }
        }

        // 3. Cập nhật hoạt ảnh 3D
        size_t typeIdx = static_cast<size_t>(e.type);
        if (typeIdx < ENEMY_TYPE_COUNT)
        {
            const auto& cfg = modelConfigs_[typeIdx];
            if (cfg.handle >= 0 && cfg.animAttachIndex >= 0 && cfg.animTotalTime > 0.0f)
            {
                float animRate = (e.type == EnemyType::Charge && e.chargeState == 2) ? 2.5f : 1.2f;
                e.animTime += dt * animRate * 30.0f;
                while (e.animTime >= cfg.animTotalTime) e.animTime -= cfg.animTotalTime;
            }
        }
    }
}

int EnemyManager::ApplyShockwave(const Vec3& center, float radius, float damage, float knockbackStrength, EffectSystem& effects)
{
    int hitCount = 0;
    float radSq = radius * radius;
    for (auto& e : enemies_)
    {
        if (!e.active) continue;

        Vec3 diff = e.pos - center;
        diff.y = 0.0f;
        float dSq = diff.LengthSq();

        if (dSq <= radSq)
        {
            hitCount++;
            float dist = std::sqrt((std::max)(0.001f, dSq));
            Vec3 dir = diff / dist;

            // Sát thương giảm nhẹ theo khoảng cách: 100% tại tâm, 60% ở rìa (J6)
            float falloff = 1.0f - 0.4f * (dist / radius);
            float actualDmg = damage * falloff;

            // Quy tắc theo loài:
            // Rùa (Tanker - giáp): bị giẫm thì nhận thêm sát thương (+50%) và bị choáng 0.5s (J6)
            if (e.type == EnemyType::Tanker)
            {
                actualDmg *= 1.5f;
                e.stunTimer = 0.5f;
            }

            e.hp -= actualDmg;
            e.hitFlashTimer = 0.15f;

            bool isCrit = (e.type == EnemyType::Tanker) || (actualDmg >= 50.0f);
            effects.SpawnDamagePopup(e.pos + Vec3{ 0, e.radius + 6.0f, 0 }, actualDmg, isCrit);

            // Đẩy lùi
            float kbFactor = 1.0f - (dist / radius) * 0.4f;
            e.knockback = dir * (knockbackStrength * kbFactor);

            // Bắn hạt nước tung tóe
            effects.SpawnHitParticles(e.pos + Vec3{ 0, 10, 0 }, 6, GetColor(120, 220, 255));

            if (e.hp <= 0.0f)
            {
                e.active = false;
                killedThisWave_++;
                totalKills_++; // Task 10 T4e: đếm tổng quái hạ cho Endless
                effects.SpawnHitParticles(e.pos + Vec3{ 0, 12, 0 }, 18, GetColor(255, 120, 40));
                // G3: Bỏ EXP, quái chết có tỉ lệ 10% rơi vật hồi máu nhỏ (+10 HP)
                if ((rand() % 100) < 10)
                {
                    effects.SpawnExpOrb(e.pos, 10.0f);
                }
            }
        }
    }
    return hitCount;
}

Enemy* EnemyManager::FindClosestEnemy(const Vec3& fromPos, float maxRange)
{
    Enemy* closest = nullptr;
    float closestDistSq = maxRange * maxRange;

    for (auto& e : enemies_)
    {
        if (!e.active) continue;
        Vec3 diff = e.pos - fromPos;
        diff.y = 0.0f;
        float dSq = diff.LengthSq();
        if (dSq < closestDistSq)
        {
            closestDistSq = dSq;
            closest = &e;
        }
    }
    return closest;
}

bool EnemyManager::CheckBulletHit(const Vec3& bulletPos, float bulletRadius, float damage, Vec3 bulletDir, EffectSystem& effects, int& outExp)
{
    outExp = 0;
    for (auto& e : enemies_)
    {
        if (!e.active) continue;

        Vec3 diff = e.pos - bulletPos;
        // Kiểm tra khoảng cách 3D
        float totalRad = e.radius + bulletRadius;
        if (std::abs(diff.y) < 25.0f)
        {
            diff.y = 0.0f;
            if (diff.LengthSq() <= totalRad * totalRad)
            {
                // Trúng đạn!
                e.hp -= damage;
                e.hitFlashTimer = 0.14f;
                e.knockback = bulletDir * 120.0f;

                effects.SpawnHitParticles(bulletPos, 6, GetColor(255, 230, 80));
                effects.SpawnDamagePopup(e.pos + Vec3{ 0, e.radius + 6.0f, 0 }, damage, false);

                if (e.hp <= 0.0f)
                {
                    e.active = false;
                    killedThisWave_++;
                    totalKills_++; // Task 10 T4e: đếm tổng quái hạ cho Endless
                    effects.SpawnHitParticles(e.pos + Vec3{ 0, 12, 0 }, 20, GetColor(255, 90, 70));
                    if ((rand() % 100) < 10)
                    {
                        effects.SpawnExpOrb(e.pos, 10.0f);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

int EnemyManager::GetActiveCount() const
{
    int count = 0;
    for (const auto& e : enemies_)
    {
        if (e.active) count++;
    }
    return count;
}

void EnemyManager::KillAll(EffectSystem& effects)
{
    for (auto& e : enemies_)
    {
        if (e.active)
        {
            e.active = false;
            effects.SpawnHitParticles(e.pos + Vec3{ 0, 10, 0 }, 12, GetColor(255, 120, 60));
        }
    }
}

void EnemyManager::Draw() const
{
    for (const auto& e : enemies_)
    {
        if (!e.active) continue;

        // Vạch đỏ báo hiệu hướng lao thẳng của cá mập trên sàn (G4)
        if (e.type == EnemyType::Charge && e.chargeState == 1)
        {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
            Vec3 pStart = e.pos + Vec3{ 0, 1.2f, 0 };
            Vec3 pEnd = e.pos + e.chargeDir * 280.0f + Vec3{ 0, 1.2f, 0 };
            DxLib::DrawLine3D(DxConv::ToVECTOR(pStart), DxConv::ToVECTOR(pEnd), GetColor(255, 50, 50));
            Render3D::DrawCircle3D(pEnd, 14.0f, 16, GetColor(255, 60, 60), false);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }

        // 1. Bóng dẹt dưới sàn
        float heightRatio = e.pos.y / 35.0f;
        float shadowRad = e.radius * 1.3f * (1.0f - 0.35f * heightRatio);
        Render3D::DrawFlatShadow(e.pos, shadowRad, 0.9f);

        // 2. Mô hình 3D theo từng loại quái thay thế khối box
        size_t typeIdx = static_cast<size_t>(e.type);
        if (typeIdx < ENEMY_TYPE_COUNT && modelConfigs_[typeIdx].handle >= 0)
        {
            const auto& cfg = modelConfigs_[typeIdx];
            VECTOR modelPos = VGet(e.pos.x, e.pos.y + cfg.offsetY, e.pos.z);
            MV1SetPosition(cfg.handle, modelPos);

            float finalYaw = e.yaw + cfg.rotYDeg * (DX_PI_F / 180.0f);
            MV1SetRotationXYZ(cfg.handle, VGet(0.0f, finalYaw, 0.0f));

            MV1SetScale(cfg.handle, VGet(cfg.scale, cfg.scale, cfg.scale));

            if (cfg.animAttachIndex >= 0 && cfg.animTotalTime > 0.0f)
            {
                MV1SetAttachAnimTime(cfg.handle, cfg.animAttachIndex, e.animTime);
            }

            // Hiệu ứng chớp trắng trúng đòn hoặc hiệu ứng nạp lực cá mập
            if (e.hitFlashTimer > 0.0f)
            {
                MV1SetDifColorScale(cfg.handle, GetColorF(3.0f, 3.0f, 3.0f, 1.0f));
            }
            else if (e.type == EnemyType::Charge && e.chargeState == 1)
            {
                // Khựng nạp lực: chớp đỏ rực báo hiệu
                MV1SetDifColorScale(cfg.handle, GetColorF(2.8f, 0.35f, 0.35f, 1.0f));
            }
            else if (e.type == EnemyType::Charge && e.chargeState == 2)
            {
                // Đang lao vút: viền cam rực
                MV1SetDifColorScale(cfg.handle, GetColorF(2.0f, 0.8f, 0.4f, 1.0f));
            }
            else
            {
                MV1SetDifColorScale(cfg.handle, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
            }

            MV1DrawModel(cfg.handle);
            continue;
        }

        // 2. Màu sắc và kích thước theo loại
        int fillColor = 0;
        int edgeColor = 0;
        Vec3 halfSize{ e.radius, e.radius, e.radius };

        switch (e.type)
        {
        case EnemyType::Minion:
            fillColor = GetColor(225, 45, 45); // Đỏ lính
            edgeColor = GetColor(140, 20, 20);
            halfSize = Vec3{ 10.0f, 9.0f, 10.0f };
            break;
        case EnemyType::Hopper:
            fillColor = GetColor(240, 120, 30); // Cam cua
            edgeColor = GetColor(160, 60, 10);
            halfSize = Vec3{ 12.0f, 8.0f, 12.0f };
            break;
        case EnemyType::Tanker:
            fillColor = GetColor(50, 155, 80); // Xanh rùa
            edgeColor = GetColor(20, 85, 40);
            halfSize = Vec3{ 20.0f, 16.0f, 20.0f };
            break;
        case EnemyType::Float:
            fillColor = GetColor(185, 75, 235); // Tím sứa bay
            edgeColor = GetColor(235, 160, 255);
            halfSize = Vec3{ 14.0f, 13.0f, 14.0f };
            break;
        case EnemyType::Charge:
            if (e.chargeState == 1) // Khựng nạp lực báo hiệu
            {
                fillColor = GetColor(255, 45, 45); // Đỏ rực cảnh báo
                edgeColor = GetColor(255, 220, 50);
            }
            else if (e.chargeState == 2) // Đang lao vút
            {
                fillColor = GetColor(235, 70, 70);
                edgeColor = GetColor(255, 140, 40);
            }
            else
            {
                fillColor = GetColor(70, 105, 135); // Xám cá mập
                edgeColor = GetColor(35, 55, 75);
            }
            halfSize = Vec3{ 13.0f, 11.0f, 22.0f };
            break;
        }

        if (e.hitFlashTimer > 0.0f)
        {
            fillColor = GetColor(255, 255, 255);
            edgeColor = GetColor(255, 180, 180);
        }

        Vec3 center = e.pos + Vec3{ 0.0f, halfSize.y, 0.0f };
        Render3D::DrawOrientedBox3D(center, halfSize, e.yaw, fillColor, edgeColor, true);

        // Mắt quái để trông sống động
        float eyeFwd = halfSize.z + 1.5f;
        Vec3 eyePos = center + Vec3{ std::sin(e.yaw) * eyeFwd, 2.0f, std::cos(e.yaw) * eyeFwd };
        Render3D::DrawOrientedBox3D(eyePos, Vec3{ 3.0f, 2.0f, 1.5f }, e.yaw, GetColor(20, 20, 20), 0, false);
    }
}
