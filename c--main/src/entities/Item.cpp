#include "Item.h"
#include "core/Config.h"
#include "systems/ParticleSystem.h"
#include <cmath>
#include <cstdint>

static ParticleSystem* gParticles = nullptr;
void Item::setParticleSystem(ParticleSystem* ps) { gParticles = ps; }

Item::Item()
    : type(ItemType::HealCore)
    , position(0.f, 0.f)
    , active(false)
    , lifetime(0.f)
    , rotationAngle(0.f)
    , pulsePhase(0.f)
{
}

void Item::spawn(ItemType t, sf::Vector2f pos) {
    type = t;
    position = pos;
    active = true;
    lifetime = ITEM_LIFETIME;
    rotationAngle = 0.f;
    pulsePhase = 0.f;

    if (gParticles) {
        sf::Color col = getColor(type);
        gParticles->spawnOrbitParticles(position, col, 8, ITEM_RADIUS + 12.f);
    }
}

void Item::update(float dt) {
    if (!active) return;
    lifetime -= dt;
    if (lifetime <= 0.f) {
        active = false;
    }
    rotationAngle += dt * 120.f;
    pulsePhase += dt * 5.f;

    if (gParticles && active) {
        gParticles->spawnOrbitParticles(position, getColor(type), 1, ITEM_RADIUS + 10.f);
    }
}

void Item::render(sf::RenderWindow& window) const {
    if (!active) return;

    float glow = 0.85f + 0.15f * std::sin(pulsePhase);
    float angle = rotationAngle * 0.0174533f;

    switch (type) {
        case ItemType::HealCore:    drawHealCore(window, position, angle, glow); break;
        case ItemType::ShieldOrb:   drawShieldOrb(window, position, angle, glow); break;
        case ItemType::Overdrive:   drawOverdrive(window, position, angle, glow); break;
        case ItemType::BulletTime:  drawBulletTime(window, position, angle, glow); break;
        case ItemType::NovaBomb:    drawNovaBomb(window, position, angle, glow); break;
        case ItemType::DashBattery: drawDashBattery(window, position, angle, glow); break;
        case ItemType::WeaponEssence: drawWeaponEssence(window, position, angle, glow); break;
        case ItemType::PhaseCrystal: drawPhaseCrystal(window, position, angle, glow); break;
        case ItemType::SpeedCoil:   drawSpeedCoil(window, position, angle, glow); break;
        case ItemType::AttackModule: drawAttackModule(window, position, angle, glow); break;
        case ItemType::NovaCore:    drawNovaCore(window, position, angle, glow); break;
    }
}

void Item::drawHealCore(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // Outer glow ring
    sf::CircleShape outerGlow(ITEM_RADIUS + 8.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 8.f, ITEM_RADIUS + 8.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(60, 255, 100, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    // Hexagon shape for core
    sf::ConvexShape hex;
    hex.setPointCount(6);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        hex.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    hex.setOrigin(sf::Vector2f(0.f, 0.f));
    hex.setPosition(pos);
    hex.setFillColor(sf::Color(60, 255, 100, 220));
    hex.setOutlineColor(sf::Color(150, 255, 180));
    hex.setOutlineThickness(2.f);
    window.draw(hex);

    // Inner cross (medical symbol)
    sf::RectangleShape vBar(sf::Vector2f(4.f, ITEM_RADIUS * 0.8f));
    vBar.setOrigin(sf::Vector2f(2.f, ITEM_RADIUS * 0.4f));
    vBar.setPosition(pos);
    vBar.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(vBar);

    sf::RectangleShape hBar(sf::Vector2f(ITEM_RADIUS * 0.8f, 4.f));
    hBar.setOrigin(sf::Vector2f(ITEM_RADIUS * 0.4f, 2.f));
    hBar.setPosition(pos);
    hBar.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(hBar);

    // Rotating outer ring segments
    for (int i = 0; i < 4; ++i) {
        float a = angle + 3.14159f * 2.f * i / 4.f;
        sf::CircleShape dot(3.f);
        dot.setOrigin(sf::Vector2f(3.f, 3.f));
        dot.setPosition(pos + sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 6.f), std::sinf(a) * (ITEM_RADIUS + 6.f)));
        dot.setFillColor(sf::Color(60, 255, 100, static_cast<uint8_t>(200 * glow)));
        window.draw(dot);
    }
}

void Item::drawShieldOrb(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // Layered energy orb
    sf::CircleShape outerRing(ITEM_RADIUS + 6.f);
    outerRing.setOrigin(sf::Vector2f(ITEM_RADIUS + 6.f, ITEM_RADIUS + 6.f));
    outerRing.setPosition(pos);
    outerRing.setFillColor(sf::Color(60, 140, 255, static_cast<uint8_t>(25 * glow)));
    outerRing.setOutlineColor(sf::Color(60, 140, 255, static_cast<uint8_t>(150 * glow)));
    outerRing.setOutlineThickness(2.f);
    window.draw(outerRing);

    // Main orb - hexagonal
    sf::ConvexShape hex;
    hex.setPointCount(6);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        hex.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    hex.setOrigin(sf::Vector2f(0.f, 0.f));
    hex.setPosition(pos);
    hex.setFillColor(sf::Color(60, 140, 255, 210));
    hex.setOutlineColor(sf::Color(120, 200, 255));
    hex.setOutlineThickness(2.f);
    window.draw(hex);

    // Inner shield icon
    sf::ConvexShape shieldIcon;
    shieldIcon.setPointCount(5);
    shieldIcon.setPoint(0, sf::Vector2f(0.f, -8.f));
    shieldIcon.setPoint(1, sf::Vector2f(6.f, -4.f));
    shieldIcon.setPoint(2, sf::Vector2f(6.f, 4.f));
    shieldIcon.setPoint(3, sf::Vector2f(0.f, 8.f));
    shieldIcon.setPoint(4, sf::Vector2f(-6.f, 4.f));
    shieldIcon.setOrigin(sf::Vector2f(0.f, 0.f));
    shieldIcon.setPosition(pos);
    shieldIcon.setFillColor(sf::Color(180, 220, 255, 220));
    window.draw(shieldIcon);

    // Orbiting dots
    for (int i = 0; i < 3; ++i) {
        float a = angle + 3.14159f * 2.f * i / 3.f + pulsePhase * 0.5f;
        sf::CircleShape dot(2.5f);
        dot.setOrigin(sf::Vector2f(2.5f, 2.5f));
        dot.setPosition(pos + sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 4.f), std::sinf(a) * (ITEM_RADIUS + 4.f)));
        dot.setFillColor(sf::Color(100, 180, 255, static_cast<uint8_t>(220 * glow)));
        window.draw(dot);
    }
}

void Item::drawOverdrive(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // Flame aura
    sf::CircleShape flameAura(ITEM_RADIUS + 10.f);
    flameAura.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    flameAura.setPosition(pos);
    flameAura.setFillColor(sf::Color(255, 80, 20, static_cast<uint8_t>(35 * glow)));
    window.draw(flameAura);

    // Arrow/dart shape
    sf::ConvexShape arrow;
    arrow.setPointCount(5);
    arrow.setPoint(0, sf::Vector2f(0.f, -ITEM_RADIUS * glow));
    arrow.setPoint(1, sf::Vector2f(ITEM_RADIUS * 0.6f * glow, ITEM_RADIUS * 0.3f * glow));
    arrow.setPoint(2, sf::Vector2f(ITEM_RADIUS * 0.25f * glow, ITEM_RADIUS * 0.3f * glow));
    arrow.setPoint(3, sf::Vector2f(ITEM_RADIUS * 0.25f * glow, ITEM_RADIUS * glow));
    arrow.setPoint(4, sf::Vector2f(-ITEM_RADIUS * 0.25f * glow, ITEM_RADIUS * glow));
    arrow.setOrigin(sf::Vector2f(0.f, 0.f));
    arrow.setPosition(pos);
    arrow.setRotation(sf::radians(rotationAngle * 0.0174533f));
    arrow.setFillColor(sf::Color(255, 100, 40, 230));
    arrow.setOutlineColor(sf::Color(255, 200, 100));
    arrow.setOutlineThickness(2.f);
    window.draw(arrow);

    // Speed lines
    for (int i = 0; i < 3; ++i) {
        float offset = (i - 1) * 8.f;
        sf::RectangleShape line(sf::Vector2f(2.f, ITEM_RADIUS * 0.6f * glow));
        line.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.3f * glow));
        line.setPosition(pos + sf::Vector2f(offset, ITEM_RADIUS * 0.5f * glow));
        line.setFillColor(sf::Color(255, 150, 50, static_cast<uint8_t>(180 * glow)));
        window.draw(line);
    }

    // Glow core
    sf::CircleShape core(5.f);
    core.setOrigin(sf::Vector2f(5.f, 5.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 200, 250));
    window.draw(core);
}

void Item::drawBulletTime(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // Time dial background
    sf::CircleShape dial(ITEM_RADIUS + 5.f);
    dial.setOrigin(sf::Vector2f(ITEM_RADIUS + 5.f, ITEM_RADIUS + 5.f));
    dial.setPosition(pos);
    dial.setFillColor(sf::Color(180, 60, 255, static_cast<uint8_t>(30 * glow)));
    dial.setOutlineColor(sf::Color(180, 60, 255, static_cast<uint8_t>(120 * glow)));
    dial.setOutlineThickness(2.f);
    window.draw(dial);

    // Clock face - octagon
    sf::ConvexShape octo;
    octo.setPointCount(8);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 8; ++i) {
        float a = angle + 3.14159f * 2.f * i / 8.f;
        octo.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    octo.setOrigin(sf::Vector2f(0.f, 0.f));
    octo.setPosition(pos);
    octo.setFillColor(sf::Color(140, 40, 220, 200));
    octo.setOutlineColor(sf::Color(200, 120, 255));
    octo.setOutlineThickness(2.f);
    window.draw(octo);

    // Clock hands
    sf::RectangleShape hourHand(sf::Vector2f(3.f, ITEM_RADIUS * 0.35f));
    hourHand.setOrigin(sf::Vector2f(1.5f, ITEM_RADIUS * 0.35f));
    hourHand.setPosition(pos);
    hourHand.setRotation(sf::radians(rotationAngle * 0.0087266f));
    hourHand.setFillColor(sf::Color(255, 220, 255, 230));
    window.draw(hourHand);

    sf::RectangleShape minHand(sf::Vector2f(2.f, ITEM_RADIUS * 0.5f));
    minHand.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.5f));
    minHand.setPosition(pos);
    minHand.setRotation(sf::radians(-rotationAngle * 0.0261799f));
    minHand.setFillColor(sf::Color(255, 220, 255, 230));
    window.draw(minHand);

    // Center dot
    sf::CircleShape center(4.f);
    center.setOrigin(sf::Vector2f(4.f, 4.f));
    center.setPosition(pos);
    center.setFillColor(sf::Color(255, 255, 255, 240));
    window.draw(center);

    // Orbiting sparkles
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        sf::CircleShape sparkle(2.f);
        sparkle.setOrigin(sf::Vector2f(2.f, 2.f));
        sparkle.setPosition(pos + sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 3.f), std::sinf(a) * (ITEM_RADIUS + 3.f)));
        sparkle.setFillColor(sf::Color(220, 150, 255, static_cast<uint8_t>(200 * glow)));
        window.draw(sparkle);
    }
}

void Item::drawNovaBomb(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // Intense white glow
    sf::CircleShape outerGlow(ITEM_RADIUS + 14.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 14.f, ITEM_RADIUS + 14.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 255, 200, static_cast<uint8_t>(40 * glow)));
    window.draw(outerGlow);

    // Star burst shape (8 points)
    sf::ConvexShape star;
    star.setPointCount(8);
    float r1 = ITEM_RADIUS * 0.5f * glow;
    float r2 = ITEM_RADIUS * glow;
    for (int i = 0; i < 8; ++i) {
        float r = (i % 2 == 0) ? r2 : r1;
        float a = angle + 3.14159f * 2.f * i / 8.f;
        star.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    star.setOrigin(sf::Vector2f(0.f, 0.f));
    star.setPosition(pos);
    star.setFillColor(sf::Color(255, 255, 220, 230));
    star.setOutlineColor(sf::Color(255, 255, 180));
    star.setOutlineThickness(2.f);
    window.draw(star);

    // Inner star
    sf::ConvexShape innerStar;
    innerStar.setPointCount(6);
    float ir = ITEM_RADIUS * 0.4f * glow;
    for (int i = 0; i < 6; ++i) {
        float a = -angle + 3.14159f * 2.f * i / 6.f;
        innerStar.setPoint(i, sf::Vector2f(std::cosf(a) * ir, std::sinf(a) * ir));
    }
    innerStar.setOrigin(sf::Vector2f(0.f, 0.f));
    innerStar.setPosition(pos);
    innerStar.setFillColor(sf::Color(255, 255, 255, 240));
    window.draw(innerStar);

    // Radiating lines
    for (int i = 0; i < 12; ++i) {
        float a = angle + 3.14159f * 2.f * i / 12.f;
        sf::RectangleShape ray(sf::Vector2f(2.f, ITEM_RADIUS * 0.3f * glow));
        ray.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.3f * glow));
        ray.setPosition(pos + sf::Vector2f(std::cosf(a) * ITEM_RADIUS * 0.6f, std::sinf(a) * ITEM_RADIUS * 0.6f));
        ray.setRotation(sf::radians(3.14159f + a));
        ray.setFillColor(sf::Color(255, 255, 150, static_cast<uint8_t>(160 * glow)));
        window.draw(ray);
    }

    // Core
    sf::CircleShape core(6.f);
    core.setOrigin(sf::Vector2f(6.f, 6.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 255, 255));
    window.draw(core);
}

void Item::drawDashBattery(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // Electric field
    sf::CircleShape eField(ITEM_RADIUS + 8.f);
    eField.setOrigin(sf::Vector2f(ITEM_RADIUS + 8.f, ITEM_RADIUS + 8.f));
    eField.setPosition(pos);
    eField.setFillColor(sf::Color(255, 255, 60, static_cast<uint8_t>(30 * glow)));
    eField.setOutlineColor(sf::Color(255, 255, 60, static_cast<uint8_t>(100 * glow)));
    eField.setOutlineThickness(2.f);
    window.draw(eField);

    // Lightning bolt shape
    sf::ConvexShape bolt;
    bolt.setPointCount(6);
    bolt.setPoint(0, sf::Vector2f(4.f * glow, -ITEM_RADIUS * glow));
    bolt.setPoint(1, sf::Vector2f(-2.f * glow, -2.f * glow));
    bolt.setPoint(2, sf::Vector2f(3.f * glow, -2.f * glow));
    bolt.setPoint(3, sf::Vector2f(-4.f * glow, ITEM_RADIUS * glow));
    bolt.setPoint(4, sf::Vector2f(2.f * glow, 2.f * glow));
    bolt.setPoint(5, sf::Vector2f(-3.f * glow, 2.f * glow));
    bolt.setOrigin(sf::Vector2f(0.f, 0.f));
    bolt.setPosition(pos);
    bolt.setFillColor(sf::Color(255, 255, 60, 230));
    bolt.setOutlineColor(sf::Color(255, 255, 180));
    bolt.setOutlineThickness(2.f);
    window.draw(bolt);

    // Electric arcs
    for (int i = 0; i < 4; ++i) {
        float a = angle + 3.14159f * 2.f * i / 4.f + pulsePhase;
        sf::Vector2f arcPos = sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 5.f), std::sinf(a) * (ITEM_RADIUS + 5.f));
        sf::RectangleShape arc(sf::Vector2f(3.f, 6.f));
        arc.setOrigin(sf::Vector2f(1.5f, 3.f));
        arc.setPosition(pos + arcPos);
        arc.setRotation(sf::radians(3.14159f + a));
        arc.setFillColor(sf::Color(255, 255, 120, static_cast<uint8_t>(200 * glow)));
        window.draw(arc);
    }

    // Inner glow
    sf::CircleShape inner(5.f);
    inner.setOrigin(sf::Vector2f(5.f, 5.f));
    inner.setPosition(pos + sf::Vector2f(2.f, 0.f));
    inner.setFillColor(sf::Color(255, 255, 220, 240));
    window.draw(inner);
}

void Item::drawWeaponEssence(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    sf::CircleShape outerGlow(ITEM_RADIUS + 10.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(200, 100, 255, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    sf::ConvexShape diamond;
    diamond.setPointCount(4);
    float r = ITEM_RADIUS * glow;
    diamond.setPoint(0, sf::Vector2f(0.f, -r));
    diamond.setPoint(1, sf::Vector2f(r * 0.6f, 0.f));
    diamond.setPoint(2, sf::Vector2f(0.f, r));
    diamond.setPoint(3, sf::Vector2f(-r * 0.6f, 0.f));
    diamond.setOrigin(sf::Vector2f(0.f, 0.f));
    diamond.setPosition(pos);
    diamond.setRotation(sf::radians(rotationAngle * 0.0174533f));
    diamond.setFillColor(sf::Color(180, 80, 255, 220));
    diamond.setOutlineColor(sf::Color(220, 150, 255));
    diamond.setOutlineThickness(2.f);
    window.draw(diamond);

    sf::CircleShape core(5.f);
    core.setOrigin(sf::Vector2f(5.f, 5.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 255, 240));
    window.draw(core);
}

void Item::drawPhaseCrystal(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    sf::CircleShape outerGlow(ITEM_RADIUS + 10.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(150, 50, 200, static_cast<uint8_t>(35 * glow)));
    window.draw(outerGlow);

    sf::ConvexShape hex;
    hex.setPointCount(6);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        hex.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    hex.setOrigin(sf::Vector2f(0.f, 0.f));
    hex.setPosition(pos);
    hex.setFillColor(sf::Color(120, 40, 180, 220));
    hex.setOutlineColor(sf::Color(180, 120, 220));
    hex.setOutlineThickness(2.f);
    window.draw(hex);

    for (int i = 0; i < 3; ++i) {
        float a = angle + 3.14159f * 2.f * i / 3.f;
        sf::CircleShape dot(3.f);
        dot.setOrigin(sf::Vector2f(3.f, 3.f));
        dot.setPosition(pos + sf::Vector2f(std::cosf(a) * r * 0.5f, std::sinf(a) * r * 0.5f));
        dot.setFillColor(sf::Color(200, 150, 255, static_cast<uint8_t>(200 * glow)));
        window.draw(dot);
    }
}

void Item::drawSpeedCoil(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    sf::CircleShape outerGlow(ITEM_RADIUS + 8.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 8.f, ITEM_RADIUS + 8.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(100, 255, 200, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    sf::ConvexShape arrow;
    arrow.setPointCount(5);
    arrow.setPoint(0, sf::Vector2f(0.f, -ITEM_RADIUS * glow));
    arrow.setPoint(1, sf::Vector2f(ITEM_RADIUS * 0.5f * glow, -ITEM_RADIUS * 0.3f * glow));
    arrow.setPoint(2, sf::Vector2f(ITEM_RADIUS * 0.2f * glow, -ITEM_RADIUS * 0.3f * glow));
    arrow.setPoint(3, sf::Vector2f(ITEM_RADIUS * 0.2f * glow, ITEM_RADIUS * glow));
    arrow.setPoint(4, sf::Vector2f(-ITEM_RADIUS * 0.2f * glow, ITEM_RADIUS * glow));
    arrow.setOrigin(sf::Vector2f(0.f, 0.f));
    arrow.setPosition(pos);
    arrow.setRotation(sf::radians(rotationAngle * 0.0174533f));
    arrow.setFillColor(sf::Color(80, 255, 180, 230));
    arrow.setOutlineColor(sf::Color(150, 255, 220));
    arrow.setOutlineThickness(2.f);
    window.draw(arrow);

    for (int i = 0; i < 3; ++i) {
        float offset = (i - 1) * 8.f;
        sf::RectangleShape line(sf::Vector2f(2.f, ITEM_RADIUS * 0.5f * glow));
        line.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.25f * glow));
        line.setPosition(pos + sf::Vector2f(offset, ITEM_RADIUS * 0.4f * glow));
        line.setFillColor(sf::Color(100, 255, 200, static_cast<uint8_t>(180 * glow)));
        window.draw(line);
    }
}

void Item::drawAttackModule(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    sf::CircleShape outerGlow(ITEM_RADIUS + 10.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 80, 80, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    sf::ConvexShape cross;
    cross.setPointCount(8);
    float r = ITEM_RADIUS * glow;
    float w = ITEM_RADIUS * 0.35f * glow;
    cross.setPoint(0, sf::Vector2f(-w, -r));
    cross.setPoint(1, sf::Vector2f(w, -r));
    cross.setPoint(2, sf::Vector2f(w, -w));
    cross.setPoint(3, sf::Vector2f(r, -w));
    cross.setPoint(4, sf::Vector2f(r, w));
    cross.setPoint(5, sf::Vector2f(w, w));
    cross.setPoint(6, sf::Vector2f(w, r));
    cross.setPoint(7, sf::Vector2f(-w, r));
    cross.setOrigin(sf::Vector2f(0.f, 0.f));
    cross.setPosition(pos);
    cross.setRotation(sf::radians(rotationAngle * 0.0174533f));
    cross.setFillColor(sf::Color(220, 60, 60, 220));
    cross.setOutlineColor(sf::Color(255, 150, 150));
    cross.setOutlineThickness(2.f);
    window.draw(cross);

    sf::CircleShape core(5.f);
    core.setOrigin(sf::Vector2f(5.f, 5.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 200, 240));
    window.draw(core);
}

void Item::drawNovaCore(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    sf::CircleShape outerGlow(ITEM_RADIUS + 15.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 15.f, ITEM_RADIUS + 15.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 150, 50, static_cast<uint8_t>(40 * glow)));
    window.draw(outerGlow);

    sf::ConvexShape star;
    star.setPointCount(8);
    float r1 = ITEM_RADIUS * 0.4f * glow;
    float r2 = ITEM_RADIUS * glow;
    for (int i = 0; i < 8; ++i) {
        float r = (i % 2 == 0) ? r2 : r1;
        float a = angle + 3.14159f * 2.f * i / 8.f;
        star.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    star.setOrigin(sf::Vector2f(0.f, 0.f));
    star.setPosition(pos);
    star.setFillColor(sf::Color(255, 120, 40, 230));
    star.setOutlineColor(sf::Color(255, 200, 100));
    star.setOutlineThickness(2.f);
    window.draw(star);

    sf::CircleShape core(7.f);
    core.setOrigin(sf::Vector2f(7.f, 7.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 200, 250));
    window.draw(core);

    for (int i = 0; i < 8; ++i) {
        float a = angle + 3.14159f * 2.f * i / 8.f;
        sf::RectangleShape ray(sf::Vector2f(2.f, ITEM_RADIUS * 0.25f * glow));
        ray.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.125f * glow));
        ray.setPosition(pos + sf::Vector2f(std::cosf(a) * ITEM_RADIUS * 0.5f, std::sinf(a) * ITEM_RADIUS * 0.5f));
        ray.setRotation(sf::radians(3.14159f + a));
        ray.setFillColor(sf::Color(255, 180, 80, static_cast<uint8_t>(160 * glow)));
        window.draw(ray);
    }
}

bool Item::isActive() const { return active; }
void Item::deactivate() { active = false; }
ItemType Item::getType() const { return type; }
sf::Vector2f Item::getPosition() const { return position; }
float Item::getRadius() const { return ITEM_RADIUS; }
float Item::getLifetime() { return ITEM_LIFETIME; }

sf::Color Item::getColor(ItemType type) {
    switch (type) {
        case ItemType::HealCore:    return sf::Color(60, 255, 100);
        case ItemType::ShieldOrb:   return sf::Color(60, 140, 255);
        case ItemType::Overdrive:   return sf::Color(255, 100, 40);
        case ItemType::BulletTime:  return sf::Color(180, 60, 255);
        case ItemType::NovaBomb:    return sf::Color(255, 255, 240);
        case ItemType::DashBattery: return sf::Color(255, 255, 60);
        case ItemType::WeaponEssence: return sf::Color(200, 100, 255);
        case ItemType::PhaseCrystal: return sf::Color(150, 50, 200);
        case ItemType::SpeedCoil:   return sf::Color(100, 255, 200);
        case ItemType::AttackModule: return sf::Color(255, 80, 80);
        case ItemType::NovaCore:    return sf::Color(255, 150, 50);
    }
    return sf::Color::White;
}