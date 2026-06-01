// ============================================================
// PlayerBulletSpread.h - 玩家散射子弹类声明
// ============================================================
// 职责：玩家散射武器发射的子弹（对象池模式）
// 5发扇形弹幕，覆盖面广，可反弹一次
// 反弹后伤害降低（SPREAD_BOUNCE_DAMAGE_MULT）
// 渲染效果：蓝色发光子弹（外圈+主体+内心）
// ============================================================

#pragma once

// SFML图形库
#include <SFML/Graphics.hpp>

// ============================================================
// PlayerBulletSpread类
// ============================================================
// 玩家散射武器的子弹对象，5路扇形发射，可墙壁反弹一次
// 由Player统一管理（spreadBullets数组），Game.cpp负责碰撞检测
// 反弹后damage乘以SPREAD_BOUNCE_DAMAGE_MULT
class PlayerBulletSpread {
public:
    // 构造函数
    PlayerBulletSpread();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置位置、朝向方向、角度偏移（用于扇形分布）
    void spawn(sf::Vector2f pos, sf::Vector2f dir, float angleOffset);
    // 每帧更新：位置 += 速度 × dt，检测墙壁反弹
    void update(float dt);
    // 渲染：3层蓝色子弹（外圈 → 主体 → 内心）
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                        // 是否激活
    void deactivate();                            // 设为非激活
    sf::Vector2f getPosition() const;             // 获取位置
    float getRadius() const;                      // 获取碰撞半径
    int getDamage() const;                         // 获取伤害值
    void setDamage(int dmg);                       // 设置伤害值
    bool hasBounced() const;                      // 是否已反弹过
    void triggerBounce();                         // 触发反弹（标记为已反弹）

private:
    // ---- 物理属性 ----
    sf::Vector2f position;  // 子弹当前位置
    sf::Vector2f velocity; // 子弹速度向量

    // ---- 渲染图形 ----
    sf::CircleShape shape;  // 主体圆形（蓝色）

    // ---- 状态 ----
    bool active;   // 是否激活
    int damage;    // 子弹伤害值
    bool bounced;  // 是否已反弹过（反弹后不能再反弹，伤害降低）
};