#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "core/GameStats.h"
#include "BossBullet.h"

enum class BossType {
    Inferno,   // 烈焰恶魔
    Void,      // 虚空幽灵
    Thunder    // 雷霆泰坦
};

class Boss {
public:
    Boss();
    void update(float dt, const sf::Vector2f& playerPosition);
    void render(sf::RenderWindow& window) const;

    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos);
    float getRadius() const;
    int getHP() const;
    int getMaxHP() const;
    int getPhase() const;
    bool isDead() const;

    void setBossType(BossType type);
    void setEntranceAnimation(float progress);
    void setEntranceStartPos(sf::Vector2f pos);
    void setEntranceTargetPos(sf::Vector2f pos);
    std::string getCurrentAttackName() const;
    BossAttackType getCurrentAttackType() const;
    void takeDamage(int damage);

    std::vector<BossBullet>& getBullets();
    void spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg);
    void clearAllBullets();
    void setBulletSpeedMultiplier(float mult);
    float getBulletSpeedMultiplier() const;

    // Cross laser state for UI
    bool isLaserWarning() const;
    bool isLaserActive() const;

private:
    void updateAttackPattern(float dt, const sf::Vector2f& playerPos);
    void switchToNextAttack();
    void executeCircleBurst(float dt);
    void executeSpiralShot(float dt);
    void executeAimedShot(float dt, const sf::Vector2f& playerPos);
    void executeWaveBarrage(float dt);
    void executeCrossLaser(float dt, const sf::Vector2f& playerPos);
    void executeRainBullets(float dt);

    sf::Vector2f position;
    sf::CircleShape shape;

    std::vector<BossBullet> bullets;

    int hp;
    int maxHP;
    int phase;
    float bulletSpeedMultiplier = 1.f;
    BossType bossType = BossType::Inferno;
    float entranceProgress = 0.f;
    sf::Vector2f entranceStartPos;
    sf::Vector2f entranceTargetPos;

    // Attack state
    BossAttackType currentAttack;
    std::string currentAttackName;
    float attackTimer;
    float fireTimer;
    int attackIndex;
    float spiralAngle;
    int crossLaserStep;
    int crossLaserCount;
    float crossLaserTimer;
};
