// ============================================================
// PlayerBulletOrbital.cpp - 环绕子弹类实现
// ============================================================
// 职责：玩家环绕武器发射的子弹（对象池模式）
// 4发子弹绕玩家旋转，持续伤害靠近的敌人
// 每转一圈（经过4个象限）增加orbitCount，影响渲染时的电荷聚集效果
// 渲染效果：多层青色圆形子弹 + orbitCount相关的电荷光晕
// ============================================================

#include "PlayerBulletOrbital.h"
#include "core/Config.h"
#include <cmath> // std::cos, std::sin

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), angleOffset=0, orbitRadius=PLAYER_ORBITAL_RADIUS
// orbitSpeed=PLAYER_ORBITAL_SPEED, active=false, damage=ORBITAL_BULLET_DAMAGE
// orbitCount=0, lastQuadrant=0
// shape构造为半径8.f的圆形，填充青色(0,255,180)
PlayerBulletOrbital::PlayerBulletOrbital()
    : position(0.f, 0.f)
    , angleOffset(0.f)
    , orbitRadius(PLAYER_ORBITAL_RADIUS)
    , orbitSpeed(PLAYER_ORBITAL_SPEED)
    , shape(8.f)
    , active(false)
    , damage(ORBITAL_BULLET_DAMAGE)
    , orbitCount(0)
    , lastQuadrant(0)
{
    shape.setFillColor(sf::Color(0, 255, 180)); // 青色
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// center: 环绕中心（玩家位置）
// angleOff: 初始角度偏移（弧度，4发子弹各差π/2）
// radius: 环绕半径（从Config或参数传入）
void PlayerBulletOrbital::spawn(sf::Vector2f center, float angleOff, float radius) {
    angleOffset = angleOff;                    // 设置角度偏移
    orbitRadius = radius;                      // 设置环绕半径
    orbitSpeed = PLAYER_ORBITAL_SPEED;          // 设置角速度
    // 根据中心点和角度计算初始位置：x = center.x + cos(angle) * radius
    position.x = center.x + std::cos(angleOffset) * orbitRadius;
    position.y = center.y + std::sin(angleOffset) * orbitRadius;
    active = true;                             // 标记激活
    damage = ORBITAL_BULLET_DAMAGE;            // 重置伤害值
    orbitCount = 0;                             // 重置环绕计数
    // 计算初始所在象限：angle / (π/2) 并取模4
    // 象限0: 0~π/2, 象限1: π/2~π, 象限2: π~3π/2, 象限3: 3π/2~2π
    lastQuadrant = static_cast<int>(angleOff / (3.14159f * 0.5f)) & 3;
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// playerPos: 玩家当前位置（作为环绕中心）
// 环绕更新：angleOffset += orbitSpeed * dt / orbitRadius（角速度=线速度/半径）
// 位置更新：position = playerPos + (cos(angle), sin(angle)) * radius
// 象限检测：每转过一个象限（π/2）orbitCount++
void PlayerBulletOrbital::update(float dt, const sf::Vector2f& playerPos) {
    if (!active) return;                                       // 非激活子弹跳过
    // 角速度 = orbitSpeed / orbitRadius（保证线速度恒定）
    angleOffset += orbitSpeed * dt / orbitRadius;
    // 重新计算位置（以玩家位置为中心）
    position.x = playerPos.x + std::cos(angleOffset) * orbitRadius;
    position.y = playerPos.y + std::sin(angleOffset) * orbitRadius;
    shape.setPosition(position);                               // 更新渲染图形位置

    // ---- 象限检测 ----
    // 将角度按π/2分割成4个象限，& 3等价于 % 4
    int currentQuadrant = static_cast<int>(angleOffset / (3.14159f * 0.5f)) & 3;
    if (currentQuadrant != lastQuadrant) {                     // 象限变化=转过90度
        orbitCount++;                                          // 环绕计数+1
        lastQuadrant = currentQuadrant;                        // 更新上次象限
    }
}

// ============================================================
// render - 渲染子弹
// ============================================================
// 4层圆形子弹 + 电荷聚集效果（orbitCount > 0时显示）
// 1. 外圈（Outer）：2倍半径，低透明度(80/255)
// 2. 中圈（Mid）：1.3倍半径，中透明度(150/255)
// 3. 主体（shape）：标准半径
// 4. 内心（Inner）：40%半径，高亮
// 5. 电荷聚集（ChargeGlow）：orbitCount相关大小和透明度
void PlayerBulletOrbital::render(sf::RenderWindow& window) const {
    if (!active) return;

    // ---- 第1层：外圈 ----
    sf::CircleShape outer(shape.getRadius() * 2.f);
    outer.setOrigin(sf::Vector2f(shape.getRadius() * 2.f, shape.getRadius() * 2.f));
    outer.setPosition(position);
    outer.setFillColor(sf::Color(0, 200, 150, 80));
    window.draw(outer);

    // ---- 第2层：中圈 ----
    sf::CircleShape mid(shape.getRadius() * 1.3f);
    mid.setOrigin(sf::Vector2f(shape.getRadius() * 1.3f, shape.getRadius() * 1.3f));
    mid.setPosition(position);
    mid.setFillColor(sf::Color(0, 255, 180, 150));
    window.draw(mid);

    // ---- 第3层：主体 ----
    window.draw(shape);

    // ---- 第4层：内心 ----
    sf::CircleShape inner(shape.getRadius() * 0.4f);
    inner.setOrigin(sf::Vector2f(shape.getRadius() * 0.4f, shape.getRadius() * 0.4f));
    inner.setPosition(position);
    inner.setFillColor(sf::Color(200, 255, 240));
    window.draw(inner);

    // ---- 第5层：电荷聚集效果 ----
    // orbitCount > 0时显示，转的圈数越多光晕越大越亮
    if (orbitCount > 0) {
        sf::CircleShape chargeGlow(shape.getRadius() * (1.f + orbitCount * 0.3f));
        chargeGlow.setOrigin(sf::Vector2f(shape.getRadius() * (1.f + orbitCount * 0.3f), shape.getRadius() * (1.f + orbitCount * 0.3f)));
        chargeGlow.setPosition(position);
        // 透明度随orbitCount增加：100 + orbitCount * 30，上限约220
        chargeGlow.setFillColor(sf::Color(0, 255, 180, 100 + orbitCount * 30));
        window.draw(chargeGlow);
    }
}

// ============================================================
// 状态查询方法
// ============================================================
bool PlayerBulletOrbital::isActive() const { return active; }                               // 返回激活状态
void PlayerBulletOrbital::deactivate() { active = false; }                                 // 设为非激活
sf::Vector2f PlayerBulletOrbital::getPosition() const { return position; }                 // 返回环绕位置
float PlayerBulletOrbital::getRadius() const { return shape.getRadius(); }                 // 返回碰撞半径
int PlayerBulletOrbital::getDamage() const { return damage; }                               // 返回伤害值
int PlayerBulletOrbital::getOrbitCount() const { return orbitCount; }                     // 返回环绕计数
void PlayerBulletOrbital::resetOrbitCount() { orbitCount = 0; }                            // 重置环绕计数