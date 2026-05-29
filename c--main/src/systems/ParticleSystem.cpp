#include "ParticleSystem.h"
#include "core/Config.h"
#include <cmath>
#include <cstdlib>
#include <cstdint>

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

void ParticleSystem::spawnOrbitParticles(sf::Vector2f center, sf::Color color, int count, float orbitRadius) {
    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = 6.28318f * i / count;
            p.position = center + sf::Vector2f(std::cosf(angle) * orbitRadius, std::sinf(angle) * orbitRadius);
            p.velocity = sf::Vector2f(-std::sinf(angle) * 80.f, std::cosf(angle) * 80.f);
            p.color = color;
            p.maxLifetime = 1.5f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

void ParticleSystem::spawnLightning(sf::Vector2f from, sf::Vector2f to, sf::Color color) {
    sf::Vector2f dir = to - from;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len < 1.f) return;
    dir /= len;
    sf::Vector2f perp(-dir.y, dir.x);
    int segments = 8;
    sf::Vector2f prev = from;
    for (int i = 1; i <= segments; ++i) {
        float t = i / static_cast<float>(segments);
        sf::Vector2f base = from + dir * (len * t);
        float offset = (i == segments) ? 0.f : (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 30.f;
        sf::Vector2f curr = base + perp * offset;
        for (auto& p : particles) {
            if (p.active) continue;
            p.position = curr;
            p.velocity = sf::Vector2f(0.f, 0.f);
            p.color = color;
            p.maxLifetime = 0.15f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
        prev = curr;
    }
}

void ParticleSystem::spawnNovaRing(sf::Vector2f center, float radius, sf::Color color, int count) {
    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = 6.28318f * i / count;
            p.position = center + sf::Vector2f(std::cosf(angle) * radius, std::sinf(angle) * radius);
            p.velocity = sf::Vector2f(std::cosf(angle) * 100.f, std::sinf(angle) * 100.f);
            p.color = color;
            p.maxLifetime = 0.4f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

void ParticleSystem::spawnDashTrail(sf::Vector2f position, sf::Color color) {
    for (int i = 0; i < 8; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
            float speed = 20.f + static_cast<float>(rand()) / RAND_MAX * 40.f;
            p.position = position + sf::Vector2f(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.f);
            p.velocity = sf::Vector2f(std::cosf(angle) * speed, std::sinf(angle) * speed);
            p.color = color;
            p.maxLifetime = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.15f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

void ParticleSystem::spawnMechThrust(sf::Vector2f position, sf::Vector2f dir, sf::Color color) {
    for (int i = 0; i < 3; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float spread = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.8f;
            float speed = 100.f + static_cast<float>(rand()) / RAND_MAX * 80.f;
            sf::Vector2f perp(-dir.y, dir.x);
            p.position = position;
            p.velocity = dir * speed + perp * spread * 50.f;
            p.color = color;
            p.maxLifetime = 0.15f + static_cast<float>(rand()) / RAND_MAX * 0.1f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

void ParticleSystem::spawnBulletTrail(sf::Vector2f position, sf::Color color) {
    for (auto& p : particles) {
        if (p.active) continue;
        p.position = position;
        p.velocity = sf::Vector2f(0.f, 0.f);
        p.color = color;
        p.maxLifetime = 0.08f;
        p.lifetime = p.maxLifetime;
        p.active = true;
        break;
    }
}

void ParticleSystem::spawnHealEffect(sf::Vector2f position) {
    // Rising heal particles
    for (int i = 0; i < 20; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = -3.14159f * 0.5f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.2f;
            float speed = 80.f + static_cast<float>(rand()) / RAND_MAX * 60.f;
            p.position = position + sf::Vector2f(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 30.f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 30.f);
            p.velocity = sf::Vector2f(std::cosf(angle) * speed, std::sinf(angle) * speed);
            p.color = sf::Color(100, 255, 150, 200);
            p.maxLifetime = 0.6f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

void ParticleSystem::spawnPhaseTransition(sf::Vector2f position) {
    // Intense white flash particles in all directions
    for (int i = 0; i < 40; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
            float speed = 200.f + static_cast<float>(rand()) / RAND_MAX * 300.f;
            p.position = position;
            p.velocity = sf::Vector2f(std::cosf(angle) * speed, std::sinf(angle) * speed);
            p.color = sf::Color(255, 255, 255, 220);
            p.maxLifetime = 0.4f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
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
    sf::VertexArray va(sf::PrimitiveType::Triangles);
    std::vector<sf::VertexArray> batches;

    for (const auto& p : particles) {
        if (!p.active) continue;

        float alpha = p.lifetime / p.maxLifetime;
        sf::Color c = p.color;
        c.a = static_cast<std::uint8_t>(255.f * alpha);

        float halfSize = 3.f;
        sf::Vector2f pos = p.position;

        sf::Vertex v1, v2, v3, v4, v5, v6;
        v1.position = sf::Vector2f(pos.x - halfSize, pos.y - halfSize);
        v1.color = c;
        v2.position = sf::Vector2f(pos.x + halfSize, pos.y - halfSize);
        v2.color = c;
        v3.position = sf::Vector2f(pos.x + halfSize, pos.y + halfSize);
        v3.color = c;
        v4.position = sf::Vector2f(pos.x - halfSize, pos.y - halfSize);
        v4.color = c;
        v5.position = sf::Vector2f(pos.x + halfSize, pos.y + halfSize);
        v5.color = c;
        v6.position = sf::Vector2f(pos.x - halfSize, pos.y + halfSize);
        v6.color = c;
        va.append(v1);
        va.append(v2);
        va.append(v3);
        va.append(v4);
        va.append(v5);
        va.append(v6);
    }

    sf::BlendMode additive(sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::One);
    window.draw(va, additive);
}
