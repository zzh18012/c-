// ============================================================
// PlayerBulletPiercing.cpp - 穿透子弹类实现
// ============================================================
// 职责：玩家穿透武器发射的子弹（对象池模式）
// 高速穿透多个敌人，碰撞后不消失，继续飞行
// 特殊效果：连锁闪电（PierceChain特效）
// 渲染效果：青色矩形子弹（光晕+主体+核心）
// ============================================================

#include "PlayerBulletPiercing.h"
#include "core/Config.h"
#include <cmath> // std::atan2

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), velocity=(0,0), active=false
// damage=PLAYER_PIERCING_DAMAGE, angle=0, chained=false, chainTimer=0
// shape构造为矩形(12.f, 4.f)，填充颜色为青色
PlayerBulletPiercing::PlayerBulletPiercing()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , shape(sf::Vector2f(12.f, 4.f))
    , active(false)
    , damage(PLAYER_PIERCING_DAMAGE)
    , angle(0.f)
    , chained(false)
    , chainTimer(0.f)
{
    shape.setFillColor(sf::Color(100, 255, 200)); // 青色
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// pos: 子弹生成位置
// dir: 子弹飞行方向（归一化向量）
void PlayerBulletPiercing::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;                                      // 设置位置
    velocity = dir * PLAYER_PIERCING_SPEED;              // 速度 = 方向 × 穿透子弹速度
    angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f; // 弧度转度（用于设置矩形旋转）
    active = true;                                       // 标记激活
    damage = PLAYER_PIERCING_DAMAGE;                     // 重置伤害值
    chained = false;                                     // 重置连锁标记
    chainStartPos = pos;                                // 记录连锁起始位置
    chainDir = dir;                                      // 记录连锁方向
    chainTimer = 0.f;                                    // 重置连锁计时器
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 物理更新：position += velocity * dt
// 旋转更新：根据角度设置shape的旋转（SFML的setRotation接受度）
// 边界检测：超出屏幕±50像素则回收
void PlayerBulletPiercing::update(float dt) {
    if (!active) return;                    // 非激活子弹跳过
    position += velocity * dt;             // 位置更新
    shape.setPosition(position);           // 更新渲染图形位置
    shape.setRotation(angle); // 设置旋转角度
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

// ============================================================
// render - 渲染子弹
// ============================================================
// 3层矩形子弹：
// 1. 光晕（GlowShape）：1.5倍大小，较低透明度(100/255)
// 2. 主体（shape）：标准大小，青色
// 3. 核心（CoreShape）：60%×40%大小，高亮青色(200,255,240)
void PlayerBulletPiercing::render(sf::RenderWindow& window) const {
    if (!active) return;

    // ---- 第1层：光晕 ----
    sf::RectangleShape glowShape(shape.getSize() * 1.5f);
    glowShape.setOrigin(sf::Vector2f(shape.getSize().x * 0.75f, shape.getSize().y * 0.75f));
    glowShape.setPosition(position);
    glowShape.setRotation(shape.getRotation()); // 与主体保持同角度
    glowShape.setFillColor(sf::Color(100, 255, 200, 100));
    window.draw(glowShape);

    // ---- 第2层：主体 ----
    window.draw(shape);

    // ---- 第3层：核心 ----
    sf::RectangleShape coreShape(sf::Vector2f(shape.getSize().x * 0.6f, shape.getSize().y * 0.4f));
    coreShape.setOrigin(sf::Vector2f(coreShape.getSize().x * 0.5f, coreShape.getSize().y * 0.5f));
    coreShape.setPosition(position);
    coreShape.setRotation(shape.getRotation());
    coreShape.setFillColor(sf::Color(200, 255, 240));
    window.draw(coreShape);
}

// ============================================================
// 状态查询和连锁相关方法
// ============================================================
bool PlayerBulletPiercing::isActive() const { return active; }                                  // 返回激活状态
void PlayerBulletPiercing::deactivate() { active = false; }                                     // 设为非激活
sf::Vector2f PlayerBulletPiercing::getPosition() const { return position; }                     // 返回位置
float PlayerBulletPiercing::getRadius() const { return shape.getSize().y * 0.5f; }              // 返回碰撞半径（矩形短边的一半）
int PlayerBulletPiercing::getDamage() const { return damage; }                                  // 返回伤害值
bool PlayerBulletPiercing::hasChained() const { return chained; }                              // 返回是否已触发连锁
void PlayerBulletPiercing::triggerChain() { chained = true; }                                   // 标记为已连锁
// getChainTarget：连锁目标位置 = 起始位置 + 方向 × 连锁距离
sf::Vector2f PlayerBulletPiercing::getChainTarget() const {
    return chainStartPos + chainDir * PIERCE_CHAIN_RADIUS;
}
float PlayerBulletPiercing::getChainTimer() const { return chainTimer; }                       // 返回连锁计时器