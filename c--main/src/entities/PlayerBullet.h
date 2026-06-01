// ============================================================
// PlayerBullet.h - 玩家普通子弹类声明
// ============================================================
// 职责：玩家普通武器发射的子弹（对象池模式）
// 单发、高射速、高伤害，弹道笔直
// 渲染效果：青色发光子弹（外发光+主体+亮心）
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::CircleShape, sf::RenderWindow）
#include <SFML/Graphics.hpp>

// ============================================================
// PlayerBullet类
// ============================================================
// 玩家普通武器的子弹对象，使用对象池模式管理
// 由Player统一管理，Game.cpp负责碰撞检测
// 渲染使用3层圆形叠加：外发光（最大）→ 主体 → 亮心（最小）
class PlayerBullet {
public:
    // 构造函数：初始化位置(0,0)、速度(0,0)、非激活、伤害为默认值
    PlayerBullet();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置位置、朝向方向，自动计算速度向量
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    // 每帧更新：位置 += 速度 × dt（匀速直线运动）
    // 边界检测：超出屏幕±10像素则回收
    void update(float dt);
    // 渲染：3层青色发光子弹（外发光 → 主体 → 亮心）
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                     // 是否激活
    void deactivate();                        // 设为非激活
    void setDamage(int dmg);                   // 设置伤害值
    sf::Vector2f getPosition() const;         // 获取位置（碰撞检测用）
    float getRadius() const;                   // 获取碰撞半径
    int getDamage() const;                     // 获取伤害值

private:
    // ---- 物理属性 ----
    sf::Vector2f position;  // 子弹当前位置
    sf::Vector2f velocity; // 子弹速度向量（方向 × 速度）

    // ---- 渲染图形 ----
    // 使用SFML的CircleShape绘制子弹
    sf::CircleShape shape;

    // ---- 状态 ----
    bool active;  // 是否激活（对象池中已使用=true）
    int damage;   // 子弹伤害值
};