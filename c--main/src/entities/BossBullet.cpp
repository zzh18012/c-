#include "BossBullet.h"
#include "core/Config.h"

BossBullet::BossBullet()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , active(false)
    , damage(BOSS_BULLET_DAMAGE)
{
    shape.setRadius(BOSS_BULLET_RADIUS);
    shape.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS, BOSS_BULLET_RADIUS));
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

    sf::Vector2f pos = shape.getPosition();

    // Outer glow
    sf::CircleShape outerGlow(BOSS_BULLET_RADIUS + 5.f);
    outerGlow.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS + 5.f, BOSS_BULLET_RADIUS + 5.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 80, 130, 50));
    window.draw(outerGlow);

    // Main organic plasma orb
    sf::CircleShape mainOrb(BOSS_BULLET_RADIUS);
    mainOrb.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS, BOSS_BULLET_RADIUS));
    mainOrb.setPosition(pos);
    mainOrb.setFillColor(sf::Color(255, 100, 160, 220));
    window.draw(mainOrb);

    // Inner bright core
    sf::CircleShape innerCore(BOSS_BULLET_RADIUS * 0.5f);
    innerCore.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS * 0.5f, BOSS_BULLET_RADIUS * 0.5f));
    innerCore.setPosition(pos);
    innerCore.setFillColor(sf::Color(255, 200, 220, 255));
    window.draw(innerCore);

    // Pulsing nucleus
    sf::CircleShape nucleus(BOSS_BULLET_RADIUS * 0.25f);
    nucleus.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS * 0.25f, BOSS_BULLET_RADIUS * 0.25f));
    nucleus.setPosition(pos);
    nucleus.setFillColor(sf::Color(255, 255, 255, 255));
    window.draw(nucleus);
}

bool BossBullet::isActive() const { return active; }
void BossBullet::deactivate() { active = false; }
sf::Vector2f BossBullet::getPosition() const { return position; }
float BossBullet::getRadius() const { return BOSS_BULLET_RADIUS; }
int BossBullet::getDamage() const { return damage; }