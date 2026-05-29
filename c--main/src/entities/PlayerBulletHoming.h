#pragma once
#include <SFML/Graphics.hpp>

class PlayerBulletHoming {
public:
    PlayerBulletHoming();
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    void update(float dt, const sf::Vector2f& targetPos);
    void render(sf::RenderWindow& window) const;
    bool isActive() const;
    void deactivate();
    void setDamage(int dmg);
    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getDamage() const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    float angle;
    sf::CircleShape shape;
    bool active;
    int damage;
};
