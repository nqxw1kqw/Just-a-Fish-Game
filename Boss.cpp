#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <algorithm>
#include <cmath>
#include "Boss.h"
#include "RenderInterp.h"
#include "PlayerFish.h"
#include "Enemy.h"
#include "Effects.h"
#include "Render3DUtil.h"
#include "DxConv.h"
#include "Effects/Whale/WhaleFxSet.h"
#include "Effects/Hit/HitFeedback.h"
#include "SoundManager.h"
#include "DxLib.h"
#include "CollisionUtil.h"
#include "MapObstacles.h"
#include "LowPolyModels.h"

namespace
{
    // Ánh xạ vòng gameplay sang trạng thái hiển thị cho hiệu ứng nước
    TsunamiWaveState ToWaveState(const TsunamiRing& r)
    {
        TsunamiWaveState w;
        w.active = r.active;
        w.center = r.center;
        w.radius = r.radius;
        w.thickness = r.thickness;
        w.speed = r.speed;
        w.gapCenterRad = r.gapCenterRad;
        w.gapHalfRad = r.gapHalfRad;
        return w;
    }

    std::array<TsunamiWaveState, TsunamiWaterEffect::MAX_WAVES> ToWaveStates(const std::array<TsunamiRing, GameConsts::TSUNAMI_MAX_ACTIVE>& rings)
    {
        std::array<TsunamiWaveState, TsunamiWaterEffect::MAX_WAVES> out{};
        for (int i = 0; i < TsunamiWaterEffect::MAX_WAVES && i < GameConsts::TSUNAMI_MAX_ACTIVE; ++i) out[i] = ToWaveState(rings[i]);
        return out;
    }
}

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
    prevSkill_ = BossSkill::None;
    burstCount_ = 0;
    burstSize_ = GameConsts::BURST_MAX_P1;
    restTimer_ = 0.0f;
    hasLastPlayerPos_ = false;
    playerVel_ = Vec3{ 0.0f, 0.0f, 0.0f };
    timeSinceLastTsunami_ = 10.0f;
    currentTsunamiGapDeg_ = 0.0f;
    waterFxTimer_ = 0.0f;

    for (auto& r : tsunamiRings_) r.active = false;
    tsunamiWater_.Reset();
    tailCooldown_ = 0.0f;
    breachFxArmed_ = false;
    netBreachPending_ = false;
    if (fx_) fx_->ClearAll();
    SoundManager::Instance().StopSFX(SfxId::BossInhaleLoop);
    diveCooldown_ = 0.0f;
    comboNext_ = BossSkill::None;
    pendingPhaseWave_ = false;
    enraged_ = false;
    packSpotCount_ = 0;
    bubblePillars_.clear();
    bubbleNetCooldown_ = 0.0f;
    inhaleCooldown_ = 0.0f;
    netHitCooldown_ = 0.0f;
    hugTimer_ = 0.0f;
    stunTimer_ = 0.0f;
    stunDamageMul_ = 1.0f;
    telegraphFxTimer_ = 0.0f;
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
    prevSkill_ = BossSkill::None;
    burstCount_ = 0;
    burstSize_ = GameConsts::BURST_MAX_P1;
    restTimer_ = 0.0f;
    hasLastPlayerPos_ = false;
    playerVel_ = Vec3{ 0.0f, 0.0f, 0.0f };
    timeSinceLastTsunami_ = 10.0f;
    currentTsunamiGapDeg_ = 0.0f;

    for (auto& r : tsunamiRings_) r.active = false;
    tsunamiWater_.Reset();
    tailCooldown_ = 0.0f;
    breachFxArmed_ = false;
    netBreachPending_ = false;
    if (fx_) fx_->ClearAll();
    SoundManager::Instance().StopSFX(SfxId::BossInhaleLoop);
    diveCooldown_ = 0.0f;
    comboNext_ = BossSkill::None;
    pendingPhaseWave_ = false;
    enraged_ = false;
    packSpotCount_ = 0;
    bubblePillars_.clear();
    bubbleNetCooldown_ = 0.0f;
    inhaleCooldown_ = 0.0f;
    netHitCooldown_ = 0.0f;
    hugTimer_ = 0.0f;
    stunTimer_ = 0.0f;
    stunDamageMul_ = 1.0f;
    telegraphFxTimer_ = 0.0f;
}

bool WhaleBoss::TakeDamage(float dmg, EffectSystem& effects)
{
    if (!active_ || hp_ <= 0.0f) return false;
    dmg *= damageTakenMul_;

    // Boss rework: đang choáng thì nhận thêm sát thương
    if (action_ == BossAction::Stunned) dmg *= stunDamageMul_;

    hp_ -= dmg;
    hitFlashTimer_ = GameConsts::HIT_FLASH_TIME;
    effects.SpawnHitFeedback(pos_ + Vec3{ 0, 30.0f, 0 }, Vec3{ 0.0f, 1.0f, 0.0f }, dmg >= GameConsts::HIT_CRIT_DAMAGE);
    effects.SpawnDamagePopup(pos_ + Vec3{ 0, 45.0f, 0 }, dmg, (dmg >= 50.0f));

    // Chuyển sang Giai đoạn 2 (< 50% máu: 500 HP)
    if (hp_ <= maxHp_ * 0.5f && phase_ == BossPhase::Phase1)
    {
        phase_ = BossPhase::Phase2;
        phaseTransitionTimer_ = 1.2f; // Khựng ngắn gầm rú và đổi màu cuồng nộ
        idleDuration_ = 0.95f;        // Nhịp đòn nhanh hơn
        effects.AddTrauma(0.7f);
        effects.SpawnShockwave(pos_, 260.0f);
        SoundManager::Instance().PlaySFX(SfxId::BossRoar); // Chỉ phát 1 lần lúc đổi phase
        pendingPhaseWave_ = true;     // PR D: hết khựng -> phóng 1 vòng sóng không khe
    }

    // PR D: < 20% HP -> cuồng nộ
    if (!enraged_ && hp_ > 0.0f && hp_ <= maxHp_ * GameConsts::RAGE_HP_RATIO)
    {
        enraged_ = true;
        if (phase_ == BossPhase::Phase1) phase_ = BossPhase::Phase2;
        idleDuration_ = GameConsts::RAGE_IDLE;
        effects.AddTrauma(0.8f);
        effects.SpawnShockwave(pos_, 300.0f);
        effects.SpawnBubbles(pos_, 40, 160.0f, 3.0f, 7.0f);
        SoundManager::Instance().PlaySFX(SfxId::BossRoar);
    }

    if (hp_ <= 0.0f)
    {
        hp_ = 0.0f;
        effects.AddTrauma(1.0f);
        effects.SpawnFishDeath(pos_ + Vec3{ 0, 30, 0 }, 150.0f); // HitFx: nổ bọt/vảy khi Boss chết
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

    SoundManager::Instance().PlaySFX(SfxId::TsunamiCharge);

    if (gapDeg > 0.0f)
    {
        RecordActionHistory(L"大波（隙間あり）");
    }
    else
    {
        RecordActionHistory(L"大波");
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
    slamTarget_ = MapObstacles::Resolve(slamTarget_, slamTarget_, 60.0f);
    slamCount_ = 0;
    RecordActionHistory(L"連続スラム (F7)");
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
                Vec3 toTarget = Collision::FlatDiff(targetPos, pos_);
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
            tsunamiWater_.OnLaunch(ToWaveState(r));
            break;
        }
    }
}

void WhaleBoss::PickNextAction(PlayerFish& player, EnemyManager& enemies)
{
    actionTimer_ = 0.0f;
    actionCycle_++;

    Vec3 toPlayer = Collision::FlatDiff(player.GetPosition(), pos_);
    float distToPlayer = toPlayer.Length();

    bool canJumpAttack = (timeSinceLastJumpAttack_ >= GameConsts::BOSS_MIN_JUMP_GAP);
    bool canSummon = (enemies.GetActiveCount() < GameConsts::BOSS_MINION_ALIVE_CAP);
    bool canTsunami = canJumpAttack && (timeSinceLastTsunami_ >= GameConsts::BOSS_TSUNAMI_COOLDOWN);

    struct Candidate
    {
        BossSkill skill;
        float weight;
    };
    std::array<Candidate, 12> candidates{};
    int count = 0;

    auto TryAddCandidate = [&](BossSkill skill, float baseWeight)
    {
        // 1. AI pacing: không bao giờ 3 lần liên tiếp cùng 1 chiêu
        if (skill == lastSkill_ && skill == prevSkill_) return;

        // 2. Luật loại trừ theo kỹ năng
        if ((skill == BossSkill::Slam || skill == BossSkill::DoubleSlam) && !canJumpAttack) return;
        if (skill == BossSkill::Minions && !canSummon) return;
        if (skill == BossSkill::Tsunami && !canTsunami) return;
        if (skill == BossSkill::BubbleNet && bubbleNetCooldown_ > 0.0f) return;
        if (skill == BossSkill::Dive && diveCooldown_ > 0.0f) return;
        if (skill == BossSkill::Inhale && !(inhaleCooldown_ <= 0.0f && distToPlayer < GameConsts::INHALE_RANGE)) return;
        if (skill == BossSkill::Tail && !(tailCooldown_ <= 0.0f && distToPlayer < GameConsts::TAIL_TRIGGER_RANGE && IsPlayerBehind(player))) return;

        // 3. Điều chỉnh trọng số theo cự ly người chơi (Boss rework 16)
        float w = baseWeight;
        if (distToPlayer < GameConsts::BOSS_NEAR_RANGE)
        {
            if (skill == BossSkill::Tail) w *= 2.0f;
            if (skill == BossSkill::Slam || skill == BossSkill::DoubleSlam) w *= 1.5f;
        }
        else if (distToPlayer > GameConsts::BOSS_FAR_RANGE)
        {
            if (skill == BossSkill::Tsunami) w *= 1.5f;
            if (skill == BossSkill::Inhale) w *= 1.5f;
            if (skill == BossSkill::Dive) w *= 2.0f;
        }
        if (hell_)
        {
            if (skill == BossSkill::DoubleSlam || skill == BossSkill::Tsunami || skill == BossSkill::BubbleNet || skill == BossSkill::Dive)
                w *= GameConsts::HELL_WHALE_STRONG_WEIGHT_MUL;
            if (skill == BossSkill::Minions) w *= GameConsts::HELL_WHALE_MINION_WEIGHT_MUL;
        }
        // AI pacing: giảm trọng số chiêu dùng gần đây
        if (skill == lastSkill_) w *= GameConsts::AI_REPEAT_WEIGHT_LAST;
        else if (skill == prevSkill_) w *= GameConsts::AI_REPEAT_WEIGHT_PREV;
        if (count >= static_cast<int>(candidates.size())) return;

        candidates[count++] = Candidate{ skill, w };
    };

    if (phase_ == BossPhase::Phase1)
    {
        // Phase 1 cơ bản: Nhảy đập 3.0, Triệu hồi 2.0, Sóng thần 3.0
        TryAddCandidate(BossSkill::Slam, 3.0f);
        TryAddCandidate(BossSkill::Minions, 2.0f);
        TryAddCandidate(BossSkill::Tsunami, 3.0f);
        TryAddCandidate(BossSkill::Tail, 2.5f);
        TryAddCandidate(BossSkill::Inhale, 1.5f);
        TryAddCandidate(BossSkill::BubbleNet, 1.0f);
        TryAddCandidate(BossSkill::Dive, 1.5f);
        if (hell_) TryAddCandidate(BossSkill::DoubleSlam, GameConsts::HELL_WHALE_P1_DOUBLESLAM_WEIGHT);
    }
    else
    {
        // Phase 2 cuồng nộ: Đập kép 3.0, Sóng thần 3.0, Triệu hồi 2.0, Nhảy đập 2.0
        TryAddCandidate(BossSkill::DoubleSlam, 3.0f);
        TryAddCandidate(BossSkill::Tsunami, 3.0f);
        TryAddCandidate(BossSkill::Minions, 2.0f);
        TryAddCandidate(BossSkill::Slam, 2.0f);
        TryAddCandidate(BossSkill::Tail, 2.5f);
        TryAddCandidate(BossSkill::Inhale, 2.0f);
        TryAddCandidate(BossSkill::BubbleNet, 2.5f);
        TryAddCandidate(BossSkill::Dive, 2.5f);
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

    // PR D: combo đã xếp hàng được ưu tiên (combo = cùng 1 bước của đợt, không tính thêm)
    if (comboNext_ != BossSkill::None)
    {
        chosenSkill = comboNext_;
        comboNext_ = BossSkill::None;
    }
    else
    {
        burstCount_++;
    }
    NoteSkill(chosenSkill);

    // Thiết lập trạng thái theo chiêu đã chọn
    switch (chosenSkill)
    {
    case BossSkill::Slam:
    {
        StartBreach(player);
        break;
    }
    case BossSkill::DoubleSlam:
    {
        action_ = BossAction::TelegraphDoubleSlam;
        slamCount_ = 0;
        RecordActionHistory(L"三連スラム");
        break;
    }
    case BossSkill::Tail:
    {
        StartTail();
        break;
    }
    case BossSkill::Minions:
    {
        StartPackTelegraph(player);
        RecordActionHistory((phase_ == BossPhase::Phase1) ? L"群れ召喚" : L"混成群れ召喚");
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
            // Phase 2 (PR B): 1/3 một vòng có khe, 1/3 hai vòng cách 2.3s, 1/3 SÓNG ĐÔI (không khe -> 0.8s -> có khe)
            int variant = rand() % 3;
            if (variant == 0)
            {
                needSecondRing_ = false;
                StartTsunamiTelegraph(player, GameConsts::TSUNAMI_GAP_DEG_P2);
            }
            else if (variant == 1)
            {
                needSecondRing_ = true;
                secondRingDelay_ = GameConsts::TSUNAMI_REPEAT_DELAY;
                secondRingGapDeg_ = 0.0f;
                StartTsunamiTelegraph(player, 0.0f);
            }
            else
            {
                needSecondRing_ = true;
                secondRingDelay_ = GameConsts::TSUNAMI_DOUBLE_DELAY;
                secondRingGapDeg_ = GameConsts::TSUNAMI_GAP_DEG_P2;
                StartTsunamiTelegraph(player, 0.0f);
                RecordActionHistory(L"二重の大波");
            }
        }
        break;
    }
    case BossSkill::BubbleNet:
        StartBubbleNet(player);
        break;
    case BossSkill::Inhale:
        StartInhale(player);
        break;
    case BossSkill::Dive:
        StartDive(player);
        break;
    default:
        action_ = BossAction::Idle;
        idleDuration_ = 0.8f;
        break;
    }
}

void WhaleBoss::Update(float dt, PlayerFish& player, EnemyManager& enemies, EffectSystem& effects)
{
    // PR E: hạt Cá voi vẫn tàn dần kể cả khi boss khựng/chết
    if (fx_)
    {
        fx_->Update(dt);
        if (fx_->slow.IsActive()) fx_->slow.SetTarget(DxConv::ToVECTOR(player.GetPosition()));
    }
    if (!active_ || hp_ <= 0.0f) return;

    // AI pacing: vận tốc người chơi từ chênh lệch vị trí giữa các khung (EMA)
    if (dt > 0.0001f)
    {
        const Vec3& pp = player.GetPosition();
        if (hasLastPlayerPos_)
        {
            Vec3 v{ (pp.x - lastPlayerPos_.x) / dt, 0.0f, (pp.z - lastPlayerPos_.z) / dt };
            if (v.Length() <= GameConsts::AIM_MAX_PLAYER_SPEED)
            {
                float a = GameConsts::AIM_VEL_SMOOTHING;
                playerVel_ = playerVel_ * (1.0f - a) + v * a;
            }
        }
        lastPlayerPos_ = pp;
        hasLastPlayerPos_ = true;
    }

    bossFightTimer_ += dt;

    if (hitFlashTimer_ > 0.0f) hitFlashTimer_ -= dt;
    if (phaseTransitionTimer_ > 0.0f)
    {
        phaseTransitionTimer_ -= dt;
        if (phaseTransitionTimer_ <= 0.0f && pendingPhaseWave_)
        {
            pendingPhaseWave_ = false;
            SpawnRing(0.0f, player.GetPosition());
            effects.AddTrauma(0.4f);
            effects.SpawnShockwave(pos_, 160.0f);
            SoundManager::Instance().PlaySFX(SfxId::TsunamiCharge);
        }
        return;
    }

    // Luôn hướng về phía cá trên XZ nếu không trong pha thực hiện đập
    Vec3 toPlayer = Collision::FlatDiff(player.GetPosition(), pos_);
    // Boss rework: xoay dần với tốc độ giới hạn (người chơi có thể vòng ra sau lưng -> Tail Slap)
    bool canTurn = action_ != BossAction::PerformSlam && action_ != BossAction::PerformDoubleSlam &&
                   action_ != BossAction::TelegraphTail && action_ != BossAction::PerformTail && action_ != BossAction::Stunned &&
                   action_ != BossAction::BubbleNetSwim && action_ != BossAction::PerformDive;
    if (toPlayer.LengthSq() > 1.0f && canTurn)
    {
        float targetYaw = std::atan2(toPlayer.x, toPlayer.z);
        float diff = targetYaw - yaw_;
        while (diff > 3.14159265f) diff -= 6.2831853f;
        while (diff < -3.14159265f) diff += 6.2831853f;
        float maxStep = GameConsts::BOSS_TURN_RATE * dt;
        yaw_ += std::clamp(diff, -maxStep, maxStep);
    }

    {
        float cdt = enraged_ ? dt * GameConsts::RAGE_COOLDOWN_MUL : dt;
        if (hell_) cdt *= GameConsts::HELL_BOSS_COOLDOWN_SPEED;
        if (tailCooldown_ > 0.0f) tailCooldown_ -= cdt;
        if (bubbleNetCooldown_ > 0.0f) bubbleNetCooldown_ -= cdt;
        if (inhaleCooldown_ > 0.0f) inhaleCooldown_ -= cdt;
        if (diveCooldown_ > 0.0f) diveCooldown_ -= cdt;
    }
    UpdateBossHazards(dt, player, effects);
    hugTimer_ = (toPlayer.Length() < GameConsts::BOSS_NEAR_RANGE) ? hugTimer_ + dt : 0.0f;

    actionTimer_ += dt;
    timeSinceLastJumpAttack_ += dt;
    timeSinceLastRingLaunch_ += dt;
    timeSinceLastTsunami_ += dt;

    // Hỗ trợ gọi vòng thứ 2 cách vòng 1 tối thiểu 2.3s (A5)
    if (needSecondRing_ && timeSinceLastRingLaunch_ >= secondRingDelay_)
    {
        SpawnRing(secondRingGapDeg_, player.GetPosition());
        needSecondRing_ = false;
        effects.AddTrauma(0.35f);
        effects.SpawnShockwave(pos_, 120.0f);
    }

    // Máy trạng thái các đòn đánh của Boss
    switch (action_)
    {
    case BossAction::Idle:
    {
        float distToPlayer = toPlayer.Length();

        // AI pacing: hết đợt (và không còn combo chờ) -> vào cửa sổ nghỉ
        if (comboNext_ == BossSkill::None && burstCount_ >= burstSize_ && restTimer_ <= 0.0f)
        {
            restTimer_ = CurrentRestTime() * (hell_ ? GameConsts::HELL_BOSS_REST_MUL : 1.0f);
            burstCount_ = 0;
            burstSize_ = RollBurstSize();
            RecordActionHistory(L"休息");
        }
        if (restTimer_ > 0.0f)
        {
            // Lượn vòng chậm quanh người chơi, giữ cự ly ~REST_PREFERRED_DIST, không tấn công
            restTimer_ -= dt;
            if (distToPlayer > 1.0f)
            {
                Vec3 radial = toPlayer * (1.0f / distToPlayer);
                Vec3 tangent{ radial.z, 0.0f, -radial.x };
                float radialSpd = std::clamp((distToPlayer - GameConsts::REST_PREFERRED_DIST) * 0.5f,
                                             -GameConsts::REST_DRIFT_SPEED, GameConsts::REST_DRIFT_SPEED);
                pos_ = MapObstacles::Move(pos_, Vec3{ (tangent.x * GameConsts::REST_DRIFT_SPEED + radial.x * radialSpd) * dt, 0, (tangent.z * GameConsts::REST_DRIFT_SPEED + radial.z * radialSpd) * dt }, 60.0f);
                float lim = GameConsts::ARENA_HALF_SIZE - 60.0f;
                pos_.x = std::clamp(pos_.x, -lim, lim);
                pos_.z = std::clamp(pos_.z, -lim, lim);
                pos_.y = 0.0f;
                pos_ = MapObstacles::Resolve(pos_, pos_, 60.0f);
            }
            if (restTimer_ <= 0.0f)
            {
                restTimer_ = 0.0f;
                actionTimer_ = 0.0f; // sau nghỉ: vẫn chờ BURST_GAP rồi mới ra chiêu đầu
            }
            break;
        }

        // 3e: Di chuyển chậm về phía người chơi nếu cự ly > 200 để không bị kẹt ở góc xa
        if (distToPlayer > 200.0f)
        {
            Vec3 moveDir = MapObstacles::ChaseDirection(pos_, player.GetPosition(), GetHitboxRadius());
            pos_ = MapObstacles::Move(pos_, Vec3{ moveDir.x * GameConsts::BOSS_WALK_SPEED * dt, 0, moveDir.z * GameConsts::BOSS_WALK_SPEED * dt }, 60.0f);
            pos_.y = 0.0f;
            pos_ = MapObstacles::Resolve(pos_, pos_, 60.0f);

            waterFxTimer_ += dt;
            if (waterFxTimer_ >= 0.22f)
            {
                waterFxTimer_ = 0.0f;
                effects.SpawnFishSwim(pos_, moveDir, GameConsts::BOSS_WALK_SPEED, 3.0f, false);
            }
        }

        // PR D: combo P2 — chiêu nối tiếp đã xếp hàng
        if (comboNext_ != BossSkill::None)
        {
            if (actionTimer_ >= GameConsts::COMBO_DELAY)
            {
                PickNextAction(player, enemies);
                slamTarget_ = player.GetPosition();
                slamTarget_.y = 0.0f;
                slamTarget_ = MapObstacles::Resolve(slamTarget_, slamTarget_, 60.0f);
            }
            break;
        }

        // Boss rework 16: chống bám sát — người chơi đứng sát quá lâu thì ép Quật đuôi
        if (hugTimer_ >= GameConsts::BOSS_HUG_TIME && tailCooldown_ <= 0.0f)
        {
            hugTimer_ = 0.0f;
            NoteSkill(BossSkill::Tail);
            burstCount_++;
            StartTail();
            break;
        }

        if (actionTimer_ >= CurrentBurstGap())
        {
            PickNextAction(player, enemies);
            slamTarget_ = player.GetPosition();
            slamTarget_.y = 0.0f;
            slamTarget_ = MapObstacles::Resolve(slamTarget_, slamTarget_, 60.0f);
        }
        break;
    }

    // 1. Nhảy đập (Slam)
    case BossAction::TelegraphSlam:
    {
        // Breach Slam (rework 8): lặn xuống, bọt sủi ở điểm đáp, điểm đáp bám theo người chơi
        float telegraphDuration = (phase_ == BossPhase::Phase1) ? GameConsts::BREACH_TELEGRAPH_P1 : GameConsts::BREACH_TELEGRAPH_P2;
        UpdateBreachTelegraph(dt, telegraphDuration, player, effects, true);
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
        float jumpDuration = GameConsts::BREACH_AIR_TIME;
        slamProgress_ += dt / jumpDuration;
        // Điểm đáp còn bám theo người chơi tới BREACH_LOCK_TIME trước khi đáp
        if ((1.0f - slamProgress_) * jumpDuration > GameConsts::BREACH_LOCK_TIME)
        {
            slamTarget_ = PredictPlayerPos(player, GameConsts::AIM_BREACH_LEAD); // AI pacing: đón đầu
            slamTarget_ = MapObstacles::Resolve(slamTarget_, slamTarget_, 60.0f);
        }
        if (slamProgress_ >= 1.0f)
        {
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
            timeSinceLastJumpAttack_ = 0.0f; // Đánh dấu đòn nhảy đã kết thúc
            LandBreach(player, effects, GameConsts::BREACH_INNER_DAMAGE, GameConsts::BREACH_OUTER_DAMAGE);
        }
        else
        {
            float t = slamProgress_;
            pos_.y = 4.0f * GameConsts::BREACH_JUMP_HEIGHT * t * (1.0f - t);
            pos_ = MapObstacles::Move(pos_, Vec3{ (slamTarget_.x - pos_.x) * (dt * 5.0f), 0, (slamTarget_.z - pos_.z) * (dt * 5.0f) }, 60.0f);
        }
        break;
    }

    // 2. Gọi quái lính
    case BossAction::TelegraphMinions:
    {
        // PR D: Gọi bầy — bọt sủi ở các điểm bao vây quanh người chơi, rồi cá con trồi lên
        telegraphFxTimer_ += dt;
        while (telegraphFxTimer_ >= 0.08f)
        {
            telegraphFxTimer_ -= 0.08f;
            for (int i = 0; i < packSpotCount_; ++i) effects.SpawnBubbles(packSpots_[i], 2, 14.0f, 2.0f, 4.5f);
        }
        if (actionTimer_ >= GameConsts::PACK_TELEGRAPH)
        {
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
            effects.SpawnShockwave(pos_, 160.0f);
            effects.AddTrauma(0.3f);
            SpawnPack(enemies);
        }
        break;
    }

    // 3. Nhảy đập đôi (Phase 2 - Task 10 T3b)
    case BossAction::TelegraphDoubleSlam:
    {
        UpdateBreachTelegraph(dt, GameConsts::TRIPLE_TELEGRAPH_FIRST, player, effects, false);
        if (actionTimer_ >= GameConsts::TRIPLE_TELEGRAPH_FIRST)
        {
            action_ = BossAction::PerformDoubleSlam;
            actionTimer_ = 0.0f;
            slamProgress_ = 0.0f;
            slamCount_ = 0;
        }
        break;
    }
    case BossAction::PerformDoubleSlam:
    {
        // PR B: Quật ba nhịp (P2) — 3 cú, cú cuối to nhất
        float jumpDur = GameConsts::TRIPLE_AIR_TIME;
        slamProgress_ += dt / jumpDur;
        if (slamProgress_ >= 1.0f)
        {
            slamCount_++;
            float radius = (slamCount_ >= GameConsts::TRIPLE_SLAM_COUNT) ? GameConsts::TRIPLE_FINAL_RADIUS : GameConsts::BREACH_OUTER_RADIUS;
            LandBreach(player, effects, GameConsts::TRIPLE_DAMAGE, GameConsts::TRIPLE_DAMAGE, radius, radius);

            if (slamCount_ < GameConsts::TRIPLE_SLAM_COUNT)
            {
                // Task 10 T3b: Chuyển sang Telegraph cú 2, báo hiệu 0.45s tại vị trí mới
                action_ = BossAction::TelegraphDoubleSlam2;
                actionTimer_ = 0.0f;
                slamTarget_ = player.GetPosition();
                slamTarget_.y = 0.0f;
                slamTarget_ = MapObstacles::Resolve(slamTarget_, slamTarget_, 60.0f);
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
            pos_.y = 4.0f * GameConsts::TRIPLE_JUMP_HEIGHT * t * (1.0f - t);
            pos_ = MapObstacles::Move(pos_, Vec3{ (slamTarget_.x - pos_.x) * (dt * 6.0f), 0, (slamTarget_.z - pos_.z) * (dt * 6.0f) }, 60.0f);
        }
        break;
    }
    case BossAction::TelegraphDoubleSlam2:
    {
        UpdateBreachTelegraph(dt, GameConsts::TRIPLE_TELEGRAPH_NEXT, player, effects, false);
        if (actionTimer_ >= GameConsts::TRIPLE_TELEGRAPH_NEXT)
        {
            action_ = BossAction::PerformDoubleSlam;
            actionTimer_ = 0.0f;
            slamProgress_ = 0.0f;
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
            SoundManager::Instance().PlaySFX(SfxId::TsunamiSurge);

            timeSinceLastJumpAttack_ = 0.0f;
            timeSinceLastRingLaunch_ = 0.0f;
            timeSinceLastTsunami_ = 0.0f;

            effects.AddTrauma(0.45f);

            // Boss quay lại Idle ngay sau khi phóng, không cần chờ sóng đi hết (A2)
            action_ = BossAction::Idle;
            actionTimer_ = 0.0f;
        }
        break;
    }

    default:
        // Boss rework: các trạng thái mới nằm trong BossSkills.cpp
        UpdateExtendedAction(dt, player, enemies, effects);
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
        Vec3 rel = Collision::FlatDiff(player.GetPosition(), ring.center);
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
            // NÉ BẰNG NHẢY CANH NHỊP (PR B): chỉ né được khi đang cao hơn TSUNAMI_DODGE_HEIGHT
            // lúc vành sóng đi qua (nhảy quá sớm, đáp xuống giữa sóng vẫn bị trúng)
            if (player.GetHeightY() >= GameConsts::TSUNAMI_DODGE_HEIGHT)
            {
                if (!ring.dodgedNotice)
                {
                    ring.dodgedNotice = true;
                    // Hiệu ứng hạt xanh lá biểu dương né thành công (A7)
                    effects.PlayFx(FxId::TsunamiDodge, player.GetPosition() + Vec3{ 0, 8, 0 }, Vec3{ 0, 0, 0 });
                }
            }
            else if (!player.IsInvulnerable())
            {
                // Đang ở dưới đất: Bị sóng thần đánh trúng!
                Vec3 outward = (d > 0.001f) ? (rel * (1.0f / d)) : Vec3{ 0, 0, 1 };
                player.TakeDamage(GameConsts::TSUNAMI_DAMAGE, effects, &pos_);
                player.ApplyKnockback(outward * GameConsts::TSUNAMI_KNOCKBACK);
                ring.hitPlayer = true;
                effects.AddTrauma(0.35f);
            }
        }
    }

    // Hiệu ứng nước sóng thần (chỉ hình ảnh)
    tsunamiWater_.Update(dt, action_ == BossAction::TelegraphRadialTsunami, pos_, ToWaveStates(tsunamiRings_));
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
            float dist = Collision::DistXZ(playerPos, r.center);
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
        float dist = Collision::DistXZ(playerPos, pos_);
        return remainingTelegraph + (std::max)(0.0f, dist - GameConsts::TSUNAMI_START_RADIUS) / spd;
    }

    return -1.0f;
}

void WhaleBoss::Draw() const
{
    if (!active_ || hp_ <= 0.0f) return;

    DrawBossHazards(); // PR C: cột bọt của Bubble Net

    // 1. Vẽ Decal báo hiệu trước khi tung đòn
    // Boss rework: bỏ decal đỏ/vàng của đòn đập — thay bằng bóng đổ to dần ở điểm đáp (+ bọt khí trong Update)
    if (action_ == BossAction::TelegraphSlam || action_ == BossAction::TelegraphDoubleSlam || action_ == BossAction::TelegraphDoubleSlam2 ||
        action_ == BossAction::PerformSlam || action_ == BossAction::PerformDoubleSlam)
    {
        float ratio = 1.0f;
        if (action_ == BossAction::TelegraphSlam)
            ratio = std::clamp(actionTimer_ / ((phase_ == BossPhase::Phase1) ? GameConsts::BREACH_TELEGRAPH_P1 : GameConsts::BREACH_TELEGRAPH_P2), 0.1f, 1.0f) * 0.6f;
        else if (action_ == BossAction::TelegraphDoubleSlam || action_ == BossAction::TelegraphDoubleSlam2)
            ratio = 0.6f * std::clamp(actionTimer_ / GameConsts::TRIPLE_TELEGRAPH_NEXT, 0.1f, 1.0f);
        else
            ratio = 0.6f + 0.4f * std::clamp(slamProgress_, 0.0f, 1.0f);
        Render3D::DrawFlatShadow(slamTarget_, GameConsts::BREACH_OUTER_RADIUS * ratio, 0.35f + 0.5f * ratio);
    }
    else if (action_ == BossAction::TelegraphMinions)
    {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 140);
        Render3D::DrawCircle3D(pos_, 140.0f, 32, GetColor(180, 60, 255), false);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 2. Hiệu ứng nước Radial Tsunami: không còn decal cảnh báo, chỉ có nước + hạt
    {
        float dur = (phase_ == BossPhase::Phase1) ? GameConsts::TSUNAMI_TELEGRAPH_P1 : GameConsts::TSUNAMI_TELEGRAPH_P2;
        bool charging = (action_ == BossAction::TelegraphRadialTsunami);
        float chargeRatio = charging ? std::clamp(actionTimer_ / dur, 0.0f, 1.0f) : 0.0f;
        tsunamiWater_.Draw(charging, chargeRatio, pos_, ToWaveStates(tsunamiRings_));
    }

    // 3. Vẽ bóng dẹt của boss
    float shadowRad = 65.0f * (1.0f - std::clamp(pos_.y / 150.0f, 0.0f, 0.5f));
    Render3D::DrawFlatShadow(pos_, shadowRad, 0.95f);

    // Khi nạp sóng thần, thân boss rung nhẹ và phình lên (A2)
    float scaleMul = 1.0f;
    Vec3 shakeOffset{ 0, 0, 0 };
    float pitch = 0.0f;
    if (action_ == BossAction::TelegraphRadialTsunami)
    {
        scaleMul = 1.08f;
        shakeOffset.x = (rand() % 7 - 3) * 0.8f;
        shakeOffset.z = (rand() % 7 - 3) * 0.8f;
    }
    else if (action_ == BossAction::TelegraphTail)
    {
        // Giơ đuôi: chúi đầu xuống, đuôi vểnh lên + rung
        float dur = (phase_ == BossPhase::Phase1) ? GameConsts::TAIL_TELEGRAPH_P1 : GameConsts::TAIL_TELEGRAPH_P2;
        pitch = 0.35f * std::clamp(actionTimer_ / dur, 0.0f, 1.0f);
        shakeOffset.x = (rand() % 5 - 2) * 0.6f;
    }
    else if (action_ == BossAction::PerformTail)
    {
        pitch = -0.2f;
    }
    else if (action_ == BossAction::Stunned)
    {
        shakeOffset.x = std::sin(stunTimer_ * 18.0f) * 3.0f;
        pitch = 0.12f * std::sin(stunTimer_ * 9.0f);
    }
    else if (restTimer_ > 0.0f)
    {
        // AI pacing: đang nghỉ -> đuôi đung đưa chậm, lười biếng
        pitch = 0.07f * std::sin(bossFightTimer_ * 1.3f);
    }

    // 4. Mô hình low-poly cá voi (luôn dùng model; nạp lỗi -> đã báo lỗi lúc nạp, bỏ qua vẽ thân)
    int whaleHandle = LowPolyModels::GetHandle(LowPolyId::WhaleBoss);
    if (whaleHandle >= 0)
    {
        LowPolyModels::ApplyWhalePhaseColors(phase_ == BossPhase::Phase2);
        Vec3 drawPos = pos_ + shakeOffset;
        MV1SetPosition(whaleHandle, VGet(drawPos.x, drawPos.y, drawPos.z));
        MV1SetRotationXYZ(whaleHandle, VGet(pitch, yaw_, 0.0f));
        MV1SetScale(whaleHandle, VGet(scaleMul, scaleMul, scaleMul));
        MV1SetDifColorScale(whaleHandle, (hitFlashTimer_ > 0.0f) ? GetColorF(3.0f, 3.0f, 3.0f, 1.0f) : GetColorF(1.0f, 1.0f, 1.0f, 1.0f));
        HitFlash::DrawModel(whaleHandle, hitFlashTimer_ / GameConsts::HIT_FLASH_TIME);
    }

    if (fx_) fx_->Draw(); // PR E: hạt trong suốt vẽ sau thân boss
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

void WhaleBoss::BeginRenderInterp(float alpha)
{
    RenderInterp::Apply(pos_, yaw_, prevPos_, prevYaw_, alpha, savedPos_, savedYaw_);
}
