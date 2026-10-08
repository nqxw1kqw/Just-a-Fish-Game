#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include "Boss.h"
#include "PlayerFish.h"
#include "Enemy.h"
#include "Effects.h"
#include "Render3DUtil.h"
#include "DxLib.h"

void WhaleBoss::Init()
{
    Reset();
}

void WhaleBoss::Reset()
{
    active_ = false;
    pos_ = Vec3{ 0.0f, 0.0f, 350.0f };
    yaw_ = 3.14159f;
    bossTier_ = 1;
    hpMultiplier_ = 1.0f;
    maxHp_ = GameConsts::BOSS_MAX_HP;
    hp_ = maxHp_;
    phase_ = BossPhase::Phase1;
    action_ = BossAction::Idle;
    actionTimer_ = 0.0f;
    idleDuration_ = 1.8f;
    actionCycle_ = 0;
    hitFlashTimer_ = 0.0f;
    phaseTransitionTimer_ = 0.0f;
    timeSinceLastJumpAttack_ = 5.0f;
    timeSinceLastRingLaunch_ = 5.0f;
    needSecondRing_ = false;
    bossFightTimer_ = 0.0f;
    actionHistoryCount_ = 0;
    actionHistoryHead_ = 0;
    for (auto& a : actionHistory_) a = nullptr;
    lastSkill_ = BossSkill::None;
    timeSinceLastTsunami_ = 10.0f;
    currentTsunamiGapDeg_ = 0.0f;

    for (auto& r : tsunamiRings_) r.active = false;
}

void WhaleBoss::Spawn(const Vec3& arenaCenter)
{
    active_ = true;
    pos_ = arenaCenter + Vec3{ 0.0f, 0.0f, 380.0f };
    hp_ = maxHp_;
    phase_ = BossPhase::Phase1;
    action_ = BossAction::Idle;
    actionTimer_ = 1.2f;
    idleDuration_ = 1.8f;
    actionCycle_ = 0;
    timeSinceLastJumpAttack_ = 5.0f;
    timeSinceLastRingLaunch_ = 5.0f;
    needSecondRing_ = false;
    bossFightTimer_ = 0.0f;
    actionHistoryCount_ = 0;
    actionHistoryHead_ = 0;
    for (auto& a : actionHistory_) a = nullptr;
    lastSkill_ = BossSkill::None;
    timeSinceLastTsunami_ = 10.0f;
    currentTsunamiGapDeg_ = 0.0f;

    for (auto& r : tsunamiRings_) r.active = false;
}

bool WhaleBoss::TakeDamage(float dmg, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return false;

    hp_ -= dmg;
    hitFlashTimer_ = 0.14f;
    effects.SpawnHitParticles(pos_ + Vec3{ 0, 25, 0 }, 6, GetColor(255, 120, 60));
    effects.SpawnDamagePopup(pos_ + Vec3{ 0, 45.0f, 0 }, dmg, (dmg >= 50.0f));

    // Chuyển sang Giai đoạn 2 (< 50% máu: 500 HP)
    if (hp_ <= maxHp_ * 0.5f && phase_ == BossPhase::Phase1)
    {
        phase_ = BossPhase::Phase2;
        phaseTransitionTimer_ = 1.2f; // Khựng ngắn gầm rú và đổi màu cuồng nộ
        idleDuration_ = 0.95f;        // Nhịp đòn nhanh hơn
        effects.AddTrauma(0.7f);
        effects.SpawnShockwave(pos_, 260.0f);
    }

    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
        effects.AddTrauma(1.0f);
        effects.SpawnHitParticles(pos_ + Vec3{ 0, 30, 0 }, 50, GetColor(255, 220, 60));
    }

    return true;
}

void WhaleBoss::StartTsunamiTelegraph(PlayerFish& player, float gapDeg)
{
    // Quy tắc công bằng A5: Hồi ngay lập tức hồi chiêu nhảy nếu người chơi đang ở dưới đất
    if (player.GetHeightY() <= 0.01f && !player.IsAirborne())
    {
        player.ResetJumpCooldown();
    }

    currentTsunamiGapDeg_ = gapDeg;
    action_ = BossAction::TelegraphRadialTsunami;
    actionTimer_ = 0.0f;
    timeSinceLastTsunami_ = 0.0f;

    if (gapDeg > 0.0f)
    {
        RecordActionHistory(L"Sóng thần (Khe hở)");
    }
    else
    {
        RecordActionHistory(L"Sóng thần");
    }
}

void WhaleBoss::ForceTsunami(PlayerFish& player)
{
    if (!active_)
    {
        active_ = true;
        pos_ = Vec3{ 0.0f, 0.0f, 350.0f };
    }
    StartTsunamiTelegraph(player, 0.0f);
}

void WhaleBoss::ForceTsunamiWithGap(PlayerFish& player)
{
    if (!active_)
    {
        active_ = true;
        pos_ = Vec3{ 0.0f, 0.0f, 350.0f };
    }
    StartTsunamiTelegraph(player, GameConsts::TSUNAMI_GAP_DEG_P2);
}

void WhaleBoss::ForceDoubleSlam(PlayerFish& player)
{
    if (!active_)
    {
        active_ = true;
        pos_ = Vec3{ 0.0f, 0.0f, 350.0f };
    }
    action_ = BossAction::TelegraphDoubleSlam;
    actionTimer_ = 0.0f;
    slamTarget_ = player.GetPosition();
    slamTarget_.y = 0.0f;
    isSecondSlam_ = false;
    RecordActionHistory(L"Đập kép (F7)");
    if (player.GetHeightY() <= 0.01f && !player.IsAirborne()) player.ResetJumpCooldown();
}

void WhaleBoss::SpawnRing(float gapDeg, const Vec3& targetPos)
{
    for (auto& r : tsunamiRings_)
    {
        if (!r.active)
        {
            r.active = true;
            r.center = pos_;
            r.center.y = 0.0f; // Tâm cố định tại vị trí phóng
            r.radius = GameConsts::TSUNAMI_START_RADIUS;
            r.speed = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_SPEED_P1 : GameConsts::TSUNAMI_SPEED_P2;
            r.thickness = GameConsts::TSUNAMI_THICKNESS;
            r.hitPlayer = false;
            r.dodgedNotice = false;

            if (gapDeg > 0.0f)
            {
                // Thống nhất hệ góc: hướng về phía người chơi lúc phóng trong hệ atan2(x, z)
                Vec3 toTarget = targetPos - pos_;
                toTarget.y = 0.0f;
                if (toTarget.LengthSq() > 1.0f)
                {
                    r.gapCenterRad = std::atan2(toTarget.x, toTarget.z);
                }
                else
                {
                    r.gapCenterRad = yaw_;
                }
                r.gapHalfRad = (gapDeg * 3.14159265f / 180.0f) * 0.5f;
            }
            else
            {
                r.gapCenterRad = 0.0f;
                r.gapHalfRad = 0.0f;
            }
            break;
        }
    }
}

void WhaleBoss::PickNextAction(PlayerFish& player, EnemyManager& enemies)
{
    actionTimer_ = 0.0f;
    actionCycle_++;

    Vec3 toPlayer = player.GetPosition() - pos_;
    toPlayer.y = 0.0f;
    float distToPlayer = toPlayer.Length();

    bool canJumpAttack = (timeSinceLastJumpAttack_ >= GameConsts::BOSS_MIN_JUMP_GAP);
    bool canSummon = (enemies.GetActiveCount() < GameConsts::BOSS_MINION_ALIVE_CAP);
    bool canTsunami = canJumpAttack && (timeSinceLastTsunami_ >= GameConsts::BOSS_TSUNAMI_COOLDOWN);

    struct Candidate
    {
        BossSkill skill;
        float weight;
    };
    std::array<Candidate, 4> candidates{};
    int count = 0;

    auto TryAddCandidate = [&](BossSkill skill, float baseWeight)
    {
        // 1. Không lặp lại ngay chiêu vừa dùng
        if (skill == lastSkill_) return;

        // 2. Luật loại trừ theo kỹ năng
        if ((skill == BossSkill::Slam || skill == BossSkill::DoubleSlam) && !canJumpAttack) return;
        if (skill == BossSkill::Minions && !canSummon) return;
        if (skill == BossSkill::Tsunami && !canTsunami) return;

        // 3. Điều chỉnh trọng số theo cự ly người chơi
        float w = baseWeight;
        if (distToPlayer < 200.0f && (skill == BossSkill::Slam || skill == BossSkill::DoubleSlam))
        {
            w *= 1.5f;
        }
        else if (distToPlayer > 450.0f && skill == BossSkill::Tsunami)
        {
            w *= 1.5f;
        }

        candidates[count++] = Candidate{ skill, w };
    };

    if (phase_ == BossPhase::Phase1)
    {
        // Phase 1 cơ bản: Nhảy đập 3.0, Triệu hồi 2.0, Sóng thần 3.0
        TryAddCandidate(BossSkill::Slam, 3.0f);
        TryAddCandidate(BossSkill::Minions, 2.0f);
        TryAddCandidate(BossSkill::Tsunami, 3.0f);
    }
    else
    {
        // Phase 2 cuồng nộ: Đập kép 3.0, Sóng thần 3.0, Triệu hồi 2.0, Nhảy đập 2.0
        TryAddCandidate(BossSkill::DoubleSlam, 3.0f);
        TryAddCandidate(BossSkill::Tsunami, 3.0f);
        TryAddCandidate(BossSkill::Minions, 2.0f);
        TryAddCandidate(BossSkill::Slam, 2.0f);
    }

    // Nếu danh sách hợp lệ rỗng: chọn Triệu hồi (nếu được) hoặc kéo dài Idle thêm nhịp ngắn, không được kẹt
    BossSkill chosenSkill = BossSkill::None;
    if (count == 0)
    {
        if (canSummon)
        {
            chosenSkill = BossSkill::Minions;
        }
        else
        {
            action_ = BossAction::Idle;
            idleDuration_ = 0.8f;
            return;
        }
    }
    else
    {
        float totalWeight = 0.0f;
        for (int i = 0; i < count; ++i) totalWeight += candidates[i].weight;

        float roll = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * totalWeight;
        float accum = 0.0f;
        chosenSkill = candidates[0].skill;
        for (int i = 0; i < count; ++i)
        {
            accum += candidates[i].weight;
            if (roll <= accum)
            {
                chosenSkill = candidates[i].skill;
                break;
            }
        }
    }

    lastSkill_ = chosenSkill;

    // Thiết lập trạng thái theo chiêu đã chọn
    switch (chosenSkill)
    {
    case BossSkill::Slam:
    {
        action_ = BossAction::TelegraphSlam;
        RecordActionHistory(L"Nhảy đập");
        if (player.GetHeightY() <= 0.01f && !player.IsAirborne()) player.ResetJumpCooldown();
        break;
    }
    case BossSkill::DoubleSlam:
    {
        action_ = BossAction::TelegraphDoubleSlam;
        isSecondSlam_ = false;
        RecordActionHistory(L"Đập kép");
        if (player.GetHeightY() <= 0.01f && !player.IsAirborne()) player.ResetJumpCooldown();
        break;
    }
    case BossSkill::Minions:
    {
        action_ = BossAction::TelegraphMinions;
        RecordActionHistory((phase_ == BossPhase::Phase1) ? L"Triệu hồi" : L"Triệu hồi (Hỗn hợp)");
        break;
    }
    case BossSkill::Tsunami:
    {
        if (phase_ == BossPhase::Phase1)
        {
            needSecondRing_ = false;
            StartTsunamiTelegraph(player, 0.0f);
        }
        else
        {
            // Phase 2: 50% hai vòng liên tiếp, 50% một vòng có khe hở 50 độ
            bool withGap = (rand() % 2 == 0);
            if (withGap)
            {
                needSecondRing_ = false;
                StartTsunamiTelegraph(player, GameConsts::TSUNAMI_GAP_DEG_P2);
            }
            else
            {
                needSecondRing_ = true;
                StartTsunamiTelegraph(player, 0.0f);
            }
        }
        break;
    }
    default:
        action_ = BossAction::Idle;
        idleDuration_ = 0.8f;
        break;
    }
}

void WhaleBoss::Update(float dt, PlayerFish& player, EnemyManager& enemies, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return;

    bossFightTimer_ += dt;

    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;
    if (phaseTransitionTimer_ > 0.0f)
    {
        phaseTransitionTimer_ -= dt;
        return;
    }

    // Luôn hướng về phía cá trên XZ nếu không trong pha thực hiện đập
    Vec3 toPlayer = player.GetPosition() - pos_;
    toPlayer.y = 0.0f;
    if (toPlayer.LengthSq() > 1.0f && action_ != BossAction::PerformSlam && action_ != BossAction::PerformDoubleSlam)
    {
        yaw_ = std::atan2(toPlayer.x, toPlayer.z);
    }

    actionTimer_ += dt;
    timeSinceLastJumpAttack_ += dt;
    timeSinceLastRingLaunch_ += dt;
    timeSinceLastTsunami_ += dt;

    // Hỗ trợ gọi vòng thứ 2 cách vòng 1 tối thiểu 2.3s (A5)
    if (needSecondRing_ && timeSinceLastRingLaunch_ >= 2.3f)
    {
        SpawnRing(0.0f, player.GetPosition());
        needSecondRing_ = false;
        effects.AddTrauma(0.35f);
        effects.SpawnShockwave(pos_, 120.0f);
    }

    // Máy trạng thái các đòn đánh của Boss
    switch (action_)
    {
    case BossAction::Idle:
    {
        // 3e: Di chuyển chậm về phía người chơi nếu cự ly > 200 để không bị kẹt ở góc xa
        float distToPlayer = toPlayer.Length();
        if (distToPlayer > 200.0f)
        {
            Vec3 moveDir = toPlayer * (1.0f / distToPlayer);
            pos_.x += moveDir.x * GameConsts::BOSS_WALK_SPEED * dt;
            pos_.z += moveDir.z * GameConsts::BOSS_WALK_SPEED * dt;
            pos_.y = 0.0f;
        }

        if (actionTimer_ >= idleDuration_)
        {
            PickNextAction(player, enemies);
            slamTarget_ = player.GetPosition();
            slamTarget_.y = 0.0f;
        }
        break;
    }

    // 1. Nhảy đập (Slam)
    case BossAction::TelegraphSlam:
    {
        float telegraphDuration = (phase_ == BossPhase::Phase1) ? 1.2f : 0.75f;
        if (actionTimer_ >= telegraphDuration)
        {
            action_ = BossAction::PerformSlam;
            actionTimer_ = 0.0f;
            slamProgress_ = 0.0f;
        }
        break;
    }
    case BossAction::PerformSlam:
    {
        float jumpDuration = 0.65f;
        slamProgress_ += dt / jumpDuration;
        if (slamProgress_ >= 1.0f)
        {
            pos_ = slamTarget_;
            pos_.y = 0.0f;
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
            timeSinceLastJumpAttack_ = 0.0f; // Đánh dấu đòn nhảy đã kết thúc

            effects.AddTrauma(0.55f);
            effects.SpawnShockwave(pos_, slamRadius_);

            Vec3 pDiff = player.GetPosition() - pos_;
            pDiff.y = 0.0f;
            if (pDiff.Length() <= slamRadius_ && player.GetHeightY() < 28.0f)
            {
                player.TakeDamage(35.0f, effects);
            }
        }
        else
        {
            float t = slamProgress_;
            pos_.y = 4.0f * 130.0f * t * (1.0f - t);
            pos_.x += (slamTarget_.x - pos_.x) * (dt * 5.0f);
            pos_.z += (slamTarget_.z - pos_.z) * (dt * 5.0f);
        }
        break;
    }

    // 2. Gọi quái lính
    case BossAction::TelegraphMinions:
    {
        float duration = 0.85f;
        if (actionTimer_ >= duration)
        {
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;

            effects.SpawnShockwave(pos_, 160.0f);
            effects.AddTrauma(0.3f);

            int currentAlive = enemies.GetActiveCount();
            int maxCanSpawn = (std::max)(0, GameConsts::BOSS_MINION_ALIVE_CAP - currentAlive);

            if (phase_ == BossPhase::Phase1)
            {
                int desired = 6;
                int actual = (std::min)(desired, maxCanSpawn);
                if (actual > 0)
                {
                    float angleStep = 6.28318f / actual;
                    for (int i = 0; i < actual; ++i)
                    {
                        float a = i * angleStep;
                        Vec3 sp = pos_ + Vec3{ std::cos(a) * 120.0f, 0.0f, std::sin(a) * 120.0f };
                        enemies.SpawnEnemy(EnemyType::Minion, sp);
                    }
                }
            }
            else
            {
                // Phase 2: Triệu hồi hỗn hợp 5 Minion + 3 Hopper up to cap 12 (Task 10 T3d)
                int minionCount = (std::min)(5, maxCanSpawn);
                int hopperCount = (std::min)(3, maxCanSpawn - minionCount);
                int totalSpawn = minionCount + hopperCount;
                if (totalSpawn > 0)
                {
                    float angleStep = 6.28318f / totalSpawn;
                    for (int i = 0; i < totalSpawn; ++i)
                    {
                        float a = i * angleStep;
                        Vec3 sp = pos_ + Vec3{ std::cos(a) * 125.0f, 0.0f, std::sin(a) * 125.0f };
                        EnemyType type = (i < minionCount) ? EnemyType::Minion : EnemyType::Hopper;
                        enemies.SpawnEnemy(type, sp);
                    }
                }
            }
        }
        break;
    }

    // 3. Nhảy đập đôi (Phase 2 - Task 10 T3b)
    case BossAction::TelegraphDoubleSlam:
    {
        if (actionTimer_ >= 0.6f)
        {
            action_ = BossAction::PerformDoubleSlam;
            actionTimer_ = 0.0f;
            slamProgress_ = 0.0f;
            isSecondSlam_ = false;
        }
        break;
    }
    case BossAction::PerformDoubleSlam:
    {
        float jumpDur = 0.5f;
        slamProgress_ += dt / jumpDur;
        if (slamProgress_ >= 1.0f)
        {
            pos_ = slamTarget_;
            pos_.y = 0.0f;
            effects.AddTrauma(0.55f);
            effects.SpawnShockwave(pos_, slamRadius_);

            Vec3 pDiff = player.GetPosition() - pos_;
            pDiff.y = 0.0f;
            if (pDiff.Length() <= slamRadius_ && player.GetHeightY() < 28.0f)
            {
                player.TakeDamage(30.0f, effects);
            }

            if (!isSecondSlam_)
            {
                // Task 10 T3b: Chuyển sang Telegraph cú 2, báo hiệu 0.45s tại vị trí mới
                action_ = BossAction::TelegraphDoubleSlam2;
                actionTimer_ = 0.0f;
                slamTarget_ = player.GetPosition();
                slamTarget_.y = 0.0f;
                // Nếu người chơi ở dưới đất, reset jump cooldown để người chơi có cơ hội né cú 2
                if (player.GetHeightY() <= 0.01f && !player.IsAirborne())
                {
                    player.ResetJumpCooldown();
                }
            }
            else
            {
                action_ = BossAction::Idle;
                actionTimer_ = 0.0f;
                timeSinceLastJumpAttack_ = 0.0f;
            }
        }
        else
        {
            float t = slamProgress_;
            pos_.y = 4.0f * 110.0f * t * (1.0f - t);
            pos_.x += (slamTarget_.x - pos_.x) * (dt * 6.0f);
            pos_.z += (slamTarget_.z - pos_.z) * (dt * 6.0f);
        }
        break;
    }
    case BossAction::TelegraphDoubleSlam2:
    {
        if (actionTimer_ >= GameConsts::BOSS_DOUBLE_SLAM_TELEGRAPH2)
        {
            action_ = BossAction::PerformDoubleSlam;
            actionTimer_ = 0.0f;
            slamProgress_ = 0.0f;
            isSecondSlam_ = true;
        }
        break;
    }

    // 4. Kỹ năng Sóng thần tỏa tròn (Radial Tsunami - Phần A)
    case BossAction::TelegraphRadialTsunami:
    {
        float duration = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_TELEGRAPH_P1 : GameConsts::TSUNAMI_TELEGRAPH_P2;
        if (actionTimer_ >= duration)
        {
            // Phóng vòng sóng thần tỏa tròn từ vị trí hiện tại của boss (Task 10 T3c)
            SpawnRing(currentTsunamiGapDeg_, player.GetPosition());

            timeSinceLastJumpAttack_ = 0.0f;
            timeSinceLastRingLaunch_ = 0.0f;
            timeSinceLastTsunami_ = 0.0f;

            effects.AddTrauma(0.45f);
            effects.SpawnShockwave(pos_, 150.0f);

            // Boss quay lại Idle ngay sau khi phóng, không cần chờ sóng đi hết (A2)
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
        }
        break;
    }

    default:
        break;
    }

    // Cập nhật các Vòng Sóng thần tỏa tròn (A3, A4)
    for (auto& ring : tsunamiRings_)
    {
        if (!ring.active) continue;

        ring.radius += ring.speed * dt;

        // Tự hủy khi mép trong vượt qua góc xa nhất của sàn đấu (khoảng cách > 1100)
        if (ring.radius - ring.thickness * 0.5f > 1100.0f)
        {
            ring.active = false;
            continue;
        }

        // Kiểm tra va chạm hình vành khuyên (Annulus) với người chơi
        Vec3 rel = player.GetPosition() - ring.center;
        rel.y = 0.0f;
        float d = rel.Length();
        float half = ring.thickness * 0.5f + player.GetHitboxRadius();

        bool inBand = (d >= ring.radius - half) && (d <= ring.radius + half);

        // Kiểm tra khe hở (nếu có gapHalfRad > 0)
        if (inBand && ring.gapHalfRad > 0.0f)
        {
            float ang = std::atan2(rel.x, rel.z);
            float diff = std::abs(ang - ring.gapCenterRad);
            while (diff > 3.14159265f) diff = std::abs(diff - 6.2831853f);
            if (diff <= ring.gapHalfRad)
            {
                inBand = false; // Đang ở trong khe hở an toàn
            }
        }

        if (inBand && !ring.hitPlayer)
        {
            // NÉ BẰNG NHẢY TRÊN KHÔNG (A4):
            // Cá đang ở trên không (Y > 0.01f hoặc IsAirborne()) thì hoàn toàn bất tử, không bị thương!
            if (player.GetHeightY() > 0.01f || player.IsAirborne())
            {
                if (!ring.dodgedNotice)
                {
                    ring.dodgedNotice = true;
                    // Hiệu ứng hạt xanh lá biểu dương né thành công (A7)
                    effects.SpawnHitParticles(player.GetPosition() + Vec3{ 0, 8, 0 }, 8, GetColor(100, 255, 180));
                }
            }
            else if (!player.IsInvulnerable())
            {
                // Đang ở dưới đất: Bị sóng thần đánh trúng!
                Vec3 outward = (d > 0.001f) ? (rel * (1.0f / d)) : Vec3{ 0, 0, 1 };
                player.TakeDamage(GameConsts::TSUNAMI_DAMAGE, effects);
                player.ApplyKnockback(outward * GameConsts::TSUNAMI_KNOCKBACK);
                ring.hitPlayer = true;
                effects.AddTrauma(0.35f);
                effects.SpawnHitParticles(player.GetPosition() + Vec3{ 0, 15, 0 }, 18, GetColor(0, 180, 255));
            }
        }
    }
}

int WhaleBoss::GetActiveTsunamiCount() const
{
    int count = 0;
    for (const auto& r : tsunamiRings_)
    {
        if (r.active) count++;
    }
    return count;
}

float WhaleBoss::GetTsunamiThicknessOverSpeed() const
{
    float spd = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_SPEED_P1 : GameConsts::TSUNAMI_SPEED_P2;
    return GameConsts::TSUNAMI_THICKNESS / spd;
}

float WhaleBoss::GetTsunamiJumpWindow() const
{
    return GameConsts::JUMP_DURATION - GetTsunamiThicknessOverSpeed();
}

float WhaleBoss::GetTimeToHitPlayer(const Vec3& playerPos) const
{
    float spd = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_SPEED_P1 : GameConsts::TSUNAMI_SPEED_P2;

    // Nếu đang có vòng sóng active lan tới:
    for (const auto& r : tsunamiRings_)
    {
        if (r.active)
        {
            Vec3 diff = playerPos - r.center;
            diff.y = 0.0f;
            float dist = diff.Length();
            float waveFront = r.radius + r.thickness * 0.5f;
            if (dist > waveFront)
            {
                return (dist - waveFront) / r.speed;
            }
            else
            {
                return 0.0f;
            }
        }
    }

    // Nếu đang trong pha báo hiệu (Telegraph):
    if (action_ == BossAction::TelegraphRadialTsunami)
    {
        float dur = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_TELEGRAPH_P1 : GameConsts::TSUNAMI_TELEGRAPH_P2;
        float remainingTelegraph = (std::max)(0.0f, dur - actionTimer_);
        Vec3 diff = playerPos - pos_;
        diff.y = 0.0f;
        float dist = diff.Length();
        return remainingTelegraph + (std::max)(0.0f, dist - GameConsts::TSUNAMI_START_RADIUS) / spd;
    }

    return -1.0f;
}

void WhaleBoss::Draw() const
{
    if (!active_ || hp_ <= 0.0f) return;

    // 1. Vẽ Decal báo hiệu trước khi tung đòn
    if (action_ == BossAction::TelegraphSlam || action_ == BossAction::TelegraphDoubleSlam || action_ == BossAction::TelegraphDoubleSlam2)
    {
        float maxDur = (action_ == BossAction::TelegraphDoubleSlam2) ? GameConsts::BOSS_DOUBLE_SLAM_TELEGRAPH2 : 1.0f;
        float ratio = std::clamp(actionTimer_ / maxDur, 0.1f, 1.0f);
        int alpha = static_cast<int>(120.0f + ratio * 80.0f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        Render3D::DrawCircle3D(slamTarget_, slamRadius_ * ratio, 32, GetColor(255, 40, 40), true);
        Render3D::DrawCircle3D(slamTarget_, slamRadius_, 32, GetColor(255, 230, 80), false);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else if (action_ == BossAction::TelegraphMinions)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
        Render3D::DrawCircle3D(pos_, 140.0f, 32, GetColor(180, 60, 255), false);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else if (action_ == BossAction::TelegraphRadialTsunami)
    {
        // Vòng cảnh báo đỏ phình dần quanh boss + các vòng mờ đồng tâm (A2, A7)
        float dur = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_TELEGRAPH_P1 : GameConsts::TSUNAMI_TELEGRAPH_P2;
        float ratio = std::clamp(actionTimer_ / dur, 0.0f, 1.0f);

        // Nhấp nháy nhanh dần trong 0.3s cuối
        bool flash = true;
        if (dur - actionTimer_ <= 0.3f)
        {
            float fT = dur - actionTimer_;
            flash = (static_cast<int>(fT * 30.0f) % 2 == 0);
        }

        if (flash)
        {
            int alpha = static_cast<int>(130.0f + ratio * 90.0f);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            float currentExpandR = GameConsts::TSUNAMI_START_RADIUS + 50.0f * ratio;
            Render3D::DrawCircle3D(pos_, currentExpandR, 36, GetColor(255, 50, 50), true);
            Render3D::DrawCircle3D(pos_, currentExpandR, 36, GetColor(255, 220, 60), false);

            // 2-3 vòng đồng tâm mờ thể hiện hướng lan tỏa
            Render3D::DrawCircle3D(pos_, currentExpandR + 40.0f, 36, GetColor(255, 100, 80), false);
            Render3D::DrawCircle3D(pos_, currentExpandR + 80.0f, 36, GetColor(255, 150, 100), false);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // 2. Vẽ Vành Sóng Thần Tỏa Tròn 3D (A7, Task 10 T3c thống nhất hệ góc atan2(x, z))
    for (const auto& ring : tsunamiRings_)
    {
        if (!ring.active) continue;

        int numSegments = 56;
        float angleStep = 6.2831853f / numSegments;
        float rInner = (std::max)(1.0f, ring.radius - ring.thickness * 0.5f);
        float rOuter = ring.radius + ring.thickness * 0.5f;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 190);
        int waterColor = GetColor(35, 150, 245);
        int foamColor = GetColor(230, 245, 255);

        for (int i = 0; i < numSegments; ++i)
        {
            float a1 = i * angleStep;
            float a2 = (i + 1) * angleStep;

            // Bỏ qua đoạn nằm trong khe hở (nếu có)
            if (ring.gapHalfRad > 0.0f)
            {
                float midA = (a1 + a2) * 0.5f;
                float diff = std::abs(midA - ring.gapCenterRad);
                while (diff > 3.14159265f) diff = std::abs(diff - 6.2831853f);
                if (diff <= ring.gapHalfRad) continue;
            }

            // Dùng sin(a) cho X và cos(a) cho Z để khớp 100% với atan2(x, z)
            float s1 = std::sin(a1), c1 = std::cos(a1);
            float s2 = std::sin(a2), c2 = std::cos(a2);

            VECTOR pIn1 = VGet(ring.center.x + s1 * rInner, 1.0f, ring.center.z + c1 * rInner);
            VECTOR pIn2 = VGet(ring.center.x + s2 * rInner, 1.0f, ring.center.z + c2 * rInner);
            VECTOR pOut1 = VGet(ring.center.x + s1 * rOuter, 14.0f, ring.center.z + c1 * rOuter);
            VECTOR pOut2 = VGet(ring.center.x + s2 * rOuter, 14.0f, ring.center.z + c2 * rOuter);

            // Vẽ 2 tam giác tạo nên hình thang vành khuyên
            DrawTriangle3D(pIn1, pOut1, pOut2, waterColor, TRUE);
            DrawTriangle3D(pIn1, pOut2, pIn2, waterColor, TRUE);
            DrawTriangle3D(pIn1, pOut2, pOut1, waterColor, TRUE);
            DrawTriangle3D(pIn1, pIn2, pOut2, waterColor, TRUE);

            // Mép bọt nước trắng phía trước
            ::DrawLine3D(pOut1, pOut2, foamColor);
        }

        // Vẽ vạch chỉ báo phát sáng xanh ngọc tại 2 mép khe hở để người chơi nhận diện lối thoát an toàn (Task 10 T3c)
        if (ring.gapHalfRad > 0.0f)
        {
            int gapLineColor = GetColor(50, 255, 230);
            float g1 = ring.gapCenterRad - ring.gapHalfRad;
            float g2 = ring.gapCenterRad + ring.gapHalfRad;

            VECTOR g1In = VGet(ring.center.x + std::sin(g1) * rInner, 1.0f, ring.center.z + std::cos(g1) * rInner);
            VECTOR g1Out = VGet(ring.center.x + std::sin(g1) * rOuter, 14.0f, ring.center.z + std::cos(g1) * rOuter);
            VECTOR g2In = VGet(ring.center.x + std::sin(g2) * rInner, 1.0f, ring.center.z + std::cos(g2) * rInner);
            VECTOR g2Out = VGet(ring.center.x + std::sin(g2) * rOuter, 14.0f, ring.center.z + std::cos(g2) * rOuter);

            ::DrawLine3D(g1In, g1Out, gapLineColor);
            ::DrawLine3D(g2In, g2Out, gapLineColor);

            VECTOR g1Top = VGet(ring.center.x + std::sin(g1) * rOuter, 28.0f, ring.center.z + std::cos(g1) * rOuter);
            VECTOR g2Top = VGet(ring.center.x + std::sin(g2) * rOuter, 28.0f, ring.center.z + std::cos(g2) * rOuter);
            ::DrawLine3D(g1Out, g1Top, gapLineColor);
            ::DrawLine3D(g2Out, g2Top, gapLineColor);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 3. Vẽ bóng dẹt của boss
    float shadowRad = 65.0f * (1.0f - std::clamp(pos_.y / 150.0f, 0.0f, 0.5f));
    Render3D::DrawFlatShadow(pos_, shadowRad, 0.95f);

    // 4. Vẽ thân Boss Cá Voi 3D Box
    int bodyColor = (phase_ == BossPhase::Phase1) ? GetColor(45, 70, 105) : GetColor(145, 30, 45);
    int edgeColor = (phase_ == BossPhase::Phase1) ? GetColor(80, 130, 190) : GetColor(255, 90, 60);

    if (hitFlashTimer_ > 0.0f)
    {
        bodyColor = GetColor(255, 255, 255);
        edgeColor = GetColor(255, 120, 120);
    }

    // Khi nạp sóng thần, thân boss rung nhẹ và phình lên (A2)
    float scaleMul = 1.0f;
    Vec3 shakeOffset{ 0, 0, 0 };
    if (action_ == BossAction::TelegraphRadialTsunami)
    {
        scaleMul = 1.08f;
        shakeOffset.x = (rand() % 7 - 3) * 0.8f;
        shakeOffset.z = (rand() % 7 - 3) * 0.8f;
    }

    Vec3 halfBody{ 50.0f * scaleMul, 26.0f * scaleMul, 75.0f * scaleMul };
    Vec3 center = pos_ + shakeOffset + Vec3{ 0.0f, halfBody.y, 0.0f };
    Render3D::DrawOrientedBox3D(center, halfBody, yaw_, bodyColor, edgeColor, true);

    // Mắt phát quang
    float eyeForward = halfBody.z * 0.6f;
    float eyeSide = halfBody.x + 1.5f;
    Vec3 leftEye = center + Vec3{
        -std::cos(yaw_) * eyeSide + std::sin(yaw_) * eyeForward,
        8.0f,
        std::sin(yaw_) * eyeSide + std::cos(yaw_) * eyeForward
    };
    Vec3 rightEye = center + Vec3{
        std::cos(yaw_) * eyeSide + std::sin(yaw_) * eyeForward,
        8.0f,
        -std::sin(yaw_) * eyeSide + std::cos(yaw_) * eyeForward
    };
    int eyeCol = (phase_ == BossPhase::Phase2) ? GetColor(255, 230, 40) : GetColor(255, 60, 60);
    Render3D::DrawOrientedBox3D(leftEye, Vec3{ 3.5f, 3.5f, 3.5f }, yaw_, eyeCol, 0, false);
    Render3D::DrawOrientedBox3D(rightEye, Vec3{ 3.5f, 3.5f, 3.5f }, yaw_, eyeCol, 0, false);

    // Đuôi cá voi
    float tailDist = -halfBody.z - 18.0f;
    Vec3 tailCenter = center + Vec3{ std::sin(yaw_) * tailDist, 6.0f, std::cos(yaw_) * tailDist };
    Render3D::DrawOrientedBox3D(tailCenter, Vec3{ 14.0f, 20.0f, 18.0f }, yaw_, bodyColor, edgeColor, true);
}

void WhaleBoss::RecordActionHistory(const wchar_t* actionName)
{
    if (!actionName) return;
    actionHistory_[actionHistoryHead_] = actionName;
    actionHistoryHead_ = (actionHistoryHead_ + 1) % ACTION_HISTORY_CAPACITY;
    if (actionHistoryCount_ < ACTION_HISTORY_CAPACITY)
    {
        actionHistoryCount_++;
    }
}

const wchar_t* WhaleBoss::GetRecentAction(int indexFromNewest) const
{
    if (indexFromNewest < 0 || indexFromNewest >= actionHistoryCount_) return nullptr;
    int idx = (actionHistoryHead_ - 1 - indexFromNewest + ACTION_HISTORY_CAPACITY * 2) % ACTION_HISTORY_CAPACITY;
    return actionHistory_[idx];
}
