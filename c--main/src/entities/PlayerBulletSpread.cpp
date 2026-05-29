#include "PlayerBulletSpread.h"
#include "core/Config.h"
#include <cmath>

PlayerBulletSpread::PlayerBulletSpread()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , shape(PLAYER_BULLET_RADIUS)
    , active(false)
    , damage(PLAYER_BULLET_DAMAGE)
{
    shape.setFillColor(sf::Color(0, 200, 255));
}

void PlayerBulletSpread::spawn(sf::Vector2f pos, sf::Vector2f dir, float angleOffset) {
    position = pos;
    float speed = PLAYER_BULLET_SPEED;
    float angle = std::atan2(dir.y, dir.x) + angleOffset;
    velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
    active = true;
    damage = PLAYER_BULLET_DAMAGE;
}

void PlayerBulletSpread::update(float dt) {
    if (!active) return;
    position += velocity * dt;
    shape.setPosition(position);
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

void PlayerBulletSpread::render(sf::RenderWindow& window) const {
    if (!active) return;
    sf::CircleShape outer(shape.getRadius() * 2.f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 2.f, shape.getRadius() * 2.f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(0, 150, 200, 80));
    window.draw(outer);

    window.draw(shape);

    sf::CircleShape inner(shape.getRadius() * 0.5f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.5f, shape.getRadius() * 0.5f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(200, 255, 255, 255));
    window.draw(inner);
}

bool PlayerBulletSpread::isActive() const { return active; }
void PlayerBulletSpread::deactivate() { active = false; }
sf::Vector2f PlayerBulletSpread::getPosition() const { return position; }
float PlayerBulletSpread::getRadius() const { return shape.getRadius(); }
int PlayerBulletSpread::getDamage() const { return damage; }
