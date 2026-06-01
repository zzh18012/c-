// ============================================================
// Item.cpp - 道具类实现
// ============================================================
// 职责：游戏中掉落的12种道具的生成、更新、渲染
// 道具在Boss死亡或阶段转换时生成，被玩家碰撞后触发效果
// 每种道具有独特的外观（12种不同的几何图形组合）
// ============================================================

#include "Item.h"
#include "core/Config.h"
#include "systems/ParticleSystem.h"
#include <cmath>      // std::cosf, std::sinf
#include <cstdint>    // uint8_t（SFML颜色透明度参数类型）

// ============================================================
// 全局粒子系统指针
// ============================================================
// Item不拥有ParticleSystem，只是通过此指针调用其接口
// 在Game.cpp初始化时通过setParticleSystem设置
static ParticleSystem* gParticles = nullptr;

// 设置全局粒子系统指针（Game.cpp调用）
void Item::setParticleSystem(ParticleSystem* ps) { gParticles = ps; }

// ============================================================
// 构造函数
// ============================================================
// 初始化为默认值：type=HealCore, position=(0,0), active=false, lifetime=0
Item::Item()
    : type(ItemType::HealCore)
    , position(0.f, 0.f)
    , active(false)
    , lifetime(0.f)
    , rotationAngle(0.f)
    , pulsePhase(0.f)
{
}

// ============================================================
// spawn - 生成道具
// ============================================================
// t: 道具类型
// pos: 生成位置（屏幕坐标系，通常是Boss死亡位置或阶段转换位置）
void Item::spawn(ItemType t, sf::Vector2f pos) {
    type = t;                       // 设置道具类型
    position = pos;                 // 设置生成位置
    active = true;                  // 标记为已激活
    lifetime = ITEM_LIFETIME;       // 重置生命周期（Config中定义）
    rotationAngle = 0.f;            // 重置旋转角度
    pulsePhase = 0.f;               // 重置脉动相位

    // 生成轨道粒子效果（环绕道具的小光点）
    if (gParticles) {
        sf::Color col = getColor(type); // 获取道具对应颜色
        // 在道具周围生成8个粒子，半径为ITEM_RADIUS + 12.f
        gParticles->spawnOrbitParticles(position, col, 8, ITEM_RADIUS + 12.f);
    }
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 更新内容：生命周期倒计时 → 旋转角度递增 → 脉动相位递增 → 生成轨道粒子
void Item::update(float dt) {
    if (!active) return;                   // 非激活道具跳过

    lifetime -= dt;                         // 生命周期倒计时
    if (lifetime <= 0.f) {                 // 生命周期结束
        active = false;                     // 设为非激活（对象池回收）
        return;
    }

    // 旋转角度：每秒增加120度（rotationAngle单位是度）
    rotationAngle += dt * 120.f;
    // 脉动相位：每秒增加5弧度（用于光晕明暗变化）
    pulsePhase += dt * 5.f;

    // 生成轨道粒子（持续生成营造持续发光效果）
    if (gParticles && active) {
        gParticles->spawnOrbitParticles(position, getColor(type), 1, ITEM_RADIUS + 10.f);
    }
}

// ============================================================
// render - 渲染道具
// ============================================================
// 根据道具类型分发到对应的draw方法
// 每个draw方法绘制独特的几何形状组合
void Item::render(sf::RenderWindow& window) const {
    if (!active) return; // 非激活道具不渲染

    // glow: 0.85~1.0之间周期性变化（脉动效果）
    float glow = 0.85f + 0.15f * std::sin(pulsePhase);
    // angle: 旋转角度转弧度（SFML的setRotation接受弧度）
    float angle = rotationAngle * 0.0174533f; // 度转弧度：angle * π/180

    // switch分发到对应的绘制方法
    switch (type) {
        case ItemType::HealCore:    drawHealCore(window, position, angle, glow); break;
        case ItemType::ShieldOrb:   drawShieldOrb(window, position, angle, glow); break;
        case ItemType::Overdrive:   drawOverdrive(window, position, angle, glow); break;
        case ItemType::BulletTime:  drawBulletTime(window, position, angle, glow); break;
        case ItemType::NovaBomb:    drawNovaBomb(window, position, angle, glow); break;
        case ItemType::DashBattery: drawDashBattery(window, position, angle, glow); break;
        case ItemType::WeaponEssence: drawWeaponEssence(window, position, angle, glow); break;
        case ItemType::PhaseCrystal: drawPhaseCrystal(window, position, angle, glow); break;
        case ItemType::SpeedCoil:   drawSpeedCoil(window, position, angle, glow); break;
        case ItemType::AttackModule: drawAttackModule(window, position, angle, glow); break;
        case ItemType::NovaCore:    drawNovaCore(window, position, angle, glow); break;
    }
}

// ============================================================
// drawHealCore - 绘制治疗核心
// ============================================================
// 外观：绿色六边形 + 白色十字（医疗符号）+ 外圈光晕 + 4个轨道小点
// 颜色主题：绿色(60,255,100)
void Item::drawHealCore(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 外圈光晕 ----
    // 半径 = ITEM_RADIUS + 8.f，比主体大8像素
    // 透明度随glow变化（30 * glow）
    sf::CircleShape outerGlow(ITEM_RADIUS + 8.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 8.f, ITEM_RADIUS + 8.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(60, 255, 100, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    // ---- 六边形主体 ----
    // 使用sf::ConvexShape绘制自定义六边形
    sf::ConvexShape hex;
    hex.setPointCount(6); // 6个顶点构成六边形
    float r = ITEM_RADIUS * glow; // 半径随glow脉动
    for (int i = 0; i < 6; ++i) {
        // 每个顶点角度：angle + i * 60度
        float a = angle + 3.14159f * 2.f * i / 6.f;
        hex.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    hex.setOrigin(sf::Vector2f(0.f, 0.f)); // 原点设为(0,0)配合setPosition为中心点
    hex.setPosition(pos);
    hex.setFillColor(sf::Color(60, 255, 100, 220));
    hex.setOutlineColor(sf::Color(150, 255, 180)); // 浅绿色边框
    hex.setOutlineThickness(2.f);
    window.draw(hex);

    // ---- 白色十字（医疗符号）----
    // 垂直条
    sf::RectangleShape vBar(sf::Vector2f(4.f, ITEM_RADIUS * 0.8f));
    vBar.setOrigin(sf::Vector2f(2.f, ITEM_RADIUS * 0.4f)); // 中心点偏移使十字中心在pos
    vBar.setPosition(pos);
    vBar.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(vBar);

    // 水平条
    sf::RectangleShape hBar(sf::Vector2f(ITEM_RADIUS * 0.8f, 4.f));
    hBar.setOrigin(sf::Vector2f(ITEM_RADIUS * 0.4f, 2.f));
    hBar.setPosition(pos);
    hBar.setFillColor(sf::Color(255, 255, 255, 200));
    window.draw(hBar);

    // ---- 4个轨道小点 ----
    // 环绕在六边形外围，随angle旋转
    for (int i = 0; i < 4; ++i) {
        float a = angle + 3.14159f * 2.f * i / 4.f;
        sf::CircleShape dot(3.f);
        dot.setOrigin(sf::Vector2f(3.f, 3.f));
        // 位置 = 中心 + 方向向量 × (半径 + 6.f)
        dot.setPosition(pos + sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 6.f), std::sinf(a) * (ITEM_RADIUS + 6.f)));
        dot.setFillColor(sf::Color(60, 255, 100, static_cast<uint8_t>(200 * glow)));
        window.draw(dot);
    }
}

// ============================================================
// drawShieldOrb - 绘制护盾球
// ============================================================
// 外观：蓝色六边形 + 盾牌图标 + 3个轨道点 + 外圈光环
// 颜色主题：蓝色(60,140,255)
void Item::drawShieldOrb(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 外圈光环 ----
    // 带边框的圆环，填充低透明蓝色
    sf::CircleShape outerRing(ITEM_RADIUS + 6.f);
    outerRing.setOrigin(sf::Vector2f(ITEM_RADIUS + 6.f, ITEM_RADIUS + 6.f));
    outerRing.setPosition(pos);
    outerRing.setFillColor(sf::Color(60, 140, 255, static_cast<uint8_t>(25 * glow)));
    outerRing.setOutlineColor(sf::Color(60, 140, 255, static_cast<uint8_t>(150 * glow)));
    outerRing.setOutlineThickness(2.f);
    window.draw(outerRing);

    // ---- 六边形主体 ----
    sf::ConvexShape hex;
    hex.setPointCount(6);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        hex.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    hex.setOrigin(sf::Vector2f(0.f, 0.f));
    hex.setPosition(pos);
    hex.setFillColor(sf::Color(60, 140, 255, 210));
    hex.setOutlineColor(sf::Color(120, 200, 255));
    hex.setOutlineThickness(2.f);
    window.draw(hex);

    // ---- 盾牌图标（五边形）----
    // 5个顶点构成盾牌形状
    sf::ConvexShape shieldIcon;
    shieldIcon.setPointCount(5);
    shieldIcon.setPoint(0, sf::Vector2f(0.f, -8.f));      // 顶部尖端
    shieldIcon.setPoint(1, sf::Vector2f(6.f, -4.f));       // 右上
    shieldIcon.setPoint(2, sf::Vector2f(6.f, 4.f));        // 右下
    shieldIcon.setPoint(3, sf::Vector2f(0.f, 8.f));        // 底部
    shieldIcon.setPoint(4, sf::Vector2f(-6.f, 4.f));       // 左下
    shieldIcon.setOrigin(sf::Vector2f(0.f, 0.f));
    shieldIcon.setPosition(pos);
    shieldIcon.setFillColor(sf::Color(180, 220, 255, 220));
    window.draw(shieldIcon);

    // ---- 3个轨道点 ----
    // 随pulsePhase额外旋转，产生流动感
    for (int i = 0; i < 3; ++i) {
        float a = angle + 3.14159f * 2.f * i / 3.f + pulsePhase * 0.5f;
        sf::CircleShape dot(2.5f);
        dot.setOrigin(sf::Vector2f(2.5f, 2.5f));
        dot.setPosition(pos + sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 4.f), std::sinf(a) * (ITEM_RADIUS + 4.f)));
        dot.setFillColor(sf::Color(100, 180, 255, static_cast<uint8_t>(220 * glow)));
        window.draw(dot);
    }
}

// ============================================================
// drawOverdrive - 绘制火力超载
// ============================================================
// 外观：火焰光环 + 箭头/飞镖形状 + 速度线 + 核心光点
// 颜色主题：橙红色(255,100,40)
void Item::drawOverdrive(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 火焰光环 ----
    sf::CircleShape flameAura(ITEM_RADIUS + 10.f);
    flameAura.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    flameAura.setPosition(pos);
    flameAura.setFillColor(sf::Color(255, 80, 20, static_cast<uint8_t>(35 * glow)));
    window.draw(flameAura);

    // ---- 箭头/飞镖形状 ----
    // 5个顶点构成箭头形状
    sf::ConvexShape arrow;
    arrow.setPointCount(5);
    arrow.setPoint(0, sf::Vector2f(0.f, -ITEM_RADIUS * glow));                      // 顶部
    arrow.setPoint(1, sf::Vector2f(ITEM_RADIUS * 0.6f * glow, ITEM_RADIUS * 0.3f * glow)); // 右上
    arrow.setPoint(2, sf::Vector2f(ITEM_RADIUS * 0.25f * glow, ITEM_RADIUS * 0.3f * glow)); // 中右
    arrow.setPoint(3, sf::Vector2f(ITEM_RADIUS * 0.25f * glow, ITEM_RADIUS * glow)); // 下右
    arrow.setPoint(4, sf::Vector2f(-ITEM_RADIUS * 0.25f * glow, ITEM_RADIUS * glow)); // 下左
    arrow.setOrigin(sf::Vector2f(0.f, 0.f));
    arrow.setPosition(pos);
    arrow.setRotation(sf::radians(rotationAngle * 0.0174533f)); // 随rotationAngle旋转
    arrow.setFillColor(sf::Color(255, 100, 40, 230));
    arrow.setOutlineColor(sf::Color(255, 200, 100));
    arrow.setOutlineThickness(2.f);
    window.draw(arrow);

    // ---- 3条速度线 ----
    // 表示快速移动的感觉
    for (int i = 0; i < 3; ++i) {
        float offset = (i - 1) * 8.f; // -8, 0, +8
        sf::RectangleShape line(sf::Vector2f(2.f, ITEM_RADIUS * 0.6f * glow));
        line.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.3f * glow));
        line.setPosition(pos + sf::Vector2f(offset, ITEM_RADIUS * 0.5f * glow));
        line.setFillColor(sf::Color(255, 150, 50, static_cast<uint8_t>(180 * glow)));
        window.draw(line);
    }

    // ---- 核心光点 ----
    sf::CircleShape core(5.f);
    core.setOrigin(sf::Vector2f(5.f, 5.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 200, 250));
    window.draw(core);
}

// ============================================================
// drawBulletTime - 绘制子弹时间
// ============================================================
// 外观：八边形表盘 + 时针/分针 + 中心点 + 6个轨道闪光
// 颜色主题：紫色(180,60,255)
void Item::drawBulletTime(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 时间刻度盘背景 ----
    sf::CircleShape dial(ITEM_RADIUS + 5.f);
    dial.setOrigin(sf::Vector2f(ITEM_RADIUS + 5.f, ITEM_RADIUS + 5.f));
    dial.setPosition(pos);
    dial.setFillColor(sf::Color(180, 60, 255, static_cast<uint8_t>(30 * glow)));
    dial.setOutlineColor(sf::Color(180, 60, 255, static_cast<uint8_t>(120 * glow)));
    dial.setOutlineThickness(2.f);
    window.draw(dial);

    // ---- 八边形表盘 ----
    sf::ConvexShape octo;
    octo.setPointCount(8);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 8; ++i) {
        float a = angle + 3.14159f * 2.f * i / 8.f;
        octo.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    octo.setOrigin(sf::Vector2f(0.f, 0.f));
    octo.setPosition(pos);
    octo.setFillColor(sf::Color(140, 40, 220, 200));
    octo.setOutlineColor(sf::Color(200, 120, 255));
    octo.setOutlineThickness(2.f);
    window.draw(octo);

    // ---- 时针 ----
    // 旋转速度较慢（0.0087266 = π/360，每秒转0.5度）
    sf::RectangleShape hourHand(sf::Vector2f(3.f, ITEM_RADIUS * 0.35f));
    hourHand.setOrigin(sf::Vector2f(1.5f, ITEM_RADIUS * 0.35f));
    hourHand.setPosition(pos);
    hourHand.setRotation(sf::radians(rotationAngle * 0.0087266f));
    hourHand.setFillColor(sf::Color(255, 220, 255, 230));
    window.draw(hourHand);

    // ---- 分针 ----
    // 旋转速度较快（0.0261799 = π/120，每秒转1.5度）
    sf::RectangleShape minHand(sf::Vector2f(2.f, ITEM_RADIUS * 0.5f));
    minHand.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.5f));
    minHand.setPosition(pos);
    minHand.setRotation(sf::radians(-rotationAngle * 0.0261799f)); // 逆时针
    minHand.setFillColor(sf::Color(255, 220, 255, 230));
    window.draw(minHand);

    // ---- 中心点 ----
    sf::CircleShape center(4.f);
    center.setOrigin(sf::Vector2f(4.f, 4.f));
    center.setPosition(pos);
    center.setFillColor(sf::Color(255, 255, 255, 240));
    window.draw(center);

    // ---- 6个轨道闪光 ----
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        sf::CircleShape sparkle(2.f);
        sparkle.setOrigin(sf::Vector2f(2.f, 2.f));
        sparkle.setPosition(pos + sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 3.f), std::sinf(a) * (ITEM_RADIUS + 3.f)));
        sparkle.setFillColor(sf::Color(220, 150, 255, static_cast<uint8_t>(200 * glow)));
        window.draw(sparkle);
    }
}

// ============================================================
// drawNovaBomb - 绘制新星炸弹
// ============================================================
// 外观：强烈白色光晕 + 8角星形 + 内六角星 + 12条放射线 + 核心
// 颜色主题：白色/黄色(255,255,200)
void Item::drawNovaBomb(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 强烈白色光晕 ----
    sf::CircleShape outerGlow(ITEM_RADIUS + 14.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 14.f, ITEM_RADIUS + 14.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 255, 200, static_cast<uint8_t>(40 * glow)));
    window.draw(outerGlow);

    // ---- 8角星形（外层）----
    // 交替使用r2和r1制造星形效果
    sf::ConvexShape star;
    star.setPointCount(8);
    float r1 = ITEM_RADIUS * 0.5f * glow;
    float r2 = ITEM_RADIUS * glow;
    for (int i = 0; i < 8; ++i) {
        float r = (i % 2 == 0) ? r2 : r1; // 偶数顶点用r2，奇数用r1
        float a = angle + 3.14159f * 2.f * i / 8.f;
        star.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    star.setOrigin(sf::Vector2f(0.f, 0.f));
    star.setPosition(pos);
    star.setFillColor(sf::Color(255, 255, 220, 230));
    star.setOutlineColor(sf::Color(255, 255, 180));
    star.setOutlineThickness(2.f);
    window.draw(star);

    // ---- 内六角星（反向旋转）----
    sf::ConvexShape innerStar;
    innerStar.setPointCount(6);
    float ir = ITEM_RADIUS * 0.4f * glow;
    for (int i = 0; i < 6; ++i) {
        float a = -angle + 3.14159f * 2.f * i / 6.f; // -angle实现反向旋转
        innerStar.setPoint(i, sf::Vector2f(std::cosf(a) * ir, std::sinf(a) * ir));
    }
    innerStar.setOrigin(sf::Vector2f(0.f, 0.f));
    innerStar.setPosition(pos);
    innerStar.setFillColor(sf::Color(255, 255, 255, 240));
    window.draw(innerStar);

    // ---- 12条放射线 ----
    for (int i = 0; i < 12; ++i) {
        float a = angle + 3.14159f * 2.f * i / 12.f;
        sf::RectangleShape ray(sf::Vector2f(2.f, ITEM_RADIUS * 0.3f * glow));
        ray.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.3f * glow));
        // 位置在半径60%处
        ray.setPosition(pos + sf::Vector2f(std::cosf(a) * ITEM_RADIUS * 0.6f, std::sinf(a) * ITEM_RADIUS * 0.6f));
        ray.setRotation(sf::radians(3.14159f + a)); // 指向外侧
        ray.setFillColor(sf::Color(255, 255, 150, static_cast<uint8_t>(160 * glow)));
        window.draw(ray);
    }

    // ---- 核心 ----
    sf::CircleShape core(6.f);
    core.setOrigin(sf::Vector2f(6.f, 6.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 255, 255));
    window.draw(core);
}

// ============================================================
// drawDashBattery - 绘制冲刺电池
// ============================================================
// 外观：电能场 + 闪电形状 + 4个电弧 + 内部光点
// 颜色主题：黄色(255,255,60)
void Item::drawDashBattery(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 电能场（带边框的圆）----
    sf::CircleShape eField(ITEM_RADIUS + 8.f);
    eField.setOrigin(sf::Vector2f(ITEM_RADIUS + 8.f, ITEM_RADIUS + 8.f));
    eField.setPosition(pos);
    eField.setFillColor(sf::Color(255, 255, 60, static_cast<uint8_t>(30 * glow)));
    eField.setOutlineColor(sf::Color(255, 255, 60, static_cast<uint8_t>(100 * glow)));
    eField.setOutlineThickness(2.f);
    window.draw(eField);

    // ---- 闪电形状 ----
    // 6个顶点构成闪电/之字形
    sf::ConvexShape bolt;
    bolt.setPointCount(6);
    bolt.setPoint(0, sf::Vector2f(4.f * glow, -ITEM_RADIUS * glow));    // 左上
    bolt.setPoint(1, sf::Vector2f(-2.f * glow, -2.f * glow));           // 中左
    bolt.setPoint(2, sf::Vector2f(3.f * glow, -2.f * glow));           // 中右
    bolt.setPoint(3, sf::Vector2f(-4.f * glow, ITEM_RADIUS * glow));  // 右下
    bolt.setPoint(4, sf::Vector2f(2.f * glow, 2.f * glow));            // 中右
    bolt.setPoint(5, sf::Vector2f(-3.f * glow, 2.f * glow));          // 中左
    bolt.setOrigin(sf::Vector2f(0.f, 0.f));
    bolt.setPosition(pos);
    bolt.setFillColor(sf::Color(255, 255, 60, 230));
    bolt.setOutlineColor(sf::Color(255, 255, 180));
    bolt.setOutlineThickness(2.f);
    window.draw(bolt);

    // ---- 4个电弧 ----
    // 随pulsePhase额外偏移，产生跳动效果
    for (int i = 0; i < 4; ++i) {
        float a = angle + 3.14159f * 2.f * i / 4.f + pulsePhase;
        sf::Vector2f arcPos = sf::Vector2f(std::cosf(a) * (ITEM_RADIUS + 5.f), std::sinf(a) * (ITEM_RADIUS + 5.f));
        sf::RectangleShape arc(sf::Vector2f(3.f, 6.f));
        arc.setOrigin(sf::Vector2f(1.5f, 3.f));
        arc.setPosition(pos + arcPos);
        arc.setRotation(sf::radians(3.14159f + a));
        arc.setFillColor(sf::Color(255, 255, 120, static_cast<uint8_t>(200 * glow)));
        window.draw(arc);
    }

    // ---- 内部光点 ----
    sf::CircleShape inner(5.f);
    inner.setOrigin(sf::Vector2f(5.f, 5.f));
    inner.setPosition(pos + sf::Vector2f(2.f, 0.f));
    inner.setFillColor(sf::Color(255, 255, 220, 240));
    window.draw(inner);
}

// ============================================================
// drawWeaponEssence - 绘制武器精华
// ============================================================
// 外观：紫色菱形 + 外圈光晕 + 白色核心
// 颜色主题：紫色(200,100,255)
void Item::drawWeaponEssence(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 外圈光晕 ----
    sf::CircleShape outerGlow(ITEM_RADIUS + 10.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(200, 100, 255, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    // ---- 菱形（4边形）----
    sf::ConvexShape diamond;
    diamond.setPointCount(4);
    float r = ITEM_RADIUS * glow;
    diamond.setPoint(0, sf::Vector2f(0.f, -r));       // 上
    diamond.setPoint(1, sf::Vector2f(r * 0.6f, 0.f)); // 右
    diamond.setPoint(2, sf::Vector2f(0.f, r));         // 下
    diamond.setPoint(3, sf::Vector2f(-r * 0.6f, 0.f)); // 左
    diamond.setOrigin(sf::Vector2f(0.f, 0.f));
    diamond.setPosition(pos);
    diamond.setRotation(sf::radians(rotationAngle * 0.0174533f));
    diamond.setFillColor(sf::Color(180, 80, 255, 220));
    diamond.setOutlineColor(sf::Color(220, 150, 255));
    diamond.setOutlineThickness(2.f);
    window.draw(diamond);

    // ---- 白色核心 ----
    sf::CircleShape core(5.f);
    core.setOrigin(sf::Vector2f(5.f, 5.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 255, 240));
    window.draw(core);
}

// ============================================================
// drawPhaseCrystal - 绘制相位水晶
// ============================================================
// 外观：紫色六边形 + 外圈光晕 + 3个内部光点
// 颜色主题：深紫色(150,50,200)
void Item::drawPhaseCrystal(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 外圈光晕 ----
    sf::CircleShape outerGlow(ITEM_RADIUS + 10.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(150, 50, 200, static_cast<uint8_t>(35 * glow)));
    window.draw(outerGlow);

    // ---- 六边形主体 ----
    sf::ConvexShape hex;
    hex.setPointCount(6);
    float r = ITEM_RADIUS * glow;
    for (int i = 0; i < 6; ++i) {
        float a = angle + 3.14159f * 2.f * i / 6.f;
        hex.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    hex.setOrigin(sf::Vector2f(0.f, 0.f));
    hex.setPosition(pos);
    hex.setFillColor(sf::Color(120, 40, 180, 220));
    hex.setOutlineColor(sf::Color(180, 120, 220));
    hex.setOutlineThickness(2.f);
    window.draw(hex);

    // ---- 3个内部光点 ----
    for (int i = 0; i < 3; ++i) {
        float a = angle + 3.14159f * 2.f * i / 3.f;
        sf::CircleShape dot(3.f);
        dot.setOrigin(sf::Vector2f(3.f, 3.f));
        // 位置在半径50%处
        dot.setPosition(pos + sf::Vector2f(std::cosf(a) * r * 0.5f, std::sinf(a) * r * 0.5f));
        dot.setFillColor(sf::Color(200, 150, 255, static_cast<uint8_t>(200 * glow)));
        window.draw(dot);
    }
}

// ============================================================
// drawSpeedCoil - 绘制速度线圈
// ============================================================
// 外观：青色光环 + 箭头形状 + 3条速度线
// 颜色主题：青色(100,255,200)
void Item::drawSpeedCoil(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 外圈光晕 ----
    sf::CircleShape outerGlow(ITEM_RADIUS + 8.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 8.f, ITEM_RADIUS + 8.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(100, 255, 200, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    // ---- 箭头形状 ----
    // 与Overdrive的箭头类似但颜色不同
    sf::ConvexShape arrow;
    arrow.setPointCount(5);
    arrow.setPoint(0, sf::Vector2f(0.f, -ITEM_RADIUS * glow));
    arrow.setPoint(1, sf::Vector2f(ITEM_RADIUS * 0.5f * glow, -ITEM_RADIUS * 0.3f * glow));
    arrow.setPoint(2, sf::Vector2f(ITEM_RADIUS * 0.2f * glow, -ITEM_RADIUS * 0.3f * glow));
    arrow.setPoint(3, sf::Vector2f(ITEM_RADIUS * 0.2f * glow, ITEM_RADIUS * glow));
    arrow.setPoint(4, sf::Vector2f(-ITEM_RADIUS * 0.2f * glow, ITEM_RADIUS * glow));
    arrow.setOrigin(sf::Vector2f(0.f, 0.f));
    arrow.setPosition(pos);
    arrow.setRotation(sf::radians(rotationAngle * 0.0174533f));
    arrow.setFillColor(sf::Color(80, 255, 180, 230));
    arrow.setOutlineColor(sf::Color(150, 255, 220));
    arrow.setOutlineThickness(2.f);
    window.draw(arrow);

    // ---- 3条速度线 ----
    for (int i = 0; i < 3; ++i) {
        float offset = (i - 1) * 8.f;
        sf::RectangleShape line(sf::Vector2f(2.f, ITEM_RADIUS * 0.5f * glow));
        line.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.25f * glow));
        line.setPosition(pos + sf::Vector2f(offset, ITEM_RADIUS * 0.4f * glow));
        line.setFillColor(sf::Color(100, 255, 200, static_cast<uint8_t>(180 * glow)));
        window.draw(line);
    }
}

// ============================================================
// drawAttackModule - 绘制攻击模块
// ============================================================
// 外观：红色光环 + 八边形十字 + 白色核心
// 颜色主题：红色(255,80,80)
void Item::drawAttackModule(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 外圈光晕 ----
    sf::CircleShape outerGlow(ITEM_RADIUS + 10.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 10.f, ITEM_RADIUS + 10.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 80, 80, static_cast<uint8_t>(30 * glow)));
    window.draw(outerGlow);

    // ---- 八边形十字 ----
    // 8个顶点：上、左上、左下、右上、右下的顺序构建十字形状
    sf::ConvexShape cross;
    cross.setPointCount(8);
    float r = ITEM_RADIUS * glow;
    float w = ITEM_RADIUS * 0.35f * glow;
    cross.setPoint(0, sf::Vector2f(-w, -r));   // 上左
    cross.setPoint(1, sf::Vector2f(w, -r));     // 上右
    cross.setPoint(2, sf::Vector2f(w, -w));    // 右上
    cross.setPoint(3, sf::Vector2f(r, -w));     // 右上
    cross.setPoint(4, sf::Vector2f(r, w));      // 右下
    cross.setPoint(5, sf::Vector2f(w, w));     // 右下
    cross.setPoint(6, sf::Vector2f(w, r));      // 下右
    cross.setPoint(7, sf::Vector2f(-w, r));     // 下左
    cross.setOrigin(sf::Vector2f(0.f, 0.f));
    cross.setPosition(pos);
    cross.setRotation(sf::radians(rotationAngle * 0.0174533f));
    cross.setFillColor(sf::Color(220, 60, 60, 220));
    cross.setOutlineColor(sf::Color(255, 150, 150));
    cross.setOutlineThickness(2.f);
    window.draw(cross);

    // ---- 核心 ----
    sf::CircleShape core(5.f);
    core.setOrigin(sf::Vector2f(5.f, 5.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 200, 240));
    window.draw(core);
}

// ============================================================
// drawNovaCore - 绘制新星核心
// ============================================================
// 外观：橙色大光晕 + 8角星形 + 8条放射线 + 大核心
// 颜色主题：橙色(255,150,50)
void Item::drawNovaCore(sf::RenderWindow& window, sf::Vector2f pos, float angle, float glow) const {
    // ---- 橙色大光晕 ----
    sf::CircleShape outerGlow(ITEM_RADIUS + 15.f);
    outerGlow.setOrigin(sf::Vector2f(ITEM_RADIUS + 15.f, ITEM_RADIUS + 15.f));
    outerGlow.setPosition(pos);
    outerGlow.setFillColor(sf::Color(255, 150, 50, static_cast<uint8_t>(40 * glow)));
    window.draw(outerGlow);

    // ---- 8角星形 ----
    // 与NovaBomb的星形类似但颜色为橙色
    sf::ConvexShape star;
    star.setPointCount(8);
    float r1 = ITEM_RADIUS * 0.4f * glow;
    float r2 = ITEM_RADIUS * glow;
    for (int i = 0; i < 8; ++i) {
        float r = (i % 2 == 0) ? r2 : r1;
        float a = angle + 3.14159f * 2.f * i / 8.f;
        star.setPoint(i, sf::Vector2f(std::cosf(a) * r, std::sinf(a) * r));
    }
    star.setOrigin(sf::Vector2f(0.f, 0.f));
    star.setPosition(pos);
    star.setFillColor(sf::Color(255, 120, 40, 230));
    star.setOutlineColor(sf::Color(255, 200, 100));
    star.setOutlineThickness(2.f);
    window.draw(star);

    // ---- 核心 ----
    sf::CircleShape core(7.f);
    core.setOrigin(sf::Vector2f(7.f, 7.f));
    core.setPosition(pos);
    core.setFillColor(sf::Color(255, 255, 200, 250));
    window.draw(core);

    // ---- 8条放射线 ----
    for (int i = 0; i < 8; ++i) {
        float a = angle + 3.14159f * 2.f * i / 8.f;
        sf::RectangleShape ray(sf::Vector2f(2.f, ITEM_RADIUS * 0.25f * glow));
        ray.setOrigin(sf::Vector2f(1.f, ITEM_RADIUS * 0.125f * glow));
        ray.setPosition(pos + sf::Vector2f(std::cosf(a) * ITEM_RADIUS * 0.5f, std::sinf(a) * ITEM_RADIUS * 0.5f));
        ray.setRotation(sf::radians(3.14159f + a));
        ray.setFillColor(sf::Color(255, 180, 80, static_cast<uint8_t>(160 * glow)));
        window.draw(ray);
    }
}

// ============================================================
// 状态查询和工具方法
// ============================================================
bool Item::isActive() const { return active; }                       // 返回激活状态
void Item::deactivate() { active = false; }                          // 设为非激活
ItemType Item::getType() const { return type; }                      // 返回道具类型
sf::Vector2f Item::getPosition() const { return position; }         // 返回位置
float Item::getRadius() const { return ITEM_RADIUS; }                // 返回碰撞半径

// getLifetime返回道具生命周期常量（在Config.h中定义）
float Item::getLifetime() { return ITEM_LIFETIME; }

// getColor根据道具类型返回对应颜色（用于粒子系统和绘制）
sf::Color Item::getColor(ItemType type) {
    switch (type) {
        case ItemType::HealCore:    return sf::Color(60, 255, 100);      // 绿色
        case ItemType::ShieldOrb:   return sf::Color(60, 140, 255);     // 蓝色
        case ItemType::Overdrive:   return sf::Color(255, 100, 40);     // 橙红色
        case ItemType::BulletTime:  return sf::Color(180, 60, 255);     // 紫色
        case ItemType::NovaBomb:    return sf::Color(255, 255, 240);    // 白色
        case ItemType::DashBattery: return sf::Color(255, 255, 60);     // 黄色
        case ItemType::WeaponEssence: return sf::Color(200, 100, 255);  // 紫色
        case ItemType::PhaseCrystal: return sf::Color(150, 50, 200);    // 深紫色
        case ItemType::SpeedCoil:   return sf::Color(100, 255, 200);    // 青色
        case ItemType::AttackModule: return sf::Color(255, 80, 80);     // 红色
        case ItemType::NovaCore:    return sf::Color(255, 150, 50);     // 橙色
    }
    return sf::Color::White; // 默认白色
}