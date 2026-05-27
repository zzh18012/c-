#include "Player.h"
#include "core/Config.h"
#include <cmath>

Player::Player()
    : position(PLAYER_START_POS)
    , aimDir(0.f, -1.f)
    , shootCooldown(0.f)
    , hp(PLAYER_MAX_HP)
    , maxHP(PLAYER_MAX_HP)
    , invincibleTimer(0.f)
    , godMode(false)
    , shieldTimer(0.f)
    , overdriveTimer(0.f)
    , dashBatteryTimer(0.f)
    , dashCooldown(0.f)
    , dashing(false)
    , dashTimer(0.f)
{
    shape.setRadius(PLAYER_RADIUS);
    shape.setOrigin(PLAYER_RADIUS, PLAYER_RADIUS);
    shape.setFillColor(sf::Color(0, 255, 255, 200));
    shape.setOutlineColor(sf::Color(0, 255, 255));
    shape.setOutlineThickness(2.f);

    bullets.resize(MAX_PLAYER_BULLETS);
}

void Player::handleInput(float dt, const sf::RenderWindow& window) {
    // Dash
    if (dashing) return;

    sf::Vector2f move(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) move.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) move.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) move.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) move.x += 1.f;

    float len = std::sqrt(move.x * move.x + move.y * move.y);
    if (len > 0.f) {
        move /= len;
    }

    // Shift to dash
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && dashCooldown <= 0.f) {
        if (len > 0.f) {
            dashDir = move;
        } else {
            dashDir = aimDir;
        }
        dashing = true;
        dashTimer = PLAYER_DASH_DURATION;
        dashCooldown = PLAYER_DASH_COOLDOWN;
    }

    if (!dashing) {
        float speed = PLAYER_SPEED;
        if (dashBatteryTimer > 0.f) {
            speed *= DASH_BATTERY_SPEED_MULT;
        }
        position += move * speed * dt;
    }

    if (position.x < PLAYER_RADIUS) position.x = PLAYER_RADIUS;
    if (position.x > WINDOW_WIDTH - PLAYER_RADIUS) position.x = WINDOW_WIDTH - PLAYER_RADIUS;
    if (position.y < PLAYER_RADIUS) position.y = PLAYER_RADIUS;
    if (position.y > WINDOW_HEIGHT - PLAYER_RADIUS) position.y = WINDOW_HEIGHT - PLAYER_RADIUS;

    // Mouse aiming
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f worldMouse(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    aimDir = worldMouse - position;
    float aimLen = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);
    if (aimLen > 0.f) {
        aimDir /= aimLen;
    }

    // Shooting
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        shoot();
    }
}

void Player::shoot() {
    if (shootCooldown > 0.f) return;

    float rateMult = (overdriveTimer > 0.f) ? OVERDRIVE_FIRE_RATE_MULT : 1.f;
    shootCooldown = PLAYER_SHOOT_COOLDOWN * rateMult;

    for (auto& bullet : bullets) {
        if (!bullet.isActive()) {
            bullet.spawn(position, aimDir);
            if (overdriveTimer > 0.f) {
                bullet.setDamage(static_cast<int>(PLAYER_BULLET_DAMAGE * OVERDRIVE_DAMAGE_MULT));
            }
            break;
        }
    }
}

void Player::update(float dt) {
    if (shootCooldown > 0.f) {
        shootCooldown -= dt;
    }
    if (invincibleTimer > 0.f) {
        invincibleTimer -= dt;
    }
    if (shieldTimer > 0.f) {
        shieldTimer -= dt;
    }
    if (overdriveTimer > 0.f) {
        overdriveTimer -= dt;
    }
    if (dashBatteryTimer > 0.f) {
        dashBatteryTimer -= dt;
    }
    if (dashCooldown > 0.f) {
        dashCooldown -= dt;
    }

    // Dash movement
    if (dashing) {
        dashTimer -= dt;
        position += dashDir * PLAYER_DASH_SPEED * dt;
        if (position.x < PLAYER_RADIUS) position.x = PLAYER_RADIUS;
        if (position.x > WINDOW_WIDTH - PLAYER_RADIUS) position.x = WINDOW_WIDTH - PLAYER_RADIUS;
        if (position.y < PLAYER_RADIUS) position.y = PLAYER_RADIUS;
        if (position.y > WINDOW_HEIGHT - PLAYER_RADIUS) position.y = WINDOW_HEIGHT - PLAYER_RADIUS;
        if (dashTimer <= 0.f) {
            dashing = false;
        }
    }

    shape.setPosition(position);

    for (auto& bullet : bullets) {
        bullet.update(dt);
    }
}

void Player::render(sf::RenderWindow& window) const {
    for (const auto& bullet : bullets) {
        bullet.render(window);
    }
    window.draw(shape);
}

sf::Vector2f Player::getPosition() const { return position; }
float Player::getRadius() const { return PLAYER_RADIUS; }
int Player::getHP() const { return hp; }
int Player::getMaxHP() const { return maxHP; }
bool Player::isDead() const { return hp <= 0; }
bool Player::isInvincible() const { return invincibleTimer > 0.f; }

std::vector<PlayerBullet>& Player::getBullets() { return bullets; }

void Player::takeDamage(int damage) {
    if (godMode) return;
    if (invincibleTimer > 0.f) return;
    if (shieldTimer > 0.f) return;
    hp -= damage;
    if (hp < 0) hp = 0;
    invincibleTimer = PLAYER_INVINCIBLE_TIME;
}

void Player::heal(int amount) {
    hp += amount;
    if (hp > maxHP) hp = maxHP;
}

void Player::setGodMode(bool enabled) {
    godMode = enabled;
}

void Player::activateShieldOrb() {
    shieldTimer = SHIELD_ORB_DURATION;
}

void Player::activateOverdrive() {
    overdriveTimer = OVERDRIVE_DURATION;
}

void Player::activateDashBattery() {
    dashCooldown = 0.f;
    dashBatteryTimer = DASH_BATTERY_DURATION;
}

float Player::getShieldTimer() const { return shieldTimer; }
float Player::getOverdriveTimer() const { return overdriveTimer; }
float Player::getDashBatteryTimer() const { return dashBatteryTimer; }
float Player::getFireRateMultiplier() const { return (overdriveTimer > 0.f) ? 1.f / OVERDRIVE_FIRE_RATE_MULT : 1.f; }
float Player::getDamageMultiplier() const { return (overdriveTimer > 0.f) ? OVERDRIVE_DAMAGE_MULT : 1.f; }
float Player::getSpeedMultiplier() const { return (dashBatteryTimer > 0.f) ? DASH_BATTERY_SPEED_MULT : 1.f; }
float Player::getDashCooldown() const { return dashCooldown; }
bool Player::isDashing() const { return dashing; }
