// ============================================================
// PlayerBulletHoming.h - 追踪子弹类声明
// ============================================================
// 职责：玩家追踪武器发射的子弹（对象池模式）
// 自动追踪Boss方向，有最大转向速度限制（HOMING_TURN_RATE）
// 速度逐渐加快，从60%最高速到80%最高速
// 渲染效果：粉红色子弹（轨迹 + 光晕 + 主体 + 内心）
// ============================================================

#pragma once

// SFML图形库
#include <SFML/Graphics.hpp>

// ============================================================
// PlayerBulletHoming类
// ============================================================
// 玩家追踪武器的子弹对象，特点：
// 1. 发射时朝鼠标方向，飞行中逐渐转向Boss
// 2. 每次更新计算朝向目标的最佳角度差，受HOMING_TURN_RATE限制
// 3. 速度从60%逐渐加快到80%最高速
// 由Player统一管理（homingBullets数组），Game.cpp负责碰撞检测
class PlayerBulletHoming {
public:
    // 构造函数
    PlayerBulletHoming();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置位置、朝向方向
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    // 每帧更新：计算目标角度差 → 限制转向速度 → 更新速度方向 → 更新位置
    void update(float dt, const sf::Vector2f& targetPos);
    // 渲染：粉红色子弹（轨迹 + 光晕 + 主体 + 内心）
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                     // 是否激活
    void deactivate();                        // 设为非激活
    void setDamage(int dmg);                   // 设置伤害值
    sf::Vector2f getPosition() const;         // 获取位置
    float getRadius() const;                  // 获取碰撞半径
    int getDamage() const;                     // 获取伤害值

private:
    // ---- 物理属性 ----
    sf::Vector2f position;  // 子弹当前位置
    sf::Vector2f velocity; // 子弹速度向量

    // ---- 渲染相关 ----
    float angle;   // 当前角度（度，用于渲染旋转）
    sf::CircleShape shape; // 主体圆形（粉红色）

    // ---- 状态 ----
    bool active;  // 是否激活
    int damage;    // 子弹伤害值
};