#include "PlayerBulletOrbital.h"
#include "core/Config.h"
#include <cmath>

PlayerBulletOrbital::PlayerBulletOrbital()
    : position(0.f, 0.f)
    , angleOffset(0.f)
    , orbitRadius(PLAYER_ORBITAL_RADIUS)
    , orbitSpeed(PLAYER_ORBITAL_SPEED)
    , shape(8.f)
    , active(false)
    , damage(ORBITAL_BULLET_DAMAGE)
{
    shape.setFillColor(sf::Color(0, 255, 180));
}

void PlayerBulletOrbital::spawn(sf::Vector2f center, float angleOff, float radius) {
    angleOffset = angleOff;
    orbitRadius = radius;
    orbitSpeed = PLAYER_ORBITAL_SPEED;
    position.x = center.x + std::cos(angleOffset) * orbitRadius;
    position.y = center.y + std::sin(angleOffset) * orbitRadius;
    active = true;
    damage = ORBITAL_BULLET_DAMAGE;
}

void PlayerBulletOrbital::update(float dt, const sf::Vector2f& playerPos) {
    if (!active) return;
    angleOffset += orbitSpeed * dt / orbitRadius;
    position.x = playerPos.x + std::cos(angleOffset) * orbitRadius;
    position.y = playerPos.y + std::sin(angleOffset) * orbitRadius;
    shape.setPosition(position);
}

void PlayerBulletOrbital::render(sf::RenderWindow& window) const {
    if (!active) return;

    sf::CircleShape outer(shape.getRadius() * 2.f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 2.f, shape.getRadius()) * 2.f);
    outer.setPosition(position);
    outer.setFillColor(sf::Color(0, 200, 150, 80));
    window.draw(outer);

    sf::CircleShape mid(shape.getRadius() * 1.3f);
    mid.setOrigin(sf::Vector2f(shape.getRadius() * 1.3f, shape.getRadius()) * 1.3f);
    mid.setPosition(position);
    mid.setFillColor(sf::Color(0, 255, 180, 150));
    window.draw(mid);

    window.draw(shape);

    sf::CircleShape inner(shape.getRadius() * 0.4f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.4f, shape.getRadius()) * 0.4f);
    inner.setPosition(position);
    inner.setFillColor(sf::Color(200, 255, 240));
    window.draw(inner);
}

bool PlayerBulletOrbital::isActive() const { return active; }
void PlayerBulletOrbital::deactivate() { active = false; }
sf::Vector2f PlayerBulletOrbital::getPosition() const { return position; }
float PlayerBulletOrbital::getRadius() const { return shape.getRadius(); }
int PlayerBulletOrbital::getDamage() const { return damage; }
