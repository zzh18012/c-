#pragma once
#include <SFML/Graphics.hpp>

class ParticleSystem;

enum class ItemType {
    HealCore,      // 治疗核心 - 恢复25%最大生命值
    ShieldOrb,     // 护盾球 - 3秒无敌护盾
    Overdrive,     // 火力超载 - 射速+100%, 伤害+30%
    BulletTime,    // 子弹时间 - Boss子弹速度-50%
    NovaBomb,      // 新星炸弹 - 清除Boss子弹 + 伤害
    DashBattery,   // 冲刺电池 - 刷新冲刺冷却 + 移速提升
    WeaponEssence, // 武器精华 - 切换武器形态
    PhaseCrystal,  // 相位水晶 - Phase Shift无敌帧
    SpeedCoil,     // 速度线圈 - 移速+50%持续8秒
    AttackModule,  // 攻击模块 - 伤害+30%持续10秒
    NovaCore       // 新星核心 - Nova Form全屏冲击波
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
    static void setParticleSystem(ParticleSystem* ps);

private:
    void drawHealCore(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawShieldOrb(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawOverdrive(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawBulletTime(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawNovaBomb(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawDashBattery(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawWeaponEssence(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawPhaseCrystal(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawSpeedCoil(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawAttackModule(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;
    void drawNovaCore(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const;

    ItemType type;
    sf::Vector2f position;
    bool active;
    float lifetime;
    float rotationAngle;
    float pulsePhase;
};