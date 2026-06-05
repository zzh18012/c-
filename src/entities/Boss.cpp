#include "Boss.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>

static const float PI = 3.14159265358979323846f;

Boss::Boss()
    : position(BOSS_POS)
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
{
    shape.setRadius(BOSS_RADIUS);
    shape.setOrigin(BOSS_RADIUS, BOSS_RADIUS);
    shape.setFillColor(sf::Color(255, 50, 100, 200));
    shape.setOutlineColor(sf::Color(255, 50, 100));
    shape.setOutlineThickness(3.f);

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
        attackIndex = -1;  // Reset: next switchToNextAttack will start from index 0
        switchToNextAttack();
        attackTimer = 0.f;
        fireTimer = 0.f;
        spiralAngle = 0.f;
        crossLaserStep = 0;
        crossLaserCount = 0;
        crossLaserTimer = 0.f;
    }

    updateAttackPattern(dt, playerPosition);

    shape.setPosition(position);

    for (auto& bullet : bullets) {
        bullet.update(dt);
    }
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
    }

    switch (currentAttack) {
        case BossAttackType::CircleBurst: executeCircleBurst(dt); break;
        case BossAttackType::SpiralShot:  executeSpiralShot(dt); break;
        case BossAttackType::AimedShot:   executeAimedShot(dt, playerPos); break;
        case BossAttackType::WaveBarrage: executeWaveBarrage(dt); break;
        case BossAttackType::CrossLaser:  executeCrossLaser(dt, playerPos); break;
        case BossAttackType::RainBullets: executeRainBullets(dt); break;
        default: break;
    }
}

void Boss::switchToNextAttack() {
    // Phase-dependent attack sequences
    static const BossAttackType seqPhase1[] = {
        BossAttackType::CircleBurst,
        BossAttackType::AimedShot,
        BossAttackType::SpiralShot,
    };
    static const char* namesPhase1[] = {
        "Circle Burst",
        "Aimed Shot",
        "Spiral Shot",
    };

    static const BossAttackType seqPhase2[] = {
        BossAttackType::CircleBurst,
        BossAttackType::AimedShot,
        BossAttackType::WaveBarrage,
        BossAttackType::CrossLaser,
        BossAttackType::SpiralShot,
    };
    static const char* namesPhase2[] = {
        "Circle Burst",
        "Aimed Shot",
        "Wave Barrage",
        "Cross Laser",
        "Spiral Shot",
    };

    static const BossAttackType seqPhase3[] = {
        BossAttackType::CircleBurst,
        BossAttackType::SpiralShot,
        BossAttackType::AimedShot,
        BossAttackType::WaveBarrage,
        BossAttackType::CrossLaser,
        BossAttackType::RainBullets,
    };
    static const char* namesPhase3[] = {
        "Circle Burst",
        "Spiral Shot",
        "Aimed Shot",
        "Wave Barrage",
        "Cross Laser",
        "Rain Bullets",
    };

    const BossAttackType* seq;
    const char* const* names;
    int seqLen;

    if (phase == 1) {
        seq = seqPhase1; names = namesPhase1; seqLen = 3;
    } else if (phase == 2) {
        seq = seqPhase2; names = namesPhase2; seqLen = 5;
    } else {
        seq = seqPhase3; names = namesPhase3; seqLen = 6;
    }

    attackIndex = (attackIndex + 1) % seqLen;
    currentAttack = seq[attackIndex];
    currentAttackName = names[attackIndex];
}

// ===== Circle Burst =====
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

// ===== Spiral Shot =====
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

// ===== Aimed Shot =====
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

// ===== Wave Barrage =====
void Boss::executeWaveBarrage(float dt) {
    fireTimer += dt;
    float interval = WAVE_INTERVAL;
    if (phase >= 3) interval = WAVE_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        // Left side waves
        {
            sf::Vector2f spawnPos(-10.f, position.y + std::sin(attackTimer * WAVE_FREQUENCY * 2.f) * WAVE_AMPLITUDE);
            sf::Vector2f dir(1.f, std::sin(attackTimer * WAVE_FREQUENCY * 2.f + 1.f) * 0.5f);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= len;
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
        // Right side waves
        {
            sf::Vector2f spawnPos(WINDOW_WIDTH + 10.f, position.y + std::cos(attackTimer * WAVE_FREQUENCY * 2.f) * WAVE_AMPLITUDE);
            sf::Vector2f dir(-1.f, std::cos(attackTimer * WAVE_FREQUENCY * 2.f + 1.f) * 0.5f);
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= len;
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ===== Cross Laser =====
void Boss::executeCrossLaser(float dt, const sf::Vector2f& playerPos) {
    crossLaserTimer += dt;

    if (crossLaserStep == 0) {
        // Warning phase
        if (crossLaserTimer >= CROSS_LASER_WARNING_TIME) {
            crossLaserStep = 1;
            crossLaserTimer = 0.f;
        }
    } else {
        // Active phase - spawn "laser bullets" along cross pattern
        if (crossLaserTimer >= CROSS_LASER_ACTIVE_TIME) {
            crossLaserCount++;
            if (crossLaserCount >= CROSS_LASER_REPEATS) {
                // Attack ends
                crossLaserStep = 2;
                return;
            }
            crossLaserStep = 0;
            crossLaserTimer = 0.f;
            return;
        }

        // Fire laser bullets (simulated as fast bullets along cross pattern)
        if (fireTimer >= 0.03f) {
            fireTimer -= 0.03f;

            float speed = BOSS_BULLET_SPEED_FAST * 1.5f;
            float laserY = playerPos.y;
            float laserX = playerPos.x;

            // Horizontal laser - bullets going up/down along Y axis
            for (int y = 0; y < WINDOW_HEIGHT; y += 40) {
                sf::Vector2f bulletPos(laserX, static_cast<float>(y));
                sf::Vector2f dir(0.f, 1.f);
                spawnBullet(bulletPos, dir, speed * 0.3f, BOSS_BULLET_DAMAGE * 2);
            }

            // Vertical laser - bullets going left/right along X axis
            for (int x = 0; x < WINDOW_WIDTH; x += 40) {
                sf::Vector2f bulletPos(static_cast<float>(x), laserY);
                sf::Vector2f dir(1.f, 0.f);
                spawnBullet(bulletPos, dir, speed * 0.3f, BOSS_BULLET_DAMAGE * 2);
            }
        }
    }
}

// ===== Rain Bullets =====
void Boss::executeRainBullets(float dt) {
    fireTimer += dt;
    float interval = RAIN_INTERVAL;
    if (phase >= 3) interval = RAIN_INTERVAL * 0.6f;

    if (fireTimer >= interval) {
        fireTimer -= interval;

        float speed = BOSS_BULLET_SPEED_NORMAL;
        if (phase >= 3) speed = BOSS_BULLET_SPEED_FAST;

        // Spawn 2-3 bullets at random X positions from top
        int count = 2 + (std::rand() % 2);
        for (int i = 0; i < count; ++i) {
            float x = 20.f + static_cast<float>(std::rand() % (WINDOW_WIDTH - 40));
            sf::Vector2f spawnPos(x, -10.f);
            sf::Vector2f dir(0.f, 1.f);
            spawnBullet(spawnPos, dir, speed, BOSS_BULLET_DAMAGE);
        }
    }
}

// ===== Laser state queries =====
bool Boss::isLaserWarning() const {
    return currentAttack == BossAttackType::CrossLaser && crossLaserStep == 0;
}

bool Boss::isLaserActive() const {
    return currentAttack == BossAttackType::CrossLaser && crossLaserStep == 1;
}

// ===== Rendering & Accessors =====
void Boss::render(sf::RenderWindow& window) const {
    for (const auto& bullet : bullets) {
        bullet.render(window);
    }
    window.draw(shape);
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
    hp -= damage;
    if (hp < 0) hp = 0;
}

void Boss::spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg) {
    for (auto& bullet : bullets) {
        if (!bullet.isActive()) {
            bullet.spawn(pos, dir, speed * bulletSpeedMultiplier, dmg);
            return;
        }
    }
}

void Boss::setBulletSpeedMultiplier(float mult) { bulletSpeedMultiplier = mult; }
float Boss::getBulletSpeedMultiplier() const { return bulletSpeedMultiplier; }

void Boss::setPosition(sf::Vector2f pos) { position = pos; }
void Boss::setBossType(BossType type) { bossType = type; }
void Boss::setEntranceAnimation(float progress) { entranceProgress = progress; }
void Boss::setEntranceStartPos(sf::Vector2f pos) { entranceStartPos = pos; }
void Boss::setEntranceTargetPos(sf::Vector2f pos) { entranceTargetPos = pos; }

void Boss::clearAllBullets() {
    for (auto& bullet : bullets) {
        bullet.deactivate();
    }
}
