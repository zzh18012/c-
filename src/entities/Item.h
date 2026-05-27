#pragma once
#include <SFML/Graphics.hpp>

enum class ItemType {
    HealCore,      // 治疗核心 - 恢复25%最大生命值
    ShieldOrb,     // 护盾球 - 3秒无敌护盾
    Overdrive,     // 火力超载 - 射速+100%, 伤害+30%
    BulletTime,    // 子弹时间 - Boss子弹速度-50%
    NovaBomb,      // 新星炸弹 - 清除Boss子弹 + 伤害
    DashBattery    // 冲刺电池 - 刷新冲刺冷却 + 移速提升
};

class Item {
public:
    Item();

    void spawn(ItemType type, sf::Vector2f pos);
    void update(float dt);
    void render(sf::RenderWindow& window) const;
    bool isActive() const;
    void deactivate();

    ItemType getType() const;
    sf::Vector2f getPosition() const;
    float getRadius() const;

    static sf::Color getColor(ItemType type);
    static float getLifetime();

private:
    ItemType type;
    sf::Vector2f position;
    sf::CircleShape shape;
    bool active;
    float lifetime;
};
