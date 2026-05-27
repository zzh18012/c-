#include "ParticleSystem.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>

ParticleSystem::ParticleSystem() {
    particles.resize(PARTICLE_POOL_SIZE);
}

void ParticleSystem::spawnExplosion(sf::Vector2f position, sf::Color color, int count) {
    int spawned = 0;
    for (auto& p : particles) {
        if (spawned >= count) break;
        if (p.active) continue;

        p.position = position;
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        float speed = 80.f + static_cast<float>(rand()) / RAND_MAX * 250.f;
        p.velocity = sf::Vector2f(cosf(angle) * speed, sinf(angle) * speed);
        p.color = color;
        p.maxLifetime = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
        p.lifetime = p.maxLifetime;
        p.active = true;
        spawned++;
    }
}

void ParticleSystem::spawnHitSpark(sf::Vector2f position, sf::Color color) {
    int count = 5 + rand() % 6;
    for (auto& p : particles) {
        if (count <= 0) break;
        if (p.active) continue;

        p.position = position;
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        float speed = 150.f + static_cast<float>(rand()) / RAND_MAX * 200.f;
        p.velocity = sf::Vector2f(cosf(angle) * speed, sinf(angle) * speed);
        p.color = color;
        p.maxLifetime = 0.1f + static_cast<float>(rand()) / RAND_MAX * 0.2f;
        p.lifetime = p.maxLifetime;
        p.active = true;
        count--;
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : particles) {
        if (!p.active) continue;

        p.position += p.velocity * dt;
        p.velocity *= 0.96f;
        p.lifetime -= dt;

        if (p.lifetime <= 0.f) {
            p.active = false;
        }
    }
}

void ParticleSystem::render(sf::RenderWindow& window) {
    // Use VertexArray for efficient batch rendering
    sf::VertexArray va(sf::Quads);
    std::vector<sf::VertexArray> batches;

    for (const auto& p : particles) {
        if (!p.active) continue;

        float alpha = p.lifetime / p.maxLifetime;
        sf::Color c = p.color;
        c.a = static_cast<sf::Uint8>(255.f * alpha);

        float halfSize = 3.f;
        sf::Vector2f pos = p.position;

        va.append(sf::Vertex(sf::Vector2f(pos.x - halfSize, pos.y - halfSize), c));
        va.append(sf::Vertex(sf::Vector2f(pos.x + halfSize, pos.y - halfSize), c));
        va.append(sf::Vertex(sf::Vector2f(pos.x + halfSize, pos.y + halfSize), c));
        va.append(sf::Vertex(sf::Vector2f(pos.x - halfSize, pos.y + halfSize), c));
    }

    sf::BlendMode additive(sf::BlendMode::SrcAlpha, sf::BlendMode::One);
    window.draw(va, additive);
}
