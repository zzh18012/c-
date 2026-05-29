#pragma once
#include <SFML/Graphics.hpp>

class PlayerBulletOrbital {
public:
    PlayerBulletOrbital();
    void spawn(sf::Vector2f center, float angleOffset, float radius);
    void update(float dt, const sf::Vector2f& playerPos);
    void render(sf::RenderWindow& window) const;
    bool isActive() const;
    void deactivate();
    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getDamage() const;

private:
    sf::Vector2f position;
    float angleOffset;
    float orbitRadius;
    float orbitSpeed;
    sf::CircleShape shape;
    bool active;
    int damage;
};
