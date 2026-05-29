#include "PlayerBulletPiercing.h"
#include "core/Config.h"
#include <cmath>

PlayerBulletPiercing::PlayerBulletPiercing()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , shape(sf::Vector2f(12.f, 4.f))
    , active(false)
    , damage(PLAYER_PIERCING_DAMAGE)
    , angle(0.f)
    , chained(false)
    , chainTimer(0.f)
{
    shape.setFillColor(sf::Color(100, 255, 200));
}

void PlayerBulletPiercing::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;
    velocity = dir * PLAYER_PIERCING_SPEED;
    angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;
    active = true;
    damage = PLAYER_PIERCING_DAMAGE;
    chained = false;
    chainStartPos = pos;
    chainDir = dir;
    chainTimer = 0.f;
}

void PlayerBulletPiercing::update(float dt) {
    if (!active) return;
    position += velocity * dt;
    shape.setPosition(position);
    shape.setRotation(sf::degrees(angle));
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

void PlayerBulletPiercing::render(sf::RenderWindow& window) const {
    if (!active) return;

    sf::RectangleShape glowShape(shape.getSize() * 1.5f);
    glowShape.setOrigin(sf::Vector2f(shape.getSize().x * 0.75f, shape.getSize().y * 0.75f));
    glowShape.setPosition(position);
    glowShape.setRotation(shape.getRotation());
    glowShape.setFillColor(sf::Color(100, 255, 200, 100));
    window.draw(glowShape);

    window.draw(shape);

    sf::RectangleShape coreShape(sf::Vector2f(shape.getSize().x * 0.6f, shape.getSize().y * 0.4f));
    coreShape.setOrigin(sf::Vector2f(coreShape.getSize().x * 0.5f, coreShape.getSize().y * 0.5f));
    coreShape.setPosition(position);
    coreShape.setRotation(shape.getRotation());
    coreShape.setFillColor(sf::Color(200, 255, 240));
    window.draw(coreShape);
}

bool PlayerBulletPiercing::isActive() const { return active; }
void PlayerBulletPiercing::deactivate() { active = false; }
sf::Vector2f PlayerBulletPiercing::getPosition() const { return position; }
float PlayerBulletPiercing::getRadius() const { return shape.getSize().y * 0.5f; }
int PlayerBulletPiercing::getDamage() const { return damage; }
bool PlayerBulletPiercing::hasChained() const { return chained; }
void PlayerBulletPiercing::triggerChain() { chained = true; }
sf::Vector2f PlayerBulletPiercing::getChainTarget() const {
    return chainStartPos + chainDir * PIERCE_CHAIN_RADIUS;
}
float PlayerBulletPiercing::getChainTimer() const { return chainTimer; }