#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include "PlayerFish.h"
#include "Effects.h"
#include "Render3DUtil.h"
#include "DxLib.h"

void PlayerFish::Init()
{
    if (fishModelHandle_ < 0)
    {
        // 1. Ưu tiên nạp mô hình PreTest theo yêu cầu của Shin
        fishModelHandle_ = MV1LoadModel(L"Data/Model/PreModelTest/Pre_Fish.mv1");
        if (fishModelHandle_ >= 0)
        {
            AppLogAdd(L"[FISH_MODEL] Load Data/Model/PreModelTest/Pre_Fish.mv1 SUCCESS, handle=%d\n", fishModelHandle_);
        }
        else
        {
            // 2. Fallback về mô hình cũ trong Data/Model/Main/fish.mv1
            fishModelHandle_ = MV1LoadModel(L"Data/Model/Main/fish.mv1");
            if (fishModelHandle_ >= 0)
            {
                AppLogAdd(L"[FISH_MODEL] Load Data/Model/Main/fish.mv1 SUCCESS, handle=%d\n", fishModelHandle_);
            }
            else
            {
                fishModelHandle_ = MV1LoadModel(L"Data/Model/fish.mv1");
            }
        }

        if (fishModelHandle_ >= 0)
        {
            int meshNum = MV1GetMeshNum(fishModelHandle_);
            VECTOR minP = VGet(1e9f, 1e9f, 1e9f);
            VECTOR maxP = VGet(-1e9f, -1e9f, -1e9f);
            for (int i = 0; i < meshNum; ++i)
            {
                VECTOR mi = MV1GetMeshMinPosition(fishModelHandle_, i);
                VECTOR ma = MV1GetMeshMaxPosition(fishModelHandle_, i);
                if (mi.x < minP.x) minP.x = mi.x;
                if (mi.y < minP.y) minP.y = mi.y;
                if (mi.z < minP.z) minP.z = mi.z;
                if (ma.x > maxP.x) maxP.x = ma.x;
                if (ma.y > maxP.y) maxP.y = ma.y;
                if (ma.z > maxP.z) maxP.z = ma.z;
            }
            AppLogAdd(L"[FISH_MODEL] Meshes=%d, Min=(%.2f, %.2f, %.2f), Max=(%.2f, %.2f, %.2f)\n",
                meshNum, minP.x, minP.y, minP.z, maxP.x, maxP.y, maxP.z);
            AppLogAdd(L"[FISH_MODEL] Size=(%.2f, %.2f, %.2f), Center=(%.2f, %.2f, %.2f)\n",
                maxP.x - minP.x, maxP.y - minP.y, maxP.z - minP.z,
                (minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f);
            int animNum = MV1GetAnimNum(fishModelHandle_);
            int matNum = MV1GetMaterialNum(fishModelHandle_);
            int texNum = MV1GetTextureNum(fishModelHandle_);

            // Nạp texture colormap.png nếu có cho mô hình cá
            if (fishTextureHandle_ < 0)
            {
                fishTextureHandle_ = MV1LoadTexture(L"Data/Model/PreModelTest/colormap.png");
                if (fishTextureHandle_ < 0)
                {
                    fishTextureHandle_ = LoadGraph(L"Data/Model/PreModelTest/colormap.png");
                }
                if (fishTextureHandle_ < 0)
                {
                    fishTextureHandle_ = MV1LoadTexture(L"Data/Model/PreModelTest/Pre_Fish.png");
                }
            }

            // Gắn và kiểm tra texture của mô hình
            bool boundAnyTexture = false;
            for (int i = 0; i < texNum; ++i)
            {
                int gh = MV1GetTextureGraphHandle(fishModelHandle_, i);
                const TCHAR* tName = MV1GetTextureName(fishModelHandle_, i);
                int gw = 0, ghH = 0;
                if (gh > 0) GetGraphSize(gh, &gw, &ghH);
                AppLogAdd(L"[FISH_MODEL] Tex[%d]: '%s', gh=%d, size=(%dx%d)\n", i, tName ? tName : L"null", gh, gw, ghH);
                if (fishTextureHandle_ >= 0)
                {
                    int fw = 0, fh = 0;
                    GetGraphSize(fishTextureHandle_, &fw, &fh);
                    int retSet = MV1SetTextureGraphHandle(fishModelHandle_, i, fishTextureHandle_, FALSE);
                    int ghNow = MV1GetTextureGraphHandle(fishModelHandle_, i);
                    AppLogAdd(L"[FISH_MODEL] MV1SetTextureGraphHandle ret=%d, ghBefore=%d, ghNow=%d, fishTextureHandle=(%d, %dx%d)\n",
                        retSet, gh, ghNow, fishTextureHandle_, fw, fh);
                    int retPath = MV1SetTextureColorFilePath(fishModelHandle_, i, L"Data/Model/PreModelTest/colormap.png");
                    int ghAfterPath = MV1GetTextureGraphHandle(fishModelHandle_, i);
                    AppLogAdd(L"[FISH_MODEL] MV1SetTextureColorFilePath ret=%d, ghAfterPath=%d\n", retPath, ghAfterPath);
                    boundAnyTexture = true;
                }
                else if (gh > 0)
                {
                    boundAnyTexture = true;
                }
            }

            fishModelHasTexture_ = boundAnyTexture;
            AppLogAdd(L"[FISH_MODEL] AnimNum=%d, MatNum=%d, TexNum=%d, fishTextureHandle=%d, HasTex=%d\n",
                animNum, matNum, texNum, fishTextureHandle_, fishModelHasTexture_ ? 1 : 0);

            for (int m = 0; m < matNum; ++m)
            {
                const TCHAR* mName = MV1GetMaterialName(fishModelHandle_, m);
                int mType = MV1GetMaterialType(fishModelHandle_, m);
                int difTex = MV1GetMaterialDifMapTexture(fishModelHandle_, m);
                COLOR_F difC = MV1GetMaterialDifColor(fishModelHandle_, m);
                COLOR_F ambC = MV1GetMaterialAmbColor(fishModelHandle_, m);
                COLOR_F spcC = MV1GetMaterialSpcColor(fishModelHandle_, m);
                COLOR_F emiC = MV1GetMaterialEmiColor(fishModelHandle_, m);
                AppLogAdd(L"[FISH_MODEL] Mat[%d]: '%s', Type=%d, DifTex=%d, Dif=(%.2f,%.2f,%.2f), Amb=(%.2f,%.2f,%.2f), Spc=(%.2f,%.2f,%.2f), Emi=(%.2f,%.2f,%.2f)\n",
                    m, mName ? mName : L"null", mType, difTex, difC.r, difC.g, difC.b, ambC.r, ambC.g, ambC.b, spcC.r, spcC.g, spcC.b, emiC.r, emiC.g, emiC.b);
            }

            for (int m = 0; m < meshNum; ++m)
            {
                int meshMat = MV1GetMeshMaterial(fishModelHandle_, m);
                int vNum = MV1GetMeshVertexNum(fishModelHandle_, m);
                AppLogAdd(L"[FISH_MODEL] Mesh[%d]: Mat=%d, Vertices=%d\n", m, meshMat, vNum);
            }

            int frameNum = MV1GetFrameNum(fishModelHandle_);
            AppLogAdd(L"[FISH_MODEL] FrameNum=%d\n", frameNum);
            for (int i = 0; i < frameNum && i < 15; ++i)
            {
                const wchar_t* fName = MV1GetFrameName(fishModelHandle_, i);
                VECTOR fPos = MV1GetFramePosition(fishModelHandle_, i);
                AppLogAdd(L"[FISH_MODEL] Frame[%d]: '%s', pos=(%.2f, %.2f, %.2f)\n", i, fName ? fName : L"(null)", fPos.x, fPos.y, fPos.z);
            }

            // Gắn hoạt ảnh bơi 3D nếu mô hình có animation
            if (animNum > 2)
            {
                // Mô hình Pre_Fish (có nhiều anim: 1=idle, 2=walk, 3=run)
                currentAnimIndex_ = 2; // Bắt đầu bằng bơi (walk)
                fishAnimAttachIndex_ = MV1AttachAnim(fishModelHandle_, currentAnimIndex_);
                fishAnimTime_ = 0.0f;
                fishModelScale_ = 0.22f;   // Tỉ lệ scale phù hợp với hitbox
                fishModelOffsetY_ = 4.5f;  // Chạm mặt sàn đấu
                fishModelRotYDeg_ = 180.0f; // Xoay bù 180 độ để đầu cá hướng thẳng theo chiều bơi tiến
                AppLogAdd(L"[FISH_MODEL] Pre_Fish attached anim 2 (walk), scale=%.2f, offset=%.2f, rotY=%.2f\n",
                    fishModelScale_, fishModelOffsetY_, fishModelRotYDeg_);
            }
            else if (animNum > 0)
            {
                // Mô hình fish.mv1 cũ (1 animation quẫy đuôi)
                currentAnimIndex_ = 0;
                fishAnimAttachIndex_ = MV1AttachAnim(fishModelHandle_, 0);
                fishAnimTime_ = 0.0f;
                fishModelScale_ = GameConsts::PLAYER_MODEL_SCALE;
                fishModelOffsetY_ = GameConsts::PLAYER_MODEL_OFFSET_Y;
                fishModelRotYDeg_ = GameConsts::PLAYER_MODEL_ROT_Y_DEG;
                AppLogAdd(L"[FISH_MODEL] Attached animation 0, attachIndex=%d\n", fishAnimAttachIndex_);
            }

            // Cấu hình vật liệu
            if (fishModelHasTexture_)
            {
                // Mô hình có texture (ví dụ colormap.png): giữ màu gốc tươi sáng của texture
                for (int i = 0; i < matNum; ++i)
                {
                    MV1SetMaterialDifMapTexture(fishModelHandle_, i, 0);
                    MV1SetMaterialDifColor(fishModelHandle_, i, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
                    MV1SetMaterialAmbColor(fishModelHandle_, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                    MV1SetMaterialSpcColor(fishModelHandle_, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                    MV1SetMaterialEmiColor(fishModelHandle_, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                }
            }
            else
            {
                // Mô hình không texture: tự động tô màu xanh biển tươi sáng bằng code
                for (int i = 0; i < matNum; ++i)
                {
                    MV1SetMaterialDifColor(fishModelHandle_, i, GetColorF(0.18f, 0.72f, 0.96f, 1.0f));
                    MV1SetMaterialAmbColor(fishModelHandle_, i, GetColorF(0.12f, 0.40f, 0.60f, 1.0f));
                    MV1SetMaterialSpcColor(fishModelHandle_, i, GetColorF(0.60f, 0.90f, 1.0f, 1.0f));
                    MV1SetMaterialEmiColor(fishModelHandle_, i, GetColorF(0.06f, 0.18f, 0.28f, 1.0f));
                }
            }
        }
        else
        {
            AppLogAdd(L"[FISH_MODEL] WARNING: Failed to load fish model, fallback to 3D box.\n");
        }
    }
    Reset();
}

void PlayerFish::Release()
{
    if (fishModelHandle_ >= 0)
    {
        MV1DeleteModel(fishModelHandle_);
        fishModelHandle_ = -1;
        fishAnimAttachIndex_ = -1;
    }
    if (fishTextureHandle_ >= 0)
    {
        DeleteGraph(fishTextureHandle_);
        fishTextureHandle_ = -1;
    }
}

void PlayerFish::ApplyStats(const PlayerStats& stats)
{
    float oldMax = maxHp_;
    maxHp_ = (std::clamp)(stats.maxHp.GetValue(), 20.0f, GameConsts::MAX_PLAYER_HP);
    if (maxHp_ > oldMax) hp_ += (maxHp_ - oldMax);
    hp_ = (std::min)(maxHp_, hp_);

    speed_ = (std::min)(stats.moveSpeed.GetValue(), GameConsts::MAX_MOVE_SPEED);
    jumpDuration_ = stats.bounceDuration.GetValue();
    jumpHeight_ = stats.jumpHeight.GetValue();
    jumpCooldown_ = (std::max)(stats.jumpCooldown.GetValue(), GameConsts::MIN_JUMP_COOLDOWN);

    if (stats.moveSpeed.GetValue() > 1.0f)
    {
        airSpeedMul_ = stats.airSpeed.GetValue() / stats.moveSpeed.GetValue();
    }
    else
    {
        airSpeedMul_ = GameConsts::AIR_SPEED_MUL;
    }

    slamDamage_ = (std::min)(stats.slamDamage.GetValue(), GameConsts::MAX_SLAM_DAMAGE);
    slamRadius_ = (std::min)(stats.slamRadius.GetValue(), GameConsts::MAX_SLAM_RADIUS);
    slamKnockback_ = (std::min)(stats.slamKnockback.GetValue(), GameConsts::MAX_SLAM_KNOCKBACK);
    extraJumps_ = static_cast<int>(stats.extraJumps.GetValue());
}

void PlayerFish::SetHooks(bool hasPufferfish, bool hasBulletRain, bool hasContinuousBounce)
{
    hasPufferfishHook_ = hasPufferfish;
    hasBulletRainHook_ = hasBulletRain;
    hasContinuousBounceHook_ = hasContinuousBounce;
}

void PlayerFish::Reset()
{
    pos_ = Vec3{ 0.0f, 0.0f, 0.0f };
    moveDir_ = Vec3{ 0.0f, 0.0f, 1.0f };
    airVel_ = Vec3{ 0.0f, 0.0f, 0.0f };
    predictedLandPos_ = Vec3{ 0.0f, 0.5f, 0.0f };
    yaw_ = 0.0f;
    swimTimer_ = 0.0f;
    knockback_ = Vec3{ 0.0f, 0.0f, 0.0f };
    invulnerableTimer_ = 0.0f;
    isGodMode_ = false;
    damageTakenThisWave_ = 0.0f;

    isJumping_ = false;
    justLanded_ = false;
    jumpPhase_ = 0.0f;
    jumpDuration_ = GameConsts::JUMP_DURATION;
    jumpHeight_ = GameConsts::JUMP_HEIGHT;
    jumpCooldown_ = GameConsts::JUMP_COOLDOWN;
    jumpCooldownTimer_ = 0.0f;
    landRecoveryDuration_ = GameConsts::LAND_RECOVERY;
    landRecoveryTimer_ = 0.0f;
    speed_ = GameConsts::FISH_SPEED;
    airSpeedMul_ = GameConsts::AIR_SPEED_MUL;

    slamDamage_ = GameConsts::SLAM_DAMAGE;
    slamRadius_ = GameConsts::SLAM_RADIUS;
    slamKnockback_ = GameConsts::SLAM_KNOCKBACK;
    extraJumps_ = 0;
    remainingJumps_ = 0;

    hasPufferfishHook_ = false;
    hasBulletRainHook_ = false;
    hasContinuousBounceHook_ = false;
    triggerPufferfishSpikes_ = false;
    triggerBulletRain_ = false;
    bounceComboCount_ = 0;
    bounceComboBuffTimer_ = 0.0f;

    hp_ = 100.0f;
    maxHp_ = 100.0f;
    exp_ = 0;
    expNext_ = 30; // 20 + 10 * 1
    level_ = 1;

    hitFlashTimer_ = 0.0f;
    tailWagAngle_ = 0.0f;
}

void PlayerFish::StartJump(const Vec3& inputDir)
{
    isJumping_ = true;
    jumpPhase_ = 0.0f;
    remainingJumps_ = extraJumps_;

    if (hasContinuousBounceHook_)
    {
        bounceComboCount_++;
        if (bounceComboCount_ >= 5)
        {
            bounceComboBuffTimer_ = 3.0f; // Kích hoạt buff +50% sát thương trong 3s
        }
    }

    if (inputDir.LengthSq() > 0.001f)
    {
        moveDir_ = inputDir.Normalized();
        yaw_ = std::atan2(moveDir_.x, moveDir_.z);
        airVel_ = moveDir_ * (speed_ * airSpeedMul_);
    }
    else
    {
        airVel_ = Vec3{ 0.0f, 0.0f, 0.0f };
    }

    // Khởi tạo điểm rơi dự đoán
    float limit = GameConsts::ARENA_HALF_SIZE - 25.0f;
    predictedLandPos_ = pos_ + airVel_ * jumpDuration_;
    predictedLandPos_.x = std::clamp(predictedLandPos_.x, -limit, limit);
    predictedLandPos_.y = 0.5f;
    predictedLandPos_.z = std::clamp(predictedLandPos_.z, -limit, limit);
}

void PlayerFish::OnLand(EffectSystem& effects)
{
    pos_.y = 0.0f;
    jumpPhase_ = 0.0f;
    isJumping_ = false;
    justLanded_ = true;

    // J2: Hồi chiêu 1.2s bắt đầu tính từ thời điểm tiếp đất
    jumpCooldownTimer_ = jumpCooldown_;
    // J4: Khựng 0.2s sau tiếp đất, cá đứng yên và dễ bị trúng đòn
    landRecoveryTimer_ = landRecoveryDuration_;

    // Hook 3: Mưa đạn khi tiếp đất
    if (hasBulletRainHook_)
    {
        triggerBulletRain_ = true;
    }

    // Sinh sóng nước khi tiếp đất (J6)
    effects.SpawnShockwave(pos_, slamRadius_);

    // Chấn động nhẹ
    effects.AddTrauma(GameConsts::LAND_TRAUMA_AMOUNT);
}

void PlayerFish::Update(float dt, EffectSystem& effects)
{
    if (hitFlashTimer_ > 0.0f)
    {
        hitFlashTimer_ -= dt;
    }
    if (invulnerableTimer_ > 0.0f)
    {
        invulnerableTimer_ -= dt;
    }

    float arenaLimit = GameConsts::ARENA_HALF_SIZE - 25.0f;

    // Áp dụng lực giật nảy knockback lên cá và tiêu giảm dần
    if (knockback_.LengthSq() > 0.1f)
    {
        pos_.x += knockback_.x * dt;
        pos_.z += knockback_.z * dt;
        knockback_ = knockback_ * (std::max)(0.0f, 1.0f - dt * 8.5f);
        pos_.x = std::clamp(pos_.x, -arenaLimit, arenaLimit);
        pos_.z = std::clamp(pos_.z, -arenaLimit, arenaLimit);
    }

    // Đọc input WASD và phím mũi tên
    Vec3 inputDir{ 0.0f, 0.0f, 0.0f };
    if (CheckHitKey(KEY_INPUT_W) || CheckHitKey(KEY_INPUT_UP)) inputDir.z += 1.0f;
    if (CheckHitKey(KEY_INPUT_S) || CheckHitKey(KEY_INPUT_DOWN)) inputDir.z -= 1.0f;
    if (CheckHitKey(KEY_INPUT_A) || CheckHitKey(KEY_INPUT_LEFT)) inputDir.x -= 1.0f;
    if (CheckHitKey(KEY_INPUT_D) || CheckHitKey(KEY_INPUT_RIGHT)) inputDir.x += 1.0f;

    bool hasMoveInput = (inputDir.LengthSq() > 0.01f);
    static bool spacePrev = false;
    bool spaceNow = (CheckHitKey(KEY_INPUT_SPACE) != 0);
    bool spaceJustPressed = (spaceNow && !spacePrev);
    spacePrev = spaceNow;

    // 1. TRẠNG THÁI Ở DƯỚI ĐẤT / BƠI (J1, J2, J4)
    if (!isJumping_)
    {
        pos_.y = 0.0f;

        // Giảm hồi chiêu nhảy (J2)
        if (jumpCooldownTimer_ > 0.0f)
        {
            jumpCooldownTimer_ -= dt;
        }

        // Đếm lùi nhịp hồi phục sau tiếp đất (dành cho hiệu ứng nẩy Squash/Stretch & giãn cách cú nhảy)
        if (landRecoveryTimer_ > 0.0f)
        {
            landRecoveryTimer_ -= dt;
        }

        // CẢI THIỆN: Cho phép cá bơi mượt mà bằng WASD tiếp tục được luôn sau khi đáp đất!
        if (hasMoveInput)
        {
            moveDir_ = inputDir.Normalized();
            yaw_ = std::atan2(moveDir_.x, moveDir_.z);

            pos_.x += moveDir_.x * speed_ * dt;
            pos_.z += moveDir_.z * speed_ * dt;

            swimTimer_ += dt;
            tailWagAngle_ = std::sin(swimTimer_ * 14.0f) * 0.25f;
        }
        else
        {
            tailWagAngle_ *= 0.85f;
        }

        pos_.x = std::clamp(pos_.x, -arenaLimit, arenaLimit);
        pos_.z = std::clamp(pos_.z, -arenaLimit, arenaLimit);

        // Bấm Space nhảy khi cooldown đã sẵn sàng (J2)
        if (spaceJustPressed && jumpCooldownTimer_ <= 0.0f)
        {
            StartJump(hasMoveInput ? moveDir_ : Vec3{ 0.0f, 0.0f, 0.0f });
        }
    }
    // 2. TRẠNG THÁI ĐANG BAY TRÊN KHÔNG (J2, J3, J4, J5)
    else
    {
        // Cho phép nhảy thêm trên không nếu có hook/thẻ nhảy liên hoàn
        if (spaceJustPressed && remainingJumps_ > 0)
        {
            remainingJumps_--;
            jumpPhase_ = 0.0f; // Nhảy thêm nhịp mới trên không
            if (hasMoveInput)
            {
                moveDir_ = inputDir.Normalized();
                yaw_ = std::atan2(moveDir_.x, moveDir_.z);
                airVel_ = moveDir_ * (speed_ * airSpeedMul_);
            }
        }

        jumpPhase_ += dt / jumpDuration_;

        if (jumpPhase_ >= 1.0f)
        {
            // Tiếp đất (J4, J6)
            OnLand(effects);

            // CẢI THIỆN: Giữ đà bơi liên tục ngay tại frame tiếp đất (không khựng 1 frame nào)
            if (hasMoveInput)
            {
                moveDir_ = inputDir.Normalized();
                yaw_ = std::atan2(moveDir_.x, moveDir_.z);
                pos_.x += moveDir_.x * speed_ * dt;
                pos_.z += moveDir_.z * speed_ * dt;
                swimTimer_ += dt;
                tailWagAngle_ = std::sin(swimTimer_ * 14.0f) * 0.25f;
            }
            else
            {
                pos_.x += airVel_.x * 0.4f * dt;
                pos_.z += airVel_.z * 0.4f * dt;
            }
            pos_.x = std::clamp(pos_.x, -arenaLimit, arenaLimit);
            pos_.z = std::clamp(pos_.z, -arenaLimit, arenaLimit);
        }
        else
        {
            // 1. Quỹ đạo Parabol trên trục Y (J2)
            float t = jumpPhase_;
            pos_.y = 4.0f * jumpHeight_ * t * (1.0f - t);

            // 2. Điều khiển trên không (Air Control - J3)
            Vec3 targetAirVel = (hasMoveInput ? inputDir.Normalized() : moveDir_) * (speed_ * airSpeedMul_);
            airVel_ = airVel_ * 0.88f + targetAirVel * 0.12f;

            pos_.x += airVel_.x * dt;
            pos_.z += airVel_.z * dt;

            pos_.x = std::clamp(pos_.x, -arenaLimit, arenaLimit);
            pos_.z = std::clamp(pos_.z, -arenaLimit, arenaLimit);

            if (airVel_.LengthSq() > 1.0f)
            {
                yaw_ = std::atan2(airVel_.x, airVel_.z);
            }

            // Vẫy đuôi cá theo nhịp bay
            tailWagAngle_ = std::sin(jumpPhase_ * 3.14159f * 4.0f) * 0.35f;

            // 3. Cập nhật vị trí rơi dự đoán cho vòng báo decal (J5)
            float timeRemaining = (1.0f - jumpPhase_) * jumpDuration_;
            predictedLandPos_.x = std::clamp(pos_.x + airVel_.x * timeRemaining, -arenaLimit, arenaLimit);
            predictedLandPos_.y = 0.5f;
            predictedLandPos_.z = std::clamp(pos_.z + airVel_.z * timeRemaining, -arenaLimit, arenaLimit);
        }
    }

    if (bounceComboBuffTimer_ > 0.0f)
    {
        bounceComboBuffTimer_ -= dt;
    }

    // Cập nhật hoạt ảnh 3D cho mô hình cá
    if (fishModelHandle_ >= 0 && fishAnimAttachIndex_ >= 0)
    {
        int animNum = MV1GetAnimNum(fishModelHandle_);
        if (animNum > 2)
        {
            // Tự động chuyển đổi giữa idle (1) và walk (2) / run (3) khi nhảy
            int desiredAnim = isJumping_ ? 3 : (hasMoveInput ? 2 : 1);
            if (currentAnimIndex_ != desiredAnim)
            {
                MV1DetachAnim(fishModelHandle_, fishAnimAttachIndex_);
                fishAnimAttachIndex_ = MV1AttachAnim(fishModelHandle_, desiredAnim);
                currentAnimIndex_ = desiredAnim;
                fishAnimTime_ = 0.0f;
            }
        }

        float totalTime = MV1GetAnimTotalTime(fishModelHandle_, currentAnimIndex_);
        if (totalTime > 0.0f)
        {
            float animRate = isJumping_ ? 1.6f : (hasMoveInput ? 1.2f : 0.6f);
            fishAnimTime_ += dt * animRate * 30.0f;
            while (fishAnimTime_ >= totalTime) fishAnimTime_ -= totalTime;
            MV1SetAttachAnimTime(fishModelHandle_, fishAnimAttachIndex_, fishAnimTime_);
        }
    }
}

bool PlayerFish::TakeDamage(float dmg, EffectSystem& effects)
{
    // J4: Bất tử suốt lúc ở trên không, đang trong thời gian I-Frame, hoặc bật God Mode (F9)
    if (isGodMode_ || IsAirborne() || invulnerableTimer_ > 0.0f)
    {
        return false;
    }

    hp_ -= dmg;
    damageTakenThisWave_ += dmg;
    invulnerableTimer_ = 0.35f; // Thời gian I-Frame 0.35s bảo vệ cá không bị dồn sát thương liên tục
    hitFlashTimer_ = 0.16f;
    effects.AddTrauma(0.28f); // Rung nhẹ vừa phải tạo phản hồi va đập
    effects.SpawnHitParticles(pos_ + Vec3{ 0, 10, 0 }, 8, GetColor(255, 60, 60));

    // Hook 6: Cá nóc - khi trúng đòn, bắn gai phản đòn
    if (hasPufferfishHook_)
    {
        triggerPufferfishSpikes_ = true;
    }

    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
    }
    return true;
}

void PlayerFish::AddExp(int amount)
{
    exp_ += amount;
    if (exp_ >= expNext_)
    {
        exp_ -= expNext_;
        level_++;
        expNext_ = static_cast<int>(expNext_ * 1.4f);
    }
}

void PlayerFish::Draw() const
{
    // J5: Vòng báo điểm rơi (Vẽ decal dự đoán trên mặt đất khi đang bay)
    if (isJumping_)
    {
        int pulseAlpha = 130 + static_cast<int>(50.0f * std::sin(jumpPhase_ * 18.0f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulseAlpha);

        // Vòng ngoài đúng với bán kính slamRadius
        Render3D::DrawCircle3D(predictedLandPos_, slamRadius_, 36, GetColor(70, 220, 255), false);
        // Vòng trong viền mỏng
        Render3D::DrawCircle3D(predictedLandPos_, slamRadius_ * 0.65f, 28, GetColor(100, 240, 255), false);
        // Điểm tâm nhắm mục tiêu
        Render3D::DrawCircle3D(predictedLandPos_, 10.0f, 16, GetColor(255, 255, 255), true);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // Bóng đổ dẹt trên sàn tại vị trí thực tế của cá
    float heightRatio = pos_.y / jumpHeight_;
    float shadowRadius = 24.0f * (1.0f - 0.45f * heightRatio);
    float shadowAlpha = (1.0f - 0.5f * heightRatio);
    Render3D::DrawFlatShadow(pos_, shadowRadius, shadowAlpha);

    // Xác định màu sắc (Cá phát sáng vàng kim/neon khi bay bất tử)
    int bodyColor = 0;
    int edgeColor = 0;

    if (hitFlashTimer_ > 0.0f)
    {
        bodyColor = GetColor(255, 255, 255);
        edgeColor = GetColor(255, 100, 100);
    }
    else if (invulnerableTimer_ > 0.0f && static_cast<int>(invulnerableTimer_ * 24.0f) % 2 == 0)
    {
        // Nhấp nháy nhẹ trong thời gian I-Frame sau va chạm
        bodyColor = GetColor(255, 200, 150);
        edgeColor = GetColor(255, 120, 80);
    }
    else if (IsAirborne())
    {
        // Khi bay bất tử: phát quang màu vàng rực rỡ
        bodyColor = GetColor(255, 220, 60);
        edgeColor = GetColor(255, 255, 180);
    }
    else if (landRecoveryTimer_ > 0.0f)
    {
        // Khi vừa chạm đất khựng hồi phục: màu cam sẫm nhẹ
        bodyColor = GetColor(230, 110, 30);
        edgeColor = GetColor(160, 50, 15);
    }
    else
    {
        // Bơi bình thường: màu cam tươi
        bodyColor = GetColor(255, 140, 35);
        edgeColor = GetColor(180, 70, 15);
    }

    // J7: Squash & Stretch Animation
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;

    if (isJumping_)
    {
        if (jumpPhase_ < 0.25f)
        {
            // Cất cánh: kéo dài Y, Z, ép hẹp X
            scaleX = 0.85f;
            scaleY = 1.20f;
            scaleZ = 1.15f;
        }
        else if (jumpPhase_ > 0.82f)
        {
            // Chuẩn bị chạm đất: ép dẹt Y, phình X, Z
            scaleX = 1.18f;
            scaleY = 0.82f;
            scaleZ = 1.12f;
        }
    }
    else if (landRecoveryTimer_ > 0.05f)
    {
        // Vừa chạm đất: ép dẹt dội nảy
        float recRatio = landRecoveryTimer_ / landRecoveryDuration_;
        scaleX = 1.0f + 0.28f * recRatio;
        scaleY = 1.0f - 0.35f * recRatio;
        scaleZ = 1.0f + 0.22f * recRatio;
    }

    // Vẽ mô hình 3D cá chính nếu đã nạp thành công
    if (fishModelHandle_ >= 0)
    {
        VECTOR modelPos = VGet(pos_.x, pos_.y + fishModelOffsetY_, pos_.z);
        MV1SetPosition(fishModelHandle_, modelPos);

        float finalYaw = yaw_ + fishModelRotYDeg_ * (DX_PI_F / 180.0f);
        MV1SetRotationXYZ(fishModelHandle_, VGet(0.0f, finalYaw, 0.0f));

        MV1SetScale(fishModelHandle_, VGet(
            fishModelScale_ * scaleX,
            fishModelScale_ * scaleY,
            fishModelScale_ * scaleZ
        ));

        // Tự động tô màu bằng code theo trạng thái cá
        int matNum = MV1GetMaterialNum(fishModelHandle_);
        if (hitFlashTimer_ > 0.0f)
        {
            // Chớp trắng khi trúng đòn
            MV1SetDifColorScale(fishModelHandle_, GetColorF(3.0f, 3.0f, 3.0f, 1.0f));
        }
        else if (invulnerableTimer_ > 0.0f && static_cast<int>(invulnerableTimer_ * 24.0f) % 2 == 0)
        {
            // Nhấp nháy cam nhạt trong thời gian I-Frame
            MV1SetDifColorScale(fishModelHandle_, GetColorF(1.8f, 1.3f, 0.9f, 1.0f));
            if (!fishModelHasTexture_)
            {
                for (int i = 0; i < matNum; ++i)
                {
                    MV1SetMaterialDifColor(fishModelHandle_, i, GetColorF(1.0f, 0.65f, 0.35f, 1.0f));
                    MV1SetMaterialEmiColor(fishModelHandle_, i, GetColorF(0.3f, 0.15f, 0.05f, 1.0f));
                }
            }
        }
        else if (IsAirborne())
        {
            // Vàng kim neon khi nhảy bất tử
            MV1SetDifColorScale(fishModelHandle_, GetColorF(1.6f, 1.4f, 0.35f, 1.0f));
            if (!fishModelHasTexture_)
            {
                for (int i = 0; i < matNum; ++i)
                {
                    MV1SetMaterialDifColor(fishModelHandle_, i, GetColorF(1.0f, 0.85f, 0.15f, 1.0f));
                    MV1SetMaterialAmbColor(fishModelHandle_, i, GetColorF(0.6f, 0.5f, 0.1f, 1.0f));
                    MV1SetMaterialSpcColor(fishModelHandle_, i, GetColorF(1.0f, 1.0f, 0.6f, 1.0f));
                    MV1SetMaterialEmiColor(fishModelHandle_, i, GetColorF(0.40f, 0.32f, 0.05f, 1.0f));
                }
            }
        }
        else
        {
            // Bình thường
            MV1SetDifColorScale(fishModelHandle_, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
            if (!fishModelHasTexture_)
            {
                // Xanh ngọc biển tươi sáng (Aqua Blue / Cyan) cho model không có texture
                for (int i = 0; i < matNum; ++i)
                {
                    MV1SetMaterialDifColor(fishModelHandle_, i, GetColorF(0.18f, 0.72f, 0.96f, 1.0f));
                    MV1SetMaterialAmbColor(fishModelHandle_, i, GetColorF(0.12f, 0.40f, 0.60f, 1.0f));
                    MV1SetMaterialSpcColor(fishModelHandle_, i, GetColorF(0.60f, 0.90f, 1.0f, 1.0f));
                    MV1SetMaterialEmiColor(fishModelHandle_, i, GetColorF(0.06f, 0.18f, 0.28f, 1.0f));
                }
            }
            else
            {
                // Đảm bảo phục hồi màu vật liệu gốc tươi sáng cho model có texture
                for (int i = 0; i < matNum; ++i)
                {
                    MV1SetMaterialDifColor(fishModelHandle_, i, GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
                    MV1SetMaterialAmbColor(fishModelHandle_, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                    MV1SetMaterialSpcColor(fishModelHandle_, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                    MV1SetMaterialEmiColor(fishModelHandle_, i, GetColorF(0.0f, 0.0f, 0.0f, 1.0f));
                }
            }
        }

        MV1DrawModel(fishModelHandle_);
        return; // Bỏ qua vẽ 3D box cá
    }

    // Vẽ thân cá (3D Box chính áp dụng Squash & Stretch)
    Vec3 bodyCenter = pos_ + Vec3{ 0.0f, (GameConsts::FISH_SIZE_Y * scaleY) * 0.5f, 0.0f };
    Vec3 bodyHalfSize{
        GameConsts::FISH_SIZE_X * 0.5f * scaleX,
        GameConsts::FISH_SIZE_Y * 0.5f * scaleY,
        GameConsts::FISH_SIZE_Z * 0.5f * scaleZ
    };
    Render3D::DrawOrientedBox3D(bodyCenter, bodyHalfSize, yaw_, bodyColor, edgeColor, true);

    // Mõm cá / Đầu cá
    float headOffsetZ = (GameConsts::FISH_SIZE_Z * 0.5f * scaleZ) + 4.0f;
    Vec3 headCenter = bodyCenter + Vec3{
        std::sin(yaw_) * headOffsetZ,
        -1.0f * scaleY,
        std::cos(yaw_) * headOffsetZ
    };
    Vec3 headHalfSize{ 7.0f * scaleX, 6.0f * scaleY, 4.0f * scaleZ };
    int headColor = IsAirborne() ? GetColor(255, 235, 120) : GetColor(255, 175, 75);
    Render3D::DrawOrientedBox3D(headCenter, headHalfSize, yaw_, headColor, edgeColor, true);

    // Mắt cá
    int eyeColor = GetColor(20, 30, 45);
    float eyeSpread = 10.5f * scaleX;
    float eyeForward = 10.0f * scaleZ;
    Vec3 leftEye = bodyCenter + Vec3{
        -std::cos(yaw_) * eyeSpread + std::sin(yaw_) * eyeForward,
        3.0f * scaleY,
        std::sin(yaw_) * eyeSpread + std::cos(yaw_) * eyeForward
    };
    Vec3 rightEye = bodyCenter + Vec3{
        std::cos(yaw_) * eyeSpread + std::sin(yaw_) * eyeForward,
        3.0f * scaleY,
        -std::sin(yaw_) * eyeSpread + std::cos(yaw_) * eyeForward
    };
    Vec3 eyeHalfSize{ 2.5f, 2.5f, 2.5f };
    Render3D::DrawOrientedBox3D(leftEye, eyeHalfSize, yaw_, eyeColor, eyeColor, false);
    Render3D::DrawOrientedBox3D(rightEye, eyeHalfSize, yaw_, eyeColor, eyeColor, false);

    // Đuôi cá (Vẫy đuôi)
    float tailOffsetZ = -(GameConsts::FISH_SIZE_Z * 0.5f * scaleZ) - 6.0f;
    Vec3 tailCenter = bodyCenter + Vec3{
        std::sin(yaw_ + tailWagAngle_) * tailOffsetZ,
        0.0f,
        std::cos(yaw_ + tailWagAngle_) * tailOffsetZ
    };
    Vec3 tailHalfSize{ 3.5f * scaleX, 10.0f * scaleY, 6.0f * scaleZ };
    int tailColor = IsAirborne() ? GetColor(255, 245, 140) : GetColor(255, 195, 60);
    Render3D::DrawOrientedBox3D(tailCenter, tailHalfSize, yaw_ + tailWagAngle_, tailColor, edgeColor, true);
}

