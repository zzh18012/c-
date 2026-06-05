// ============================================================
// PlayerBulletOrbital.h - 环绕子弹类声明
// ============================================================
// 职责：玩家环绕武器发射的子弹（对象池模式）
// 4发子弹绕玩家旋转，持续伤害靠近的敌人
// 每转一圈（经过4个象限）增加orbitCount，影响伤害
// 渲染效果：多层青色圆形子弹，有电荷聚集效果
// ============================================================

#pragma once

// SFML图形库
#include <SFML/Graphics.hpp>

// ============================================================
// PlayerBulletOrbital类
// ============================================================
// 玩家环绕武器的子弹对象，不发射，挂在玩家身边绕圈
// 4发子弹同时存在，以玩家位置为中心旋转
// orbitCount记录转过的象限数，影响渲染时的电荷聚集效果
class PlayerBulletOrbital {
public:
    // 构造函数
    PlayerBulletOrbital();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置环绕中心、初始角度、环绕半径
    void spawn(sf::Vector2f center, float angleOff, float radius);
    // 每帧更新：更新angleOffset，计算新位置，检测象限变化
    void update(float dt, const sf::Vector2f& playerPos);
    // 渲染：多层圆形子弹 + 电荷聚集效果
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                       // 是否激活
    void deactivate();                          // 设为非激活
    sf::Vector2f getPosition() const;           // 获取位置（当前环绕位置）
    float getRadius() const;                    // 获取碰撞半径
    int getDamage() const;                       // 获取伤害值
    int getOrbitCount() const;                  // 获取转过的象限数
    void resetOrbitCount();                     // 重置orbitCount

private:
    // ---- 环绕状态 ----
    sf::Vector2f position;    // 当前环绕位置
    float angleOffset;       // 当前角度偏移（弧度）
    float orbitRadius;       // 环绕半径
    float orbitSpeed;        // 环绕角速度（弧度/秒）

    // ---- 渲染图形 ----
    sf::CircleShape shape;   // 主体圆形

    // ---- 状态 ----
    bool active;   // 是否激活
    int damage;    // 子弹伤害值
    int orbitCount;      // 转过的象限数（每转过一个象限+1）
    int lastQuadrant;    // 上次所在的象限（用于检测象限变化）
};