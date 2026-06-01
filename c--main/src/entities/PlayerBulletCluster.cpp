// ============================================================
// PlayerBulletCluster.cpp - 集群子弹类实现
// ============================================================
// 职责：玩家集群武器发射的子弹（对象池模式）
// 母弹飞行0.5秒后爆炸成多发子弹向四周扩散
// 物理效果：重力（velocity.y += 200 * dt）
// 渲染效果：橙色子弹（光晕+主体+内心）+ 爆炸时的子弹分布
// ============================================================

#include "PlayerBulletCluster.h"
#include "core/Config.h"
#include <cmath> // std::cos, std::sin

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), velocity=(0,0), active=false, exploded=false
// damage=PLAYER_BULLET_DAMAGE, lifetime=0, splitSpawned=false, splitCount=0
// shape构造为PLAYER_BULLET_RADIUS * 1.5f（比普通子弹大50%）
// 填充颜色为橙色(255,150,50)
PlayerBulletCluster::PlayerBulletCluster()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , shape(PLAYER_BULLET_RADIUS * 1.5f)
    , active(false)
    , exploded(false)
    , damage(PLAYER_BULLET_DAMAGE)
    , lifetime(0.f)
    , splitSpawned(false)
    , splitCount(0)
{
    shape.setFillColor(sf::Color(255, 150, 50)); // 橙色
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// pos: 子弹生成位置
// dir: 子弹飞行方向（归一化向量）
void PlayerBulletCluster::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;                                 // 设置位置
    velocity = dir * PLAYER_CLUSTER_SPEED;         // 速度 = 方向 × 集群子弹速度
    active = true;                                  // 标记激活
    exploded = false;                               // 重置爆炸标记
    damage = PLAYER_BULLET_DAMAGE;                  // 重置伤害值
    lifetime = 0.f;                                 // 重置已飞行时间
    splitSpawned = false;                           // 重置子弹生成标记
    splitCount = 0;                                 // 重置分裂数量
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 物理更新：position += velocity * dt
// 重力效果：velocity.y += 200 * dt（每帧增加200像素/秒的向下速度）
// 爆炸检测：lifetime > 0.5秒且未爆炸时触发爆炸
void PlayerBulletCluster::update(float dt) {
    if (!active) return;                    // 非激活子弹跳过

    position += velocity * dt;             // 位置更新
    velocity.y += 200.f * dt;              // 重力效果：每帧向下加速
    lifetime += dt;                         // 飞行时间累加
    shape.setPosition(position);            // 同步渲染图形位置

    // ---- 爆炸检测 ----
    // 飞行超过0.5秒且未爆炸时触发爆炸
    if (lifetime > 0.5f && !exploded) {
        triggerExplosion();
    }

    // ---- 边界回收 ----
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

// ============================================================
// triggerExplosion - 触发爆炸
// ============================================================
// 标记为已爆炸，设置分裂数量
// 注意：这个方法只是标记爆炸，实际子弹生成在Game.cpp中进行
void PlayerBulletCluster::triggerExplosion() {
    exploded = true;                         // 标记为已爆炸
    splitCount = CLUSTER_SPLIT_COUNT;        // 设置分裂数量
}

// ============================================================
// render - 渲染子弹
// ============================================================
// 未爆炸时：3层橙色子弹
// 1. 光晕（Outer）：2倍半径，低透明度(100/255)
// 2. 主体（shape）：1.5倍普通子弹半径，橙色
// 3. 内心（Inner）：50%半径，亮橙色(255,220,150)
// 爆炸后（未生成子弹）：额外绘制爆炸分布图（8发子弹围绕母弹位置）
void PlayerBulletCluster::render(sf::RenderWindow& window) const {
    if (!active) return;

    // ---- 第1层：光晕 ----
    sf::CircleShape outer(shape.getRadius() * 2.f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 2.f, shape.getRadius() * 2.f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(255, 100, 30, 100));
    window.draw(outer);

    // ---- 第2层：主体 ----
    window.draw(shape);

    // ---- 第3层：内心 ----
    sf::CircleShape inner(shape.getRadius() * 0.5f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.5f, shape.getRadius() * 0.5f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(255, 220, 150));
    window.draw(inner);

    // ---- 爆炸分布预览（爆炸后但未生成子弹时）----
    // 在母弹周围显示8个位置点，表示即将分裂的子弹方向
    if (exploded && !splitSpawned) {
        for (int i = 0; i < splitCount; ++i) {
            // 计算每个子弹的方向：等分圆周 + 0.3弧度偏移
            float angle = 3.14159f * 2.f * i / splitCount + 0.3f;
            // 计算子弹位置：母弹位置 + 方向 × 2倍子弹半径
            sf::Vector2f offset(std::cos(angle) * PLAYER_BULLET_RADIUS * 2.f, std::sin(angle) * PLAYER_BULLET_RADIUS * 2.f);
            // 绘制小圆点表示子弹位置
            sf::CircleShape splitBullet(PLAYER_BULLET_RADIUS * 0.6f);
            splitBullet.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS * 0.6f, PLAYER_BULLET_RADIUS * 0.6f));
            splitBullet.setPosition(position + offset);
            splitBullet.setFillColor(sf::Color(255, 200, 80, 220));
            window.draw(splitBullet);
        }
    }
}

// ============================================================
// 状态查询和分裂相关方法
// ============================================================
bool PlayerBulletCluster::isActive() const { return active; }                                // 返回激活状态
void PlayerBulletCluster::deactivate() { active = false; }                                   // 设为非激活
// shouldExplode：lifetime > 0.5秒 且 未爆炸 = 应该爆炸
bool PlayerBulletCluster::shouldExplode() const { return lifetime > 0.5f && !exploded; }
sf::Vector2f PlayerBulletCluster::getPosition() const { return position; }                   // 返回位置
float PlayerBulletCluster::getRadius() const { return shape.getRadius(); }                   // 返回半径
int PlayerBulletCluster::getDamage() const { return damage; }                                 // 返回伤害值
bool PlayerBulletCluster::hasExploded() const { return exploded; }                           // 返回是否已爆炸
bool PlayerBulletCluster::hasSpawnedSplit() const { return splitSpawned; }                   // 返回是否已生成子弹
void PlayerBulletCluster::setSplitSpawned() { splitSpawned = true; }                         // 标记为已生成子弹
void PlayerBulletCluster::setDamage(int dmg) { damage = dmg; }                                // 设置伤害值
// getSplitDirection：获取第index发子弹的方向向量
// 角度 = index / splitCount * 2π + 0.3弧度偏移
sf::Vector2f PlayerBulletCluster::getSplitDirection(int index) const {
    float angle = 3.14159f * 2.f * index / splitCount + 0.3f;
    return sf::Vector2f(std::cos(angle), std::sin(angle));
}