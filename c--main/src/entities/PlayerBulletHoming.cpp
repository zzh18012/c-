#include "PlayerBulletHoming.h"
#include "core/Config.h"
#include <cmath>

PlayerBulletHoming::PlayerBulletHoming()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , angle(-90.f)
    , shape(PLAYER_BULLET_RADIUS * 0.8f)
    , active(false)
    , damage(PLAYER_HOMING_DAMAGE)
{
    shape.setFillColor(sf::Color(255, 100, 200));
}

void PlayerBulletHoming::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;
    float speed = PLAYER_BULLET_SPEED * 0.6f;
    velocity = dir * speed;
    angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;
    active = true;
    damage = PLAYER_HOMING_DAMAGE;
}

void PlayerBulletHoming::update(float dt, const sf::Vector2f& targetPos) {
    if (!active) return;

    sf::Vector2f toTarget = targetPos - position;
    float targetAngle = std::atan2(toTarget.y, toTarget.x);

    float currentAngle = std::atan2(velocity.y, velocity.x);
    float angleDiff = targetAngle - currentAngle;

    while (angleDiff > 3.14159f) angleDiff -= 2.f * 3.14159f;
    while (angleDiff < -3.14159f) angleDiff += 2.f * 3.14159f;

    float turn = std::max(-HOMING_TURN_RATE, std::min(HOMING_TURN_RATE, angleDiff * 0.05f));
    float newAngle = currentAngle + turn;
    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    speed = std::min(speed + 50.f * dt, PLAYER_BULLET_SPEED * 0.8f);
    velocity = sf::Vector2f(std::cos(newAngle) * speed, std::sin(newAngle) * speed);

    position += velocity * dt;
    angle = newAngle * 180.f / 3.14159f;
    shape.setPosition(position);

    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

void PlayerBulletHoming::render(sf::RenderWindow& window) const {
    if (!active) return;

    sf::CircleShape trail(shape.getRadius() * 3.f);
    trail.setOrigin(sf::Vector2f(shape.getRadius() * 3.f, shape.getRadius() * 3.f));
    trail.setPosition(sf::Vector2f(position.x - velocity.x * 0.02f, position.y - velocity.y * 0.02f));
    trail.setFillColor(sf::Color(255, 50, 150, 80));
    window.draw(trail);

    sf::CircleShape outer(shape.getRadius() * 1.5f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 1.5f, shape.getRadius() * 1.5f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(255, 100, 200, 120));
    window.draw(outer);

    window.draw(shape);

    sf::CircleShape inner(shape.getRadius() * 0.4f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.4f, shape.getRadius() * 0.4f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(255, 220, 255));
    window.draw(inner);
}

bool PlayerBulletHoming::isActive() const { return active; }
void PlayerBulletHoming::deactivate() { active = false; }
sf::Vector2f PlayerBulletHoming::getPosition() const { return position; }
float PlayerBulletHoming::getRadius() const { return shape.getRadius(); }
int PlayerBulletHoming::getDamage() const { return damage; }
