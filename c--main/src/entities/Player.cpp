// ============================================================
// Player.cpp - 玩家类实现
// ============================================================
// 职责：玩家控制（移动、射击、技能）、状态管理、渲染外观
// 玩家是一个机械战甲（mech），有6种武器形态，通过道具获得不同技能
// ============================================================

#include "Player.h"
#include "core/Config.h"
#include "systems/AudioSystem.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>

// ============================================================
// 构造函数
// ============================================================
// 初始化玩家位置、朝向、所有冷却计时器、所有子弹数组
// 每种子弹数组预分配固定大小（对象池），避免运行时动态分配
// ============================================================
Player::Player()
    // ---- 位置与朝向 ----
    : position(PLAYER_START_POS)                 // 初始位置：屏幕中央偏下(640, 550)
    , aimDir(0.f, -1.f)                          // 默认朝向：向上（负y方向）
    // ---- 射击冷却（初始为0，表示可以立即射击）----
    , shootCooldown(0.f)
    , spreadCooldown(0.f)
    , piercingCooldown(0.f)
    , orbitalCooldown(0.f)
    , clusterCooldown(0.f)
    , homingCooldown(0.f)
    // ---- 武器 ----
    , currentWeapon(WeaponType::Normal)          // 默认武器：普通
    // ---- 生命值 ----
    , hp(PLAYER_MAX_HP)                         // 初始HP：100（从Config获取）
    , maxHP(PLAYER_MAX_HP)
    // ---- 无敌状态 ----
    , invincibleTimer(0.f)                      // 无敌计时器（初始为0，未激活）
    , godMode(false)                            // 调试模式（默认关闭）
    // ---- 技能持续时间（初始为0，表示未激活）----
    , shieldTimer(0.f)
    , overdriveTimer(0.f)
    , dashBatteryTimer(0.f)
    , phaseShiftTimer(0.f)
    , novaFormTimer(0.f)
    , speedCoilTimer(0.f)
    , attackModuleTimer(0.f)
    // ---- 闪避状态 ----
    , dashCooldown(0.f)                         // 初始可闪避
    , dashing(false)                           // 未在闪避
    , dashTimer(0.f)
    // ---- 动画反馈 ----
    , weaponRecoilTimer(0.f)
    // ---- 连击系统 ----
    , comboCount(0)
    , comboTimer(0.f)
    , lastHitTargetId(-1)                       // -1表示没有上次命中目标
    // ---- 减速Debuff ----
    , slowStacks(0)
    , slowTimer(0.f)
{
    // 预分配子弹数组大小（对象池，避免运行时new）
    bullets.resize(MAX_PLAYER_BULLETS);         // 200发普通子弹
    spreadBullets.resize(50);                    // 50发散射子弹
    piercingBullets.resize(20);                  // 20发穿透子弹
    orbitalBullets.resize(4);                    // 4发环绕子弹（绕玩家旋转）
    clusterBullets.resize(20);                   // 20发集群子弹
    homingBullets.resize(15);                    // 15发追踪子弹
}

// ============================================================
// 处理输入
// ============================================================
// dt：帧间隔时间（秒）
// window：SFML窗口（用于获取鼠标位置）
//
// 控制方式：
// - WASD：移动（8方向，支持斜向）
// - 鼠标：瞄准方向（战甲朝向跟随鼠标）
// - 左键：射击（按住不放持续射击）
// - Shift：闪避（朝移动方向或朝向方向快速位移，无敌0.15秒）
// - Q：切换武器
// ============================================================
void Player::handleInput(float dt, const sf::RenderWindow& window) {
    // 闪避中不允许其他输入（防止闪避时移动/射击打断动作）
    if (dashing) return;

    // ---- 武器切换（Q键）----
    // 使用static变量追踪Q键状态，避免按住时每帧切换多次
    static bool qPressed = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
        if (!qPressed) {          // 只在按键首次按下时切换（避免按住时每帧切换）
            qPressed = true;
            cycleWeapon();       // 切换到下一种武器
        }
    } else {
        qPressed = false;        // 按键释放后重置状态，允许下次切换
    }

    // ---- 移动输入（WASD）----
    sf::Vector2f move(0.f, 0.f);     // 移动向量
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) move.y -= 1.f;  // 上
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) move.y += 1.f;  // 下
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) move.x -= 1.f;  // 左
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) move.x += 1.f;  // 右

    // 归一化移动向量（斜向移动速度不会比正向快）
    float len = std::sqrt(move.x * move.x + move.y * move.y);
    if (len > 0.f) {
        move /= len;  // 除以长度，变成单位向量（x²+y²=1）
    }

    // ---- 闪避输入（Shift）----
    // shift按住 + 冷却已结束 + 正在按方向键（或有朝向方向）→ 触发闪避
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) && dashCooldown <= 0.f) {
        // 确定闪避方向：优先使用当前移动方向，否则使用朝向方向
        if (len > 0.f) {
            dashDir = move;
        } else {
            dashDir = aimDir;  // 没有移动输入时，朝鼠标朝向闪避
        }
        // 开始闪避
        dashing = true;                          // 标记为闪避中
        dashTimer = PLAYER_DASH_DURATION;        // 闪避持续0.15秒
        dashCooldown = PLAYER_DASH_COOLDOWN;     // 闪避冷却2秒

        // 35%概率触发PhaseShift（穿怪+无敌，持续1秒）
        if (static_cast<float>(rand()) / RAND_MAX < PHASE_SHIFT_TRIGGER_CHANCE) {
            phaseShiftTimer = PHASE_SHIFT_DURATION;    // 1秒
            invincibleTimer = PHASE_SHIFT_DURATION;    // 同时无敌
        }
    }

    // ---- 移动（不在闪避时）----
    if (!dashing) {
        float speed = PLAYER_SPEED;  // 基础速度300像素/秒
        // DashBattery加速（×1.5）
        if (dashBatteryTimer > 0.f) {
            speed *= DASH_BATTERY_SPEED_MULT;
        }
        // SpeedCoil加速（×1.5）
        if (speedCoilTimer > 0.f) {
            speed *= SPEED_COIL_MULT;
        }
        // 应用移动：position += direction * speed * dt
        position += move * speed * dt;
    }

    // ---- 边界限制（防止玩家移出屏幕）----
    // clamp到屏幕范围内，PLAYER_RADIUS确保不会有一半在屏幕外
    if (position.x < PLAYER_RADIUS) position.x = PLAYER_RADIUS;
    if (position.x > WINDOW_WIDTH - PLAYER_RADIUS) position.x = WINDOW_WIDTH - PLAYER_RADIUS;
    if (position.y < PLAYER_RADIUS) position.y = PLAYER_RADIUS;
    if (position.y > WINDOW_HEIGHT - PLAYER_RADIUS) position.y = WINDOW_HEIGHT - PLAYER_RADIUS;

    // ---- 鼠标瞄准 ----
    // 获取鼠标在窗口中的像素坐标
    sf::Vector2i mousePos = sf::Mouse::getPosition();
    sf::Vector2f worldMouse(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    // 计算朝向：鼠标位置 - 玩家位置
    aimDir = worldMouse - position;
    // 归一化朝向向量（只关心方向，不关心距离）
    float aimLen = std::sqrt(aimDir.x * aimDir.x + aimDir.y * aimDir.y);
    if (aimLen > 0.f) {
        aimDir /= aimLen;
    }

    // ---- 射击（左键按住）----
    if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
        shoot();
    }
}

// ============================================================
// 射击
// ============================================================
// 根据当前武器类型，生成相应的子弹
// 每次射击会激活对象池中第一个非活跃的子弹
// 射速受Overdrive影响（激活时冷却时间×0.5，即射速×2）
// ============================================================
void Player::shoot() {
    // ---- 计算射速和伤害倍率 ----
    float rateMult = (overdriveTimer > 0.f) ? OVERDRIVE_FIRE_RATE_MULT : 1.f;  // 0.5或1
    float dmgMult = 1.f;
    if (overdriveTimer > 0.f) dmgMult = OVERDRIVE_DAMAGE_MULT;  // 1.3
    if (attackModuleTimer > 0.f) dmgMult = std::max(dmgMult, ATTACK_MODULE_MULT);  // max(1.3,1.3)=1.3

    // ---- 分发到对应武器的射击逻辑 ----
    switch (currentWeapon) {
        case WeaponType::Normal: {
            // 普通武器：单发，高射速（0.15秒）
            if (shootCooldown > 0.f) return;                  // 冷却中不能射击
            shootCooldown = PLAYER_SHOOT_COOLDOWN * rateMult; // 设置冷却时间（Overdrive时×0.5）
            // 遍历对象池，找第一个非活跃子弹激活
            for (auto& bullet : bullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);           // 在玩家位置朝瞄准方向生成
                    bullet.setDamage(static_cast<int>(PLAYER_BULLET_DAMAGE * dmgMult));  // 设置伤害（含加成）
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();  // 播放射击音效
                    break;  // 只生成一颗，跳出循环
                }
            }
            break;
        }

        case WeaponType::Spread: {
            // 散射武器：5发扇形弹幕，角度30度
            if (spreadCooldown > 0.f) return;
            spreadCooldown = PLAYER_SPREAD_COOLDOWN * rateMult;
            {
                // 计算基础角度（弧度）
                float baseAngle = std::atan2(aimDir.y, aimDir.x);
                int count = PLAYER_SPREAD_COUNT;                              // 5发
                float spreadAngle = PLAYER_SPREAD_ANGLE * 3.14159f / 180.f;    // 30度转弧度
                float step = spreadAngle / (count - 1);                        // 每发间隔 = 30/4 = 7.5度
                float startAngle = baseAngle - spreadAngle * 0.5f;             // 起始角度 = 基础-15度
                for (int i = 0; i < count; ++i) {
                    for (auto& bullet : spreadBullets) {
                        if (!bullet.isActive()) {
                            float angle = startAngle + step * i;  // 第i发的角度
                            sf::Vector2f dir(std::cos(angle), std::sin(angle));
                            bullet.spawn(position, dir, 0.f);     // 散射无初速度，子弹自己飞
                            bullet.setDamage(static_cast<int>(PLAYER_BULLET_DAMAGE * dmgMult));
                            if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                            break;
                        }
                    }
                }
            }
            break;
        }

        case WeaponType::Piercing: {
            // 穿透武器：高速子弹，穿透所有路径上的敌人（不消失）
            if (piercingCooldown > 0.f) return;
            piercingCooldown = PLAYER_PIERCING_COOLDOWN * rateMult;
            for (auto& bullet : piercingBullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;
        }

        case WeaponType::Orbital: {
            // 环绕武器：4发子弹同时激活，均匀分布在玩家周围绕圈
            // 环绕子弹不是射出去的，而是直接生成在玩家身边旋转
            for (auto& bullet : orbitalBullets) {
                if (!bullet.isActive()) {
                    // 计算这颗子弹在圆周上的初始角度（均匀分布）
                    float angleOffset = 3.14159f * 2.f / orbitalBullets.size() * (&bullet - &orbitalBullets[0]);
                    bullet.spawn(position, angleOffset, PLAYER_ORBITAL_RADIUS);  // 绕玩家旋转，半径60
                }
            }
            break;
        }

        case WeaponType::Cluster: {
            // 集群武器：母弹飞行一段后爆炸成多发（由Cluster子弹自己的update处理分裂）
            if (clusterCooldown > 0.f) return;
            clusterCooldown = PLAYER_CLUSTER_COOLDOWN * rateMult;
            for (auto& bullet : clusterBullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    bullet.setDamage(static_cast<int>(PLAYER_BULLET_DAMAGE * dmgMult));
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;
        }

        case WeaponType::Homing: {
            // 追踪武器：自动追踪Boss，有减速Debuff
            if (homingCooldown > 0.f) return;
            homingCooldown = PLAYER_HOMING_COOLDOWN * rateMult;
            for (auto& bullet : homingBullets) {
                if (!bullet.isActive()) {
                    bullet.spawn(position, aimDir);
                    bullet.setDamage(static_cast<int>(PLAYER_HOMING_DAMAGE * dmgMult));
                    if (AudioSystem::getInstance()) AudioSystem::getInstance()->playShoot();
                    break;
                }
            }
            break;
        }
    }
}

// ============================================================
// 更新逻辑（每帧调用）
// ============================================================
// dt：帧间隔时间
// 更新所有冷却计时器、状态效果、子弹位置
// ============================================================
void Player::update(float dt) {
    // ---- 冷却计时器倒计时 ----
    // 每种武器的射击冷却独立倒计时（归零时可再次射击）
    if (shootCooldown > 0.f) shootCooldown -= dt;
    if (spreadCooldown > 0.f) spreadCooldown -= dt;
    if (piercingCooldown > 0.f) piercingCooldown -= dt;
    if (orbitalCooldown > 0.f) orbitalCooldown -= dt;
    if (clusterCooldown > 0.f) clusterCooldown -= dt;
    if (homingCooldown > 0.f) homingCooldown -= dt;

    // ---- 无敌状态倒计时 ----
    if (invincibleTimer > 0.f) invincibleTimer -= dt;

    // ---- 技能效果倒计时 ----
    if (shieldTimer > 0.f) shieldTimer -= dt;         // 护盾持续时间
    if (overdriveTimer > 0.f) overdriveTimer -= dt;   // 火力提升持续时间
    if (dashBatteryTimer > 0.f) dashBatteryTimer -= dt;  // 闪避电池持续时间
    if (phaseShiftTimer > 0.f) phaseShiftTimer -= dt;  // 相位偏移持续时间
    if (novaFormTimer > 0.f) novaFormTimer -= dt;     // Nova形态持续时间
    if (speedCoilTimer > 0.f) speedCoilTimer -= dt;   // 速度线圈持续时间
    if (attackModuleTimer > 0.f) attackModuleTimer -= dt;  // 攻击模块持续时间

    // ---- 闪避冷却 ----
    if (dashCooldown > 0.f) dashCooldown -= dt;

    // ---- 武器后座力动画 ----
    if (weaponRecoilTimer > 0.f) weaponRecoilTimer -= dt;

    // ---- 连击超时 ----
    // 如果2秒内没有命中，连击重置
    if (comboTimer > 0.f) {
        comboTimer -= dt;
        if (comboTimer <= 0.f) {
            comboCount = 0;         // 重置连击数
            lastHitTargetId = -1;   // 重置目标ID
        }
    }

    // ---- 减速Debuff超时 ----
    // 减速效果持续2秒，结束后层数归零
    if (slowTimer > 0.f) {
        slowTimer -= dt;
        if (slowTimer <= 0.f) {
            slowStacks = 0;
        }
    }

    // ---- 闪避状态更新 ----
    if (dashing) {
        dashTimer -= dt;  // 闪避持续时间倒计时
        // 闪避中高速移动（900像素/秒）
        position += dashDir * PLAYER_DASH_SPEED * dt;
        // 边界限制（与正常移动相同）
        if (position.x < PLAYER_RADIUS) position.x = PLAYER_RADIUS;
        if (position.x > WINDOW_WIDTH - PLAYER_RADIUS) position.x = WINDOW_WIDTH - PLAYER_RADIUS;
        if (position.y < PLAYER_RADIUS) position.y = PLAYER_RADIUS;
        if (position.y > WINDOW_HEIGHT - PLAYER_RADIUS) position.y = WINDOW_HEIGHT - PLAYER_RADIUS;
        // 闪避时间结束
        if (dashTimer <= 0.f) {
            dashing = false;
        }
    }

    // ---- 更新所有子弹的位置 ----
    // 普通子弹：直线飞行
    for (auto& bullet : bullets) bullet.update(dt);
    // 散射子弹：直线飞行
    for (auto& bullet : spreadBullets) bullet.update(dt);
    // 穿透子弹：直线飞行（不消失，穿透多个敌人）
    for (auto& bullet : piercingBullets) bullet.update(dt);
    // 环绕子弹：绕玩家旋转（需要传入玩家位置作为旋转中心）
    for (auto& bullet : orbitalBullets) bullet.update(dt, position);
    // 集群子弹：飞行+接近目标时爆炸分裂
    for (auto& bullet : clusterBullets) bullet.update(dt);
    // 追踪子弹：转向追踪当前目标（需要传入玩家位置作为追踪依据）
    for (auto& bullet : homingBullets) bullet.update(dt, position);
}

// ============================================================
// 渲染（每帧调用）
// ============================================================
// window：SFML窗口
// 渲染顺序：子弹（先画，飞在玩家后面）→ 特效 → 战甲身体 → 武器
// ============================================================
void Player::render(sf::RenderWindow& window) const {
    // ---- Layer 1: 所有子弹 ----
    for (const auto& bullet : bullets) bullet.render(window);
    for (const auto& bullet : spreadBullets) bullet.render(window);
    for (const auto& bullet : piercingBullets) bullet.render(window);
    for (const auto& bullet : orbitalBullets) bullet.render(window);
    for (const auto& bullet : clusterBullets) bullet.render(window);
    for (const auto& bullet : homingBullets) bullet.render(window);

    // ---- Layer 2-4: 玩家战甲（从后到前）----
    renderMechEffects(window);  // 特效层（护盾、闪烁、Nova光环等，在身体下面）
    renderMechBody(window);      // 身体层（装甲板、肩部推进器等）
    renderMechWeapon(window);    // 武器层（炮管，在身体前面，遮挡关系）
}

// ============================================================
// 渲染战甲身体
// ============================================================
// 战甲由多个几何形状组成：六边形核心框架、装甲板、圆形能量核心、肩部推进器、推进器火焰
// 整体随鼠标朝向旋转（mechRotationAngle）
// 不同状态（PhaseShift/NovaForm）显示不同颜色
// ============================================================
void Player::renderMechBody(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    // 将朝向向量(aimDir)的弧度转为角度，供SFML旋转使用
    float baseAngle = std::atan2(aimDir.y, aimDir.x) * 180.f / 3.14159f;

    // ---- 根据状态选择颜色 ----
    sf::Color bodyColor(30, 35, 50);        // 默认：深蓝灰
    sf::Color armorColor(55, 70, 100);      // 默认：浅蓝灰
    if (phaseShiftTimer > 0.f) {            // PhaseShift时：紫色（穿怪状态）
        bodyColor = sf::Color(80, 40, 120);
        armorColor = sf::Color(100, 60, 150);
    } else if (novaFormTimer > 0.f) {       // NovaForm时：橙红色（爆发状态）
        bodyColor = sf::Color(255, 100, 50);
        armorColor = sf::Color(255, 150, 80);
    }

    // ---- 核心框架（六边形）----
    sf::ConvexShape coreFrame;
    coreFrame.setPointCount(6);
    coreFrame.setPoint(0, sf::Vector2f(0.f, -28.f));    // 上顶点
    coreFrame.setPoint(1, sf::Vector2f(18.f, -8.f));    // 右上
    coreFrame.setPoint(2, sf::Vector2f(18.f, 16.f));    // 右下
    coreFrame.setPoint(3, sf::Vector2f(0.f, 26.f));     // 下顶点
    coreFrame.setPoint(4, sf::Vector2f(-18.f, 16.f));   // 左下
    coreFrame.setPoint(5, sf::Vector2f(-18.f, -8.f));   // 左上
    coreFrame.setOrigin(sf::Vector2f(0.f, 0.f));
    coreFrame.setPosition(pos);
    coreFrame.setRotation(sf::degrees(baseAngle));       // 整体旋转到朝向角度
    coreFrame.setFillColor(bodyColor);

    // ---- 装甲板（六边形，比核心框架小一号）----
    sf::ConvexShape armorPlate;
    armorPlate.setPointCount(6);
    armorPlate.setPoint(0, sf::Vector2f(0.f, -22.f));
    armorPlate.setPoint(1, sf::Vector2f(13.f, -6.f));
    armorPlate.setPoint(2, sf::Vector2f(13.f, 12.f));
    armorPlate.setPoint(3, sf::Vector2f(0.f, 20.f));
    armorPlate.setPoint(4, sf::Vector2f(-13.f, 12.f));
    armorPlate.setPoint(5, sf::Vector2f(-13.f, -6.f));
    armorPlate.setOrigin(sf::Vector2f(0.f, 0.f));
    armorPlate.setPosition(pos);
    armorPlate.setRotation(sf::degrees(baseAngle));
    armorPlate.setFillColor(armorColor);

    // ---- 能量核心（圆形，发光效果）----
    sf::CircleShape core(8.f);
    core.setOrigin(sf::Vector2f(8.f, 8.f));
    core.setPosition(pos);
    if (phaseShiftTimer > 0.f) {
        core.setFillColor(sf::Color(200, 100, 255, 200));   // 紫色
    } else if (novaFormTimer > 0.f) {
        core.setFillColor(sf::Color(255, 200, 100, 200));  // 橙色
    } else {
        core.setFillColor(sf::Color(0, 220, 255, 200));     // 青色（默认）
    }

    // ---- 能量核心内芯（更亮的白色圆点）----
    sf::CircleShape coreInner(4.f);
    coreInner.setOrigin(sf::Vector2f(4.f, 4.f));
    coreInner.setPosition(pos);
    coreInner.setFillColor(sf::Color(150, 255, 255, 255));

    // ---- 右肩部推进器（多边形）----
    sf::ConvexShape shoulderR;
    shoulderR.setPointCount(5);
    shoulderR.setPoint(0, sf::Vector2f(8.f, -20.f));
    shoulderR.setPoint(1, sf::Vector2f(24.f, -32.f));
    shoulderR.setPoint(2, sf::Vector2f(30.f, -24.f));
    shoulderR.setPoint(3, sf::Vector2f(16.f, -14.f));
    shoulderR.setPoint(4, sf::Vector2f(12.f, -18.f));
    shoulderR.setOrigin(sf::Vector2f(0.f, 0.f));
    shoulderR.setPosition(pos);
    shoulderR.setRotation(sf::degrees(baseAngle));
    shoulderR.setFillColor(sf::Color(45, 58, 80));

    // ---- 左肩部推进器（镜像）----
    sf::ConvexShape shoulderL;
    shoulderL.setPointCount(5);
    shoulderL.setPoint(0, sf::Vector2f(-8.f, -20.f));
    shoulderL.setPoint(1, sf::Vector2f(-24.f, -32.f));
    shoulderL.setPoint(2, sf::Vector2f(-30.f, -24.f));
    shoulderL.setPoint(3, sf::Vector2f(-16.f, -14.f));
    shoulderL.setPoint(4, sf::Vector2f(-12.f, -18.f));
    shoulderL.setOrigin(sf::Vector2f(0.f, 0.f));
    shoulderL.setPosition(pos);
    shoulderL.setRotation(sf::degrees(baseAngle));
    shoulderL.setFillColor(sf::Color(45, 58, 80));

    // ---- 肩部发光线条（右）----
    sf::RectangleShape shoulderLineR(sf::Vector2f(16.f, 2.f));
    shoulderLineR.setOrigin(sf::Vector2f(8.f, 1.f));
    shoulderLineR.setPosition(pos);
    shoulderLineR.setRotation(sf::degrees(baseAngle + 25.f));
    shoulderLineR.setFillColor(sf::Color(0, 180, 255, 150));

    // ---- 肩部发光线条（左）----
    sf::RectangleShape shoulderLineL(sf::Vector2f(16.f, 2.f));
    shoulderLineL.setOrigin(sf::Vector2f(8.f, 1.f));
    shoulderLineL.setPosition(pos);
    shoulderLineL.setRotation(sf::degrees(baseAngle - 25.f));
    shoulderLineL.setFillColor(sf::Color(0, 180, 255, 150));

    // ---- 左侧推进器舱（左下角）----
    sf::RectangleShape thrusterL(sf::Vector2f(6.f, 12.f));
    thrusterL.setOrigin(sf::Vector2f(3.f, 6.f));
    thrusterL.setPosition(sf::Vector2f(pos.x - 8.f, pos.y + 18.f));
    thrusterL.setRotation(sf::degrees(baseAngle));
    thrusterL.setFillColor(sf::Color(25, 30, 45));

    // ---- 右侧推进器舱（右下角）----
    sf::RectangleShape thrusterR(sf::Vector2f(6.f, 12.f));
    thrusterR.setOrigin(sf::Vector2f(3.f, 6.f));
    thrusterR.setPosition(sf::Vector2f(pos.x + 8.f, pos.y + 18.f));
    thrusterR.setRotation(sf::degrees(baseAngle));
    thrusterR.setFillColor(sf::Color(25, 30, 45));

    // ---- Overdrive时推进器发光 ----
    if (overdriveTimer > 0.f) {
        sf::CircleShape thrusterGlowL(5.f);
        thrusterGlowL.setOrigin(sf::Vector2f(5.f, 5.f));
        thrusterGlowL.setPosition(sf::Vector2f(pos.x - 8.f, pos.y + 22.f));
        thrusterGlowL.setFillColor(sf::Color(255, 100, 50, 200));

        sf::CircleShape thrusterGlowR(5.f);
        thrusterGlowR.setOrigin(sf::Vector2f(5.f, 5.f));
        thrusterGlowR.setPosition(sf::Vector2f(pos.x + 8.f, pos.y + 22.f));
        thrusterGlowR.setFillColor(sf::Color(255, 100, 50, 200));

        window.draw(thrusterGlowL);
        window.draw(thrusterGlowR);
    }

    // ---- 护盾效果（两层圆形）----
    if (shieldTimer > 0.f) {
        // 外层护盾（大圆，轮廓明显）
        sf::CircleShape shield(PLAYER_RADIUS + 10.f);
        shield.setOrigin(sf::Vector2f(PLAYER_RADIUS + 10.f, PLAYER_RADIUS + 10.f));
        shield.setPosition(pos);
        shield.setFillColor(sf::Color(0, 255, 255, 30));        // 半透明青色填充
        shield.setOutlineColor(sf::Color(0, 255, 255, 150));  // 边框更亮
        shield.setOutlineThickness(2.f);
        window.draw(shield);

        // 内层护盾（小圆，轮廓柔和）
        sf::CircleShape shieldInner(PLAYER_RADIUS + 4.f);
        shieldInner.setOrigin(sf::Vector2f(PLAYER_RADIUS + 4.f, PLAYER_RADIUS + 4.f));
        shieldInner.setPosition(pos);
        shieldInner.setFillColor(sf::Color(0, 255, 255, 15));
        shieldInner.setOutlineColor(sf::Color(0, 255, 255, 80));
        shieldInner.setOutlineThickness(1.f);
        window.draw(shieldInner);
    }

    // ---- 绘制所有身体部件（从后到前）----
    window.draw(coreFrame);    // 六边形核心框架（最底层）
    window.draw(armorPlate);   // 装甲板（在框架上面）
    window.draw(thrusterL);    // 左推进器舱
    window.draw(thrusterR);    // 右推进器舱
    window.draw(shoulderR);    // 右肩部
    window.draw(shoulderL);    // 左肩部
    window.draw(core);         // 能量核心（发光）
    window.draw(coreInner);    // 核心内芯（亮白点）
    window.draw(shoulderLineR); // 右肩发光线条
    window.draw(shoulderLineL); // 左肩发光线条
}

// ============================================================
// 渲染武器（炮管）
// ============================================================
// 炮管由三个部分组成：炮管基座（cannon）、炮管套（barrel）、炮口（cannonTip）
// 炮口颜色根据状态变化（PhaseShift紫色/NovaForm橙色/默认青色）
// Overdrive时炮口有额外的橙色光晕（chargeGlow）
// 射击后有后座力动画（weaponRecoilTimer控制）
// ============================================================
void Player::renderMechWeapon(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;
    // 计算朝向角度
    float baseAngle = std::atan2(aimDir.y, aimDir.x) * 180.f / 3.14159f;

    // ---- 后座力 ----
    // 射击时枪管向后缩进动画，逐渐恢复
    float recoil = weaponRecoilTimer > 0.f ? weaponRecoilTimer * 8.f : 0.f;

    // ---- 颜色 ----
    sf::Color cannonColor(40, 50, 70);    // 炮管基座：深蓝灰
    sf::Color barrelColor(25, 35, 55);    // 炮管套：更深蓝灰
    if (phaseShiftTimer > 0.f) {
        cannonColor = sf::Color(80, 50, 100);
        barrelColor = sf::Color(60, 30, 80);
    } else if (novaFormTimer > 0.f) {
        cannonColor = sf::Color(150, 80, 40);
        barrelColor = sf::Color(120, 60, 30);
    }

    // ---- 炮管基座（矩形，向前延伸）----
    sf::RectangleShape cannon(sf::Vector2f(28.f - recoil, 10.f));  // 长度减去后座力
    cannon.setOrigin(sf::Vector2f(28.f - recoil, 5.f));            // 起点在战甲中心
    cannon.setPosition(pos);
    cannon.setRotation(sf::degrees(baseAngle));                    // 朝向鼠标方向
    cannon.setFillColor(cannonColor);

    // ---- 炮管套（在基座上，比基座短一些）----
    sf::RectangleShape barrel(sf::Vector2f(12.f - recoil * 0.5f, 6.f));
    barrel.setOrigin(sf::Vector2f(12.f - recoil * 0.5f, 3.f));
    barrel.setPosition(pos);
    barrel.setRotation(sf::degrees(baseAngle));
    barrel.setFillColor(barrelColor);

    // ---- 炮口（圆形，标记发射位置）----
    sf::CircleShape cannonTip(4.f);
    cannonTip.setOrigin(sf::Vector2f(4.f, 4.f));
    // 炮口位置 = 玩家位置 + 朝向 × 距离（减去后座力）
    float tipDist = 24.f - recoil;
    float tipX = pos.x + aimDir.x * tipDist;
    float tipY = pos.y + aimDir.y * tipDist;
    cannonTip.setPosition(sf::Vector2f(tipX, tipY));
    if (phaseShiftTimer > 0.f) {
        cannonTip.setFillColor(sf::Color(200, 100, 255, 200));  // 紫色
    } else if (novaFormTimer > 0.f) {
        cannonTip.setFillColor(sf::Color(255, 200, 100, 200));  // 橙色
    } else {
        cannonTip.setFillColor(sf::Color(0, 200, 255, 200));    // 青色
    }

    // ---- Overdrive时炮口光晕 ----
    if (overdriveTimer > 0.f) {
        sf::CircleShape chargeGlow(7.f);
        chargeGlow.setOrigin(sf::Vector2f(7.f, 7.f));
        chargeGlow.setPosition(sf::Vector2f(tipX, tipY));
        chargeGlow.setFillColor(sf::Color(255, 150, 50, 180));
        window.draw(chargeGlow);
    }

    // ---- 炮管套环（圆柱形装饰）----
    sf::RectangleShape cannonSleeve(sf::Vector2f(8.f, 14.f));
    cannonSleeve.setOrigin(sf::Vector2f(4.f, 7.f));
    cannonSleeve.setPosition(pos);
    cannonSleeve.setRotation(sf::degrees(baseAngle));
    cannonSleeve.setFillColor(sf::Color(50, 65, 90));

    // ---- 绘制炮管部件 ----
    window.draw(cannon);
    window.draw(barrel);
    window.draw(cannonTip);
    window.draw(cannonSleeve);
}

// ============================================================
// 渲染战甲特效
// ============================================================
// 包含：闪避残影（ghost）、相位偏移残影、Overdrive脉冲、Nova光环、无敌闪烁
// 这些都是覆盖在玩家身体上的视觉效果
// ============================================================
void Player::renderMechEffects(sf::RenderWindow& window) const {
    sf::Vector2f pos = position;

    // ---- 闪避残影 ----
    // 闪避时画出3个半透明的残影，位置在闪避方向后方，逐渐消失
    if (dashing) {
        for (int i = 0; i < 3; i++) {
            // 第i个残影的透明度 = (3-i)/3 × 80
            float alpha = (3.f - static_cast<float>(i)) / 3.f * 80.f;
            sf::ConvexShape ghost;
            ghost.setPointCount(6);
            ghost.setPoint(0, sf::Vector2f(0.f, -22.f));
            ghost.setPoint(1, sf::Vector2f(13.f, -6.f));
            ghost.setPoint(2, sf::Vector2f(13.f, 12.f));
            ghost.setPoint(3, sf::Vector2f(0.f, 20.f));
            ghost.setPoint(4, sf::Vector2f(-13.f, 12.f));
            ghost.setPoint(5, sf::Vector2f(-13.f, -6.f));
            ghost.setOrigin(sf::Vector2f(0.f, 0.f));
            // 残影位置：玩家位置 - 闪避方向 × 距离（越远越透明）
            ghost.setPosition(sf::Vector2f(pos.x - dashDir.x * (i + 1) * 12.f,
                                            pos.y - dashDir.y * (i + 1) * 12.f));
            ghost.setFillColor(sf::Color(0, 180, 255, static_cast<std::uint8_t>(alpha)));
            window.draw(ghost);
        }
    }

    // ---- 相位偏移残影 ----
    // PhaseShift时画出5个紫色半透明圆形残影，位置向右下角偏移
    if (phaseShiftTimer > 0.f) {
        for (int i = 0; i < 5; i++) {
            float alpha = (5.f - static_cast<float>(i)) / 5.f * 60.f;
            float offset = static_cast<float>(i) * 8.f;
            sf::CircleShape ghost(PLAYER_RADIUS);
            ghost.setPosition(sf::Vector2f(pos.x + offset, pos.y + offset));
            ghost.setOrigin(sf::Vector2f(PLAYER_RADIUS, PLAYER_RADIUS));
            ghost.setFillColor(sf::Color(150, 50, 200, static_cast<std::uint8_t>(alpha)));
            window.draw(ghost);
        }
    }

    // ---- Overdrive脉冲环 ----
    // 从中心向外扩散的橙色圆环，脉冲频率高
    if (overdriveTimer > 0.f) {
        float pulsePhase = overdriveTimer * 4.f;
        // pulseRadius：从10像素逐渐扩大到50像素，然后重新开始
        float pulseRadius = (1.f - overdriveTimer / OVERDRIVE_DURATION) * 40.f + 10.f;
        float pulseAlpha = std::fmax(0.f, std::sin(pulsePhase * 3.14159f) * 100.f);

        sf::CircleShape pulseRing(pulseRadius);
        pulseRing.setOrigin(sf::Vector2f(pulseRadius, pulseRadius));
        pulseRing.setPosition(pos);
        pulseRing.setFillColor(sf::Color(255, 100, 50, static_cast<std::uint8_t>(pulseAlpha)));
        window.draw(pulseRing);
    }

    // ---- Nova形态光环 ----
    // 从0扩大到500像素的橙色圆环，透明度随时间减少
    if (novaFormTimer > 0.f) {
        float pulseRadius = NOVA_FORM_RADIUS * (1.f - novaFormTimer / NOVA_FORM_DURATION);
        float pulseAlpha = novaFormTimer / NOVA_FORM_DURATION * 150.f;

        sf::CircleShape novaRing(pulseRadius);
        novaRing.setOrigin(sf::Vector2f(pulseRadius, pulseRadius));
        novaRing.setPosition(pos);
        novaRing.setFillColor(sf::Color(255, 150, 50, static_cast<std::uint8_t>(pulseAlpha)));
        novaRing.setOutlineColor(sf::Color(255, 200, 100, static_cast<std::uint8_t>(pulseAlpha * 1.5f)));
        novaRing.setOutlineThickness(5.f);
        window.draw(novaRing);
    }

    // ---- 无敌闪烁 ----
    // 无敌时玩家快速闪烁（每帧随机跳过），视觉上像闪烁效果
    if (invincibleTimer > 0.f) {
        float flicker = std::sin(invincibleTimer * 30.f) * 0.5f + 0.5f;
        if (flicker > 0.5f) {  // 只在半透明时画（约一半时间不画）
            sf::CircleShape invincibleAura(PLAYER_RADIUS + 5.f);
            invincibleAura.setOrigin(sf::Vector2f(PLAYER_RADIUS + 5.f, PLAYER_RADIUS + 5.f));
            invincibleAura.setPosition(pos);
            invincibleAura.setFillColor(sf::Color(255, 255, 255, 60));
            window.draw(invincibleAura);
        }
    }
}

// ============================================================
// 状态查询（getter）
// ============================================================
sf::Vector2f Player::getPosition() const { return position; }
float Player::getRadius() const { return PLAYER_RADIUS; }  // 返回固定值18，不读取成员变量
int Player::getHP() const { return hp; }
int Player::getMaxHP() const { return maxHP; }
bool Player::isDead() const { return hp <= 0; }
bool Player::isInvincible() const { return invincibleTimer > 0.f; }

// ============================================================
// 子弹数组访问
// ============================================================
std::vector<PlayerBullet>& Player::getBullets() { return bullets; }
std::vector<PlayerBulletSpread>& Player::getSpreadBullets() { return spreadBullets; }
std::vector<PlayerBulletPiercing>& Player::getPiercingBullets() { return piercingBullets; }
std::vector<PlayerBulletOrbital>& Player::getOrbitalBullets() { return orbitalBullets; }
std::vector<PlayerBulletCluster>& Player::getClusterBullets() { return clusterBullets; }
std::vector<PlayerBulletHoming>& Player::getHomingBullets() { return homingBullets; }

WeaponType Player::getCurrentWeapon() const { return currentWeapon; }

// ============================================================
// 受到伤害
// ============================================================
// 无敌/穿怪/护盾状态时忽略伤害
// 受伤后触发1.5秒无敌时间（PhaseShift时更长）
// ============================================================
void Player::takeDamage(int damage) {
    if (godMode) return;              // 调试模式：忽略所有伤害
    if (invincibleTimer > 0.f) return;  // 无敌中：忽略
    if (phaseShiftTimer > 0.f) return;  // PhaseShift中：穿怪，忽略
    if (shieldTimer > 0.f) return;      // 护盾中：忽略
    hp -= damage;                      // 扣血
    if (hp < 0) hp = 0;              // clamp到0
    invincibleTimer = PLAYER_INVINCIBLE_TIME;  // 受伤后进入1.5秒无敌
}

// ============================================================
// 治疗
// ============================================================
void Player::heal(int amount) {
    hp += amount;                     // 加血
    if (hp > maxHP) hp = maxHP;     // 不超过最大HP
}

// ============================================================
// 调试模式
// ============================================================
void Player::setGodMode(bool enabled) {
    godMode = enabled;
}

// ============================================================
// 技能激活（道具触发）
// ============================================================
// 每个方法设置对应的持续计时器
void Player::activateShieldOrb() {
    shieldTimer = SHIELD_ORB_DURATION;       // 3秒护盾
}

void Player::activateOverdrive() {
    overdriveTimer = OVERDRIVE_DURATION;    // 6秒火力提升
}

void Player::activateDashBattery() {
    dashCooldown = 0.f;                     // 清零闪避冷却，立即可用
    dashBatteryTimer = DASH_BATTERY_DURATION;  // 4秒移速×1.5
}

void Player::activatePhaseShift() {
    phaseShiftTimer = PHASE_SHIFT_DURATION;  // 1秒穿怪+无敌
    invincibleTimer = PHASE_SHIFT_DURATION;  // 同时激活无敌
}

void Player::activateNovaForm() {
    novaFormTimer = NOVA_FORM_DURATION;      // 0.5秒大范围伤害
}

void Player::activateSpeedCoil() {
    speedCoilTimer = SPEED_COIL_DURATION;    // 8秒移速×1.5
}

void Player::activateAttackBoost() {
    attackModuleTimer = ATTACK_MODULE_DURATION;  // 10秒伤害×1.3
}

// ============================================================
// 切换武器
// ============================================================
// 6种武器按顺序循环：Normal→Spread→Piercing→Orbital→Cluster→Homing→Normal...
// 切换Orbital武器时会清空所有环绕子弹（避免新旧武器的子弹互相干扰）
void Player::cycleWeapon() {
    static const WeaponType allWeapons[] = {
        WeaponType::Normal, WeaponType::Spread, WeaponType::Piercing,
        WeaponType::Orbital, WeaponType::Cluster, WeaponType::Homing
    };
    constexpr int N = 6;
    WeaponType prevWeapon = currentWeapon;
    for (int i = 0; i < N; ++i) {
        if (currentWeapon == allWeapons[i]) {
            currentWeapon = allWeapons[(i + 1) % N];  // 循环到下一个
            break;
        }
    }
    // 如果从Orbital切换出去，清空所有环绕子弹
    if (prevWeapon == WeaponType::Orbital) {
        for (auto& bullet : orbitalBullets) bullet.deactivate();
    }
}

// 反向切换（向上一个切换）
void Player::cycleWeaponReverse() {
    static const WeaponType allWeapons[] = {
        WeaponType::Normal, WeaponType::Spread, WeaponType::Piercing,
        WeaponType::Orbital, WeaponType::Cluster, WeaponType::Homing
    };
    constexpr int N = 6;
    WeaponType prevWeapon = currentWeapon;
    for (int i = 0; i < N; ++i) {
        if (currentWeapon == allWeapons[i]) {
            currentWeapon = allWeapons[(i - 1 + N) % N];  // 循环到上一个
            break;
        }
    }
    if (prevWeapon == WeaponType::Orbital) {
        for (auto& bullet : orbitalBullets) bullet.deactivate();
    }
}

// ============================================================
// 状态查询（getter）
// ============================================================
float Player::getShieldTimer() const { return shieldTimer; }
float Player::getOverdriveTimer() const { return overdriveTimer; }
float Player::getDashBatteryTimer() const { return dashBatteryTimer; }

// 射速倍率：Overdrive时冷却时间×0.5（即射速×2），所以返回1/rateMult
float Player::getFireRateMultiplier() const {
    return (overdriveTimer > 0.f) ? 1.f / OVERDRIVE_FIRE_RATE_MULT : 1.f;  // Overdrive时返回2
}

// 伤害倍率：Overdrive×1.3，AttackModule×1.3（取较大值）
float Player::getDamageMultiplier() const {
    if (overdriveTimer > 0.f) return OVERDRIVE_DAMAGE_MULT;     // 1.3
    if (attackModuleTimer > 0.f) return ATTACK_MODULE_MULT;    // 1.3
    return 1.f;
}

// 移速倍率：多个加速效果相乘
float Player::getSpeedMultiplier() const {
    float mult = 1.f;
    if (dashBatteryTimer > 0.f) mult *= DASH_BATTERY_SPEED_MULT;  // ×1.5
    if (speedCoilTimer > 0.f) mult *= SPEED_COIL_MULT;           // ×1.5
    return mult;
}

float Player::getDashCooldown() const { return dashCooldown; }
bool Player::isDashing() const { return dashing; }
bool Player::isPhaseShifting() const { return phaseShiftTimer > 0.f; }
bool Player::isNovaFormActive() const { return novaFormTimer > 0.f; }
float Player::getPhaseShiftTimer() const { return phaseShiftTimer; }

// ============================================================
// 连击系统
// ============================================================
int Player::getComboCount() const { return comboCount; }
int Player::getSlowStacks() const { return slowStacks; }

// 连击伤害倍率：≥3连时×1.5，否则×1
float Player::getComboDamageMultiplier() const {
    return (comboCount >= 3) ? 1.5f : 1.f;
}

// 减速倍率：每层10%减速（1-0.1×层数），3层=30%减速
float Player::getSlowMultiplier() const {
    return 1.f - slowStacks * 0.1f;
}

// ============================================================
// 命中注册
// ============================================================
// targetId：被命中的目标ID（用于判断是否连续命中同一目标）
// 连续命中同一目标时连击+1，超时或切换目标时连击重置为1
void Player::registerHit(int targetId) {
    if (targetId == lastHitTargetId) {
        // 连续命中同一目标：连击+1，重置超时计时器
        comboCount++;
        comboTimer = COMBO_TIMEOUT;  // 2秒
    } else {
        // 切换目标或首次命中：连击重置为1
        comboCount = 1;
        lastHitTargetId = targetId;
        comboTimer = COMBO_TIMEOUT;
    }
}