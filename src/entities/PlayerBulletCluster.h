// ============================================================
// PlayerBulletCluster.h - 集群子弹类声明
// ============================================================
// 职责：玩家集群武器发射的子弹（对象池模式）
// 母弹飞行0.5秒后爆炸成多发子弹（CLUSTER_SPLIT_COUNT发）
// 子弹向四周扩散，造成范围伤害
// 渲染效果：橙色子弹（光晕+主体+内心）+ 爆炸时的子弹分布
// ============================================================

#pragma once

// SFML图形库
#include <SFML/Graphics.hpp>

// 标准向量（用于split子弹方向计算等）
#include <vector>

// 配置文件（获取CLUSTER_SPLIT_COUNT等常量）
#include "core/Config.h"

// 前向声明Player（用于友元或特定接口，本类暂未用到）
class Player;

// ============================================================
// PlayerBulletCluster类
// ============================================================
// 玩家集群武器的子弹对象，特点：
// 1. 母弹飞行0.5秒后爆炸
// 2. 爆炸产生CLUSTER_SPLIT_COUNT发子弹向四周扩散
// 3. 子弹有重力效果（velocity.y += 200 * dt），向下掉落
// 由Player统一管理（clusterBullets数组），Game.cpp负责碰撞检测
class PlayerBulletCluster {
public:
    // 构造函数
    PlayerBulletCluster();

    // ---- 子弹生命周期 ----
    // 生成子弹：设置位置、朝向方向
    void spawn(sf::Vector2f pos, sf::Vector2f dir);
    // 每帧更新：位置 += 速度 × dt，应用重力，检测爆炸时机
    void update(float dt);
    // 渲染：母弹 + 爆炸后的子弹分布
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isActive() const;                         // 是否激活
    void deactivate();                             // 设为非激活
    bool shouldExplode() const;                    // 是否应该爆炸（0.5秒到且未爆炸）
    void triggerExplosion();                      // 触发爆炸（标记为已爆炸，设置splitCount）

    // ---- 碰撞和伤害 ----
    sf::Vector2f getPosition() const;             // 获取位置
    float getRadius() const;                      // 获取碰撞半径
    int getDamage() const;                        // 获取伤害值

    // ---- 爆炸分裂相关 ----
    bool hasExploded() const;                    // 是否已爆炸
    bool hasSpawnedSplit() const;               // 是否已生成子弹
    void setSplitSpawned();                      // 标记为已生成子弹
    void setDamage(int dmg);                     // 设置伤害值
    // 获取第index发子弹的方向（用于Game.cpp生成子弹）
    sf::Vector2f getSplitDirection(int index) const;

private:
    // ---- 物理属性 ----
    sf::Vector2f position;  // 子弹当前位置
    sf::Vector2f velocity;  // 子弹速度向量

    // ---- 渲染图形 ----
    sf::CircleShape shape; // 主体圆形（橙色）

    // ---- 状态 ----
    bool active;      // 是否激活
    bool exploded;    // 是否已爆炸
    int damage;       // 子弹伤害值
    float lifetime;   // 已飞行时间（秒）

    // ---- 爆炸分裂相关 ----
    bool splitSpawned;  // 是否已生成子弹（避免重复生成）
    int splitCount;     // 分裂数量（CLUSTER_SPLIT_COUNT）
};