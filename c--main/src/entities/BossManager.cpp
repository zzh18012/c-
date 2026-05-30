#include "BossManager.h"
#include "core/Config.h"
#include <cstdlib>

BossManager::BossManager()
    : activeBossCount(0), difficulty(Difficulty::Normal)
    , secondWaveSpawned(false), secondWaveTriggered(false) {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        bossActive[i] = false;
        spawnProgress[i] = 0.f;
    }
}

BossManager::~BossManager() {}

void BossManager::init(Difficulty diff) {
    difficulty = diff;
    secondWaveSpawned = false;
    secondWaveTriggered = false;
    spawnTimer.restart();

    int bossCount = (diff == Difficulty::Hard) ? HARD_BOSS_COUNT : LUNATIC_BOSS_COUNT;
    activeBossCount = bossCount;

    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        bossActive[i] = (i < bossCount);
        spawnProgress[i] = 0.f;
        if (i < bossCount) {
            sf::Vector2f pos = getRandomSpawnPosition(i);
            spawnBoss(i, pos);
        }
    }
}

sf::Vector2f BossManager::getRandomSpawnPosition(int bossIndex) const {
    if (activeBossCount <= 1) {
        float offsetX = static_cast<float>((std::rand() % 100) - 50);
        float offsetY = static_cast<float>((std::rand() % 50) - 25);
        return sf::Vector2f(
            std::max(BOSS_SPAWN_MIN_X, std::min(BOSS_SPAWN_MAX_X, (BOSS_SPAWN_MIN_X + BOSS_SPAWN_MAX_X) * 0.5f + offsetX)),
            BOSS_SPAWN_MIN_Y + offsetY
        );
    }
    float baseX = BOSS_SPAWN_MIN_X + (BOSS_SPAWN_MAX_X - BOSS_SPAWN_MIN_X) *
                  static_cast<float>(bossIndex) / static_cast<float>(activeBossCount - 1);
    float offsetX = static_cast<float>((std::rand() % 100) - 50);
    float offsetY = static_cast<float>((std::rand() % 50) - 25);
    return sf::Vector2f(
        std::max(BOSS_SPAWN_MIN_X, std::min(BOSS_SPAWN_MAX_X, baseX + offsetX)),
        BOSS_SPAWN_MIN_Y + offsetY
    );
}

void BossManager::spawnBoss(int index, sf::Vector2f pos) {
    bosses[index].setPosition(pos);
    bossActive[index] = true;
}

void BossManager::update(float dt, const sf::Vector2f& playerPosition) {
    // 更新入场动画进度
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && spawnProgress[i] < 1.f) {
            spawnProgress[i] += dt * 0.5f;  // 2秒入场动画
            if (spawnProgress[i] > 1.f) spawnProgress[i] = 1.f;
        }
    }

    // 检查第二波入场条件
    if (difficulty == Difficulty::Lunatic && !secondWaveTriggered && !secondWaveSpawned) {
        bool hpBelowThreshold = bosses[0].getHP() <= bosses[0].getMaxHP() * BOSS_SPAWN_HP_THRESHOLD;
        bool bossDiedAfterThreshold = !bosses[0].isDead() || hpBelowThreshold;
        if (bossDiedAfterThreshold) {
            secondWaveTriggered = true;
            for (int i = 1; i < MAX_BOSS_COUNT; ++i) {
                if (!bossActive[i]) {
                    sf::Vector2f pos = getRandomSpawnPosition(i);
                    spawnBoss(i, pos);
                }
            }
            secondWaveSpawned = true;
        }
    }

    // 更新存活的BOSS
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].update(dt, playerPosition);
            if (bosses[i].isDead()) {
                bossActive[i] = false;
            }
        }
    }
}

void BossManager::render(sf::RenderWindow& window) const {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].render(window);
        }
    }
}

int BossManager::getActiveBossCount() const {
    int count = 0;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) count++;
    }
    return count;
}

bool BossManager::isAllBossesDead() const {
    return getActiveBossCount() == 0;
}

bool BossManager::shouldSpawnNextWave() const {
    return secondWaveTriggered && !secondWaveSpawned;
}

void BossManager::onBossDefeated(int index) {
    bossActive[index] = false;
}

void BossManager::triggerSecondWave() {
    if (!secondWaveSpawned) {
        secondWaveTriggered = true;
        for (int i = 1; i < MAX_BOSS_COUNT; ++i) {
            if (!bossActive[i]) {
                sf::Vector2f pos = getRandomSpawnPosition(i);
                spawnBoss(i, pos);
            }
        }
        secondWaveSpawned = true;
    }
}

void BossManager::clearAllBullets() {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].clearAllBullets();
        }
    }
}

std::vector<sf::Vector2f> BossManager::getAllBossPositions() const {
    std::vector<sf::Vector2f> positions;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) {
            positions.push_back(bosses[i].getPosition());
        }
    }
    return positions;
}

std::vector<float> BossManager::getAllBossRadii() const {
    std::vector<float> radii;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) {
            radii.push_back(bosses[i].getRadius());
        }
    }
    return radii;
}