#include "Item.h"
#include "core/Config.h"
#include <cmath>

Item::Item()
    : type(ItemType::HealCore)
    , position(0.f, 0.f)
    , active(false)
    , lifetime(0.f)
{
    shape.setRadius(ITEM_RADIUS);
    shape.setOrigin(ITEM_RADIUS, ITEM_RADIUS);
    shape.setOutlineThickness(2.f);
}

void Item::spawn(ItemType t, sf::Vector2f pos) {
    type = t;
    position = pos;
    active = true;
    lifetime = ITEM_LIFETIME;

    sf::Color fill = getColor(type);
    shape.setFillColor(sf::Color(fill.r, fill.g, fill.b, 180));
    shape.setOutlineColor(fill);
    shape.setPosition(position);
}

void Item::update(float dt) {
    if (!active) return;
    lifetime -= dt;
    if (lifetime <= 0.f) {
        active = false;
    }
    float scale = 1.f + 0.1f * std::sin(lifetime * 4.f);
    shape.setScale(scale, scale);
}

void Item::render(sf::RenderWindow& window) const {
    if (!active) return;
    window.draw(shape);
}

bool Item::isActive() const { return active; }
void Item::deactivate() { active = false; }
ItemType Item::getType() const { return type; }
sf::Vector2f Item::getPosition() const { return position; }
float Item::getRadius() const { return ITEM_RADIUS; }
float Item::getLifetime() { return ITEM_LIFETIME; }

sf::Color Item::getColor(ItemType type) {
    switch (type) {
        case ItemType::HealCore:    return sf::Color(60, 255, 100);   // 绿色
        case ItemType::ShieldOrb:   return sf::Color(60, 140, 255);   // 蓝色
        case ItemType::Overdrive:   return sf::Color(255, 100, 40);   // 橙红
        case ItemType::BulletTime:  return sf::Color(180, 60, 255);   // 紫色
        case ItemType::NovaBomb:    return sf::Color(255, 255, 240);  // 亮白
        case ItemType::DashBattery: return sf::Color(255, 255, 60);   // 黄色
    }
    return sf::Color::White;
}
