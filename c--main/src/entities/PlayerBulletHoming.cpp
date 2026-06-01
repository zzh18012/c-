// ============================================================
// PlayerBulletHoming.cpp - 追踪子弹类实现
// ============================================================
// 职责：玩家追踪武器发射的子弹（对象池模式）
// 自动追踪Boss方向，有最大转向速度限制（HOMING_TURN_RATE）
// 速度逐渐加快，从60%最高速到80%最高速
// 渲染效果：粉红色子弹（轨迹 + 光晕 + 主体 + 内心）
// ============================================================

#include "PlayerBulletHoming.h"
#include "core/Config.h"
#include <cmath> // std::atan2, std::cos, std::sin, std::sqrt, std::max, std::min

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), velocity=(0,0), angle=-90度（向上）
// active=false, damage=PLAYER_HOMING_DAMAGE
// shape构造为PLAYER_BULLET_RADIUS * 0.8f（比普通子弹小20%）
// 填充颜色为粉红色(255,100,200)
PlayerBulletHoming::PlayerBulletHoming()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , angle(-90.f)
    , shape(PLAYER_BULLET_RADIUS * 0.8f)
    , active(false)
    , damage(PLAYER_HOMING_DAMAGE)
{
    shape.setFillColor(sf::Color(255, 100, 200)); // 粉红色
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// pos: 子弹生成位置
// dir: 子弹初始飞行方向（玩家指向鼠标方向）
void PlayerBulletHoming::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;                                              // 设置位置
    float speed = PLAYER_BULLET_SPEED * 0.6f;                    // 初始速度为60%最高速
    velocity = dir * speed;                                      // 设置速度向量
    angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;        // 弧度转度
    active = true;                                               // 标记激活
    damage = PLAYER_HOMING_DAMAGE;                              // 重置伤害值
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// targetPos: 追踪目标位置（通常为Boss位置）
// 追踪算法：
// 1. 计算当前速度方向角度和目标方向角度
// 2. 计算角度差（归一化到[-π, π]范围）
// 3. 限制转向速度：turn = clamp(angleDiff * 0.05, -HOMING_TURN_RATE, HOMING_TURN_RATE)
// 4. 计算新角度和新速度
// 5. 速度逐渐加快：从60%到80%最高速
void PlayerBulletHoming::update(float dt, const sf::Vector2f& targetPos) {
    if (!active) return;                                       // 非激活子弹跳过

    // ---- 计算目标角度 ----
    // toTarget: 从子弹位置到目标的向量
    sf::Vector2f toTarget = targetPos - position;
    // targetAngle: 目标方向的角度（弧度）
    float targetAngle = std::atan2(toTarget.y, toTarget.x);

    // ---- 计算当前角度 ----
    float currentAngle = std::atan2(velocity.y, velocity.x);

    // ---- 计算角度差 ----
    // 归一化到[-π, π]范围，避免角度突变
    float angleDiff = targetAngle - currentAngle;
    while (angleDiff > 3.14159f) angleDiff -= 2.f * 3.14159f;
    while (angleDiff < -3.14159f) angleDiff += 2.f * 3.14159f;

    // ---- 限制转向速度 ----
    // turn = angleDiff * 0.05，但不超过HOMING_TURN_RATE
    float turn = std::max(-HOMING_TURN_RATE, std::min(HOMING_TURN_RATE, angleDiff * 0.05f));
    float newAngle = currentAngle + turn; // 新角度 = 当前角度 + 转向量

    // ---- 速度逐渐加快 ----
    // 当前速度大小 + 50 * dt，但不超过80%最高速
    float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    speed = std::min(speed + 50.f * dt, PLAYER_BULLET_SPEED * 0.8f);
    velocity = sf::Vector2f(std::cos(newAngle) * speed, std::sin(newAngle) * speed);

    // ---- 更新位置和角度 ----
    position += velocity * dt;                               // 位置更新
    angle = newAngle * 180.f / 3.14159f;                     // 更新角度（度）
    shape.setPosition(position);                             // 同步渲染图形位置

    // ---- 边界回收 ----
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;
    }
}

// ============================================================
// render - 渲染子弹
// ============================================================
// 4层粉红色子弹：
// 1. 轨迹（Trail）：根据上一帧速度方向向后延伸，透明度80/255
// 2. 光晕（Outer）：1.5倍半径，中透明度(120/255)
// 3. 主体（shape）：标准半径，粉红色
// 4. 内心（Inner）：40%半径，高亮粉白(255,220,255)
void PlayerBulletHoming::render(sf::RenderWindow& window) const {
    if (!active) return;

    // ---- 第1层：轨迹 ----
    // 根据上一帧的速度方向（velocity）向后延伸
    // 位置 = 当前位置 - velocity * 0.02
    sf::CircleShape trail(shape.getRadius() * 3.f);
    trail.setOrigin(sf::Vector2f(shape.getRadius() * 3.f, shape.getRadius() * 3.f));
    trail.setPosition(sf::Vector2f(position.x - velocity.x * 0.02f, position.y - velocity.y * 0.02f));
    trail.setFillColor(sf::Color(255, 50, 150, 80));
    window.draw(trail);

    // ---- 第2层：光晕 ----
    sf::CircleShape outer(shape.getRadius() * 1.5f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 1.5f, shape.getRadius() * 1.5f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(255, 100, 200, 120));
    window.draw(outer);

    // ---- 第3层：主体 ----
    window.draw(shape);

    // ---- 第4层：内心 ----
    sf::CircleShape inner(shape.getRadius() * 0.4f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.4f, shape.getRadius() * 0.4f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(255, 220, 255));
    window.draw(inner);
}

// ============================================================
// 状态查询方法
// ============================================================
bool PlayerBulletHoming::isActive() const { return active; }                               // 返回激活状态
void PlayerBulletHoming::deactivate() { active = false; }                                   // 设为非激活
void PlayerBulletHoming::setDamage(int dmg) { damage = dmg; }                              // 设置伤害值
sf::Vector2f PlayerBulletHoming::getPosition() const { return position; }                   // 返回位置
float PlayerBulletHoming::getRadius() const { return shape.getRadius(); }                   // 返回半径
int PlayerBulletHoming::getDamage() const { return damage; }                                // 返回伤害值