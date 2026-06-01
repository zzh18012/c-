// ============================================================
// BossBullet.cpp - Boss子弹类实现
// ============================================================
// 职责：Boss子弹的生成、更新、渲染
// Boss使用对象池模式管理所有子弹（最多2000发）
// 每颗子弹渲染为有机等离子体球（外发光+主体+内核+脉动核心）
// ============================================================

#include "BossBullet.h"
#include "core/Config.h"

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), velocity=(0,0), active=false, damage=BOSS_BULLET_DAMAGE
// shape.setRadius设置子弹半径，setOrigin以圆心为基准点
// 这样后续shape.setPosition(pos)时，圆心会位于pos位置
BossBullet::BossBullet()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , active(false)
    , damage(BOSS_BULLET_DAMAGE)
{
    shape.setRadius(BOSS_BULLET_RADIUS);
    shape.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS, BOSS_BULLET_RADIUS));
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// pos: 子弹生成位置（屏幕坐标系）
// dir: 子弹飞行方向（归一化向量，如(1,0)向右，(0,-1)向上）
// speed: 子弹飞行速度大小（像素/秒）
// dmg: 子弹伤害值（命中玩家时扣减的HP）
void BossBullet::spawn(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg) {
    position = pos;                             // 设置当前位置
    velocity = dir * speed;                     // 速度 = 方向 × 速度（向量乘法）
    damage = dmg;                               // 设置伤害值
    active = true;                             // 标记为已激活（对象池中已占用）
    shape.setPosition(position);                // 更新渲染图形位置
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 上一帧的时间差（秒）
// 物理更新：position += velocity * dt（匀速直线运动）
// 边界检测：超出屏幕上下左右各50像素范围则回收（设为非激活）
void BossBullet::update(float dt) {
    if (!active) return;                        // 非激活子弹直接跳过（性能优化）
    position += velocity * dt;                 // 位置更新（v = ds/dt → ds = v*dt）
    shape.setPosition(position);                // 同步更新渲染图形位置

    // 边界检测：超出屏幕范围则回收子弹到对象池
    // ±50像素的缓冲区域，避免子弹刚超出就回收造成视觉问题
    if (position.x < -50 || position.x > WINDOW_WIDTH + 50 ||
        position.y < -50 || position.y > WINDOW_HEIGHT + 50) {
        active = false;                         // 设为非激活，对象池回收
    }
}

// ============================================================
// render - 渲染子弹
// ============================================================
// Boss子弹渲染为有机等离子体球，使用4层圆形叠加：
// 1. 外发光（OuterGlow）：最大半径，低透明度，暖色调
// 2. 主体（MainOrb）：标准半径，中透明度，冷色调
// 3. 内核（InnerCore）：较小半径，高透明度，更亮的冷色调
// 4. 脉动核心（Nucleus）：最小半径，完全不透明，白色
// 由外向内依次绘制，营造发光球体效果
void BossBullet::render(sf::RenderWindow& window) const {
    if (!active) return; // 非激活子弹不渲染

    sf::Vector2f pos = shape.getPosition(); // 获取当前绘制位置

    // ---- 第1层：外发光 ----
    // 半径 = BOSS_BULLET_RADIUS + 5.f，比主体大5像素
    // 颜色：红色255，绿色80，蓝色130（橙红色），透明度50/255≈20%
    // 效果：在主体周围形成柔和的红色光晕
    sf::CircleShape outerGlow(BOSS_BULLET_RADIUS + 5.f);
    outerGlow.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS + 5.f, BOSS_BULLET_RADIUS + 5.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 80, 130, 50));
    window.draw(outerGlow);

    // ---- 第2层：主体 ----
    // 半径 = BOSS_BULLET_RADIUS（标准子弹大小）
    // 颜色：红色255，绿色100，蓝色160（粉红色），透明度220/255≈86%
    sf::CircleShape mainOrb(BOSS_BULLET_RADIUS);
    mainOrb.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS, BOSS_BULLET_RADIUS));
    mainOrb.setPosition(pos);
    mainOrb.setFillColor(sf::Color(255, 100, 160, 220));
    window.draw(mainOrb);

    // ---- 第3层：内核 ----
    // 半径 = BOSS_BULLET_RADIUS * 0.5f（主体的一半）
    // 颜色：红色255，绿色200，蓝色220（亮粉白色），透明度255=完全不透明
    sf::CircleShape innerCore(BOSS_BULLET_RADIUS * 0.5f);
    innerCore.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS * 0.5f, BOSS_BULLET_RADIUS * 0.5f));
    innerCore.setPosition(pos);
    innerCore.setFillColor(sf::Color(255, 200, 220, 255));
    window.draw(innerCore);

    // ---- 第4层：脉动核心 ----
    // 半径 = BOSS_BULLET_RADIUS * 0.25f（主体的四分之一）
    // 颜色：纯白色(255,255,255)，最亮的一层
    // 效果：中心有一个明亮的白色核心，增强有机感
    sf::CircleShape nucleus(BOSS_BULLET_RADIUS * 0.25f);
    nucleus.setOrigin(sf::Vector2f(BOSS_BULLET_RADIUS * 0.25f, BOSS_BULLET_RADIUS * 0.25f));
    nucleus.setPosition(pos);
    nucleus.setFillColor(sf::Color(255, 255, 255, 255));
    window.draw(nucleus);
}

// ============================================================
// 状态查询方法
// ============================================================
bool BossBullet::isActive() const { return active; }                       // 返回激活状态
void BossBullet::deactivate() { active = false; }                          // 设为非激活（对象池回收）
sf::Vector2f BossBullet::getPosition() const { return position; }          // 返回当前位置
float BossBullet::getRadius() const { return BOSS_BULLET_RADIUS; }         // 返回碰撞半径（常量）
int BossBullet::getDamage() const { return damage; }                       // 返回伤害值