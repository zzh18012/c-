#include "BossBullet.h"
#include "core/Config.h"

BossBullet::BossBullet()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , active(false)
    , damage(BOSS_BULLET_DAMAGE)
{
    shape.setRadius(BOSS_BULLET_RADIUS);
    shape.setOrigin(BOSS_BULLET_RADIUS, BOSS_BULLET_RADIUS);
    shape.setFillColor(sf::Color(255, 80, 130, 230));
    shape.setOutlineColor(sf::Color(255, 100, 150));
    shape.setOutlineThickness(1.f);
}

void BossBullet::spawn(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg) {
    position = pos;
    velocity = dir * speed;
    damage = dmg;
    active = true;
    shape.setPosition(position);
}

void BossBullet::update(float dt) {
    if (!active) return;
    position += velocity * dt;
    shape.setPosition(position);

    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

void BossBullet::render(sf::RenderWindow& window) const {
    if (!active) return;
    window.draw(shape);
}

bool BossBullet::isActive() const { return active; }
void BossBullet::deactivate() { active = false; }
sf::Vector2f BossBullet::getPosition() const { return position; }
float BossBullet::getRadius() const { return BOSS_BULLET_RADIUS; }
int BossBullet::getDamage() const { return damage; }
