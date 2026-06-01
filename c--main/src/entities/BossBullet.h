// ============================================================
// BossBullet.h - Boss子弹类声明
// ============================================================
// 职责：Boss发射的子弹（对象池模式）
// 每颗子弹有位置、速度、伤害值、激活状态
// 渲染时绘制有机等离子体球效果（外发光+主体+内核+脉动核心）
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::CircleShape, sf::RenderWindow）
#include <SFML/Graphics.hpp>

// ============================================================
// BossBullet类
// ============================================================
// Boss子弹对象池中的单个子弹对象
// 由Boss统一管理（最多2000发），Game.cpp负责碰撞检测
// 渲染使用多层圆形叠加营造有机等离子体球效果
class BossBullet {
public:
    // 构造函数：初始化位置速度、清零伤害值、设置初始状态为未激活
    // 注意：shape的origin设置为半径值，方便以中心点为基准设置位置
    BossBullet();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置位置、速度方向、速度大小、伤害值、激活状态
    void spawn(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg);
    // 每帧更新：位置 =位置 + 速度 × 时间差（匀速运动）
    // 同时检测是否超出屏幕边界，超出则设为非激活（对象池回收）
    void update(float dt);
    // 渲染：Boss子弹的有机等离子体球效果
    // 渲染层次：外发光 → 主体 → 内核 → 脉动核心（由外向内依次绘制）
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;          // 子弹是否激活（对象池中已使用）
    void deactivate();              // 设为非激活（对象池回收）
    sf::Vector2f getPosition() const; // 获取子弹坐标（碰撞检测用）
    float getRadius() const;        // 获取碰撞半径（BOSS_BULLET_RADIUS）
    int getDamage() const;          // 获取伤害值

private:
    // ---- 物理属性 ----
    sf::Vector2f position;  // 子弹当前位置（屏幕坐标系）
    sf::Vector2f velocity; // 子弹速度向量（方向 × 速度大小）

    // ---- 渲染图形 ----
    // 使用SFML的CircleShape绘制子弹
    // Boss子弹外观：有机等离子体球，多层圆形叠加
    sf::CircleShape shape;

    // ---- 状态 ----
    bool active;  // 是否激活（激活=true表示正在使用，false=对象池中的空闲对象）
    int damage;   // 子弹伤害值（Boss生成时指定，用于扣减玩家HP）
};