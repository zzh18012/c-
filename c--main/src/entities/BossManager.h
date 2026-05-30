#pragma once
#include "Boss.h"
#include "core/Config.h"
#include <array>
#include <vector>

class BossManager {
public:
    BossManager();
    ~BossManager();

    void init(Difficulty difficulty);
    void update(float dt, const sf::Vector2f& playerPosition);
    void render(sf::RenderWindow& window) const;

    // 状态查询
    int getActiveBossCount() const;
    bool isAllBossesDead() const;
    bool shouldSpawnNextWave() const;
    void onBossDefeated(int index);
    void triggerSecondWave();

    // 获取BOSS（用于碰撞检测）
    std::array<Boss, MAX_BOSS_COUNT>& getBosses() { return bosses; }
    const std::array<Boss, MAX_BOSS_COUNT>& getBosses() const { return bosses; }

    // 清除所有BOSS弹幕
    void clearAllBullets();

    // 获取指定BOSS的位置（用于玩家弹幕碰撞）
    std::vector<sf::Vector2f> getAllBossPositions() const;
    std::vector<float> getAllBossRadii() const;

private:
    std::array<Boss, MAX_BOSS_COUNT> bosses;
    std::array<bool, MAX_BOSS_COUNT> bossActive;
    int activeBossCount;
    Difficulty difficulty;
    bool secondWaveSpawned;
    bool secondWaveTriggered;
    sf::Clock spawnTimer;
    std::array<float, MAX_BOSS_COUNT> spawnProgress;  // 入场动画进度

    void spawnBoss(int index, sf::Vector2f pos);
    sf::Vector2f getRandomSpawnPosition(int bossIndex) const;
};