#include "PlayerBullet.h"
#include "core/Config.h"

PlayerBullet::PlayerBullet()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , active(false)
    , damage(PLAYER_BULLET_DAMAGE)
{
    shape.setRadius(PLAYER_BULLET_RADIUS);
    shape.setOrigin(PLAYER_BULLET_RADIUS, PLAYER_BULLET_RADIUS);
    shape.setFillColor(sf::Color(0, 255, 200, 230));
}

void PlayerBullet::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;
    velocity = dir * PLAYER_BULLET_SPEED;
    active = true;
    shape.setPosition(position);
}

void PlayerBullet::update(float dt) {
    if (!active) return;
    position += velocity * dt;
    shape.setPosition(position);

    if (position.x < -10 || position.x > WINDOW_WIDTH + 10 ||
        position.y < -10 || position.y > WINDOW_HEIGHT + 10) {
        active = false;
    }
}

void PlayerBullet::render(sf::RenderWindow& window) const {
    if (!active) return;
    window.draw(shape);
}

void PlayerBullet::setDamage(int dmg) { damage = dmg; }
bool PlayerBullet::isActive() const { return active; }
void PlayerBullet::deactivate() { active = false; }
sf::Vector2f PlayerBullet::getPosition() const { return position; }
float PlayerBullet::getRadius() const { return PLAYER_BULLET_RADIUS; }
int PlayerBullet::getDamage() const { return damage; }
