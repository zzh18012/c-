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
    void spawnOrbitParticles(sf::Vector2f center, sf::Color color, int count, float orbitRadius);
    void spawnLightning(sf::Vector2f from, sf::Vector2f to, sf::Color color);
    void spawnNovaRing(sf::Vector2f center, float radius, sf::Color color, int count);
    void spawnDashTrail(sf::Vector2f position, sf::Color color);
    void spawnMechThrust(sf::Vector2f position, sf::Vector2f dir, sf::Color color);
    void spawnBulletTrail(sf::Vector2f position, sf::Color color);
    void spawnHealEffect(sf::Vector2f position);
    void spawnPhaseTransition(sf::Vector2f position);
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    std::vector<Particle> particles;
};
