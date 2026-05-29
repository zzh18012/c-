#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "PlayerBullet.h"
#include "PlayerBulletSpread.h"
#include "PlayerBulletPiercing.h"
#include "PlayerBulletOrbital.h"
#include "PlayerBulletCluster.h"
#include "PlayerBulletHoming.h"

enum class WeaponType { Normal, Spread, Piercing, Orbital, Cluster, Homing };

class Player {
public:
    Player();
    void handleInput(float dt, const sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderWindow& window) const;

    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getHP() const;
    int getMaxHP() const;
    bool isDead() const;
    bool isInvincible() const;
    void takeDamage(int damage);
    void heal(int amount);
    void setGodMode(bool enabled);

    void activateShieldOrb();
    void activateOverdrive();
    void activateDashBattery();
    void activatePhaseShift();
    void activateNovaForm();
    void activateSpeedCoil();
    void activateAttackBoost();
    void cycleWeapon();
    void cycleWeaponReverse();
    float getShieldTimer() const;
    float getOverdriveTimer() const;
    float getDashBatteryTimer() const;
    float getFireRateMultiplier() const;
    float getDamageMultiplier() const;
    float getSpeedMultiplier() const;
    float getDashCooldown() const;
    bool isDashing() const;
    bool isPhaseShifting() const;
    bool isNovaFormActive() const;
    float getPhaseShiftTimer() const;
    int getComboCount() const;
    int getSlowStacks() const;
    float getComboDamageMultiplier() const;
    float getSlowMultiplier() const;
    void registerHit(int targetId);

    std::vector<PlayerBullet>& getBullets();
    std::vector<PlayerBulletSpread>& getSpreadBullets();
    std::vector<PlayerBulletPiercing>& getPiercingBullets();
    std::vector<PlayerBulletOrbital>& getOrbitalBullets();
    std::vector<PlayerBulletCluster>& getClusterBullets();
    std::vector<PlayerBulletHoming>& getHomingBullets();

    WeaponType getCurrentWeapon() const;

private:
    void shoot();
    void renderMechBody(sf::RenderWindow& window) const;
    void renderMechWeapon(sf::RenderWindow& window) const;
    void renderMechEffects(sf::RenderWindow& window) const;

    sf::Vector2f position;
    sf::Vector2f aimDir;

    std::vector<PlayerBullet> bullets;
    std::vector<PlayerBulletSpread> spreadBullets;
    std::vector<PlayerBulletPiercing> piercingBullets;
    std::vector<PlayerBulletOrbital> orbitalBullets;
    std::vector<PlayerBulletCluster> clusterBullets;
    std::vector<PlayerBulletHoming> homingBullets;

    float shootCooldown;
    float spreadCooldown;
    float piercingCooldown;
    float orbitalCooldown;
    float clusterCooldown;
    float homingCooldown;

    WeaponType currentWeapon;

    int hp;
    int maxHP;
    float invincibleTimer;
    bool godMode;

    float shieldTimer;
    float overdriveTimer;
    float dashBatteryTimer;
    float phaseShiftTimer;
    float novaFormTimer;
    float speedCoilTimer;
    float attackModuleTimer;

    float dashCooldown;
    bool dashing;
    float dashTimer;
    sf::Vector2f dashDir;

    float mechRotationAngle;
    float weaponRecoilTimer;

    // Normal weapon combo system
    int comboCount;
    float comboTimer;
    int lastHitTargetId;

    // Homing slow debuff
    int slowStacks;
    float slowTimer;
};
