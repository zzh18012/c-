#include "PlayerBullet.h"
#include "core/Config.h"

PlayerBullet::PlayerBullet()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , active(false)
    , damage(PLAYER_BULLET_DAMAGE)
{
    shape.setRadius(PLAYER_BULLET_RADIUS);
    shape.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS, PLAYER_BULLET_RADIUS));
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

    sf::Vector2f pos = shape.getPosition();

    // Glow outer
    sf::CircleShape glow(PLAYER_BULLET_RADIUS + 3.f);
    glow.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS + 3.f, PLAYER_BULLET_RADIUS + 3.f));
    glow.setPosition(pos);
    glow.setFillColor(sf::Color(0, 255, 200, 60));
    window.draw(glow);

    // Core
    sf::CircleShape core(PLAYER_BULLET_RADIUS);
    core.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS, PLAYER_BULLET_RADIUS));
    core.setPosition(pos);
    core.setFillColor(sf::Color(150, 255, 230, 255));
    window.draw(core);

    // Bright center
    sf::CircleShape bright(PLAYER_BULLET_RADIUS * 0.4f);
    bright.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS * 0.4f, PLAYER_BULLET_RADIUS * 0.4f));
    bright.setPosition(pos);
    bright.setFillColor(sf::Color(255, 255, 255, 255));
    window.draw(bright);
}

void PlayerBullet::setDamage(int dmg) { damage = dmg; }
bool PlayerBullet::isActive() const { return active; }
void PlayerBullet::deactivate() { active = false; }
sf::Vector2f PlayerBullet::getPosition() const { return position; }
float PlayerBullet::getRadius() const { return PLAYER_BULLET_RADIUS; }
int PlayerBullet::getDamage() const { return damage; }