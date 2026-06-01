// ============================================================
// Item.h - 道具类声明
// ============================================================
// 职责：游戏中掉落的12种道具的生成、更新、渲染
// 道具在Boss死亡或阶段转换时生成，拾取后触发不同效果
// 每种道具有独特的外观渲染（多层几何图形叠加）
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::Color, sf::ConvexShape等）
#include <SFML/Graphics.hpp>

// 前向声明ParticleSystem（Item不拥有，只是调用其接口生成轨道粒子）
class ParticleSystem;

// ============================================================
// 道具类型枚举
// ============================================================
// 定义游戏中所有12种道具类型
// 每种道具对应玩家的一种技能或属性提升
enum class ItemType {
    HealCore,      // 治疗核心 - 恢复25%最大生命值
    ShieldOrb,     // 护盾球 - 3秒无敌护盾
    Overdrive,     // 火力超载 - 射速×2，伤害×1.3
    BulletTime,    // 子弹时间 - Boss子弹速度×50%
    NovaBomb,      // 新星炸弹 - 清除所有Boss子弹+全屏伤害
    DashBattery,   // 冲刺电池 - 刷新冲刺冷却+移速提升
    WeaponEssence, // 武器精华 - 切换武器形态
    PhaseCrystal,  // 相位水晶 - 相位偏移（无敌+穿怪）
    SpeedCoil,     // 速度线圈 - 移速×1.5，持续8秒
    AttackModule,  // 攻击模块 - 伤害×1.3，持续10秒
    NovaCore       // 新星核心 - Nova形态（大范围伤害）
};

// ============================================================
// Item类
// ============================================================
// 游戏中的道具对象，使用对象池模式管理
// 每个道具有生命周期（超过时间自动消失）
// 渲染时根据type调用对应的绘制方法
class Item {
public:
    // 构造函数：初始化为默认值（未激活状态）
    Item();

    // ---- 道具生命周期 ----
    // 生成道具：设置类型、位置、激活、重置生命周期/旋转/脉动
    void spawn(ItemType type, sf::Vector2f pos);
    // 每帧更新：减少生命周期、更新旋转角度和脉动相位
    // 生成轨道粒子效果
    void update(float dt);
    // 渲染：根据类型分发到对应的draw方法
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                    // 是否激活
    void deactivate();                       // 设为非激活（被拾取或超时）

    // ---- 道具信息 ----
    ItemType getType() const;                 // 获取道具类型
    sf::Vector2f getPosition() const;        // 获取位置（碰撞检测用）
    float getRadius() const;                  // 获取碰撞半径（ITEM_RADIUS）

    // ---- 静态工具方法 ----
    static sf::Color getColor(ItemType type); // 根据类型获取道具颜色
    static float getLifetime();               // 获取道具生命周期（ITEM_LIFETIME）
    // 设置全局粒子系统指针（用于生成轨道粒子）
    static void setParticleSystem(ParticleSystem* ps);

private:
    // ---- 12种道具的专属绘制方法 ----
    // 每种方法绘制独特的几何形状组合
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

    // ---- 成员变量 ----
    ItemType type;             // 道具类型（决定渲染效果和拾取效果）
    sf::Vector2f position;     // 道具位置（屏幕坐标系）
    bool active;               // 是否激活（未激活=对象池空闲）
    float lifetime;           // 剩余生命周期（倒计时到0则消失）
    float rotationAngle;       // 当前旋转角度（用于道具旋转动画）
    float pulsePhase;          // 脉动相位（用于光晕闪烁效果）
};