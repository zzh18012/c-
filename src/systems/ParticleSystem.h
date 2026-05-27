#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    bool active = false;
};

class ParticleSystem {
public:
    ParticleSystem();
    void spawnExplosion(sf::Vector2f position, sf::Color color, int count);
    void spawnHitSpark(sf::Vector2f position, sf::Color color);
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    std::vector<Particle> particles;
};
