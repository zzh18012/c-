#include "Boss.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>
#include <cstdint>

static const float PI = 3.14159265358979323846f;

Boss::Boss()
    : position(BOSS_POS)
    , originalPosition(BOSS_POS)
    , hp(BOSS_MAX_HP)
    , maxHP(BOSS_MAX_HP)
    , phase(1)
    , currentAttack(BossAttackType::None)
    , currentAttackName("None")
    , attackTimer(0.f)
    , fireTimer(0.f)
    , attackIndex(5)
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
    , chargeBeamTarget(0.f, 0.f)
    , tentacleSweepY(0.f)
    , screenLockProgress(0.f)
    , pulseTimer(0.f)
    , deformTimer(0.f)
    , eyeTrackAngle(0.f)
    , bossType(BossType::Inferno)
    , entranceAnimationProgress(0.f)
    , entranceAnimationActive(true)
    , entranceStartPos(BOSS_POS)
    , entranceTargetPos(BOSS_POS)
{
    bullets.resize(MAX_BOSS_BULLETS);

    switchToNextAttack();
}

void Boss::update(float dt, const sf::Vector2f& playerPosition) {
    int prevPhase = phase;
    if (hp <= maxHP * PHASE3_HP_RATIO)
        phase = 3;
    else if (hp <= maxHP * PHASE2_HP_RATIO)
        phase = 2;
    else
        phase = 1;

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

    pulseTimer += dt;
    deformTimer += dt;

    sf::Vector2f toPlayer = playerPosition - position;
    eyeTrackAngle = std::atan2(toPlayer.y, toPlayer.x);

    if (slowTimer > 0.f) {
        slowTimer -= dt;
        if (slowTimer <= 0.f) slowStacks = 0;
    }

    updateAttackPattern(dt, playerPosition);

    for (auto& bullet : bullets) {
        bullet.update(dt);
    }
}

void Boss::resetNewAttackTimers() {
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
    slowStacks = 0;
    slowTimer = 0.f;
}

void Boss::updateAttackPattern(float dt, const sf::Vector2f& playerPos) {
    attackTimer += dt;

    float duration = 0.f;
    switch (currentAttack) {
        case BossAttackType::CircleBurst: duration = CIRCLE_BURST_DURATION; break;
        case BossAttackType::SpiralShot:  duration = SPIRAL_DURATION; break;
        case BossAttackType::AimedShot:   duration = AIMED_DURATION; break;
        case BossAttackType::WaveBarrage: duration = WAVE_DURATION; break;
        case BossAttackType::RainBullets: duration = RAIN_DURATION; break;
        case BossAttackType::CrossLaser:
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

    switch (currentAttack) {
        case BossAttackType::CircleBurst: executeCircleBurst(dt); break;
        case BossAttackType::SpiralShot:  executeSpiralShot(dt); break;
        case BossAttackType::AimedShot:   executeAimedShot(dt, playerPos); break;
        case BossAttackType::WaveBarrage: executeWaveBarrage(dt); break;
        case BossAttackType::CrossLaser:  executeCrossLaser(dt, playerPos); break;
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

void Boss::switchToNextAttack() {
    static const BossAttackType seqPhase1[] = {
        BossAttackType::CircleBurst,
        BossAttackType::AimedShot,
        BossAttackType::SpiralShot,
        BossAttackType::FlowerBurst,
        BossAttackType::EightDirections,
    };
    static const char* namesPhase1[] = {
        "Circle Burst",
        "Aimed Shot",
        "Spiral Shot",
        "Flower Burst",
        "8 Directions",
    };

    static const BossAttackType seqPhase2[] = {
        BossAttackType::CircleBurst,
        BossAttackType::AimedShot,
        BossAttackType::WaveBarrage,
        BossAttackType::CrossLaser,
        BossAttackType::SpiralShot,
        BossAttackType::DoubleSpiral,
        BossAttackType::BladeRing,
        BossAttackType::PredictiveShot,
        BossAttackType::ChargeBeam,
    };
    static const char* namesPhase2[] = {
        "Circle Burst",
        "Aimed Shot",
        "Wave Barrage",
        "Cross Laser",
        "Spiral Shot",
        "Double Spiral",
        "Blade Ring",
        "Predictive Shot",
        "Charge Beam",
    };

    static const BossAttackType seqPhase3[] = {
        BossAttackType::CircleBurst,
        BossAttackType::SpiralShot,
        BossAttackType::AimedShot,
        BossAttackType::WaveBarrage,
        BossAttackType::CrossLaser,
        BossAttackType::RainBullets,
        BossAttackType::FlowerBurst,
        BossAttackType::EightDirections,
        BossAttackType::SnakeWave,
        BossAttackType::HomingOrbs,
        BossAttackType::DoubleSpiral,
        BossAttackType::BladeRing,
        BossAttackType::ScreenWipe,
        BossAttackType::RadialBurst,
        BossAttackType::RandomSpray,
        BossAttackType::PredictiveShot,
        BossAttackType::ChargeBeam,
        BossAttackType::TentacleSweep,
        BossAttackType::ScreenLock,
        BossAttackType::BulletRainFull,
        BossAttackType::PhaseTransition,
    };
    static const char* namesPhase3[] = {
        "Circle Burst",
        "Spiral Shot",
        "Aimed Shot",
        "Wave Barrage",
        "Cross Laser",
        "Rain Bullets",
        "Flower Burst",
        "8 Directions",
        "Snake Wave",
        "Homing Orbs",
        "Double Spiral",
        "Blade Ring",
        "Screen Wipe",
        "Radial Burst",
        "Random Spray",
        "Predictive Shot",
        "Charge Beam",
        "Tentacle Sweep",
        "Screen Lock",
        "Bullet Rain Full",
        "Phase Transition",
    };

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

    attackIndex = (attackIndex + 1) % seqLen;
    currentAttack = seq[attackIndex];
    currentAttackName = names[attackIndex];
}

void Boss::executeCircleBurst(float dt) {
    fireTimer += dt;
    if (fireTimer >= CIRCLE_BURST_INTERVAL) {
        fireTimer -= CIRCLE_BURST_INTERVAL;
        int count = CIRCLE_BURST_COUNT;
        if (phase >= 2) count = CIRCLE_BURST_COUNT + 8;
        float angleStep = 2.f * PI / count;
        float speed = BOSS_BULLET_SPEED_SLOW;
        if (phase >= 2) speed = BOSS_BULLET_SPEED_NORMAL;
        for (int i = 0; i < count; ++i) {
            float angle = angleStep * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeSpiralShot(float dt) {
    fireTimer += dt;
    float interval = SPIRAL_INTERVAL;
    int bulletsPerShot = SPIRAL_BULLETS_PER_SHOT;
    float speed = BOSS_BULLET_SPEED_SLOW;
    if (phase >= 3) {
        interval = SPIRAL_INTERVAL * 0.6f;
        bulletsPerShot = SPIRAL_BULLETS_PER_SHOT + 2;
        speed = BOSS_BULLET_SPEED_NORMAL;
    } else if (phase >= 2) {
        bulletsPerShot = SPIRAL_BULLETS_PER_SHOT + 1;
    }

    if (fireTimer >= interval) {
        fireTimer -= interval;
        float angleStep = 2.f * PI / bulletsPerShot;
        for (int i = 0; i < bulletsPerShot; ++i) {
            float angle = spiralAngle + angleStep * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
        spiralAngle += 0.3f;
    }
}

void Boss::executeAimedShot(float dt, const sf::Vector2f& playerPos) {
    fireTimer += dt;
    float interval = AIMED_INTERVAL;
    if (phase >= 3) interval = AIMED_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        sf::Vector2f toPlayer = playerPos - position;
        float baseAngle = std::atan2(toPlayer.y, toPlayer.x);
        float baseDeg = baseAngle * 180.f / PI;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        for (int i = -1; i <= 1; ++i) {
            float rad = (baseDeg + AIMED_ANGLE_OFFSET * i) * PI / 180.f;
            sf::Vector2f dir(std::cos(rad), std::sin(rad));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeWaveBarrage(float dt) {
    fireTimer += dt;
    float interval = WAVE_INTERVAL;
    if (phase >= 3) interval = WAVE_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        {
            sf::Vector2f spawnPos(-10.f, position.y + std::sin(attackTimer * WAVE_FREQUENCY * 2.f) * WAVE_AMPLITUDE);
            sf::Vector2f dir(1.f, std::sin(attackTimer * WAVE_FREQUENCY * 2.f + 1.f) * 0.5f);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= len;
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
        {
            sf::Vector2f spawnPos(WINDOW_WIDTH + 10.f, position.y + std::cos(attackTimer * WAVE_FREQUENCY * 2.f) * WAVE_AMPLITUDE);
            sf::Vector2f dir(-1.f, std::cos(attackTimer * WAVE_FREQUENCY * 2.f + 1.f) * 0.5f);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= len;
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeCrossLaser(float dt, const sf::Vector2f& playerPos) {
    crossLaserTimer += dt;

    if (crossLaserStep == 0) {
        if (crossLaserTimer >= CROSS_LASER_WARNING_TIME) {
            crossLaserStep = 1;
            crossLaserTimer = 0.f;
        }
    } else {
        if (crossLaserTimer >= CROSS_LASER_ACTIVE_TIME) {
            crossLaserCount++;
            if (crossLaserCount >= CROSS_LASER_REPEATS) {
                crossLaserStep = 2;
                return;
            }
            crossLaserStep = 0;
            crossLaserTimer = 0.f;
            return;
        }

        if (fireTimer >= 0.03f) {
            fireTimer -= 0.03f;

            float speed = BOSS_BULLET_SPEED_FAST * 1.5f;
            float laserY = playerPos.y;
            float laserX = playerPos.x;

            for (int y = 0; y < WINDOW_HEIGHT; y += 40) {
                sf::Vector2f bulletPos(laserX, static_cast<float>(y));
                sf::Vector2f dir(0.f, 1.f);
                spawnBullet(bulletPos, dir, speed * 0.3f, BOSS_BULLET_DAMAGE * 2);
            }

            for (int x = 0; x < WINDOW_WIDTH; x += 40) {
                sf::Vector2f bulletPos(static_cast<float>(x), laserY);
                sf::Vector2f dir(1.f, 0.f);
                spawnBullet(bulletPos, dir, speed * 0.3f, BOSS_BULLET_DAMAGE * 2);
            }
        }
    }
}

void Boss::executeRainBullets(float dt) {
    fireTimer += dt;
    float interval = RAIN_INTERVAL;
    if (phase >= 3) interval = RAIN_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        int count = 2 + (std::rand() % 2);
        for (int i = 0; i < count; ++i) {
            float x = 20.f + static_cast<float>(std::rand() % (WINDOW_WIDTH - 40));
            sf::Vector2f spawnPos(x, -10.f);
            sf::Vector2f dir(0.f, 1.f);
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeFlowerBurst(float dt) {
    fireTimer += dt;
    float interval = FLOWER_INTERVAL;
    if (phase >= 3) interval = FLOWER_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int count = FLOWER_BURST_COUNT;
        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        for (int layer = 0; layer < FLOWER_LAYERS; ++layer) {
            float layerAngle = layer * (PI * 2.f / FLOWER_LAYERS);
            for (int i = 0; i < count; ++i) {
                float angle = layerAngle + (2.f * PI / count) * i;
                sf::Vector2f dir(std::cos(angle), std::sin(angle));
                spawnBullet(position, dir, speed * (1.f + layer * 0.2f), BOSS_BULLET_DAMAGE);
            }
        }
    }
}

void Boss::executeEightDirections(float dt) {
    fireTimer += dt;
    float interval = EIGHT_DIR_INTERVAL;
    if (phase >= 3) interval = EIGHT_DIR_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        for (int i = 0; i < 8; ++i) {
            float angle = (PI * 2.f / 8.f) * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeSnakeWave(float dt) {
    fireTimer += dt;
    float interval = SNAKE_INTERVAL;
    if (phase >= 3) interval = SNAKE_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = SNAKE_SPEED;
        float waveY = std::sin(attackTimer * SNAKE_FREQUENCY * 2.f) * SNAKE_AMPLITUDE;
        float baseY = WINDOW_HEIGHT * 0.3f + waveY;

        for (int i = 0; i < 5; ++i) {
            float offsetY = std::sin(attackTimer * SNAKE_FREQUENCY * 2.f + i * 0.5f) * 30.f;
            sf::Vector2f spawnPos(position.x + i * 30.f, baseY + offsetY);
            sf::Vector2f dir(1.f, 0.f);
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeHomingOrbs(float dt, const sf::Vector2f& playerPos) {
    fireTimer += dt;
    float interval = HOMING_ORB_INTERVAL;
    if (phase >= 3) interval = HOMING_ORB_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        sf::Vector2f toPlayer = playerPos - position;
        float angle = std::atan2(toPlayer.y, toPlayer.x);
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnBullet(position, dir, HOMING_ORB_SPEED, BOSS_BULLET_DAMAGE);
    }
}

void Boss::executeDoubleSpiral(float dt) {
    fireTimer += dt;
    float interval = DOUBLE_SPIRAL_INTERVAL;
    if (phase >= 3) interval = DOUBLE_SPIRAL_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int bulletsPerSpiral = SPIRAL_BULLETS_PER_SHOT + 2;
        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        float angleStep = 2.f * PI / bulletsPerSpiral;
        for (int i = 0; i < bulletsPerSpiral; ++i) {
            float angle1 = spiralAngle + angleStep * i;
            float angle2 = spiralAngle + angleStep * i + PI;
            sf::Vector2f dir1(std::cos(angle1), std::sin(angle1));
            sf::Vector2f dir2(std::cos(angle2), std::sin(angle2));
            spawnBullet(position, dir1, speed, BOSS_BULLET_DAMAGE);
            spawnBullet(position, dir2, speed, BOSS_BULLET_DAMAGE);
        }
        spiralAngle += 0.25f;
    }
}

void Boss::executeBladeRing(float dt) {
    fireTimer += dt;
    float interval = BLADE_RING_INTERVAL;
    if (phase >= 3) interval = BLADE_RING_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int count = 12;
        float speed = BLADE_RING_SPEED;
        if (phase >= 3) speed = BLADE_RING_SPEED * 1.2f;

        for (int i = 0; i < count; ++i) {
            float angle = (PI * 2.f / count) * i + bladeRingTimer * 0.5f;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE * 2);
        }
        bladeRingTimer += 0.2f;
    }
}

void Boss::executeScreenWipe(float dt) {
    fireTimer += dt;
    float interval = SCREEN_WIPE_INTERVAL;
    if (phase >= 3) interval = SCREEN_WIPE_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = SCREEN_WIPE_SPEED;
        float y = static_cast<float>(std::rand() % WINDOW_HEIGHT);
        sf::Vector2f spawnPos(-10.f, y);
        sf::Vector2f dir(1.f, 0.f);
        spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
    }
}

void Boss::executeRadialBurst(float dt) {
    fireTimer += dt;
    float interval = RADIAL_BURST_INTERVAL;
    if (phase >= 3) interval = RADIAL_BURST_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        int count = 16;
        float speed = BOSS_BULLET_SPEED_FAST;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST * 1.3f;

        for (int i = 0; i < count; ++i) {
            float angle = (PI * 2.f / count) * i;
            sf::Vector2f dir(std::cos(angle), std::sin(angle));
            spawnBullet(position, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executeRandomSpray(float dt) {
    fireTimer += dt;
    float interval = RANDOM_SPRAY_INTERVAL;
    if (phase >= 3) interval = RANDOM_SPRAY_INTERVAL * 0.5f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_FAST;
        sf::Vector2f spawnPos(static_cast<float>(std::rand() % WINDOW_WIDTH),
                             static_cast<float>(std::rand() % WINDOW_HEIGHT));
        float angle = static_cast<float>(std::rand() % 360) * PI / 180.f;
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
    }
}

void Boss::executePredictiveShot(float dt, const sf::Vector2f& playerPos) {
    fireTimer += dt;
    float interval = PREDICTIVE_INTERVAL;
    if (phase >= 3) interval = PREDICTIVE_INTERVAL * 0.7f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        sf::Vector2f predictedPos = playerPos;
        predictedPos.y -= 50.f;

        sf::Vector2f toTarget = predictedPos - position;
        float angle = std::atan2(toTarget.y, toTarget.x);
        sf::Vector2f dir(std::cos(angle), std::sin(angle));
        spawnBullet(position, dir, PREDICTIVE_SHOT_SPEED, BOSS_BULLET_DAMAGE * 3);
    }
}

void Boss::executeChargeBeam(float dt, const sf::Vector2f& playerPos) {
    if (chargeBeamStep == 0) {
        chargeBeamTimer += dt;
        chargeBeamTarget = playerPos;
        if (chargeBeamTimer >= CHARGE_BEAM_WARNING) {
            chargeBeamStep = 1;
            chargeBeamTimer = 0.f;
        }
    } else if (chargeBeamStep == 1) {
        chargeBeamTimer += dt;

        if (fireTimer >= 0.02f) {
            fireTimer -= 0.02f;
            sf::Vector2f toTarget = chargeBeamTarget - position;
            float angle = std::atan2(toTarget.y, toTarget.x);
            for (int i = -2; i <= 2; ++i) {
                float spreadAngle = angle + i * 0.05f;
                sf::Vector2f dir(std::cos(spreadAngle), std::sin(spreadAngle));
                spawnBullet(position, dir, PREDICTIVE_SHOT_SPEED * 1.5f, CHARGE_BEAM_DAMAGE);
            }
        }

        if (chargeBeamTimer >= CHARGE_BEAM_DURATION) {
            chargeBeamStep = 0;
            chargeBeamTimer = 0.f;
        }
    }
}

void Boss::executeTentacleSweep(float dt, const sf::Vector2f& playerPos) {
    tentacleSweepTimer += dt;
    tentacleSweepY = playerPos.y;

    if (fireTimer >= 0.1f) {
        fireTimer -= 0.1f;

        float tipX = position.x + BOSS_RADIUS * 0.8f;
        sf::Vector2f tipPos(tipX, tentacleSweepY);
        sf::Vector2f dir(-1.f, 0.f);
        spawnBullet(tipPos, dir, BOSS_BULLET_SPEED_FAST, BOSS_BULLET_DAMAGE * 2);
    }
}

void Boss::executeScreenLock(float dt) {
    screenLockTimer += dt;
    screenLockProgress = screenLockTimer / SCREEN_LOCK_DURATION;
}

void Boss::executeBulletRainFull(float dt) {
    fireTimer += dt;
    float interval = BULLET_RAIN_FULL_INTERVAL;
    if (phase >= 3) interval = BULLET_RAIN_FULL_INTERVAL * 0.5f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_FAST;
        int count = 3 + phase;
        for (int i = 0; i < count; ++i) {
            float x = static_cast<float>(std::rand() % WINDOW_WIDTH);
            sf::Vector2f spawnPos(x, -10.f);
            sf::Vector2f dir(0.f, 1.f);
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

void Boss::executePhaseTransition(float dt) {
    phaseTransitionTimer += dt;
    float teleportInterval = PHASE_TRANSITION_DURATION / 3.f;
    float phaseStage = phaseTransitionTimer / teleportInterval;

    if (phaseStage > 0.f && phaseStage < 1.f) {
        position.x = BOSS_POS.x + std::sin(phaseTransitionTimer * 10.f) * 50.f;
    } else if (phaseStage >= 1.f && phaseStage < 2.f) {
        position = originalPosition;
        hp += maxHP / 20;
        if (hp > maxHP) hp = maxHP;
    } else if (phaseStage >= 2.f) {
        position = BOSS_POS;
    }
}

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

sf::Vector2f Boss::getChargeBeamTarget() const {
    return chargeBeamTarget;
}

float Boss::getTentacleSweepY() const {
    return tentacleSweepY;
}

float Boss::getScreenLockProgress() const {
    return screenLockProgress;
}

void Boss::render(sf::RenderWindow& window) const {
    for (const auto& bullet : bullets) {
        bullet.render(window);
    }
    renderAlienEffects(window);
    if (currentAttack != BossAttackType::TentacleSweep) {
        renderAlienTentacles(window);
    }
    renderAlienBody(window);
    renderAlienEye(window);

    if (isChargeBeamWarning()) {
        renderChargeBeamWarning(window);
    }
    if (currentAttack == BossAttackType::TentacleSweep) {
        renderTentacleSweep(window);
    }
    if (isScreenLocking()) {
        renderScreenLock(window);
    }

    // 入场特效
    renderEntranceEffect(window);

    // 根据BOSS类型绘制独特光环
    switch (bossType) {
        case BossType::Inferno: renderInfernoAura(window); break;
        case BossType::Void: renderVoidAura(window); break;
        case BossType::Thunder: renderThunderAura(window); break;
    }
}

void Boss::renderAlienBody(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;

    sf::Color bodyColor, innerColor;
    // Colors based on boss type
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

    float deformScale = 1.f + std::sin(deformTimer * 2.f) * 0.03f;
    if (phase == 3) {
        deformScale += std::sin(deformTimer * 4.f) * 0.05f;
    }

    float baseRadius = BOSS_RADIUS * deformScale;

    for (int layer = 0; layer < 4; layer++) {
        float layerScale = 1.f - layer * 0.15f;
        float layerAlpha = 0.4f + layer * 0.2f;
        sf::CircleShape bodyLayer(baseRadius * layerScale);
        bodyLayer.setOrigin(sf::Vector2f(baseRadius * layerScale, baseRadius * layerScale));
        bodyLayer.setPosition(pos);

        float offsetX = std::sin(deformTimer * 1.5f + layer) * 4.f;
        float offsetY = std::cos(deformTimer * 1.2f + layer * 0.7f) * 4.f;
        bodyLayer.setPosition(sf::Vector2f(pos.x + offsetX, pos.y + offsetY));

        sf::Color layerColor = bodyColor;
        layerColor.a = static_cast<std::uint8_t>(255.f * layerAlpha);
        bodyLayer.setFillColor(layerColor);
        window.draw(bodyLayer);
    }

    sf::CircleShape innerBody(baseRadius * 0.7f);
    innerBody.setOrigin(sf::Vector2f(baseRadius * 0.7f, baseRadius * 0.7f));
    innerBody.setPosition(sf::Vector2f(pos.x + 2.f, pos.y - 2.f));
    innerBody.setFillColor(innerColor);
    window.draw(innerBody);

    for (int i = 0; i < 8; i++) {
        float angle = (PI * 2.f / 8.f) * i + deformTimer * 0.3f;
        float dist = baseRadius * 0.5f;
        float bumpX = pos.x + std::cos(angle) * dist;
        float bumpY = pos.y + std::sin(angle) * dist;
        sf::CircleShape bump(6.f + std::sin(deformTimer * 2.f + i) * 2.f);
        bump.setOrigin(sf::Vector2f(6.f, 6.f));
        bump.setPosition(sf::Vector2f(bumpX, bumpY));
        bump.setFillColor(sf::Color(bodyColor.r + 30, bodyColor.g + 10, bodyColor.b + 20, 200));
        window.draw(bump);
    }
}

void Boss::renderAlienEye(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseRadius = BOSS_RADIUS;

    // Eye colors based on boss type
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

    sf::CircleShape socket(baseRadius * 0.55f);
    socket.setOrigin(sf::Vector2f(baseRadius * 0.55f, baseRadius * 0.55f));
    socket.setPosition(sf::Vector2f(pos.x, pos.y - 5.f));
    socket.setFillColor(sf::Color(20, 5, 10));
    window.draw(socket);

    sf::CircleShape eyeball(baseRadius * 0.42f);
    eyeball.setOrigin(sf::Vector2f(baseRadius * 0.42f, baseRadius * 0.42f));
    eyeball.setPosition(sf::Vector2f(pos.x, pos.y - 5.f));
    eyeball.setFillColor(eyeballColor);
    window.draw(eyeball);

    float irisDist = baseRadius * 0.2f;
    float irisX = pos.x + std::cos(eyeTrackAngle) * irisDist;
    float irisY = pos.y - 5.f + std::sin(eyeTrackAngle) * irisDist;

    sf::CircleShape iris(baseRadius * 0.25f);
    iris.setOrigin(sf::Vector2f(baseRadius * 0.25f, baseRadius * 0.25f));
    iris.setPosition(sf::Vector2f(irisX, irisY));
    iris.setFillColor(irisColor);
    window.draw(iris);

    sf::CircleShape pupil(baseRadius * 0.12f);
    pupil.setOrigin(sf::Vector2f(baseRadius * 0.12f, baseRadius * 0.12f));
    pupil.setPosition(sf::Vector2f(irisX, irisY));
    pupil.setFillColor(sf::Color(10, 0, 5));
    window.draw(pupil);

    sf::CircleShape highlight(baseRadius * 0.08f);
    highlight.setOrigin(sf::Vector2f(baseRadius * 0.08f, baseRadius * 0.08f));
    highlight.setPosition(sf::Vector2f(irisX - 6.f, irisY - 6.f));
    highlight.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(highlight);

    float lidClose = std::sin(pulseTimer * 0.5f) * 0.5f + 0.5f;
    float lidAlpha = static_cast<std::uint8_t>(100.f * (1.f - lidClose));

    sf::ConvexShape topLid;
    topLid.setPointCount(4);
    topLid.setPoint(0, sf::Vector2f(-baseRadius * 0.6f, pos.y - 5.f));
    topLid.setPoint(1, sf::Vector2f(baseRadius * 0.6f, pos.y - 5.f));
    topLid.setPoint(2, sf::Vector2f(baseRadius * 0.5f, pos.y - 5.f - baseRadius * 0.4f * lidClose));
    topLid.setPoint(3, sf::Vector2f(-baseRadius * 0.5f, pos.y - 5.f - baseRadius * 0.4f * lidClose));
    topLid.setFillColor(sf::Color(100, 20, 40, static_cast<std::uint8_t>(lidAlpha)));
    window.draw(topLid);

    sf::ConvexShape bottomLid;
    bottomLid.setPointCount(4);
    bottomLid.setPoint(0, sf::Vector2f(-baseRadius * 0.6f, pos.y - 5.f));
    bottomLid.setPoint(1, sf::Vector2f(baseRadius * 0.6f, pos.y - 5.f));
    bottomLid.setPoint(2, sf::Vector2f(baseRadius * 0.5f, pos.y - 5.f + baseRadius * 0.4f * lidClose));
    bottomLid.setPoint(3, sf::Vector2f(-baseRadius * 0.5f, pos.y - 5.f + baseRadius * 0.4f * lidClose));
    bottomLid.setFillColor(sf::Color(100, 20, 40, static_cast<std::uint8_t>(lidAlpha)));
    window.draw(bottomLid);
}

void Boss::renderAlienTentacles(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseRadius = BOSS_RADIUS;

    for (int i = 0; i < 6; i++) {
        float baseAngle = (PI * 2.f / 6.f) * i + pulseTimer * 0.2f;
        float length = baseRadius * 1.8f;
        float segCount = 8.f;

        sf::Vector2f prevPos = pos;
        prevPos.x += std::cos(baseAngle) * baseRadius;
        prevPos.y += std::sin(baseAngle) * baseRadius;

        for (int seg = 0; seg < static_cast<int>(segCount); seg++) {
            float segAngle = baseAngle + std::sin(pulseTimer * 2.f + i + seg * 0.5f) * 0.3f;
            float segLen = length / segCount;
            float segRadius = 8.f - seg * 0.7f;

            sf::Vector2f nextPos = prevPos;
            nextPos.x += std::cos(segAngle) * segLen;
            nextPos.y += std::sin(segAngle) * segLen;

            sf::RectangleShape segment(sf::Vector2f(segLen + 2.f, segRadius * 2.f));
            float midX = (prevPos.x + nextPos.x) / 2.f;
            float midY = (prevPos.y + nextPos.y) / 2.f;
            segment.setPosition(sf::Vector2f(midX, midY));
            segment.setRotation(sf::degrees(std::atan2(nextPos.y - prevPos.y, nextPos.x - prevPos.x)));

            int alpha = 200 - seg * 20;
            if (phase == 2) alpha = 220 - seg * 20;
            if (phase == 3) alpha = 240 - seg * 20;

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

            prevPos = nextPos;
        }

        sf::CircleShape tip(6.f);
        tip.setOrigin(sf::Vector2f(6.f, 6.f));
        tip.setPosition(sf::Vector2f(prevPos.x, prevPos.y));
        tip.setFillColor(sf::Color(200, 50, 80, 180));
        window.draw(tip);
    }
}

void Boss::renderAlienEffects(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseRadius = BOSS_RADIUS;

    if (phase >= 2) {
        for (int i = 0; i < 4; i++) {
            float angle = pulseTimer * 1.5f + (PI * 2.f / 4.f) * i;
            float dist = baseRadius * 1.5f + std::sin(pulseTimer * 2.f + i) * 10.f;
            float orbX = pos.x + std::cos(angle) * dist;
            float orbY = pos.y + std::sin(angle) * dist;

            float orbRadius = 8.f + std::sin(pulseTimer * 3.f + i) * 2.f;
            sf::CircleShape orb(orbRadius);
            orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
            orb.setPosition(sf::Vector2f(orbX, orbY));

            sf::Color orbColor(255, 80, 120, 150);
            if (phase == 3) orbColor = sf::Color(255, 100, 150, 180);
            orb.setFillColor(orbColor);
            window.draw(orb);
        }
    }

    if (phase == 3) {
        float auraRadius = baseRadius * 1.3f + std::sin(pulseTimer * 3.f) * 10.f;
        sf::CircleShape aura(auraRadius);
        aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
        aura.setPosition(pos);
        aura.setFillColor(sf::Color(200, 30, 80, 40));
        window.draw(aura);

        if (isLaserWarning()) {
            float warningPulse = std::sin(pulseTimer * 15.f) * 0.5f + 0.5f;
            sf::CircleShape warningAura(baseRadius * 2.f);
            warningAura.setOrigin(sf::Vector2f(baseRadius * 2.f, baseRadius * 2.f));
            warningAura.setPosition(pos);
            warningAura.setFillColor(sf::Color(255, 50, 50, static_cast<std::uint8_t>(warningPulse * 100)));
            window.draw(warningAura);
        }
    }

    if (isLaserWarning()) {
        sf::RectangleShape warningLineH(sf::Vector2f(WINDOW_WIDTH, 3.f));
        warningLineH.setPosition(sf::Vector2f(0.f, pos.y - 5.f));
        warningLineH.setFillColor(sf::Color(255, 50, 50, 100));
        window.draw(warningLineH);

        sf::RectangleShape warningLineV(sf::Vector2f(3.f, WINDOW_HEIGHT));
        warningLineV.setPosition(sf::Vector2f(pos.x, 0.f));
        warningLineV.setFillColor(sf::Color(255, 50, 50, 100));
        window.draw(warningLineV);
    }
}

void Boss::renderChargeBeamWarning(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float warningPulse = std::sin(pulseTimer * 15.f) * 0.5f + 0.5f;

    sf::RectangleShape chargeLine(sf::Vector2f(3.f, WINDOW_HEIGHT));
    chargeLine.setPosition(pos);
    chargeLine.setFillColor(sf::Color(255, 100, 50, static_cast<std::uint8_t>(warningPulse * 150)));
    window.draw(chargeLine);

    sf::CircleShape chargeAura(BOSS_RADIUS * 1.5f);
    chargeAura.setOrigin(sf::Vector2f(BOSS_RADIUS * 1.5f, BOSS_RADIUS * 1.5f));
    chargeAura.setPosition(pos);
    chargeAura.setFillColor(sf::Color(255, 100, 50, static_cast<std::uint8_t>(warningPulse * 80)));
    window.draw(chargeAura);
}

void Boss::renderTentacleSweep(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;

    sf::RectangleShape tentacle(sf::Vector2f(400.f, 30.f));
    tentacle.setOrigin(sf::Vector2f(0.f, 15.f));
    tentacle.setPosition(sf::Vector2f(pos.x, tentacleSweepY));
    tentacle.setFillColor(sf::Color(120, 30, 50, 200));
    window.draw(tentacle);

    sf::CircleShape tip(BOSS_RADIUS * 0.8f);
    tip.setOrigin(sf::Vector2f(BOSS_RADIUS * 0.8f, BOSS_RADIUS * 0.8f));
    tip.setPosition(sf::Vector2f(pos.x + 200.f, tentacleSweepY));
    tip.setFillColor(sf::Color(200, 50, 80, 220));
    window.draw(tip);
}

void Boss::renderScreenLock(sf::RenderWindow& window) const {
    float alpha = static_cast<std::uint8_t>(screenLockProgress * 100);

    sf::RectangleShape topWall(sf::Vector2f(WINDOW_WIDTH, 50.f));
    topWall.setPosition(sf::Vector2f(0.f, 0.f));
    topWall.setFillColor(sf::Color(100, 50, 150, alpha));
    window.draw(topWall);

    sf::RectangleShape bottomWall(sf::Vector2f(WINDOW_WIDTH, 50.f));
    bottomWall.setPosition(sf::Vector2f(0.f, WINDOW_HEIGHT - 50.f));
    bottomWall.setFillColor(sf::Color(100, 50, 150, alpha));
    window.draw(bottomWall);

    sf::RectangleShape leftWall(sf::Vector2f(50.f, WINDOW_HEIGHT));
    leftWall.setPosition(sf::Vector2f(0.f, 0.f));
    leftWall.setFillColor(sf::Color(100, 50, 150, alpha));
    window.draw(leftWall);

    sf::RectangleShape rightWall(sf::Vector2f(50.f, WINDOW_HEIGHT));
    rightWall.setPosition(sf::Vector2f(WINDOW_WIDTH - 50.f, 0.f));
    rightWall.setFillColor(sf::Color(100, 50, 150, alpha));
    window.draw(rightWall);
}

sf::Vector2f Boss::getPosition() const { return position; }
float Boss::getRadius() const { return BOSS_RADIUS; }
int Boss::getHP() const { return hp; }
int Boss::getMaxHP() const { return maxHP; }
int Boss::getPhase() const { return phase; }
bool Boss::isDead() const { return hp <= 0; }
std::string Boss::getCurrentAttackName() const { return currentAttackName; }
BossAttackType Boss::getCurrentAttackType() const { return currentAttack; }

std::vector<BossBullet>& Boss::getBullets() { return bullets; }

void Boss::takeDamage(int damage) {
    if (currentAttack == BossAttackType::PhaseTransition) return;
    hp -= damage;
    if (hp < 0) hp = 0;
}

void Boss::spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg) {
    for (auto& bullet : bullets) {
        if (!bullet.isActive()) {
            float slowMult = getSlowMultiplier();
            bullet.spawn(pos, dir, speed * bulletSpeedMultiplier * slowMult, dmg);
            return;
        }
    }
}

void Boss::setBulletSpeedMultiplier(float mult) { bulletSpeedMultiplier = mult; }
float Boss::getBulletSpeedMultiplier() const { return bulletSpeedMultiplier; }

void Boss::applySlow(int stacks) {
    slowStacks = std::min(slowStacks + stacks, HOMING_SLOW_MAX_STACKS);
    slowTimer = HOMING_SLOW_DURATION;
}

float Boss::getSlowMultiplier() const {
    return 1.f - slowStacks * HOMING_SLOW_STACK;
}

void Boss::clearAllBullets() {
    for (auto& bullet : bullets) {
        bullet.deactivate();
    }
}

void Boss::setBossType(BossType type) {
    bossType = type;
}

BossType Boss::getBossType() const {
    return bossType;
}

void Boss::setEntranceStartPos(const sf::Vector2f& pos) {
    entranceStartPos = pos;
}

void Boss::setEntranceTargetPos(const sf::Vector2f& pos) {
    entranceTargetPos = pos;
}

void Boss::setEntranceAnimation(float progress) {
    entranceAnimationProgress = progress;
    if (progress >= 1.f) {
        entranceAnimationActive = false;
        position = entranceTargetPos;
    } else {
        position = entranceStartPos + (entranceTargetPos - entranceStartPos) * progress;
    }
}

float Boss::getEntranceAnimation() const {
    return entranceAnimationProgress;
}

bool Boss::isEntranceComplete() const {
    return entranceAnimationProgress >= 1.f;
}

void Boss::setPosition(const sf::Vector2f& pos) {
    position = pos;
    entranceStartPos = pos;
    entranceTargetPos = pos;
}

void Boss::renderInfernoAura(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    for (int i = 0; i < 8; ++i) {
        float angle = pulseTimer * 2.f + (PI * 2.f / 8.f) * i;
        float dist = BOSS_RADIUS * 1.8f + std::sin(pulseTimer * 3.f + i) * 15.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;
        float orbRadius = 10.f + std::sin(pulseTimer * 4.f + i) * 3.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(255, 100, 30, 180));
        window.draw(orb);
    }
    float auraRadius = BOSS_RADIUS * 2.f + std::sin(pulseTimer * 2.f) * 10.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(255, 50, 0, 30));
    window.draw(aura);
}

void Boss::renderVoidAura(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    for (int i = 0; i < 12; ++i) {
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
    float auraRadius = BOSS_RADIUS * 2.2f + std::sin(pulseTimer * 1.5f) * 15.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(80, 20, 150, 40));
    window.draw(aura);
}

void Boss::renderThunderAura(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    for (int i = 0; i < 6; ++i) {
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
        sf::RectangleShape lightning(sf::Vector2f(30.f, 3.f));
        lightning.setOrigin(sf::Vector2f(0.f, 1.5f));
        lightning.setPosition(sf::Vector2f(orbX, orbY));
        lightning.setRotation(sf::degrees(angle * 180.f / PI));
        lightning.setFillColor(sf::Color(255, 255, 100, 150));
        window.draw(lightning);
    }
    float auraRadius = BOSS_RADIUS * 2.1f + std::sin(pulseTimer * 2.5f) * 12.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(200, 200, 50, 35));
    window.draw(aura);
}

void Boss::renderEntranceEffect(sf::RenderWindow& window) const {
    if (!entranceAnimationActive || entranceAnimationProgress >= 1.f) return;
    float progress = entranceAnimationProgress;
    sf::Vector2f pos = position;
    switch (bossType) {
        case BossType::Inferno: {
            sf::Vector2f trailDir = entranceTargetPos - entranceStartPos;
            float trailLen = std::sqrt(trailDir.x * trailDir.x + trailDir.y * trailDir.y);
            if (trailLen > 0) trailDir /= trailLen;
            for (int i = 0; i < 5; ++i) {
                float t = 1.f - progress + i * 0.1f;
                if (t > 0 && t < 1.f) {
                    sf::Vector2f trailPos = entranceStartPos + trailDir * t * trailLen;
                    float radius = 20.f * (1.f - t) * progress;
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
            float crackAlpha = 200.f * progress;
            sf::RectangleShape crack(sf::Vector2f(WINDOW_WIDTH, 5.f));
            crack.setPosition(sf::Vector2f(0.f, pos.y));
            crack.setFillColor(sf::Color(180, 80, 255, static_cast<std::uint8_t>(crackAlpha)));
            window.draw(crack);
            sf::RectangleShape crack2(sf::Vector2f(WINDOW_WIDTH, 5.f));
            crack2.setPosition(sf::Vector2f(0.f, pos.y - 30.f));
            crack2.setFillColor(sf::Color(150, 60, 255, static_cast<std::uint8_t>(crackAlpha * 0.5f)));
            window.draw(crack2);
            break;
        }
        case BossType::Thunder: {
            float waveRadius = BOSS_RADIUS * (1.f + progress * 3.f);
            sf::CircleShape wave(waveRadius);
            wave.setOrigin(sf::Vector2f(waveRadius, waveRadius));
            wave.setPosition(pos);
            wave.setFillColor(sf::Color(255, 255, 100, static_cast<std::uint8_t>(100 * (1.f - progress))));
            wave.setOutlineColor(sf::Color(255, 240, 100, static_cast<std::uint8_t>(200 * (1.f - progress))));
            wave.setOutlineThickness(3.f);
            window.draw(wave);
            break;
        }
    }
}
