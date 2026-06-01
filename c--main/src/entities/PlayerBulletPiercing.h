// ============================================================
// PlayerBulletPiercing.h - 穿透子弹类声明
// ============================================================
// 职责：玩家穿透武器发射的子弹（对象池模式）
// 高速穿透多个敌人，碰撞后不消失，继续飞行
// 特殊效果：连锁闪电（PierceChain特效）
// 渲染效果：青色矩形子弹（光晕+主体+核心）
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::RectangleShape, sf::RenderWindow）
#include <SFML/Graphics.hpp>

// ============================================================
// PlayerBulletPiercing类
// ============================================================
// 玩家穿透武器的子弹对象，特点是：
// 1. 碰撞后不消失（穿透多个敌人）
// 2. 有连锁闪电特效（chain效果）
// 3. 矩形形状（细长），高速飞行
// 由Player统一管理（piercingBullets数组），Game.cpp负责碰撞检测
class PlayerBulletPiercing {
public:
    // 构造函数
    PlayerBulletPiercing();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置位置、朝向方向
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    // 每帧更新：位置 += 速度 × dt，更新旋转角度
    void update(float dt);
    // 渲染：矩形子弹（光晕 → 主体 → 核心）
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                       // 是否激活
    void deactivate();                          // 设为非激活
    sf::Vector2f getPosition() const;           // 获取位置
    float getRadius() const;                    // 获取碰撞半径（取矩形短边的一半）
    int getDamage() const;                       // 获取伤害值
    bool hasChained() const;                   // 是否已触发连锁
    void triggerChain();                        // 触发连锁（标记为已连锁）

    // ---- 连锁特效相关 ----
    // 获取连锁目标位置：从chainStartPos沿chainDir方向PIERCE_CHAIN_RADIUS距离处
    sf::Vector2f getChainTarget() const;
    float getChainTimer() const;               // 获取连锁计时器

private:
    // ---- 物理属性 ----
    sf::Vector2f position;  // 子弹当前位置
    sf::Vector2f velocity; // 子弹速度向量（方向 × 速度）

    // ---- 渲染图形 ----
    // 使用RectangleShape绘制矩形子弹（细长形状）
    sf::RectangleShape shape;

    // ---- 状态 ----
    bool active;   // 是否激活
    int damage;    // 子弹伤害值
    float angle;   // 子弹角度（度，用于设置shape旋转）

    // ---- 连锁特效相关 ----
    bool chained;          // 是否已触发连锁
    sf::Vector2f chainStartPos;  // 连锁起始位置（发射位置）
    sf::Vector2f chainDir;        // 连锁方向（发射方向）
    float chainTimer;             // 连锁计时器
};