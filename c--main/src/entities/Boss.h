#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "core/GameStats.h"
#include "BossBullet.h"

class Boss {
public:
    Boss();
    void update(float dt, const sf::Vector2f& playerPosition);
    void render(sf::RenderWindow& window) const;

    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getHP() const;
    int getMaxHP() const;
    int getPhase() const;
    bool isDead() const;
    std::string getCurrentAttackName() const;
    BossAttackType getCurrentAttackType() const;
    void takeDamage(int damage);

    std::vector<BossBullet>& getBullets();
    void spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg);
    void clearAllBullets();
    void setBulletSpeedMultiplier(float mult);
    float getBulletSpeedMultiplier() const;

    bool isLaserWarning() const;
    bool isLaserActive() const;
    bool isChargeBeamWarning() const;
    bool isChargeBeamActive() const;
    bool isScreenLocking() const;
    bool isPhaseTransitioning() const;
    sf::Vector2f getChargeBeamTarget() const;
    float getTentacleSweepY() const;
    float getScreenLockProgress() const;

private:
    void updateAttackPattern(float dt, const sf::Vector2f& playerPos);
    void switchToNextAttack();
    void executeCircleBurst(float dt);
    void executeSpiralShot(float dt);
    void executeAimedShot(float dt, const sf::Vector2f& playerPos);
    void executeWaveBarrage(float dt);
    void executeCrossLaser(float dt, const sf::Vector2f& playerPos);
    void executeRainBullets(float dt);
    void executeFlowerBurst(float dt);
    void executeEightDirections(float dt);
    void executeSnakeWave(float dt);
    void executeHomingOrbs(float dt, const sf::Vector2f& playerPos);
    void executeDoubleSpiral(float dt);
    void executeBladeRing(float dt);
    void executeScreenWipe(float dt);
    void executeRadialBurst(float dt);
    void executeRandomSpray(float dt);
    void executePredictiveShot(float dt, const sf::Vector2f& playerPos);
    void executeChargeBeam(float dt, const sf::Vector2f& playerPos);
    void executeTentacleSweep(float dt, const sf::Vector2f& playerPos);
    void executeScreenLock(float dt);
    void executeBulletRainFull(float dt);
    void executePhaseTransition(float dt);

    void resetNewAttackTimers();

    void renderAlienBody(sf::RenderWindow& window) const;
    void renderAlienEye(sf::RenderWindow& window) const;
    void renderAlienTentacles(sf::RenderWindow& window) const;
    void renderAlienEffects(sf::RenderWindow& window) const;
    void renderChargeBeamWarning(sf::RenderWindow& window) const;
    void renderTentacleSweep(sf::RenderWindow& window) const;
    void renderScreenLock(sf::RenderWindow& window) const;

    sf::Vector2f position;
    sf::Vector2f originalPosition;
    std::vector<sf::CircleShape> tentacleShapes;
    std::vector<sf::CircleShape> orbShapes;

    std::vector<BossBullet> bullets;

    int hp;
    int maxHP;
    int phase;
    float bulletSpeedMultiplier = 1.f;

    BossAttackType currentAttack;
    std::string currentAttackName;
    float attackTimer;
    float fireTimer;
    int attackIndex;
    float spiralAngle;
    int crossLaserStep;
    int crossLaserCount;
    float crossLaserTimer;

    // New attack timers
    float flowerTimer;
    float eightDirTimer;
    float snakeTimer;
    float homingOrbTimer;
    float doubleSpiralTimer;
    float bladeRingTimer;
    float screenWipeTimer;
    float radialBurstTimer;
    float randomSprayTimer;
    float predictiveTimer;
    float chargeBeamStep;
    float chargeBeamTimer;
    float tentacleSweepTimer;
    float screenLockTimer;
    float bulletRainFullTimer;
    float phaseTransitionTimer;
    sf::Vector2f chargeBeamTarget;
    float tentacleSweepY;
    float screenLockProgress;

    // Animation state
    float pulseTimer;
    float deformTimer;
    float eyeTrackAngle;
    mutable float tentaclePhases[6];
    mutable float orbPhases[4];
};
