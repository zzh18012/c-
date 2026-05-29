#pragma once
#include <SFML/Graphics.hpp>

class PlayerBulletPiercing {
public:
    PlayerBulletPiercing();
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    void update(float dt);
    void render(sf::RenderWindow& window) const;
    bool isActive() const;
    void deactivate();
    sf::Vector2f getPosition() const;
    float getRadius() const;
    int getDamage() const;
    bool hasChained() const;
    void triggerChain();
    sf::Vector2f getChainTarget() const;
    float getChainTimer() const;

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::RectangleShape shape;
    bool active;
    int damage;
    float angle;
    bool chained;
    sf::Vector2f chainStartPos;
    sf::Vector2f chainDir;
    float chainTimer;
};
