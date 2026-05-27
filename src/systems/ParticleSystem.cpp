#include "ParticleSystem.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>

ParticleSystem::ParticleSystem() {
    particles.resize(PARTICLE_POOL_SIZE);
}

void ParticleSystem::spawnExplosion(sf::Vector2f position, sf::Color color, int count) {
    (void)position;
    (void)color;
    (void)count;
}

void ParticleSystem::spawnHitSpark(sf::Vector2f position, sf::Color color) {
    (void)position;
    (void)color;
}

void ParticleSystem::update(float dt) {
    (void)dt;
}

void ParticleSystem::render(sf::RenderWindow& window) {
    (void)window;
}
