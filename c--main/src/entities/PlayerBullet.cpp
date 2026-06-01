// ============================================================
// PlayerBullet.cpp - 玩家普通子弹类实现
// ============================================================
// 职责：玩家普通武器发射的子弹（对象池模式）
// 单发、高射速、高伤害，弹道笔直
// 渲染效果：青色发光子弹（外发光+主体+亮心）
// ============================================================

#include "PlayerBullet.h"
#include "core/Config.h"

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：position=(0,0), velocity=(0,0), active=false, damage=PLAYER_BULLET_DAMAGE
// shape.setRadius设置子弹半径，setOrigin以圆心为基准点
PlayerBullet::PlayerBullet()
    : position(0.f, 0.f)
    , velocity(0.f, 0.f)
    , active(false)
    , damage(PLAYER_BULLET_DAMAGE)
{
    shape.setRadius(PLAYER_BULLET_RADIUS);
    shape.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS, PLAYER_BULLET_RADIUS));
}

// ============================================================
// spawn - 生成子弹
// ============================================================
// pos: 子弹生成位置（玩家炮口位置）
// dir: 子弹飞行方向（归一化向量，从玩家指向鼠标方向）
void PlayerBullet::spawn(sf::Vector2f pos, sf::Vector2f dir) {
    position = pos;                              // 设置当前位置
    velocity = dir * PLAYER_BULLET_SPEED;        // 速度 = 方向 × 速度（向量乘法）
    active = true;                               // 标记为已激活
    shape.setPosition(position);                 // 更新渲染图形位置
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 上一帧的时间差（秒）
// 物理更新：position += velocity * dt（匀速直线运动）
// 边界检测：超出屏幕上下左右各10像素范围则回收
void PlayerBullet::update(float dt) {
    if (!active) return;                        // 非激活子弹直接跳过
    position += velocity * dt;                 // 位置更新
    shape.setPosition(position);                // 同步更新渲染图形位置

    // 边界检测：±10像素缓冲区域
    if (position.x < -10 || position.x > WINDOW_WIDTH + 10 ||
        position.y < -10 || position.y > WINDOW_HEIGHT + 10) {
        active = false;                         // 设为非激活，对象池回收
    }
}

// ============================================================
// render - 渲染子弹
// ============================================================
// 玩家子弹渲染为青色发光子弹，使用3层圆形叠加：
// 1. 外发光（Glow）：最大半径，低透明度(60/255)，青色(0,255,200)
// 2. 主体（Core）：标准半径，完全不透明，青白色(150,255,230)
// 3. 亮心（Bright）：最小半径（40%），纯白，中心最亮
void PlayerBullet::render(sf::RenderWindow& window) const {
    if (!active) return; // 非激活子弹不渲染

    sf::Vector2f pos = shape.getPosition();

    // ---- 第1层：外发光 ----
    // 半径 = PLAYER_BULLET_RADIUS + 3.f，比主体大3像素
    // 透明度60/255≈24%，青色(0,255,200)
    sf::CircleShape glow(PLAYER_BULLET_RADIUS + 3.f);
    glow.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS + 3.f, PLAYER_BULLET_RADIUS + 3.f));
    glow.setPosition(pos);
    glow.setFillColor(sf::Color(0, 255, 200, 60));
    window.draw(glow);

    // ---- 第2层：主体 ----
    // 半径 = PLAYER_BULLET_RADIUS（标准大小）
    // 青白色(150,255,230)，完全不透明
    sf::CircleShape core(PLAYER_BULLET_RADIUS);
    core.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS, PLAYER_BULLET_RADIUS));
    core.setPosition(pos);
    core.setFillColor(sf::Color(150, 255, 230, 255));
    window.draw(core);

    // ---- 第3层：亮心 ----
    // 半径 = PLAYER_BULLET_RADIUS * 0.4f（40%大小）
    // 纯白色(255,255,255)，最亮的一层
    sf::CircleShape bright(PLAYER_BULLET_RADIUS * 0.4f);
    bright.setOrigin(sf::Vector2f(PLAYER_BULLET_RADIUS * 0.4f, PLAYER_BULLET_RADIUS * 0.4f));
    bright.setPosition(pos);
    bright.setFillColor(sf::Color(255, 255, 255, 255));
    window.draw(bright);
}

// ============================================================
// 状态查询方法
// ============================================================
void PlayerBullet::setDamage(int dmg) { damage = dmg; }             // 设置伤害值
bool PlayerBullet::isActive() const { return active; }              // 返回激活状态
void PlayerBullet::deactivate() { active = false; }                 // 设为非激活
sf::Vector2f PlayerBullet::getPosition() const { return position; } // 返回当前位置
float PlayerBullet::getRadius() const { return PLAYER_BULLET_RADIUS; } // 返回碰撞半径
int PlayerBullet::getDamage() const { return damage; }              // 返回伤害值