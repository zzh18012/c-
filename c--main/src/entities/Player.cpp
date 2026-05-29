#include "Player.h"
#include "core/Config.h"
#include "systems/AudioSystem.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>

Player::Player()
    : position(PLAYER_START_POS)
    , aimDir(0.f, -1.f)
    , shootCooldown(0.f)
    , spreadCooldown(0.f)
    , piercingCooldown(0.f)
    , orbitalCooldown(0.f)
    , clusterCooldown(0.f)
    , homingCooldown(0.f)
    , currentWeapon(WeaponType::Normal)
    , secondaryWeapon(WeaponType::Spread)
    , hp(PLAYER_MAX_HP)
    , maxHP(PLAYER_MAX_HP)
    , invincibleTimer(0.f)
    , godMode(false)
    , shieldTimer(0.f)
    , overdriveTimer(0.f)
    , dashBatteryTimer(0.f)
    , phaseShiftTimer(0.f)
    , novaFormTimer(0.f)
    , speedCoilTimer(0.f)
    , attackModuleTimer(0.f)
    , dashCooldown(0.f)
    , dashing(false)
    , dashTimer(0.f)
    , mechRotationAngle(0.f)
    , weaponRecoilTimer(0.f)
{
    bullets.resize(MAX_PLAYER_BULLETS);
    spreadBullets.resize(50);
    piercingBullets.resize(20);
    orbitalBullets.resize(4);
    clusterBullets.resize(20);
    homingBullets.resize(15);
}

void Player::handleInput(float dt, const sf::RenderWindow& window) {
    if (dashing) return;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
        static bool qPressed = false;
        if (!qPressed) {
            qPressed = true;
            cycleWeapon();
        }
    } else {
        static bool qPressed = false;
        qPressed = false;
    }

    sf::Vector2f move(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) move.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) move.y += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) move.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) move.x += 1.f;

    float len = std::sqrt(move.x * move.x + move.y * move.y);
    if (len > 0.f) {
        move /= len;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) && dashCooldown <= 0.f) {
        if (len > 0.f) {
            dashDir = move;
        } else {
            dashDir = aimDir;
        }
        dashing = true;
        dashTimer = PLAYER_DASH_DURATION;
        dashCooldown = PLAYER_DASH_COOLDOWN;

        if (static_cast<float>(rand()) / RAND_MAX < PHASE_SHIFT_TRIGGER_CHANCE) {
            phaseShiftTimer = PHASE_SHIFT_DURATION;
            invincibleTimer = PHASE_SHIFT_DURATION;
        }
    }

    if (!dashing) {
        float speed = PLAYER_SPEED;
        if (dashBatteryTimer > 0.f) {
            speed *= DASH_BATTERY_SPEED_MULT;
        }
        if (speedCoilTimer > 0.f) {
            speed *= SPEED_COIL_MULT;
        }
        position += move * speed * dt;
    }

    if (position.x < PLAYER_RADIUS) position.x = PLAYER_RADIUS;
    if (position.x > WINDOW_WIDTH - PLAYER_RADIUS) position.x = WINDOW_WIDTH - PLAYER_RADIUS;
    if (position.y < PLAYER_RADIUS) position.y = PLAYER_RADIUS;
    if (position.y > WINDOW_HEIGHT - PLAYER_RADIUS) position.y = WINDOW_HEIGHT - PLAYER_RADIUS;

    sf::Vector2i mousePos = sf::Mouse::getPosition();
    sf::Vector2f worldMouse(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    aimDir = worldMouse - position;
    float aimLen = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);
    if (aimLen > 0.f) {
        aimDir /= aimLen;
    }

    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        shoot();
    }
}

void Player::shoot() {
    float rateMult = (overdriveTimer > 0.f) ? OVERDRIVE_FIRE_RATE_MULT : 1.f;
    float dmgMult = (attackModuleTimer > 0.f) ? ATTACK_MODULE_MULT : 1.f;
    if (overdriveTimer > 0.f) dmgMult = OVERDRIVE_DAMAGE_MULT;

    switch (currentWeapon) {
        case WeaponType::Normal:
            if (shootCooldown > 0.f) return;
            shootCooldown = PLAYER_SHOOT_COOLDOWN * rateMult;
            for (auto& bullet : bullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    bullet.setDamage(static_cast<int>(PLAYER_BULLET_DAMAGE * dmgMult));
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;

        case WeaponType::Spread:
            if (spreadCooldown > 0.f) return;
            spreadCooldown = PLAYER_SPREAD_COOLDOWN * rateMult;
            {
                float baseAngle = std::atan2(aimDir.y, aimDir.x);
                int count = PLAYER_SPREAD_COUNT;
                float spreadAngle = PLAYER_SPREAD_ANGLE * 3.14159f / 180.f;
                float step = spreadAngle / (count - 1);
                float startAngle = baseAngle - spreadAngle * 0.5f;
                for (int i = 0; i < count; ++i) {
                    for (auto& bullet : spreadBullets) {
                        if (!bullet.isActive()) {
                            float angle = startAngle + step * i;
                            sf::Vector2f dir(std::cos(angle), std::sin(angle));
                            bullet.spawn(position, dir, 0.f);
                            if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                            break;
                        }
                    }
                }
            }
            break;

        case WeaponType::Piercing:
            if (piercingCooldown > 0.f) return;
            piercingCooldown = PLAYER_PIERCING_COOLDOWN * rateMult;
            for (auto& bullet : piercingBullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;

        case WeaponType::Orbital:
            for (auto& bullet : orbitalBullets) {
                if (!bullet.isActive()) {
                    float angleOffset = 3.14159f * 2.f / orbitalBullets.size() * (&bullet - &orbitalBullets[0]);
                    bullet.spawn(position, angleOffset, PLAYER_ORBITAL_RADIUS);
                }
            }
            break;

        case WeaponType::Cluster:
            if (clusterCooldown > 0.f) return;
            clusterCooldown = PLAYER_CLUSTER_COOLDOWN * rateMult;
            for (auto& bullet : clusterBullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;

        case WeaponType::Homing:
            if (homingCooldown > 0.f) return;
            homingCooldown = PLAYER_HOMING_COOLDOWN * rateMult;
            for (auto& bullet : homingBullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;
    }
}

void Player::update(float dt) {
    if (shootCooldown > 0.f) shootCooldown -= dt;
    if (spreadCooldown > 0.f) spreadCooldown -= dt;
    if (piercingCooldown > 0.f) piercingCooldown -= dt;
    if (orbitalCooldown > 0.f) orbitalCooldown -= dt;
    if (clusterCooldown > 0.f) clusterCooldown -= dt;
    if (homingCooldown > 0.f) homingCooldown -= dt;
    if (invincibleTimer > 0.f) invincibleTimer -= dt;
    if (shieldTimer > 0.f) shieldTimer -= dt;
    if (overdriveTimer > 0.f) overdriveTimer -= dt;
    if (dashBatteryTimer > 0.f) dashBatteryTimer -= dt;
    if (phaseShiftTimer > 0.f) phaseShiftTimer -= dt;
    if (novaFormTimer > 0.f) novaFormTimer -= dt;
    if (speedCoilTimer > 0.f) speedCoilTimer -= dt;
    if (attackModuleTimer > 0.f) attackModuleTimer -= dt;
    if (dashCooldown > 0.f) dashCooldown -= dt;

    if (weaponRecoilTimer > 0.f) weaponRecoilTimer -= dt;

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

    for (auto& bullet : bullets) bullet.update(dt);
    for (auto& bullet : spreadBullets) bullet.update(dt);
    for (auto& bullet : piercingBullets) bullet.update(dt);
    for (auto& bullet : orbitalBullets) bullet.update(dt, position);
    for (auto& bullet : clusterBullets) bullet.update(dt);
    for (auto& bullet : homingBullets) bullet.update(dt, position);
}

void Player::render(sf::RenderWindow& window) const {
    for (const auto& bullet : bullets) bullet.render(window);
    for (const auto& bullet : spreadBullets) bullet.render(window);
    for (const auto& bullet : piercingBullets) bullet.render(window);
    for (const auto& bullet : orbitalBullets) bullet.render(window);
    for (const auto& bullet : clusterBullets) bullet.render(window);
    for (const auto& bullet : homingBullets) bullet.render(window);

    renderMechEffects(window);
    renderMechBody(window);
    renderMechWeapon(window);
}

void Player::renderMechBody(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseAngle = std::atan2(aimDir.y, aimDir.x) * 180.f / 3.14159f;

    sf::Color bodyColor(30, 35, 50);
    sf::Color armorColor(55, 70, 100);
    if (phaseShiftTimer > 0.f) {
        bodyColor = sf::Color(80, 40, 120);
        armorColor = sf::Color(100, 60, 150);
    } else if (novaFormTimer > 0.f) {
        bodyColor = sf::Color(255, 100, 50);
        armorColor = sf::Color(255, 150, 80);
    }

    sf::ConvexShape coreFrame;
    coreFrame.setPointCount(6);
    coreFrame.setPoint(0, sf::Vector2f(0.f, -28.f));
    coreFrame.setPoint(1, sf::Vector2f(18.f, -8.f));
    coreFrame.setPoint(2, sf::Vector2f(18.f, 16.f));
    coreFrame.setPoint(3, sf::Vector2f(0.f, 26.f));
    coreFrame.setPoint(4, sf::Vector2f(-18.f, 16.f));
    coreFrame.setPoint(5, sf::Vector2f(-18.f, -8.f));
    coreFrame.setOrigin(sf::Vector2f(0.f, 0.f));
    coreFrame.setPosition(pos);
    coreFrame.setRotation(sf::degrees(baseAngle));
    coreFrame.setFillColor(bodyColor);

    sf::ConvexShape armorPlate;
    armorPlate.setPointCount(6);
    armorPlate.setPoint(0, sf::Vector2f(0.f, -22.f));
    armorPlate.setPoint(1, sf::Vector2f(13.f, -6.f));
    armorPlate.setPoint(2, sf::Vector2f(13.f, 12.f));
    armorPlate.setPoint(3, sf::Vector2f(0.f, 20.f));
    armorPlate.setPoint(4, sf::Vector2f(-13.f, 12.f));
    armorPlate.setPoint(5, sf::Vector2f(-13.f, -6.f));
    armorPlate.setOrigin(sf::Vector2f(0.f, 0.f));
    armorPlate.setPosition(pos);
    armorPlate.setRotation(sf::degrees(baseAngle));
    armorPlate.setFillColor(armorColor);

    sf::CircleShape core(8.f);
    core.setOrigin(sf::Vector2f(8.f, 8.f));
    core.setPosition(pos);
    if (phaseShiftTimer > 0.f) {
        core.setFillColor(sf::Color(200, 100, 255, 200));
    } else if (novaFormTimer > 0.f) {
        core.setFillColor(sf::Color(255, 200, 100, 200));
    } else {
        core.setFillColor(sf::Color(0, 220, 255, 200));
    }

    sf::CircleShape coreInner(4.f);
    coreInner.setOrigin(sf::Vector2f(4.f, 4.f));
    coreInner.setPosition(pos);
    coreInner.setFillColor(sf::Color(150, 255, 255, 255));

    sf::ConvexShape shoulderR;
    shoulderR.setPointCount(5);
    shoulderR.setPoint(0, sf::Vector2f(8.f, -20.f));
    shoulderR.setPoint(1, sf::Vector2f(24.f, -32.f));
    shoulderR.setPoint(2, sf::Vector2f(30.f, -24.f));
    shoulderR.setPoint(3, sf::Vector2f(16.f, -14.f));
    shoulderR.setPoint(4, sf::Vector2f(12.f, -18.f));
    shoulderR.setOrigin(sf::Vector2f(0.f, 0.f));
    shoulderR.setPosition(pos);
    shoulderR.setRotation(sf::degrees(baseAngle));
    shoulderR.setFillColor(sf::Color(45, 58, 80));

    sf::ConvexShape shoulderL;
    shoulderL.setPointCount(5);
    shoulderL.setPoint(0, sf::Vector2f(-8.f, -20.f));
    shoulderL.setPoint(1, sf::Vector2f(-24.f, -32.f));
    shoulderL.setPoint(2, sf::Vector2f(-30.f, -24.f));
    shoulderL.setPoint(3, sf::Vector2f(-16.f, -14.f));
    shoulderL.setPoint(4, sf::Vector2f(-12.f, -18.f));
    shoulderL.setOrigin(sf::Vector2f(0.f, 0.f));
    shoulderL.setPosition(pos);
    shoulderL.setRotation(sf::degrees(baseAngle));
    shoulderL.setFillColor(sf::Color(45, 58, 80));

    sf::RectangleShape shoulderLineR(sf::Vector2f(16.f, 2.f));
    shoulderLineR.setOrigin(sf::Vector2f(8.f, 1.f));
    shoulderLineR.setPosition(pos);
    shoulderLineR.setRotation(sf::degrees(baseAngle + 25.f));
    shoulderLineR.setFillColor(sf::Color(0, 180, 255, 150));

    sf::RectangleShape shoulderLineL(sf::Vector2f(16.f, 2.f));
    shoulderLineL.setOrigin(sf::Vector2f(8.f, 1.f));
    shoulderLineL.setPosition(pos);
    shoulderLineL.setRotation(sf::degrees(baseAngle - 25.f));
    shoulderLineL.setFillColor(sf::Color(0, 180, 255, 150));

    sf::RectangleShape thrusterL(sf::Vector2f(6.f, 12.f));
    thrusterL.setOrigin(sf::Vector2f(3.f, 6.f));
    thrusterL.setPosition(sf::Vector2f(pos.x - 8.f, pos.y + 18.f));
    thrusterL.setRotation(sf::degrees(baseAngle));
    thrusterL.setFillColor(sf::Color(25, 30, 45));

    sf::RectangleShape thrusterR(sf::Vector2f(6.f, 12.f));
    thrusterR.setOrigin(sf::Vector2f(3.f, 6.f));
    thrusterR.setPosition(sf::Vector2f(pos.x + 8.f, pos.y + 18.f));
    thrusterR.setRotation(sf::degrees(baseAngle));
    thrusterR.setFillColor(sf::Color(25, 30, 45));

    if (overdriveTimer > 0.f) {
        sf::CircleShape thrusterGlowL(5.f);
        thrusterGlowL.setOrigin(sf::Vector2f(5.f, 5.f));
        thrusterGlowL.setPosition(sf::Vector2f(pos.x - 8.f, pos.y + 22.f));
        thrusterGlowL.setFillColor(sf::Color(255, 100, 50, 200));

        sf::CircleShape thrusterGlowR(5.f);
        thrusterGlowR.setOrigin(sf::Vector2f(5.f, 5.f));
        thrusterGlowR.setPosition(sf::Vector2f(pos.x + 8.f, pos.y + 22.f));
        thrusterGlowR.setFillColor(sf::Color(255, 100, 50, 200));

        window.draw(thrusterGlowL);
        window.draw(thrusterGlowR);
    }

    if (shieldTimer > 0.f) {
        sf::CircleShape shield(PLAYER_RADIUS + 10.f);
        shield.setOrigin(sf::Vector2f(PLAYER_RADIUS + 10.f, PLAYER_RADIUS + 10.f));
        shield.setPosition(pos);
        shield.setFillColor(sf::Color(0, 255, 255, 30));
        shield.setOutlineColor(sf::Color(0, 255, 255, 150));
        shield.setOutlineThickness(2.f);
        window.draw(shield);

        sf::CircleShape shieldInner(PLAYER_RADIUS + 4.f);
        shieldInner.setOrigin(sf::Vector2f(PLAYER_RADIUS + 4.f, PLAYER_RADIUS + 4.f));
        shieldInner.setPosition(pos);
        shieldInner.setFillColor(sf::Color(0, 255, 255, 15));
        shieldInner.setOutlineColor(sf::Color(0, 255, 255, 80));
        shieldInner.setOutlineThickness(1.f);
        window.draw(shieldInner);
    }

    window.draw(coreFrame);
    window.draw(armorPlate);
    window.draw(thrusterL);
    window.draw(thrusterR);
    window.draw(shoulderR);
    window.draw(shoulderL);
    window.draw(core);
    window.draw(coreInner);
    window.draw(shoulderLineR);
    window.draw(shoulderLineL);
}

void Player::renderMechWeapon(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    float baseAngle = std::atan2(aimDir.y, aimDir.x) * 180.f / 3.14159f;

    float recoil = weaponRecoilTimer > 0.f ? weaponRecoilTimer * 8.f : 0.f;

    sf::Color cannonColor(40, 50, 70);
    sf::Color barrelColor(25, 35, 55);
    if (phaseShiftTimer > 0.f) {
        cannonColor = sf::Color(80, 50, 100);
        barrelColor = sf::Color(60, 30, 80);
    } else if (novaFormTimer > 0.f) {
        cannonColor = sf::Color(150, 80, 40);
        barrelColor = sf::Color(120, 60, 30);
    }

    sf::RectangleShape cannon(sf::Vector2f(28.f - recoil, 10.f));
    cannon.setOrigin(sf::Vector2f(28.f - recoil, 5.f));
    cannon.setPosition(pos);
    cannon.setRotation(sf::degrees(baseAngle));
    cannon.setFillColor(cannonColor);

    sf::RectangleShape barrel(sf::Vector2f(12.f - recoil * 0.5f, 6.f));
    barrel.setOrigin(sf::Vector2f(12.f - recoil * 0.5f, 3.f));
    barrel.setPosition(pos);
    barrel.setRotation(sf::degrees(baseAngle));
    barrel.setFillColor(barrelColor);

    sf::CircleShape cannonTip(4.f);
    cannonTip.setOrigin(sf::Vector2f(4.f, 4.f));
    float tipDist = 24.f - recoil;
    float tipX = pos.x + aimDir.x * tipDist;
    float tipY = pos.y + aimDir.y * tipDist;
    cannonTip.setPosition(sf::Vector2f(tipX, tipY));
    if (phaseShiftTimer > 0.f) {
        cannonTip.setFillColor(sf::Color(200, 100, 255, 200));
    } else if (novaFormTimer > 0.f) {
        cannonTip.setFillColor(sf::Color(255, 200, 100, 200));
    } else {
        cannonTip.setFillColor(sf::Color(0, 200, 255, 200));
    }

    if (overdriveTimer > 0.f) {
        sf::CircleShape chargeGlow(7.f);
        chargeGlow.setOrigin(sf::Vector2f(7.f, 7.f));
        chargeGlow.setPosition(sf::Vector2f(tipX, tipY));
        chargeGlow.setFillColor(sf::Color(255, 150, 50, 180));
        window.draw(chargeGlow);
    }

    sf::RectangleShape cannonSleeve(sf::Vector2f(8.f, 14.f));
    cannonSleeve.setOrigin(sf::Vector2f(4.f, 7.f));
    cannonSleeve.setPosition(pos);
    cannonSleeve.setRotation(sf::degrees(baseAngle));
    cannonSleeve.setFillColor(sf::Color(50, 65, 90));

    window.draw(cannon);
    window.draw(barrel);
    window.draw(cannonTip);
    window.draw(cannonSleeve);
}

void Player::renderMechEffects(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;

    if (dashing) {
        for (int i = 0; i < 3; i++) {
            float alpha = (3.f - static_cast<float>(i)) / 3.f * 80.f;
            sf::ConvexShape ghost;
            ghost.setPointCount(6);
            ghost.setPoint(0, sf::Vector2f(0.f, -22.f));
            ghost.setPoint(1, sf::Vector2f(13.f, -6.f));
            ghost.setPoint(2, sf::Vector2f(13.f, 12.f));
            ghost.setPoint(3, sf::Vector2f(0.f, 20.f));
            ghost.setPoint(4, sf::Vector2f(-13.f, 12.f));
            ghost.setPoint(5, sf::Vector2f(-13.f, -6.f));
            ghost.setOrigin(sf::Vector2f(0.f, 0.f));
            ghost.setPosition(sf::Vector2f(pos.x - dashDir.x * (i + 1) * 12.f, pos.y - dashDir.y * (i + 1) * 12.f));
            ghost.setFillColor(sf::Color(0, 180, 255, static_cast<std::uint8_t>(alpha)));
            window.draw(ghost);
        }
    }

    if (phaseShiftTimer > 0.f) {
        for (int i = 0; i < 5; i++) {
            float alpha = (5.f - static_cast<float>(i)) / 5.f * 60.f;
            float offset = static_cast<float>(i) * 8.f;
            sf::CircleShape ghost(PLAYER_RADIUS);
            ghost.setPosition(sf::Vector2f(pos.x + offset, pos.y + offset));
            ghost.setOrigin(sf::Vector2f(PLAYER_RADIUS, PLAYER_RADIUS));
            ghost.setFillColor(sf::Color(150, 50, 200, static_cast<std::uint8_t>(alpha)));
            window.draw(ghost);
        }
    }

    if (overdriveTimer > 0.f) {
        float pulsePhase = overdriveTimer * 4.f;
        float pulseRadius = (1.f - overdriveTimer / OVERDRIVE_DURATION) * 40.f + 10.f;
        float pulseAlpha = std::fmax(0.f, std::sin(pulsePhase * 3.14159f) * 100.f);

        sf::CircleShape pulseRing(pulseRadius);
        pulseRing.setOrigin(sf::Vector2f(pulseRadius, pulseRadius));
        pulseRing.setPosition(pos);
        pulseRing.setFillColor(sf::Color(255, 100, 50, static_cast<std::uint8_t>(pulseAlpha)));
        window.draw(pulseRing);
    }

    if (novaFormTimer > 0.f) {
        float pulseRadius = NOVA_FORM_RADIUS * (1.f - novaFormTimer / NOVA_FORM_DURATION);
        float pulseAlpha = novaFormTimer / NOVA_FORM_DURATION * 150.f;

        sf::CircleShape novaRing(pulseRadius);
        novaRing.setOrigin(sf::Vector2f(pulseRadius, pulseRadius));
        novaRing.setPosition(pos);
        novaRing.setFillColor(sf::Color(255, 150, 50, static_cast<std::uint8_t>(pulseAlpha)));
        novaRing.setOutlineColor(sf::Color(255, 200, 100, static_cast<std::uint8_t>(pulseAlpha * 1.5f)));
        novaRing.setOutlineThickness(5.f);
        window.draw(novaRing);
    }

    if (invincibleTimer > 0.f) {
        float flicker = std::sin(invincibleTimer * 30.f) * 0.5f + 0.5f;
        if (flicker > 0.5f) {
            sf::CircleShape invincibleAura(PLAYER_RADIUS + 5.f);
            invincibleAura.setOrigin(sf::Vector2f(PLAYER_RADIUS + 5.f, PLAYER_RADIUS + 5.f));
            invincibleAura.setPosition(pos);
            invincibleAura.setFillColor(sf::Color(255, 255, 255, 60));
            window.draw(invincibleAura);
        }
    }
}

sf::Vector2f Player::getPosition() const { return position; }
float Player::getRadius() const { return PLAYER_RADIUS; }
int Player::getHP() const { return hp; }
int Player::getMaxHP() const { return maxHP; }
bool Player::isDead() const { return hp <= 0; }
bool Player::isInvincible() const { return invincibleTimer > 0.f; }

std::vector<PlayerBullet>& Player::getBullets() { return bullets; }
std::vector<PlayerBulletSpread>& Player::getSpreadBullets() { return spreadBullets; }
std::vector<PlayerBulletPiercing>& Player::getPiercingBullets() { return piercingBullets; }
std::vector<PlayerBulletOrbital>& Player::getOrbitalBullets() { return orbitalBullets; }
std::vector<PlayerBulletCluster>& Player::getClusterBullets() { return clusterBullets; }
std::vector<PlayerBulletHoming>& Player::getHomingBullets() { return homingBullets; }

WeaponType Player::getCurrentWeapon() const { return currentWeapon; }

void Player::takeDamage(int damage) {
    if (godMode) return;
    if (invincibleTimer > 0.f) return;
    if (phaseShiftTimer > 0.f) return;
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

void Player::activatePhaseShift() {
    phaseShiftTimer = PHASE_SHIFT_DURATION;
    invincibleTimer = PHASE_SHIFT_DURATION;
}

void Player::activateNovaForm() {
    novaFormTimer = NOVA_FORM_DURATION;
}

void Player::activateSpeedCoil() {
    speedCoilTimer = SPEED_COIL_DURATION;
}

void Player::activateAttackBoost() {
    attackModuleTimer = ATTACK_MODULE_DURATION;
}

void Player::cycleWeapon() {
    WeaponType current = currentWeapon;
    if (current == WeaponType::Normal) currentWeapon = secondaryWeapon;
    else {
        secondaryWeapon = current;
        currentWeapon = WeaponType::Normal;
    }
}

float Player::getShieldTimer() const { return shieldTimer; }
float Player::getOverdriveTimer() const { return overdriveTimer; }
float Player::getDashBatteryTimer() const { return dashBatteryTimer; }
float Player::getFireRateMultiplier() const { return (overdriveTimer > 0.f) ? 1.f / OVERDRIVE_FIRE_RATE_MULT : 1.f; }
float Player::getDamageMultiplier() const {
    if (overdriveTimer > 0.f) return OVERDRIVE_DAMAGE_MULT;
    if (attackModuleTimer > 0.f) return ATTACK_MODULE_MULT;
    return 1.f;
}
float Player::getSpeedMultiplier() const {
    float mult = 1.f;
    if (dashBatteryTimer > 0.f) mult *= DASH_BATTERY_SPEED_MULT;
    if (speedCoilTimer > 0.f) mult *= SPEED_COIL_MULT;
    return mult;
}
float Player::getDashCooldown() const { return dashCooldown; }
bool Player::isDashing() const { return dashing; }
bool Player::isPhaseShifting() const { return phaseShiftTimer > 0.f; }
bool Player::isNovaFormActive() const { return novaFormTimer > 0.f; }
float Player::getPhaseShiftTimer() const { return phaseShiftTimer; }
