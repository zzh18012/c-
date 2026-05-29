#include "PlayerBulletCluster.h"
#include "core/Config.h"
#include <cmath>

PlayerBulletCluster::PlayerBulletCluster()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , shape(PLAYER_BULLET_RADIUS * 1.5f)
    , active(false)
    , exploded(false)
    , damage(PLAYER_BULLET_DAMAGE)
    , lifetime(0.f)
    , splitSpawned(false)
    , splitCount(0)
{
    shape.setFillColor(sf::Color(255, 150, 50));
}

void PlayerBulletCluster::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;
    velocity = dir * PLAYER_CLUSTER_SPEED;
    active = true;
    exploded = false;
    damage = PLAYER_BULLET_DAMAGE;
    lifetime = 0.f;
    splitSpawned = false;
    splitCount = 0;
}

void PlayerBulletCluster::update(float dt) {
    if (!active) return;
    position += velocity * dt;
    velocity.y += 200.f * dt;
    lifetime += dt;
    shape.setPosition(position);

    if (lifetime > 0.5f && !exploded) {
        triggerExplosion();
    }

    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

void PlayerBulletCluster::triggerExplosion() {
    exploded = true;
    splitCount = CLUSTER_SPLIT_COUNT;
}

void PlayerBulletCluster::render(sf::RenderWindow& window) const {
    if (!active) return;

    sf::CircleShape outer(shape.getRadius() * 2.f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 2.f, shape.getRadius() * 2.f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(255, 100, 30, 100));
    window.draw(outer);

    window.draw(shape);

    sf::CircleShape inner(shape.getRadius() * 0.5f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.5f, shape.getRadius() * 0.5f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(255, 220, 150));
    window.draw(inner);

    if (exploded && !splitSpawned) {
        for (int i = 0; i < splitCount; ++i) {
            float angle = 3.14159f * 2.f * i / splitCount + 0.3f;
            sf::Vector2f offset(std::cos(angle) * PLAYER_BULLET_RADIUS * 2.f, std::sin(angle) * PLAYER_BULLET_RADIUS * 2.f);
            sf::CircleShape splitBullet(PLAYER_BULLET_RADIUS * 0.6f);
            splitBullet.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS * 0.6f, PLAYER_BULLET_RADIUS * 0.6f));
            splitBullet.setPosition(position + offset);
            splitBullet.setFillColor(sf::Color(255, 200, 80, 220));
            window.draw(splitBullet);
        }
    }
}

bool PlayerBulletCluster::isActive() const { return active; }
void PlayerBulletCluster::deactivate() { active = false; }
bool PlayerBulletCluster::shouldExplode() const { return lifetime > 0.5f && !exploded; }
bool PlayerBulletCluster::hasExploded() const { return exploded; }
sf::Vector2f PlayerBulletCluster::getPosition() const { return position; }
float PlayerBulletCluster::getRadius() const { return shape.getRadius(); }
int PlayerBulletCluster::getDamage() const { return damage; }
bool PlayerBulletCluster::hasSpawnedSplit() const { return splitSpawned; }
void PlayerBulletCluster::setSplitSpawned() { splitSpawned = true; }
void PlayerBulletCluster::setDamage(int dmg) { damage = dmg; }
sf::Vector2f PlayerBulletCluster::getSplitDirection(int index) const {
    float angle = 3.14159f * 2.f * index / splitCount + 0.3f;
    return sf::Vector2f(std::cos(angle), std::sin(angle));
}