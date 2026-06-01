// ============================================================
// PlayerBulletSpread.cpp - 玩家散射子弹类实现
// ============================================================
// 职责：玩家散射武器发射的子弹（对象池模式）
// 5发扇形弹幕，覆盖面广，可反弹一次
// 反弹后伤害降低（SPREAD_BOUNCE_DAMAGE_MULT）
// 渲染效果：蓝色发光子弹（外圈+主体+内心）
// ============================================================

#include "PlayerBulletSpread.h"
#include "core/Config.h"
#include <cmath> // std::atan2, std::cos, std::sin

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), velocity=(0,0), active=false, damage=PLAYER_BULLET_DAMAGE, bounced=false
// shape直接用PLAYER_BULLET_RADIUS构造，并设置填充颜色为蓝色
PlayerBulletSpread::PlayerBulletSpread()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , shape(PLAYER_BULLET_RADIUS)
    , active(false)
    , damage(PLAYER_BULLET_DAMAGE)
    , bounced(false)
{
    shape.setFillColor(sf::Color(0, 200, 255)); // 蓝色
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// pos: 子弹生成位置（玩家炮口）
// dir: 子弹基础朝向（玩家指向鼠标方向）
// angleOffset: 角度偏移（扇形分布用，每发子弹偏移不同角度）
void PlayerBulletSpread::spawn(sf::Vector2f pos, sf::Vector2f dir, float angleOffset) {
    position = pos;                                    // 设置位置
    float speed = PLAYER_BULLET_SPEED;                  // 获取速度大小
    // 计算实际角度：基础角度 + 偏移角度
    float angle = std::atan2(dir.y, dir.x) + angleOffset;
    // 计算速度向量：角度 → 坐标
    velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
    active = true;                                     // 标记激活
    damage = PLAYER_BULLET_DAMAGE;                     // 重置伤害值
    bounced = false;                                    // 重置反弹标记
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 物理更新：position += velocity * dt
// 反弹检测：碰到左右墙壁时velocity.x取反，碰到上下墙壁时velocity.y取反
// 反弹后标记bounced=true，damage乘以SPREAD_BOUNCE_DAMAGE_MULT（伤害降低）
void PlayerBulletSpread::update(float dt) {
    if (!active) return;                    // 非激活子弹跳过

    position += velocity * dt;             // 位置更新
    shape.setPosition(position);           // 同步渲染图形

    // ---- 反弹检测（未反弹过才检测）----
    if (!bounced) {
        // 左右墙壁反弹
        if (position.x <= PLAYER_BULLET_RADIUS || position.x >= WINDOW_WIDTH - PLAYER_BULLET_RADIUS) {
            velocity.x = -velocity.x;                       // X方向反转
            bounced = true;                                 // 标记已反弹
            damage = static_cast<int>(damage * SPREAD_BOUNCE_DAMAGE_MULT); // 伤害降低
            // 修正位置避免卡墙
            position.x = std::max(PLAYER_BULLET_RADIUS, std::min(position.x, WINDOW_WIDTH - PLAYER_BULLET_RADIUS));
        }
        // 上下墙壁反弹
        if (position.y <= PLAYER_BULLET_RADIUS || position.y >= WINDOW_HEIGHT - PLAYER_BULLET_RADIUS) {
            velocity.y = -velocity.y;                       // Y方向反转
            bounced = true;                                 // 标记已反弹
            damage = static_cast<int>(damage * SPREAD_BOUNCE_DAMAGE_MULT); // 伤害降低
            position.y = std::max(PLAYER_BULLET_RADIUS, std::min(position.y, WINDOW_HEIGHT - PLAYER_BULLET_RADIUS));
        }
    }

    // ---- 边界回收 ----
    // 超出屏幕±50像素则回收（比普通子弹更大的缓冲区域）
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

// ============================================================
// render - 渲染子弹
// ============================================================
// 3层蓝色子弹：
// 1. 外圈（Outer）：2倍半径，低透明度(80/255)，较大范围蓝色光晕
// 2. 主体（shape）：标准半径，蓝色(0,200,255)
// 3. 内心（Inner）：50%半径，高透明度白色(200,255,255)
void PlayerBulletSpread::render(sf::RenderWindow& window) const {
    if (!active) return;

    // ---- 第1层：外圈 ----
    sf::CircleShape outer(shape.getRadius() * 2.f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 2.f, shape.getRadius() * 2.f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(0, 150, 200, 80));
    window.draw(outer);

    // ---- 第2层：主体 ----
    window.draw(shape);

    // ---- 第3层：内心 ----
    sf::CircleShape inner(shape.getRadius() * 0.5f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.5f, shape.getRadius() * 0.5f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(200, 255, 255, 255));
    window.draw(inner);
}

// ============================================================
// 状态查询方法
// ============================================================
bool PlayerBulletSpread::isActive() const { return active; }                          // 返回激活状态
void PlayerBulletSpread::deactivate() { active = false; }                              // 设为非激活
sf::Vector2f PlayerBulletSpread::getPosition() const { return position; }              // 返回位置
float PlayerBulletSpread::getRadius() const { return shape.getRadius(); }              // 返回半径
int PlayerBulletSpread::getDamage() const { return damage; }                           // 返回伤害值
bool PlayerBulletSpread::hasBounced() const { return bounced; }                        // 返回是否已反弹
void PlayerBulletSpread::setDamage(int dmg) { damage = dmg; }                         // 设置伤害值
void PlayerBulletSpread::triggerBounce() { bounced = true; }                           // 标记为已反弹