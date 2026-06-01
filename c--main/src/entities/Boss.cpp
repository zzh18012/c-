// ============================================================
// Boss.cpp - Boss类实现
// ============================================================
// 职责：Boss的AI攻击模式、状态管理、渲染外观、入场动画
// Boss是一个外星机械生物，有3种类型（Inferno/Void/Thunder），3个阶段
// 每阶段有独特的攻击模式和外观颜色
// 包含20+种攻击模式，每种有独立的执行和计时逻辑
// ============================================================

#include "Boss.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>
#include <cstdint>

// 圆周率常量（用于三角函数计算角度/弧度转换）
static const float PI = 3.14159265358979323846f;

// ============================================================
// 构造函数
// ============================================================
// 初始化Boss的所有状态：位置、血量、攻击模式、动画计时器、子弹池
// 默认是Inferno类型，入场动画激活，HP满值
// ============================================================
Boss::Boss()
    // ---- 位置 ----
    : position(BOSS_POS)                     // 默认位置(640, 180)
    , originalPosition(BOSS_POS)
    // ---- 血量 ----
    , hp(BOSS_MAX_HP)                      // 1000HP
    , maxHP(BOSS_MAX_HP)
    , phase(1)                             // 初始阶段1
    // ---- 攻击状态 ----
    , currentAttack(BossAttackType::None)  // 无攻击
    , currentAttackName("None")
    , attackTimer(0.f)
    , fireTimer(0.f)
    , attackIndex(5)                       // 从第5个开始（使得第一次切换到索引0）
    // ---- 攻击专用计时器 ----
    , spiralAngle(0.f)
    , crossLaserStep(0)
    , crossLaserCount(0)
    , crossLaserTimer(0.f)
    , flowerTimer(0.f)
    , eightDirTimer(0.f)
    , snakeTimer(0.f)
    , homingOrbTimer(0.f)
    , doubleSpiralTimer(0.f)
    , bladeRingTimer(0.f)
    , screenWipeTimer(0.f)
    , radialBurstTimer(0.f)
    , randomSprayTimer(0.f)
    , predictiveTimer(0.f)
    , chargeBeamStep(0)
    , chargeBeamTimer(0.f)
    , tentacleSweepTimer(0.f)
    , screenLockTimer(0.f)
    , bulletRainFullTimer(0.f)
    , phaseTransitionTimer(0.f)
    // ---- 特殊攻击动态数据 ----
    , chargeBeamTarget(0.f, 0.f)
    , tentacleSweepY(0.f)
    , screenLockProgress(0.f)
    // ---- 减速Debuff ----
    , slowStacks(0)
    , slowTimer(0.f)
    // ---- 动画状态 ----
    , pulseTimer(0.f)
    , deformTimer(0.f)
    , eyeTrackAngle(0.f)
    // ---- Boss类型和入场动画 ----
    , bossType(BossType::Inferno)              // 默认类型
    , entranceAnimationProgress(0.f)           // 初始进度0
    , entranceAnimationActive(true)             // 入场动画激活
    , entranceStartPos(BOSS_POS)
    , entranceTargetPos(BOSS_POS)
{
    // 预分配子弹池大小（2000发，对象池避免运行时分配）
    bullets.resize(MAX_BOSS_BULLETS);

    // 初始化第一个攻击（switchToNextAttack使用attackIndex计算）
    switchToNextAttack();
}

// ============================================================
// 更新逻辑（每帧调用）
// ============================================================
// dt：帧间隔时间
// playerPosition：玩家位置（用于瞄准、追踪等攻击计算）
// 更新顺序：阶段检测→攻击模式切换→攻击执行→子弹更新
// ============================================================
void Boss::update(float dt, const sf::Vector2f& playerPosition) {
    // ---- 阶段检测（根据HP百分比切换阶段）----
    int prevPhase = phase;
    if (hp <= maxHP * PHASE3_HP_RATIO)        // HP <= 35% → 阶段3
        phase = 3;
    else if (hp <= maxHP * PHASE2_HP_RATIO)    // HP <= 70% → 阶段2
        phase = 2;
    else                                       // HP > 70% → 阶段1
        phase = 1;

    // ---- 阶段切换时重置攻击状态 ----
    // 阶段变化时：
    // - attackIndex=-1（下次切换从0开始）
    // - 切换到下一个攻击
    // - 重置所有计时器
    if (phase != prevPhase) {
        attackIndex = -1;
        switchToNextAttack();
        attackTimer = 0.f;
        fireTimer = 0.f;
        spiralAngle = 0.f;
        crossLaserStep = 0;
        crossLaserCount = 0;
        crossLaserTimer = 0.f;
        resetNewAttackTimers();
    }

    // ---- 动画计时器累加 ----
    pulseTimer += dt;      // 脉动动画（身体膨胀/收缩循环）
    deformTimer += dt;     // 变形动画（身体微小抖动）

    // ---- 眼睛追踪玩家 ----
    // 计算玩家相对Boss的方向角度，使眼睛朝向玩家
    sf::Vector2f toPlayer = playerPosition - position;
    eyeTrackAngle = std::atan2(toPlayer.y, toPlayer.x);

    // ---- 减速Debuff超时处理 ----
    if (slowTimer > 0.f) {
        slowTimer -= dt;
        if (slowTimer <= 0.f) slowStacks = 0;  // 减速结束，层数归零
    }

    // ---- 更新攻击模式 ----
    updateAttackPattern(dt, playerPosition);

    // ---- 更新所有子弹的位置 ----
    for (auto& bullet : bullets) {
        bullet.update(dt);
    }
}

// ============================================================
// 重置所有新攻击的计时器
// ============================================================
// 在阶段切换或攻击切换时调用，确保新攻击从干净的状态开始
// ============================================================
void Boss::resetNewAttackTimers() {
    // 所有新攻击模式的计时器归零
    flowerTimer = 0.f;
    eightDirTimer = 0.f;
    snakeTimer = 0.f;
    homingOrbTimer = 0.f;
    doubleSpiralTimer = 0.f;
    bladeRingTimer = 0.f;
    screenWipeTimer = 0.f;
    radialBurstTimer = 0.f;
    randomSprayTimer = 0.f;
    predictiveTimer = 0.f;
    chargeBeamStep = 0;
    chargeBeamTimer = 0.f;
    tentacleSweepTimer = 0.f;
    screenLockTimer = 0.f;
    bulletRainFullTimer = 0.f;
    phaseTransitionTimer = 0.f;
    // 减速Debuff也重置
    slowStacks = 0;
    slowTimer = 0.f;
}

// ============================================================
// 更新攻击模式
// ============================================================
// 管理攻击的持续时间、切换逻辑、分发到具体执行函数
// 每种攻击有独立的持续时间（duration），到达后自动切换到下一个攻击
// ============================================================
void Boss::updateAttackPattern(float dt, const sf::Vector2f& playerPos) {
    // ---- 累加攻击时间 ----
    attackTimer += dt;

    // ---- 查询当前攻击的持续时间 ----
    float duration = 0.f;
    switch (currentAttack) {
        case BossAttackType::CircleBurst: duration = CIRCLE_BURST_DURATION; break;
        case BossAttackType::SpiralShot: duration = SPIRAL_DURATION; break;
        case BossAttackType::AimedShot: duration = AIMED_DURATION; break;
        case BossAttackType::WaveBarrage: duration = WAVE_DURATION; break;
        case BossAttackType::RainBullets: duration = RAIN_DURATION; break;
        case BossAttackType::CrossLaser:
            // 十字激光 = 预警时间×重复次数 + 实际伤害时间×重复次数
            duration = (CROSS_LASER_WARNING_TIME + CROSS_LASER_ACTIVE_TIME) * CROSS_LASER_REPEATS;
            break;
        case BossAttackType::FlowerBurst: duration = FLOWER_DURATION; break;
        case BossAttackType::EightDirections: duration = EIGHT_DIR_DURATION; break;
        case BossAttackType::SnakeWave: duration = SNAKE_DURATION; break;
        case BossAttackType::HomingOrbs: duration = HOMING_ORB_DURATION; break;
        case BossAttackType::DoubleSpiral: duration = DOUBLE_SPIRAL_DURATION; break;
        case BossAttackType::BladeRing: duration = BLADE_RING_DURATION; break;
        case BossAttackType::ScreenWipe: duration = SCREEN_WIPE_DURATION; break;
        case BossAttackType::RadialBurst: duration = RADIAL_BURST_DURATION; break;
        case BossAttackType::RandomSpray: duration = RANDOM_SPRAY_DURATION; break;
        case BossAttackType::PredictiveShot: duration = PREDICTIVE_DURATION; break;
        case BossAttackType::ChargeBeam:
            duration = CHARGE_BEAM_WARNING + CHARGE_BEAM_DURATION;
            break;
        case BossAttackType::TentacleSweep: duration = TENTACLE_SWEEP_DURATION; break;
        case BossAttackType::ScreenLock: duration = SCREEN_LOCK_DURATION; break;
        case BossAttackType::BulletRainFull: duration = BULLET_RAIN_FULL_DURATION; break;
        case BossAttackType::PhaseTransition: duration = PHASE_TRANSITION_DURATION; break;
        default: duration = 4.0f; break;
    }

    // ---- 攻击持续时间到达，切换到下一个攻击 ----
    if (attackTimer >= duration) {
        switchToNextAttack();
        attackTimer = 0.f;
        fireTimer = 0.f;
        spiralAngle = 0.f;
        crossLaserStep = 0;
        crossLaserCount = 0;
        crossLaserTimer = 0.f;
        resetNewAttackTimers();
    }

    // ---- 分发到具体攻击执行函数 ----
    switch (currentAttack) {
        case BossAttackType::CircleBurst: executeCircleBurst(dt); break;
        case BossAttackType::SpiralShot: executeSpiralShot(dt); break;
        case BossAttackType::AimedShot: executeAimedShot(dt, playerPos); break;
        case BossAttackType::WaveBarrage: executeWaveBarrage(dt); break;
        case BossAttackType::CrossLaser: executeCrossLaser(dt, playerPos); break;
        case BossAttackType::RainBullets: executeRainBullets(dt); break;
        case BossAttackType::FlowerBurst: executeFlowerBurst(dt); break;
        case BossAttackType::EightDirections: executeEightDirections(dt); break;
        case BossAttackType::SnakeWave: executeSnakeWave(dt); break;
        case BossAttackType::HomingOrbs: executeHomingOrbs(dt, playerPos); break;
        case BossAttackType::DoubleSpiral: executeDoubleSpiral(dt); break;
        case BossAttackType::BladeRing: executeBladeRing(dt); break;
        case BossAttackType::ScreenWipe: executeScreenWipe(dt); break;
        case BossAttackType::RadialBurst: executeRadialBurst(dt); break;
        case BossAttackType::RandomSpray: executeRandomSpray(dt); break;
        case BossAttackType::PredictiveShot: executePredictiveShot(dt, playerPos); break;
        case BossAttackType::ChargeBeam: executeChargeBeam(dt, playerPos); break;
        case BossAttackType::TentacleSweep: executeTentacleSweep(dt, playerPos); break;
        case BossAttackType::ScreenLock: executeScreenLock(dt); break;
        case BossAttackType::BulletRainFull: executeBulletRainFull(dt); break;
        case BossAttackType::PhaseTransition: executePhaseTransition(dt); break;
        default: break;
    }
}

// ============================================================
// 切换到下一个攻击
// ============================================================
// 根据当前阶段选择对应的攻击序列（phase1有5种，phase2有9种，phase3有21种）
// 攻击按循环顺序切换：0→1→2→...→n→0→...
// ============================================================
void Boss::switchToNextAttack() {
    // ---- 阶段1的攻击序列（5种，简单）----
    static const BossAttackType seqPhase1[] = {
        BossAttackType::CircleBurst,   // 圆形爆发
        BossAttackType::AimedShot,     // 瞄准射击
        BossAttackType::SpiralShot,    // 螺旋射击
        BossAttackType::FlowerBurst,   // 花朵爆发
        BossAttackType::EightDirections,  // 八方向
    };
    static const char* namesPhase1[] = {
        "Circle Burst", "Aimed Shot", "Spiral Shot", "Flower Burst", "8 Directions",
    };

    // ---- 阶段2的攻击序列（9种，中等）----
    static const BossAttackType seqPhase2[] = {
        BossAttackType::CircleBurst,
        BossAttackType::AimedShot,
        BossAttackType::WaveBarrage,     // 新增：波浪弹幕
        BossAttackType::CrossLaser,      // 新增：十字激光
        BossAttackType::SpiralShot,
        BossAttackType::DoubleSpiral,     // 新增：双螺旋
        BossAttackType::BladeRing,        // 新增：刀刃之环
        BossAttackType::PredictiveShot,    // 新增：预测射击
        BossAttackType::ChargeBeam,       // 新增：蓄力光束
    };
    static const char* namesPhase2[] = {
        "Circle Burst", "Aimed Shot", "Wave Barrage", "Cross Laser",
        "Spiral Shot", "Double Spiral", "Blade Ring", "Predictive Shot", "Charge Beam",
    };

    // ---- 阶段3的攻击序列（21种，复杂多样）----
    static const BossAttackType seqPhase3[] = {
        BossAttackType::CircleBurst,
        BossAttackType::SpiralShot,
        BossAttackType::AimedShot,
        BossAttackType::WaveBarrage,
        BossAttackType::CrossLaser,
        BossAttackType::RainBullets,      // 子弹雨
        BossAttackType::FlowerBurst,
        BossAttackType::EightDirections,
        BossAttackType::SnakeWave,        // 蛇形波浪
        BossAttackType::HomingOrbs,       // 追踪球
        BossAttackType::DoubleSpiral,
        BossAttackType::BladeRing,
        BossAttackType::ScreenWipe,        // 屏幕横扫
        BossAttackType::RadialBurst,       // 径向爆发
        BossAttackType::RandomSpray,       // 随机喷射
        BossAttackType::PredictiveShot,
        BossAttackType::ChargeBeam,
        BossAttackType::TentacleSweep,     // 触须横扫
        BossAttackType::ScreenLock,        // 屏幕锁定
        BossAttackType::BulletRainFull,    // 全屏弹幕雨
        BossAttackType::PhaseTransition,   // 阶段转换
    };
    static const char* namesPhase3[] = {
        "Circle Burst", "Spiral Shot", "Aimed Shot", "Wave Barrage",
        "Cross Laser", "Rain Bullets", "Flower Burst", "8 Directions",
        "Snake Wave", "Homing Orbs", "Double Spiral", "Blade Ring",
        "Screen Wipe", "Radial Burst", "Random Spray", "Predictive Shot",
        "Charge Beam", "Tentacle Sweep", "Screen Lock", "Bullet Rain Full",
        "Phase Transition",
    };

    // ---- 根据当前阶段选择序列 ----
    const BossAttackType* seq;
    const char* const* names;
    int seqLen;

    if (phase == 1) {
        seq = seqPhase1; names = namesPhase1; seqLen = 5;
    } else if (phase == 2) {
        seq = seqPhase2; names = namesPhase2; seqLen = 9;
    } else {
        seq = seqPhase3; names = namesPhase3; seqLen = 21;
    }

    // ---- 循环切换到下一个攻击 ----
    attackIndex = (attackIndex + 1) % seqLen;
    currentAttack = seq[attackIndex];
    currentAttackName = names[attackIndex];
}

// ============================================================
// 攻击执行：圆形爆发（CircleBurst）
// ============================================================
// 每0.6秒向360度均匀发射一圈子弹（共24发）
// 阶段2/3时增加子弹数量和速度
// ============================================================
void Boss::executeCircleBurst(float dt) {
    fireTimer += dt;
    if (fireTimer >= CIRCLE_BURST_INTERVAL) {  // 间隔0.6秒
        fireTimer -= CIRCLE_BURST_INTERVAL;

        int count = CIRCLE_BURST_COUNT;  // 24发
        if (phase >= 2) count = CIRCLE_BURST_COUNT + 8;  // 阶段2/3：32发

        // 均匀分布的角度间隔 = 2π / count
        float angleStep = 2.f * PI / count;
        float speed = BOSS_BULLET_SPEED_SLOW;  // 180
        if (phase >= 2) speed = BOSS_BULLET_SPEED_NORMAL;  // 240

        // 发射一圈子弹
        for (int i = 0; i < count; ++i) {
            float angle = angleStep * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);  // 15伤害
        }
    }
}

// ============================================================
// 攻击执行：螺旋射击（SpiralShot）
// ============================================================
// 持续发射螺旋状旋转的子弹，每帧角度累加形成螺旋效果
// 阶段3时加快频率、增加每次发数、提高速度
// ============================================================
void Boss::executeSpiralShot(float dt) {
    fireTimer += dt;
    float interval = SPIRAL_INTERVAL;  // 0.05秒
    int bulletsPerShot = SPIRAL_BULLETS_PER_SHOT;  // 4发
    float speed = BOSS_BULLET_SPEED_SLOW;  // 180

    if (phase >= 3) {
        // 阶段3：更快（间隔×0.6）、更多子弹（+2）、更高速度
        interval = SPIRAL_INTERVAL * 0.6f;
        bulletsPerShot = SPIRAL_BULLETS_PER_SHOT + 2;
        speed = BOSS_BULLET_SPEED_NORMAL;
    } else if (phase >= 2) {
        // 阶段2：更多子弹
        bulletsPerShot = SPIRAL_BULLETS_PER_SHOT + 1;
    }

    if (fireTimer >= interval) {
        fireTimer -= interval;

        // 均匀分布在圆周上的角度
        float angleStep = 2.f * PI / bulletsPerShot;
        for (int i = 0; i < bulletsPerShot; ++i) {
            float angle = spiralAngle + angleStep * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
        // 每次射击后螺旋角度累加（形成旋转效果）
        spiralAngle += 0.3f;
    }
}

// ============================================================
// 攻击执行：瞄准射击（AimedShot）
// ============================================================
// 朝玩家方向发射3发子弹（-8°/0°/+8°）
// 阶段3时加快频率和提高速度
// ============================================================
void Boss::executeAimedShot(float dt, const sf::Vector2f& playerPos) {
    fireTimer += dt;
    float interval = AIMED_INTERVAL;  // 0.4秒
    if (phase >= 3) interval = AIMED_INTERVAL * 0.6f;  // 阶段3：更快

    if (fireTimer >= interval) {
        fireTimer -= interval;

        // 计算玩家相对Boss的方向角（弧度）
        sf::Vector2f toPlayer = playerPos - position;
        float baseAngle = std::atan2(toPlayer.y, toPlayer.x);
        float baseDeg = baseAngle * 180.f / PI;  // 转成角度

        float speed = BOSS_BULLET_SPEED_NORMAL;  // 240
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;  // 320

        // 发射3发子弹，角度偏移-8°/0°/+8°
        for (int i = -1; i <= 1; ++i) {
            float rad = (baseDeg + AIMED_ANGLE_OFFSET * i) * PI / 180.f;
            sf::Vector2f dir(std::cos(rad), std::sin(rad));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：波浪弹幕（WaveBarrage）
// ============================================================
// 从屏幕左右两侧发出正弦波动的子弹，波浪幅度和频率可配置
// 阶段3时加快频率和提高速度
// ============================================================
void Boss::executeWaveBarrage(float dt) {
    fireTimer += dt;
    float interval = WAVE_INTERVAL;  // 0.15秒
    if (phase >= 3) interval = WAVE_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;  // 240
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;  // 320

        // ---- 左侧发出（向下波动）----
        {
            // Y坐标随时间正弦波动，产生波浪效果
            sf::Vector2f spawnPos(-10.f, position.y + std::sin(attackTimer * WAVE_FREQUENCY * 2.f) * WAVE_AMPLITUDE);
            // 方向：向右+轻微上下波动
            sf::Vector2f dir(1.f, std::sin(attackTimer * WAVE_FREQUENCY * 2.f + 1.f) * 0.5f);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= len;  // 归一化
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }

        // ---- 右侧发出（向上波动，相位差π）----
        {
            sf::Vector2f spawnPos(WINDOW_WIDTH + 10.f, position.y + std::cos(attackTimer * WAVE_FREQUENCY * 2.f) * WAVE_AMPLITUDE);
            sf::Vector2f dir(-1.f, std::cos(attackTimer * WAVE_FREQUENCY * 2.f + 1.f) * 0.5f);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= len;
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：十字激光（CrossLaser）
// ============================================================
// 分三阶段：预警（画红线）→实际伤害（十字弹幕）→重复3次
// 实际伤害阶段每0.03秒发射大量子弹，伤害×2
// ============================================================
void Boss::executeCrossLaser(float dt, const sf::Vector2f& playerPos) {
    crossLaserTimer += dt;

    // ---- Step 0：预警阶段 ----
    // 画红线警告玩家1秒
    if (crossLaserStep == 0) {
        if (crossLaserTimer >= CROSS_LASER_WARNING_TIME) {
            crossLaserStep = 1;        // 进入实际伤害阶段
            crossLaserTimer = 0.f;     // 重置计时器
        }
    }
    // ---- Step 1：实际伤害阶段 ----
    else {
        if (crossLaserTimer >= CROSS_LASER_ACTIVE_TIME) {
            // 这一轮结束，计数+1
            crossLaserCount++;
            if (crossLaserCount >= CROSS_LASER_REPEATS) {
                // 3次完成后，完全结束这个攻击
                crossLaserStep = 2;
                return;
            }
            // 进入下一轮预警
            crossLaserStep = 0;
            crossLaserTimer = 0.f;
            return;
        }

        // 每0.03秒发射一批子弹（高频）
        if (fireTimer >= 0.03f) {
            fireTimer -= 0.03f;

            float speed = BOSS_BULLET_SPEED_FAST * 1.5f;  // 320×1.5=480
            float laserY = playerPos.y;  // 跟踪玩家Y坐标
            float laserX = playerPos.x;  // 跟踪玩家X坐标

            // 从玩家位置的水平线上每隔40像素发射一列向下子弹（竖线）
            for (int y = 0; y < WINDOW_HEIGHT; y += 40) {
                sf::Vector2f bulletPos(laserX, static_cast<float>(y));
                sf::Vector2f dir(0.f, 1.f);  // 向下
                spawnBullet(bulletPos, dir, speed * 0.3f, BOSS_BULLET_DAMAGE * 2);
            }

            // 从玩家位置的垂直线上每隔40像素发射一列向右子弹（横线）
            for (int x = 0; x < WINDOW_WIDTH; x += 40) {
                sf::Vector2f bulletPos(static_cast<float>(x), laserY);
                sf::Vector2f dir(1.f, 0.f);  // 向右
                spawnBullet(bulletPos, dir, speed * 0.3f, BOSS_BULLET_DAMAGE * 2);
            }
        }
    }
}

// ============================================================
// 攻击执行：子弹雨（RainBullets）
// ============================================================
// 从屏幕顶部随机位置落下的子弹
// 阶段3时加快频率和提高速度
// ============================================================
void Boss::executeRainBullets(float dt) {
    fireTimer += dt;
    float interval = RAIN_INTERVAL;  // 0.08秒
    if (phase >= 3) interval = RAIN_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        // 随机2~3发子弹从顶部落下
        int count = 2 + (std::rand() % 2);
        for (int i = 0; i < count; ++i) {
            float x = 20.f + static_cast<float>(std::rand() % (WINDOW_WIDTH - 40));
            sf::Vector2f spawnPos(x, -10.f);  // 屏幕上方
            sf::Vector2f dir(0.f, 1.f);  // 向下
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：花朵爆发（FlowerBurst）
// ============================================================
// 多层环形子弹同时向外扩散，呈花朵形状
// 3层×24发=72发，每层速度递增10%
// ============================================================
void Boss::executeFlowerBurst(float dt) {
    fireTimer += dt;
    float interval = FLOWER_INTERVAL;  // 0.8秒
    if (phase >= 3) interval = FLOWER_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int count = FLOWER_BURST_COUNT;  // 24发/层
        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        // 3层花瓣，每层角度偏移不同，形成花朵形状
        for (int layer = 0; layer < FLOWER_LAYERS; ++layer) {
            // 每层的起始角度偏移（均匀分布）
            float layerAngle = layer * (PI * 2.f / FLOWER_LAYERS);
            for (int i = 0; i < count; ++i) {
                float angle = layerAngle + (2.f * PI / count) * i;
                sf::Vector2f dir(std::cos(angle), std::sin(angle));
                // 每层速度递增10%
                spawnBullet(position, dir, speed * (1.f + layer * 0.2f), BOSS_BULLET_DAMAGE);
            }
        }
    }
}

// ============================================================
// 攻击执行：八方向射击（EightDirections）
// ============================================================
// 同时向8个方向发射子弹，覆盖面广
// 阶段3时加快频率和提高速度
// ============================================================
void Boss::executeEightDirections(float dt) {
    fireTimer += dt;
    float interval = EIGHT_DIR_INTERVAL;  // 0.4秒
    if (phase >= 3) interval = EIGHT_DIR_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        // 8个方向：0°/45°/90°/135°/180°/225°/270°/315°
        for (int i = 0; i < 8; ++i) {
            float angle = (PI * 2.f / 8.f) * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：蛇形波浪（SnakeWave）
// ============================================================
// 子弹从Boss位置向右发出，同时上下摆动，形成蛇形路径
// 5发子弹同时存在，各自相位不同，产生波浪效果
// ============================================================
void Boss::executeSnakeWave(float dt) {
    fireTimer += dt;
    float interval = SNAKE_INTERVAL;  // 0.1秒
    if (phase >= 3) interval = SNAKE_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = SNAKE_SPEED;  // 200
        // 根据时间计算基础Y坐标（上下波动）
        float waveY = std::sin(attackTimer * SNAKE_FREQUENCY * 2.f) * SNAKE_AMPLITUDE;
        float baseY = WINDOW_HEIGHT * 0.3f + waveY;

        // 5发子弹同时发射，相位差产生波浪效果
        for (int i = 0; i < 5; ++i) {
            float offsetY = std::sin(attackTimer * SNAKE_FREQUENCY * 2.f + i * 0.5f) * 30.f;
            sf::Vector2f spawnPos(position.x + i * 30.f, baseY + offsetY);
            sf::Vector2f dir(1.f, 0.f);  // 向右
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：追踪球（HomingOrbs）
// ============================================================
// 发射缓慢但持续追踪玩家的球状弹幕
// 阶段3时加快频率
// ============================================================
void Boss::executeHomingOrbs(float dt, const sf::Vector2f& playerPos) {
    fireTimer += dt;
    float interval = HOMING_ORB_INTERVAL;  // 0.8秒
    if (phase >= 3) interval = HOMING_ORB_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        // 计算朝向玩家的角度
        sf::Vector2f toPlayer = playerPos - position;
        float angle = std::atan2(toPlayer.y, toPlayer.x);
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnBullet(position, dir, HOMING_ORB_SPEED, BOSS_BULLET_DAMAGE);  // 速度120，慢
    }
}

// ============================================================
// 攻击执行：双螺旋（DoubleSpiral）
// ============================================================
// 两股同时旋转的螺旋弹，相位差π（正好错开）
// 阶段3时加快频率和速度
// ============================================================
void Boss::executeDoubleSpiral(float dt) {
    fireTimer += dt;
    float interval = DOUBLE_SPIRAL_INTERVAL;  // 0.05秒
    if (phase >= 3) interval = DOUBLE_SPIRAL_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int bulletsPerSpiral = SPIRAL_BULLETS_PER_SHOT + 2;  // 6发/股
        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        float angleStep = 2.f * PI / bulletsPerSpiral;
        for (int i = 0; i < bulletsPerSpiral; ++i) {
            // 两股螺旋：角度相差π
            float angle1 = spiralAngle + angleStep * i;
            float angle2 = spiralAngle + angleStep * i + PI;
            sf::Vector2f dir1(std::cos(angle1), std::sin(angle1));
            sf::Vector2f dir2(std::cos(angle2), std::sin(angle2));
            spawnBullet(position, dir1, speed, BOSS_BULLET_DAMAGE);
            spawnBullet(position, dir2, speed, BOSS_BULLET_DAMAGE);
        }
        // 角度累加形成旋转效果
        spiralAngle += 0.25f;
    }
}

// ============================================================
// 攻击执行：刀刃之环（BladeRing）
// ============================================================
// 12发环形刀刃子弹，缓慢旋转，覆盖面广
// 阶段3时加快旋转和速度
// ============================================================
void Boss::executeBladeRing(float dt) {
    fireTimer += dt;
    float interval = BLADE_RING_INTERVAL;  // 0.15秒
    if (phase >= 3) interval = BLADE_RING_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int count = 12;  // 12发均布
        float speed = BLADE_RING_SPEED;  // 250
        if (phase >= 3) speed = BLADE_RING_SPEED * 1.2f;  // 300

        for (int i = 0; i < count; ++i) {
            // 角度缓慢旋转（每帧+0.5弧度）
            float angle = (PI * 2.f / count) * i + bladeRingTimer * 0.5f;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE * 2);  // 伤害×2
        }
        bladeRingTimer += 0.2f;  // 旋转累加
    }
}

// ============================================================
// 攻击执行：屏幕横扫（ScreenWipe）
// ============================================================
// 从屏幕左侧发出子弹，横扫整个屏幕
// 阶段3时加快频率和速度
// ============================================================
void Boss::executeScreenWipe(float dt) {
    fireTimer += dt;
    float interval = SCREEN_WIPE_INTERVAL;  // 0.08秒
    if (phase >= 3) interval = SCREEN_WIPE_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = SCREEN_WIPE_SPEED;  // 400
        // 在随机Y坐标处生成子弹
        float y = static_cast<float>(std::rand() % WINDOW_HEIGHT);
        sf::Vector2f spawnPos(-10.f, y);
        sf::Vector2f dir(1.f, 0.f);  // 向右
        spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
    }
}

// ============================================================
// 攻击执行：径向爆发（RadialBurst）
// ============================================================
// 16发子弹同时向周围均匀爆发
// 阶段3时加快频率和提高速度
// ============================================================
void Boss::executeRadialBurst(float dt) {
    fireTimer += dt;
    float interval = RADIAL_BURST_INTERVAL;  // 0.3秒
    if (phase >= 3) interval = RADIAL_BURST_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int count = 16;  // 16发均布
        float speed = BOSS_BULLET_SPEED_FAST;  // 320
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST * 1.3f;  // 416

        for (int i = 0; i < count; ++i) {
            float angle = (PI * 2.f / count) * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：随机喷射（RandomSpray）
// ============================================================
// 随机位置+随机方向+高频发射，完全不可预测
// 阶段3时极高频率（间隔×0.5）
// ============================================================
void Boss::executeRandomSpray(float dt) {
    fireTimer += dt;
    float interval = RANDOM_SPRAY_INTERVAL;  // 0.05秒
    if (phase >= 3) interval = RANDOM_SPRAY_INTERVAL * 0.5f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_FAST;
        // 随机位置（整个屏幕内）
        sf::Vector2f spawnPos(static_cast<float>(std::rand() % WINDOW_WIDTH),
                             static_cast<float>(std::rand() % WINDOW_HEIGHT));
        // 随机方向（0°~360°）
        float angle = static_cast<float>(std::rand() % 360) * PI / 180.f;
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
    }
}

// ============================================================
// 攻击执行：预测射击（PredictiveShot）
// ============================================================
// 预测玩家移动方向，提前朝预测位置发射
// 伤害×3，高精准度
// ============================================================
void Boss::executePredictiveShot(float dt, const sf::Vector2f& playerPos) {
    fireTimer += dt;
    float interval = PREDICTIVE_INTERVAL;  // 1.5秒
    if (phase >= 3) interval = PREDICTIVE_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        // 预测玩家位置：假设玩家会继续向上移动一段距离
        sf::Vector2f predictedPos = playerPos;
        predictedPos.y -= 50.f;  // 预测向上偏移50像素

        sf::Vector2f toTarget = predictedPos - position;
        float angle = std::atan2(toTarget.y, toTarget.x);
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnBullet(position, dir, PREDICTIVE_SHOT_SPEED, BOSS_BULLET_DAMAGE * 3);  // 伤害×3
    }
}

// ============================================================
// 攻击执行：蓄力光束（ChargeBeam）
// ============================================================
// 分两阶段：预警（1.5秒画红线）+ 实际发射（1秒内朝玩家位置发射大量子弹）
// 发射时5发扩散弹幕（-2~+2×0.05弧度），伤害50
// ============================================================
void Boss::executeChargeBeam(float dt, const sf::Vector2f& playerPos) {
    // ---- Step 0：预警阶段 ----
    if (chargeBeamStep == 0) {
        chargeBeamTimer += dt;
        chargeBeamTarget = playerPos;  // 实时追踪玩家位置
        if (chargeBeamTimer >= CHARGE_BEAM_WARNING) {
            chargeBeamStep = 1;       // 进入实际伤害阶段
            chargeBeamTimer = 0.f;
        }
    }
    // ---- Step 1：实际伤害阶段 ----
    else if (chargeBeamStep == 1) {
        chargeBeamTimer += dt;

        // 高频发射5发扩散弹幕
        if (fireTimer >= 0.02f) {
            fireTimer -= 0.02f;
            sf::Vector2f toTarget = chargeBeamTarget - position;
            float angle = std::atan2(toTarget.y, toTarget.x);
            for (int i = -2; i <= 2; ++i) {
                float spreadAngle = angle + i * 0.05f;  // 每发间隔0.05弧度（约3°）
                sf::Vector2f dir(std::cos(spreadAngle), std::sin(spreadAngle));
                spawnBullet(position, dir, PREDICTIVE_SHOT_SPEED * 1.5f, CHARGE_BEAM_DAMAGE);  // 伤害50
            }
        }

        // 1秒后结束
        if (chargeBeamTimer >= CHARGE_BEAM_DURATION) {
            chargeBeamStep = 0;
            chargeBeamTimer = 0.f;
        }
    }
}

// ============================================================
// 攻击执行：触须横扫（TentacleSweep）
// ============================================================
// 从Boss位置向右发出一个大型触须，横扫屏幕
// 触须Y坐标跟随玩家，每0.1秒发射一颗子弹
// ============================================================
void Boss::executeTentacleSweep(float dt, const sf::Vector2f& playerPos) {
    tentacleSweepTimer += dt;
    tentacleSweepY = playerPos.y;  // 触须跟随玩家Y坐标

    if (fireTimer >= 0.1f) {
        fireTimer -= 0.1f;

        // 触须顶端位置（Boss右侧）
        float tipX = position.x + BOSS_RADIUS * 0.8f;
        sf::Vector2f tipPos(tipX, tentacleSweepY);
        sf::Vector2f dir(-1.f, 0.f);  // 向左（横扫方向）
        spawnBullet(tipPos, dir, BOSS_BULLET_SPEED_FAST, BOSS_BULLET_DAMAGE * 2);  // 伤害×2
    }
}

// ============================================================
// 攻击执行：屏幕锁定（ScreenLock）
// ============================================================
// 画面两侧生成墙体，逐渐向内收缩，限制玩家移动空间
// 用于配合其他攻击增加难度
// ============================================================
void Boss::executeScreenLock(float dt) {
    screenLockTimer += dt;
    // 计算进度（0~1），用于HUD和渲染
    screenLockProgress = screenLockTimer / SCREEN_LOCK_DURATION;
    // 实际子弹逻辑由renderScreenLock处理（墙面视觉效果）
    // 这里只更新时间，不发射子弹
}

// ============================================================
// 攻击执行：全屏弹幕雨（BulletRainFull）
// ============================================================
// 大量子弹从顶部落下，覆盖整个屏幕
// 阶段3时极高频率（间隔×0.5）
// ============================================================
void Boss::executeBulletRainFull(float dt) {
    fireTimer += dt;
    float interval = BULLET_RAIN_FULL_INTERVAL;  // 0.03秒
    if (phase >= 3) interval = BULLET_RAIN_FULL_INTERVAL * 0.5f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_FAST;
        // 阶段越高，子弹越多
        int count = 3 + phase;
        for (int i = 0; i < count; ++i) {
            float x = static_cast<float>(std::rand() % WINDOW_WIDTH);
            sf::Vector2f spawnPos(x, -10.f);  // 屏幕上方
            sf::Vector2f dir(0.f, 1.f);  // 向下
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ============================================================
// 攻击执行：阶段转换（PhaseTransition）
// ============================================================
// Boss无敌+回血25%+瞬移（3个阶段各0.67秒）
// 阶段1：左右抖动；阶段2：瞬移到中心+回血；阶段3：稳定在新位置
// ============================================================
void Boss::executePhaseTransition(float dt) {
    phaseTransitionTimer += dt;
    // 总时长 = 2秒，分成3个阶段，每阶段约0.67秒
    float teleportInterval = PHASE_TRANSITION_DURATION / 3.f;
    float phaseStage = phaseTransitionTimer / teleportInterval;

    if (phaseStage > 0.f && phaseStage < 1.f) {
        // ---- 阶段1：抖动 ----
        // 左右快速抖动，形成瞬移前的抖动效果
        position.x = BOSS_POS.x + std::sin(phaseTransitionTimer * 10.f) * 50.f;
    } else if (phaseStage >= 1.f && phaseStage < 2.f) {
        // ---- 阶段2：瞬移+回血 ----
        position = originalPosition;  // 瞬移回中心
        hp += maxHP / 20;  // 回复5%（25%/5秒）
        if (hp > maxHP) hp = maxHP;  // 不超过最大HP
    } else if (phaseStage >= 2.f) {
        // ---- 阶段3：稳定 ----
        position = BOSS_POS;
    }
}

// ============================================================
// 特殊攻击状态查询
// ============================================================
bool Boss::isLaserWarning() const {
    return currentAttack == BossAttackType::CrossLaser && crossLaserStep == 0;
}

bool Boss::isLaserActive() const {
    return currentAttack == BossAttackType::CrossLaser && crossLaserStep == 1;
}

bool Boss::isChargeBeamWarning() const {
    return currentAttack == BossAttackType::ChargeBeam && chargeBeamStep == 0;
}

bool Boss::isChargeBeamActive() const {
    return currentAttack == BossAttackType::ChargeBeam && chargeBeamStep == 1;
}

bool Boss::isScreenLocking() const {
    return currentAttack == BossAttackType::ScreenLock;
}

bool Boss::isPhaseTransitioning() const {
    return currentAttack == BossAttackType::PhaseTransition;
}

sf::Vector2f Boss::getChargeBeamTarget() const { return chargeBeamTarget; }
float Boss::getTentacleSweepY() const { return tentacleSweepY; }
float Boss::getScreenLockProgress() const { return screenLockProgress; }

// ============================================================
// 渲染（每帧调用）
// ============================================================
// window：SFML窗口
// 渲染顺序很重要，后画的覆盖先画的
// ============================================================
void Boss::render(sf::RenderWindow& window) const {
    // ---- Layer 1: Boss所有子弹 ----
    for (const auto& bullet : bullets) {
        bullet.render(window);
    }

    // ---- Layer 2: 特效（环绕粒子等）----
    renderAlienEffects(window);

    // ---- Layer 3: 触须（除非是TentacleSweep攻击）----
    if (currentAttack != BossAttackType::TentacleSweep) {
        renderAlienTentacles(window);
    }

    // ---- Layer 4: 身体 ----
    renderAlienBody(window);

    // ---- Layer 5: 眼睛 ----
    renderAlienEye(window);

    // ---- Layer 6: 特殊攻击视觉效果 ----
    // 蓄力光束预警（红色光环+光线）
    if (isChargeBeamWarning()) {
        renderChargeBeamWarning(window);
    }
    // 触须横扫（大型横向矩形+圆形）
    if (currentAttack == BossAttackType::TentacleSweep) {
        renderTentacleSweep(window);
    }
    // 屏幕锁定（四面墙）
    if (isScreenLocking()) {
        renderScreenLock(window);
    }

    // ---- Layer 7: 入场特效 ----
    renderEntranceEffect(window);

    // ---- Layer 8: 类型专属光环 ----
    switch (bossType) {
        case BossType::Inferno: renderInfernoAura(window); break;
        case BossType::Void: renderVoidAura(window); break;
        case BossType::Thunder: renderThunderAura(window); break;
    }
}

// ============================================================
// 渲染外星身体
// ============================================================
// 4层同心圆逐渐缩小+透明度变化=身体深度感
// 8个圆形凸起均匀分布在身体周围，随时间轻微摆动
// 身体颜色根据Boss类型变化（Inferno橙红/Void紫蓝/Thunder金黄）
// ============================================================
void Boss::renderAlienBody(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;

    // ---- 根据Boss类型选择身体颜色 ----
    sf::Color bodyColor, innerColor;
    if (bossType == BossType::Inferno) {
        bodyColor = sf::Color(200, 60, 20);
        innerColor = sf::Color(255, 100, 40);
    } else if (bossType == BossType::Void) {
        bodyColor = sf::Color(100, 40, 180);
        innerColor = sf::Color(160, 100, 255);
    } else { // Thunder
        bodyColor = sf::Color(180, 160, 40);
        innerColor = sf::Color(240, 220, 80);
    }

    // ---- 身体脉动变形 ----
    // 正弦波动模拟呼吸/心跳效果
    float deformScale = 1.f + std::sin(deformTimer * 2.f) * 0.03f;
    // 阶段3时额外的高频抖动
    if (phase == 3) {
        deformScale += std::sin(deformTimer * 4.f) * 0.05f;
    }

    float baseRadius = BOSS_RADIUS * deformScale;  // 基础半径×变形系数

    // ---- 绘制4层同心圆（从外到内）----
    for (int layer = 0; layer < 4; layer++) {
        float layerScale = 1.f - layer * 0.15f;      // 每层缩小15%
        float layerAlpha = 0.4f + layer * 0.2f;     // 每层透明度递增
        sf::CircleShape bodyLayer(baseRadius * layerScale);
        bodyLayer.setOrigin(sf::Vector2f(baseRadius * layerScale, baseRadius * layerScale));
        bodyLayer.setPosition(pos);

        // 轻微的随机偏移，产生有机生物的感觉
        float offsetX = std::sin(deformTimer * 1.5f + layer) * 4.f;
        float offsetY = std::cos(deformTimer * 1.2f + layer * 0.7f) * 4.f;
        bodyLayer.setPosition(sf::Vector2f(pos.x + offsetX, pos.y + offsetY));

        // 设置颜色（透明度=255×alpha）
        sf::Color layerColor = bodyColor;
        layerColor.a = static_cast<std::uint8_t>(255.f * layerAlpha);
        bodyLayer.setFillColor(layerColor);
        window.draw(bodyLayer);
    }

    // ---- 内层亮色核心 ----
    sf::CircleShape innerBody(baseRadius * 0.7f);
    innerBody.setOrigin(sf::Vector2f(baseRadius * 0.7f, baseRadius * 0.7f));
    innerBody.setPosition(sf::Vector2f(pos.x + 2.f, pos.y - 2.f));
    innerBody.setFillColor(innerColor);
    window.draw(innerBody);

    // ---- 8个凸起均匀分布在身体周围 ----
    for (int i = 0; i < 8; i++) {
        // 计算凸起位置（角度+时间旋转）
        float angle = (PI * 2.f / 8.f) * i + deformTimer * 0.3f;
        float dist = baseRadius * 0.5f;  // 在半径一半处
        float bumpX = pos.x + std::cos(angle) * dist;
        float bumpY = pos.y + std::sin(angle) * dist;
        // 凸起大小随时间脉动
        sf::CircleShape bump(6.f + std::sin(deformTimer * 2.f + i) * 2.f);
        bump.setOrigin(sf::Vector2f(6.f, 6.f));
        bump.setPosition(sf::Vector2f(bumpX, bumpY));
        // 凸起颜色比身体亮一些
        bump.setFillColor(sf::Color(bodyColor.r + 30, bodyColor.g + 10, bodyColor.b + 20, 200));
        window.draw(bump);
    }
}

// ============================================================
// 渲染外星眼睛
// ============================================================
// 包含：眼眶（黑色圆形）+ 眼珠（根据类型着色）+ 虹膜（跟随玩家）+ 瞳孔 + 高光 + 眼睑（开合动画）
// 眼睛始终朝向玩家（虹膜/瞳孔位置根据eyeTrackAngle计算）
// ============================================================
void Boss::renderAlienEye(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseRadius = BOSS_RADIUS;

    // ---- 根据Boss类型选择眼睛颜色 ----
    sf::Color eyeballColor, irisColor;
    if (bossType == BossType::Inferno) {
        eyeballColor = sf::Color(255, 200, 100);
        irisColor = sf::Color(255, 80, 20);
    } else if (bossType == BossType::Void) {
        eyeballColor = sf::Color(200, 150, 255);
        irisColor = sf::Color(100, 40, 200);
    } else { // Thunder
        eyeballColor = sf::Color(255, 255, 200);
        irisColor = sf::Color(255, 220, 50);
    }

    // ---- 眼眶（黑色圆形背景）----
    sf::CircleShape socket(baseRadius * 0.55f);
    socket.setOrigin(sf::Vector2f(baseRadius * 0.55f, baseRadius * 0.55f));
    socket.setPosition(sf::Vector2f(pos.x, pos.y - 5.f));
    socket.setFillColor(sf::Color(20, 5, 10));
    window.draw(socket);

    // ---- 眼珠（着色圆形）----
    sf::CircleShape eyeball(baseRadius * 0.42f);
    eyeball.setOrigin(sf::Vector2f(baseRadius * 0.42f, baseRadius * 0.42f));
    eyeball.setPosition(sf::Vector2f(pos.x, pos.y - 5.f));
    eyeball.setFillColor(eyeballColor);
    window.draw(eyeball);

    // ---- 虹膜（跟随玩家方向偏移）----
    float irisDist = baseRadius * 0.2f;  // 偏移距离
    // 虹膜位置 = 眼眶中心 + 朝向方向 × 偏移
    float irisX = pos.x + std::cos(eyeTrackAngle) * irisDist;
    float irisY = pos.y - 5.f + std::sin(eyeTrackAngle) * irisDist;

    sf::CircleShape iris(baseRadius * 0.25f);
    iris.setOrigin(sf::Vector2f(baseRadius * 0.25f, baseRadius * 0.25f));
    iris.setPosition(sf::Vector2f(irisX, irisY));
    iris.setFillColor(irisColor);
    window.draw(iris);

    // ---- 瞳孔（黑色小圆）----
    sf::CircleShape pupil(baseRadius * 0.12f);
    pupil.setOrigin(sf::Vector2f(baseRadius * 0.12f, baseRadius * 0.12f));
    pupil.setPosition(sf::Vector2f(irisX, irisY));
    pupil.setFillColor(sf::Color(10, 0, 5));
    window.draw(pupil);

    // ---- 高光（白色小圆，制造反光效果）----
    sf::CircleShape highlight(baseRadius * 0.08f);
    highlight.setOrigin(sf::Vector2f(baseRadius * 0.08f, baseRadius * 0.08f));
    // 高光位置在瞳孔左上方
    highlight.setPosition(sf::Vector2f(irisX - 6.f, irisY - 6.f));
    highlight.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(highlight);

    // ---- 眼睑（开合动画）----
    // 计算眼睑闭合程度（正弦波动，0=全开，1=全闭）
    float lidClose = std::sin(pulseTimer * 0.5f) * 0.5f + 0.5f;
    float lidAlpha = static_cast<std::uint8_t>(100.f * (1.f - lidClose));

    // 上眼睑（4边形）
    sf::ConvexShape topLid;
    topLid.setPointCount(4);
    topLid.setPoint(0, sf::Vector2f(-baseRadius * 0.6f, pos.y - 5.f));
    topLid.setPoint(1, sf::Vector2f(baseRadius * 0.6f, pos.y - 5.f));
    topLid.setPoint(2, sf::Vector2f(baseRadius * 0.5f, pos.y - 5.f - baseRadius * 0.4f * lidClose));
    topLid.setPoint(3, sf::Vector2f(-baseRadius * 0.5f, pos.y - 5.f - baseRadius * 0.4f * lidClose));
    topLid.setFillColor(sf::Color(100, 20, 40, static_cast<std::uint8_t>(lidAlpha)));
    window.draw(topLid);

    // 下眼睑（4边形，镜像）
    sf::ConvexShape bottomLid;
    bottomLid.setPointCount(4);
    bottomLid.setPoint(0, sf::Vector2f(-baseRadius * 0.6f, pos.y - 5.f));
    bottomLid.setPoint(1, sf::Vector2f(baseRadius * 0.6f, pos.y - 5.f));
    bottomLid.setPoint(2, sf::Vector2f(baseRadius * 0.5f, pos.y - 5.f + baseRadius * 0.4f * lidClose));
    bottomLid.setPoint(3, sf::Vector2f(-baseRadius * 0.5f, pos.y - 5.f + baseRadius * 0.4f * lidClose));
    bottomLid.setFillColor(sf::Color(100, 20, 40, static_cast<std::uint8_t>(lidAlpha)));
    window.draw(bottomLid);
}

// ============================================================
// 渲染触须
// ============================================================
// 6条触须均匀分布在Boss周围，每条由8段组成（分段矩形）
// 每段有轻微的角度偏差（正弦波动），产生自然的摆动效果
// 触须颜色根据Boss类型变化
// ============================================================
void Boss::renderAlienTentacles(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseRadius = BOSS_RADIUS;

    for (int i = 0; i < 6; i++) {
        // 计算触须基础角度（随时间缓慢旋转）
        float baseAngle = (PI * 2.f / 6.f) * i + pulseTimer * 0.2f;
        float length = baseRadius * 1.8f;  // 触须长度
        float segCount = 8.f;              // 8段

        // 触须起点（在身体边缘）
        sf::Vector2f prevPos = pos;
        prevPos.x += std::cos(baseAngle) * baseRadius;
        prevPos.y += std::sin(baseAngle) * baseRadius;

        // ---- 逐段绘制 ----
        for (int seg = 0; seg < static_cast<int>(segCount); seg++) {
            // 每段的角度有轻微偏移（正弦波动）
            float segAngle = baseAngle + std::sin(pulseTimer * 2.f + i + seg * 0.5f) * 0.3f;
            float segLen = length / segCount;  // 每段长度
            float segRadius = 8.f - seg * 0.7f;  // 越靠尖端越细

            // 计算这段触须的终点位置
            sf::Vector2f nextPos = prevPos;
            nextPos.x += std::cos(segAngle) * segLen;
            nextPos.y += std::sin(segAngle) * segLen;

            // 创建矩形段（长度+宽度）
            sf::RectangleShape segment(sf::Vector2f(segLen + 2.f, segRadius * 2.f));
            // 矩形中点位置
            float midX = (prevPos.x + nextPos.x) / 2.f;
            float midY = (prevPos.y + nextPos.y) / 2.f;
            segment.setPosition(sf::Vector2f(midX, midY));
            // 旋转到触须方向
            segment.setRotation(sf::degrees(std::atan2(nextPos.y - prevPos.y, nextPos.x - prevPos.x)));

            // ---- 计算透明度（越靠尖端越透明）----
            int alpha = 200 - seg * 20;
            if (phase == 2) alpha = 220 - seg * 20;
            if (phase == 3) alpha = 240 - seg * 20;

            // ---- 根据Boss类型选择触须颜色 ----
            sf::Color tentacleColor;
            if (bossType == BossType::Inferno) {
                tentacleColor = sf::Color(180, 50, 20, static_cast<std::uint8_t>(alpha));
            } else if (bossType == BossType::Void) {
                tentacleColor = sf::Color(80, 40, 160, static_cast<std::uint8_t>(alpha));
            } else { // Thunder
                tentacleColor = sf::Color(160, 140, 40, static_cast<std::uint8_t>(alpha));
            }

            segment.setFillColor(tentacleColor);
            window.draw(segment);

            // 终点变成下一段起点
            prevPos = nextPos;
        }

        // ---- 触须尖端（小圆）----
        sf::CircleShape tip(6.f);
        tip.setOrigin(sf::Vector2f(6.f, 6.f));
        tip.setPosition(sf::Vector2f(prevPos.x, prevPos.y));
        tip.setFillColor(sf::Color(200, 50, 80, 180));
        window.draw(tip);
    }
}

// ============================================================
// 渲染外星特效
// ============================================================
// 包含：
// - 阶段2/3：4个环绕粒子（能量球）
// - 阶段3：外层光晕 + 激光预警时的脉冲光环
// - 激光预警：红色十字线
// ============================================================
void Boss::renderAlienEffects(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseRadius = BOSS_RADIUS;

    // ---- 阶段2/3：4个环绕粒子 ----
    if (phase >= 2) {
        for (int i = 0; i < 4; i++) {
            // 粒子位置围绕Boss旋转
            float angle = pulseTimer * 1.5f + (PI * 2.f / 4.f) * i;
            float dist = baseRadius * 1.5f + std::sin(pulseTimer * 2.f + i) * 10.f;
            float orbX = pos.x + std::cos(angle) * dist;
            float orbY = pos.y + std::sin(angle) * dist;

            // 粒子大小脉动
            float orbRadius = 8.f + std::sin(pulseTimer * 3.f + i) * 2.f;
            sf::CircleShape orb(orbRadius);
            orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
            orb.setPosition(sf::Vector2f(orbX, orbY));

            // 阶段3时更亮
            sf::Color orbColor(255, 80, 120, 150);
            if (phase == 3) orbColor = sf::Color(255, 100, 150, 180);
            orb.setFillColor(orbColor);
            window.draw(orb);
        }
    }

    // ---- 阶段3：外层光晕 ----
    if (phase == 3) {
        float auraRadius = baseRadius * 1.3f + std::sin(pulseTimer * 3.f) * 10.f;
        sf::CircleShape aura(auraRadius);
        aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
        aura.setPosition(pos);
        aura.setFillColor(sf::Color(200, 30, 80, 40));
        window.draw(aura);

        // ---- 激光预警时的脉冲光环 ----
        if (isLaserWarning()) {
            float warningPulse = std::sin(pulseTimer * 15.f) * 0.5f + 0.5f;
            sf::CircleShape warningAura(baseRadius * 2.f);
            warningAura.setOrigin(sf::Vector2f(baseRadius * 2.f, baseRadius * 2.f));
            warningAura.setPosition(pos);
            warningAura.setFillColor(sf::Color(255, 50, 50, static_cast<std::uint8_t>(warningPulse * 100)));
            window.draw(warningAura);
        }
    }

    // ---- 激光预警时的红色十字线 ----
    if (isLaserWarning()) {
        // 水平红线
        sf::RectangleShape warningLineH(sf::Vector2f(WINDOW_WIDTH, 3.f));
        warningLineH.setPosition(sf::Vector2f(0.f, pos.y - 5.f));
        warningLineH.setFillColor(sf::Color(255, 50, 50, 100));
        window.draw(warningLineH);

        // 垂直红线
        sf::RectangleShape warningLineV(sf::Vector2f(3.f, WINDOW_HEIGHT));
        warningLineV.setPosition(sf::Vector2f(pos.x, 0.f));
        warningLineV.setFillColor(sf::Color(255, 50, 50, 100));
        window.draw(warningLineV);
    }
}

// ============================================================
// 渲染蓄力光束预警
// ============================================================
// 在预警阶段画出红色圆环+竖线，警告玩家即将发射
// 脉冲闪烁效果（透明度正弦波动）
// ============================================================
void Boss::renderChargeBeamWarning(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    // 脉冲透明度（15Hz闪烁）
    float warningPulse = std::sin(pulseTimer * 15.f) * 0.5f + 0.5f;

    // 红色竖线
    sf::RectangleShape chargeLine(sf::Vector2f(3.f, WINDOW_HEIGHT));
    chargeLine.setPosition(pos);
    chargeLine.setFillColor(sf::Color(255, 100, 50, static_cast<std::uint8_t>(warningPulse * 150)));
    window.draw(chargeLine);

    // 红色圆环
    sf::CircleShape chargeAura(BOSS_RADIUS * 1.5f);
    chargeAura.setOrigin(sf::Vector2f(BOSS_RADIUS * 1.5f, BOSS_RADIUS * 1.5f));
    chargeAura.setPosition(pos);
    chargeAura.setFillColor(sf::Color(255, 100, 50, static_cast<std::uint8_t>(warningPulse * 80)));
    window.draw(chargeAura);
}

// ============================================================
// 渲染触须横扫
// ============================================================
// 大型横向矩形（400×30像素）从左向右横扫
// 矩形尾部有一个大圆球（BOSS_RADIUS×0.8），看起来像触须顶端
// ============================================================
void Boss::renderTentacleSweep(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;

    // 触须主体（横向矩形）
    sf::RectangleShape tentacle(sf::Vector2f(400.f, 30.f));
    tentacle.setOrigin(sf::Vector2f(0.f, 15.f));  // 原点在左边缘中间
    tentacle.setPosition(sf::Vector2f(pos.x, tentacleSweepY));  // Y坐标跟随玩家
    tentacle.setFillColor(sf::Color(120, 30, 50, 200));
    window.draw(tentacle);

    // 触须顶端（圆形）
    sf::CircleShape tip(BOSS_RADIUS * 0.8f);
    tip.setOrigin(sf::Vector2f(BOSS_RADIUS * 0.8f, BOSS_RADIUS * 0.8f));
    tip.setPosition(sf::Vector2f(pos.x + 200.f, tentacleSweepY));
    tip.setFillColor(sf::Color(200, 50, 80, 220));
    window.draw(tip);
}

// ============================================================
// 渲染屏幕锁定
// ============================================================
// 4个方向的墙（上下左右），从屏幕边缘向内延伸
// alpha随screenLockProgress变化，从0逐渐变深（屏幕逐渐收缩）
// ============================================================
void Boss::renderScreenLock(sf::RenderWindow& window) const {
    // 计算透明度（progress越大越深）
    float alpha = static_cast<std::uint8_t>(screenLockProgress * 100);

    // 上边墙
    sf::RectangleShape topWall(sf::Vector2f(WINDOW_WIDTH, 50.f));
    topWall.setPosition(sf::Vector2f(0.f, 0.f));
    topWall.setFillColor(sf::Color(100, 50, 150, static_cast<std::uint8_t>(alpha)));
    window.draw(topWall);

    // 下边墙
    sf::RectangleShape bottomWall(sf::Vector2f(WINDOW_WIDTH, 50.f));
    bottomWall.setPosition(sf::Vector2f(0.f, WINDOW_HEIGHT - 50.f));
    bottomWall.setFillColor(sf::Color(100, 50, 150, static_cast<std::uint8_t>(alpha)));
    window.draw(bottomWall);

    // 左边墙
    sf::RectangleShape leftWall(sf::Vector2f(50.f, WINDOW_HEIGHT));
    leftWall.setPosition(sf::Vector2f(0.f, 0.f));
    leftWall.setFillColor(sf::Color(100, 50, 150, static_cast<std::uint8_t>(alpha)));
    window.draw(leftWall);

    // 右边墙
    sf::RectangleShape rightWall(sf::Vector2f(50.f, WINDOW_HEIGHT));
    rightWall.setPosition(sf::Vector2f(WINDOW_WIDTH - 50.f, 0.f));
    rightWall.setFillColor(sf::Color(100, 50, 150, static_cast<std::uint8_t>(alpha)));
    window.draw(rightWall);
}

// ============================================================
// 状态查询（getter）
// ============================================================
sf::Vector2f Boss::getPosition() const { return position; }
float Boss::getRadius() const { return BOSS_RADIUS; }
int Boss::getHP() const { return hp; }
int Boss::getMaxHP() const { return maxHP; }
int Boss::getPhase() const { return phase; }
bool Boss::isDead() const { return hp <= 0; }
std::string Boss::getCurrentAttackName() const { return currentAttackName; }
BossAttackType Boss::getCurrentAttackType() const { return currentAttack; }

// ============================================================
// 子弹数组访问
// ============================================================
std::vector<BossBullet>& Boss::getBullets() { return bullets; }

// ============================================================
// 受到伤害
// ============================================================
// PhaseTransition（阶段转换）时无敌，不扣血
// ============================================================
void Boss::takeDamage(int damage) {
    if (currentAttack == BossAttackType::PhaseTransition) return;  // 无敌
    hp -= damage;
    if (hp < 0) hp = 0;  // clamp到0
}

// ============================================================
// 生成子弹
// ============================================================
// 从对象池中找第一个非活跃的子弹，激活它
// 速度受bulletSpeedMultiplier和slowMultiplier影响
// ============================================================
void Boss::spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg) {
    for (auto& bullet : bullets) {
        if (!bullet.isActive()) {
            // 计算最终速度（含玩家技能影响）
            float slowMult = getSlowMultiplier();
            bullet.spawn(pos, dir, speed * bulletSpeedMultiplier * slowMult, dmg);
            return;  // 只生成一颗
        }
    }
}

void Boss::clearAllBullets() {
    for (auto& bullet : bullets) {
        bullet.deactivate();
    }
}

void Boss::setBulletSpeedMultiplier(float mult) { bulletSpeedMultiplier = mult; }
float Boss::getBulletSpeedMultiplier() const { return bulletSpeedMultiplier; }

// ============================================================
// 减速Debuff
// ============================================================
// 玩家Homing子弹命中时叠加，每层10%减速，上限3层
void Boss::applySlow(int stacks) {
    slowStacks = std::min(slowStacks + stacks, HOMING_SLOW_MAX_STACKS);  // 上限3层
    slowTimer = HOMING_SLOW_DURATION;  // 持续2秒
}

float Boss::getSlowMultiplier() const {
    return 1.f - slowStacks * HOMING_SLOW_STACK;  // 1-层数×0.1
}

// ============================================================
// Boss类型设置
// ============================================================
void Boss::setBossType(BossType type) {
    bossType = type;
}

BossType Boss::getBossType() const {
    return bossType;
}

// ============================================================
// 入场动画控制
// ============================================================
void Boss::setEntranceStartPos(const sf::Vector2f& pos) {
    entranceStartPos = pos;
}

void Boss::setEntranceTargetPos(const sf::Vector2f& pos) {
    entranceTargetPos = pos;
}

// 设置入场进度（0~1），自动计算位置插值
void Boss::setEntranceAnimation(float progress) {
    entranceAnimationProgress = progress;
    if (progress >= 1.f) {
        // 入场完成：Boss固定在终点，无敌状态解除
        entranceAnimationActive = false;
        position = entranceTargetPos;
    } else {
        // 线性插值：start + (target - start) × progress
        position = entranceStartPos + (entranceTargetPos - entranceStartPos) * progress;
    }
}

float Boss::getEntranceAnimation() const {
    return entranceAnimationProgress;
}

bool Boss::isEntranceComplete() const {
    return entranceAnimationProgress >= 1.f;
}

// 设置位置时同时重置入场动画起点/终点（使setPosition用于初始化）
void Boss::setPosition(const sf::Vector2f& pos) {
    position = pos;
    entranceStartPos = pos;
    entranceTargetPos = pos;
}

// ============================================================
// 渲染火焰光环（Inferno类型专属）
// ============================================================
// 8个橙色粒子围绕Boss旋转 + 红色半透明圆形光晕
// ============================================================
void Boss::renderInfernoAura(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    for (int i = 0; i < 8; ++i) {
        // 粒子位置围绕Boss旋转（速度2 rad/s）
        float angle = pulseTimer * 2.f + (PI * 2.f / 8.f) * i;
        float dist = BOSS_RADIUS * 1.8f + std::sin(pulseTimer * 3.f + i) * 15.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;
        // 粒子大小脉动
        float orbRadius = 10.f + std::sin(pulseTimer * 4.f + i) * 3.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(255, 100, 30, 180));
        window.draw(orb);
    }
    // 外层光晕
    float auraRadius = BOSS_RADIUS * 2.f + std::sin(pulseTimer * 2.f) * 10.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(255, 50, 0, 30));
    window.draw(aura);
}

// ============================================================
// 渲染虚空光环（Void类型专属）
// ============================================================
// 12个紫色粒子以更慢的频率环绕 + 更大范围的紫色光晕
// ============================================================
void Boss::renderVoidAura(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    for (int i = 0; i < 12; ++i) {
        // 更慢的旋转（0.8 rad/s），相位错开
        float angle = pulseTimer * 0.8f + (PI * 2.f / 12.f) * i;
        float dist = BOSS_RADIUS * 1.5f + std::sin(pulseTimer * 1.5f + i * 0.3f) * 20.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;
        float orbRadius = 4.f + std::sin(pulseTimer * 2.f + i) * 2.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(150, 80, 255, 200));
        window.draw(orb);
    }
    // 更大范围的光晕
    float auraRadius = BOSS_RADIUS * 2.2f + std::sin(pulseTimer * 1.5f) * 15.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(80, 20, 150, 40));
    window.draw(aura);
}

// ============================================================
// 渲染雷电光环（Thunder类型专属）
// ============================================================
// 6个金黄色粒子 + 每帧额外画闪电装饰 + 金色光晕
// ============================================================
void Boss::renderThunderAura(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    for (int i = 0; i < 6; ++i) {
        // 快速旋转（3 rad/s）
        float angle = pulseTimer * 3.f + (PI * 2.f / 6.f) * i;
        float dist = BOSS_RADIUS * 1.6f + std::sin(pulseTimer * 4.f + i) * 10.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;
        float orbRadius = 8.f + std::sin(pulseTimer * 5.f + i) * 3.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(255, 240, 100, 220));
        window.draw(orb);

        // ---- 闪电装饰（矩形，从粒子发出）----
        sf::RectangleShape lightning(sf::Vector2f(30.f, 3.f));
        lightning.setOrigin(sf::Vector2f(0.f, 1.5f));
        lightning.setPosition(sf::Vector2f(orbX, orbY));
        lightning.setRotation(sf::degrees(angle * 180.f / PI));
        lightning.setFillColor(sf::Color(255, 255, 100, 150));
        window.draw(lightning);
    }
    // 金色光晕
    float auraRadius = BOSS_RADIUS * 2.1f + std::sin(pulseTimer * 2.5f) * 12.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(200, 200, 50, 35));
    window.draw(aura);
}

// ============================================================
// 渲染入场特效
// ============================================================
// 三种Boss类型有各自独特的入场动画：
// - Inferno：火球拖尾（沿路径画淡出的圆）
// - Void：裂缝（两条水平线从两侧合拢）
// - Thunder：扩散圆环（从中心向外扩散的圆）
// 入场完成后不画
// ============================================================
void Boss::renderEntranceEffect(sf::RenderWindow& window) const {
    // 入场未激活或已完成时不画
    if (!entranceAnimationActive || entranceAnimationProgress >= 1.f) return;

    float progress = entranceAnimationProgress;
    sf::Vector2f pos = position;

    switch (bossType) {
        case BossType::Inferno: {
            // ---- 火球拖尾 ----
            // 计算从起点到终点的方向向量
            sf::Vector2f trailDir = entranceTargetPos - entranceStartPos;
            float trailLen = std::sqrt(trailDir.x * trailDir.x + trailDir.y * trailDir.y);
            if (trailLen > 0) trailDir /= trailLen;  // 归一化

            // 在路径上画5个拖尾圆，逐渐淡出
            for (int i = 0; i < 5; ++i) {
                float t = 1.f - progress + i * 0.1f;  // 从后往前
                if (t > 0 && t < 1.f) {
                    sf::Vector2f trailPos = entranceStartPos + trailDir * t * trailLen;
                    float radius = 20.f * (1.f - t) * progress;  // 前面的圆更大
                    sf::CircleShape trail(radius);
                    trail.setOrigin(sf::Vector2f(radius, radius));
                    trail.setPosition(trailPos);
                    trail.setFillColor(sf::Color(255, 100, 30, static_cast<std::uint8_t>(150 * (1.f - t))));
                    window.draw(trail);
                }
            }
            break;
        }
        case BossType::Void: {
            // ---- 裂缝效果 ----
            // 两条水平线从两侧向Boss位置合拢
            float crackAlpha = 200.f * progress;
            // 上方裂缝
            sf::RectangleShape crack(sf::Vector2f(WINDOW_WIDTH, 5.f));
            crack.setPosition(sf::Vector2f(0.f, pos.y));
            crack.setFillColor(sf::Color(180, 80, 255, static_cast<std::uint8_t>(crackAlpha)));
            window.draw(crack);
            // 下方裂缝（淡一些）
            sf::RectangleShape crack2(sf::Vector2f(WINDOW_WIDTH, 5.f));
            crack2.setPosition(sf::Vector2f(0.f, pos.y - 30.f));
            crack2.setFillColor(sf::Color(150, 60, 255, static_cast<std::uint8_t>(crackAlpha * 0.5f)));
            window.draw(crack2);
            break;
        }
        case BossType::Thunder: {
            // ---- 扩散圆环 ----
            float waveRadius = BOSS_RADIUS * (1.f + progress * 3.f);  // 从1倍到4倍半径
            sf::CircleShape wave(waveRadius);
            wave.setOrigin(sf::Vector2f(waveRadius, waveRadius));
            wave.setPosition(pos);
            // 填充透明度逐渐降低
            wave.setFillColor(sf::Color(255, 255, 100, static_cast<std::uint8_t>(100 * (1.f - progress))));
            // 边框透明度更高，更亮
            wave.setOutlineColor(sf::Color(255, 240, 100, static_cast<std::uint8_t>(200 * (1.f - progress))));
            wave.setOutlineThickness(3.f);
            window.draw(wave);
            break;
        }
    }
}