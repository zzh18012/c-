#pragma once
#include <SFML/Graphics.hpp>

class PlayerBulletSpread {
public:
    PlayerBulletSpread();
    void spawn(sf::Vector2f pos, sf::Vector2f dir, float angleOffset);
    void update(float dt);
    void render(sf::RenderWindow& window) const;
    bool isActive() const;
    void deactivate();
    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getDamage() const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::CircleShape shape;
    bool active;
    int damage;
};
