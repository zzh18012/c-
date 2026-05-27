#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "PlayerBullet.h"

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
    float getShieldTimer() const;
    float getOverdriveTimer() const;
    float getDashBatteryTimer() const;
    float getFireRateMultiplier() const;
    float getDamageMultiplier() const;
    float getSpeedMultiplier() const;
    float getDashCooldown() const;
    bool isDashing() const;

    std::vector<PlayerBullet>& getBullets();

private:
    void shoot();

    sf::Vector2f position;
    sf::CircleShape shape;
    sf::Vector2f aimDir;

    std::vector<PlayerBullet> bullets;
    float shootCooldown;

    int hp;
    int maxHP;
    float invincibleTimer;
    bool godMode;

    float shieldTimer;
    float overdriveTimer;
    float dashBatteryTimer;

    float dashCooldown;
    bool dashing;
    float dashTimer;
    sf::Vector2f dashDir;
};
