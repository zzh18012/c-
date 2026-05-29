#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

#include "core/Config.h"

class Player;

class PlayerBulletCluster {
public:
    PlayerBulletCluster();
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    void update(float dt);
    void render(sf::RenderWindow& window) const;
    bool isActive() const;
    void deactivate();
    bool shouldExplode() const;
    void triggerExplosion();
    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getDamage() const;
    bool hasExploded() const;
    bool hasSpawnedSplit() const;
    void setSplitSpawned();
    void setDamage(int dmg);
    sf::Vector2f getSplitDirection(int index) const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::CircleShape shape;
    bool active;
    bool exploded;
    int damage;
    float lifetime;

    bool splitSpawned;
    int splitCount;
};