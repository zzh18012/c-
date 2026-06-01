// ============================================================
// Player.h - 玩家类声明
// ============================================================
// 职责：玩家控制（移动、射击、技能）、状态管理、渲染外观
// 玩家是一个机械战甲（mech），有6种武器形态，通过道具获得不同技能
// ============================================================

#pragma once

// 标准库（用于std::vector）
#include <string>
#include <array>

// 配置文件（获取所有游戏常量的值）
#include "core/Config.h"

// ---- 武器子弹类型 ----
// 每种武器对应一个子弹类（使用对象池管理）
#include "PlayerBullet.h"           // 普通子弹：单发，高射速
#include "PlayerBulletSpread.h"      // 散射子弹：5发扇形
#include "PlayerBulletPiercing.h"    // 穿透子弹：高速穿透多个敌人
#include "PlayerBulletOrbital.h"    // 环绕子弹：绕玩家旋转的子弹
#include "PlayerBulletCluster.h"    // 集群子弹：爆炸分裂
#include "PlayerBulletHoming.h"     // 追踪子弹：自动追踪Boss

// ============================================================
// 武器类型枚举
// ============================================================
// 玩家有6种武器，通过道具WeaponEssence切换
// 每次只使用一种武器，按Q循环切换
enum class WeaponType {
    Normal,   // 普通武器（默认）：单发，快速，高伤害
    Spread,   // 散射武器：5发扇形弹幕，覆盖面广
    Piercing, // 穿透武器：高速子弹，穿透所有路径上的敌人
    Orbital,  // 环绕武器：4发子弹绕玩家旋转，持续伤害
    Cluster,  // 集群武器：母弹飞行一段距离后爆炸成多发
    Homing    // 追踪武器：自动追踪Boss，有减速效果
};

// ============================================================
// Player类
// ============================================================
// 玩家是游戏中的主动角色，通过WASD移动，鼠标瞄准，左键射击
// 同时管理6种子弹数组、多种状态效果（护盾、Overdrive、闪避等）
class Player {
public:
    // 构造函数：初始化所有子弹数组、位置、状态
    Player();

    // ---- 核心方法 ----
    // 处理输入：WASD移动、Shift闪避、鼠标瞄准、左键射击、Q换武器
    void handleInput(float dt, const sf::RenderWindow& window);
    // 更新逻辑：所有冷却计时器、状态效果、子弹移动
    void update(float dt);
    // 渲染：先渲染子弹，再渲染玩家战甲
    void render(sf::RenderWindow& window) const;

    // ============================================================
    // 状态查询接口（供Game.cpp碰撞检测和HUD使用）
    // ============================================================
    sf::Vector2f getPosition() const;  // 获取玩家坐标（用于碰撞检测）
    float getRadius() const;           // 获取碰撞半径（PLAYER_RADIUS=18）
    int getHP() const;                 // 获取当前HP
    int getMaxHP() const;              // 获取最大HP
    bool isDead() const;               // 是否死亡（HP<=0）
    bool isInvincible() const;         // 是否无敌（受伤后短暂无敌）

    // ---- 伤害与治疗 ----
    void takeDamage(int damage);       // 受到伤害（扣血+触发无敌时间）
    void heal(int amount);             // 治疗（加血，不超过maxHP）
    void setGodMode(bool enabled);     // 设置调试无敌模式（F3快捷键）

    // ============================================================
    // 技能激活接口（道具触发）
    // ============================================================
    void activateShieldOrb();      // 激活护盾（3秒无敌）
    void activateOverdrive();      // 激活火力提升（射速×2，伤害×1.3）
    void activateDashBattery();    // 激活闪避电池（移速×1.5，冷却清零）
    void activatePhaseShift();     // 激活相位偏移（无敌+穿怪）
    void activateNovaForm();       // 激活Nova形态（大范围伤害）
    void activateSpeedCoil();      // 激活速度线圈（移速×1.5）
    void activateAttackBoost();    // 激活攻击模块（伤害×1.3）
    void cycleWeapon();            // 切换到下一种武器
    void cycleWeaponReverse();     // 切换到上一种武器

    // ---- 状态查询 ----
    float getShieldTimer() const;      // 护盾剩余时间
    float getOverdriveTimer() const;    // 火力提升剩余时间
    float getDashBatteryTimer() const; // 闪避电池剩余时间
    float getFireRateMultiplier() const;  // 射速倍率（Overdrive时变化）
    float getDamageMultiplier() const;    // 伤害倍率（Overdrive/AttackModule时变化）
    float getSpeedMultiplier() const;     // 移速倍率（DashBattery/SpeedCoil时变化）
    float getDashCooldown() const;        // 闪避冷却剩余时间
    bool isDashing() const;              // 是否正在闪避
    bool isPhaseShifting() const;        // 是否正在相位偏移
    bool isNovaFormActive() const;        // 是否处于Nova形态
    float getPhaseShiftTimer() const;     // 相位偏移剩余时间

    // ---- 连击系统 ----
    int getComboCount() const;          // 当前连击数
    int getSlowStacks() const;          // 减速Debuff层数
    float getComboDamageMultiplier() const;  // 连击伤害倍率（≥3连时×1.5）
    float getSlowMultiplier() const;    // 减速倍率（1-层数×0.1）

    // ---- 命中注册 ----
    // 每次子弹命中Boss时调用，用于统计连击和切换目标
    void registerHit(int targetId);

    // ============================================================
    // 子弹数组访问接口（供Game.cpp碰撞检测遍历）
    // ============================================================
    std::vector<PlayerBullet>& getBullets();           // 普通子弹
    std::vector<PlayerBulletSpread>& getSpreadBullets();  // 散射子弹
    std::vector<PlayerBulletPiercing>& getPiercingBullets();  // 穿透子弹
    std::vector<PlayerBulletOrbital>& getOrbitalBullets();   // 环绕子弹
    std::vector<PlayerBulletCluster>& getClusterBullets();   // 集群子弹
    std::vector<PlayerBulletHoming>& getHomingBullets();     // 追踪子弹

    WeaponType getCurrentWeapon() const;  // 获取当前武器类型

private:
    // ---- 射击逻辑 ----
    void shoot();  // 根据当前武器生成子弹（处理所有6种武器的射击逻辑）

    // ---- 渲染方法 ----
    void renderMechBody(sf::RenderWindow& window) const;    // 渲染战甲身体
    void renderMechWeapon(sf::RenderWindow& window) const;   // 渲染武器（炮管）
    void renderMechEffects(sf::RenderWindow& window) const;  // 渲染特效（护盾、闪烁等）

    // ============================================================
    // 成员变量
    // ============================================================

    // ---- 位置与朝向 ----
    sf::Vector2f position;   // 玩家当前坐标
    sf::Vector2f aimDir;     // 玩家朝向（鼠标位置-玩家位置，归一化）

    // ---- 子弹数组（对象池）----
    std::vector<PlayerBullet> bullets;           // 普通子弹（最多200发）
    std::vector<PlayerBulletSpread> spreadBullets;  // 散射子弹（最多50发）
    std::vector<PlayerBulletPiercing> piercingBullets;  // 穿透子弹（最多20发）
    std::vector<PlayerBulletOrbital> orbitalBullets;    // 环绕子弹（最多4发）
    std::vector<PlayerBulletCluster> clusterBullets;    // 集群子弹（最多20发）
    std::vector<PlayerBulletHoming> homingBullets;      // 追踪子弹（最多15发）

    // ---- 射击冷却 ----
    // 每种武器独立的冷却计时器（避免切换武器后冷却时间丢失）
    float shootCooldown;      // 普通武器射击冷却
    float spreadCooldown;     // 散射武器射击冷却
    float piercingCooldown;   // 穿透武器射击冷却
    float orbitalCooldown;    // 环绕武器射击冷却
    float clusterCooldown;    // 集群武器射击冷却
    float homingCooldown;     // 追踪武器射击冷却

    // ---- 当前武器 ----
    WeaponType currentWeapon;  // 当前使用的武器类型

    // ---- 生命值 ----
    int hp;        // 当前HP
    int maxHP;     // 最大HP（从Config获取）

    // ---- 无敌状态 ----
    float invincibleTimer;   // 无敌倒计时（受伤后1.5秒无敌，PhaseShift时延长）
    bool godMode;           // 调试无敌模式（按F3开启）

    // ============================================================
    // 技能持续时间
    // ============================================================
    float shieldTimer;       // 护盾（ShieldOrb）持续时间
    float overdriveTimer;    // 火力提升（Overdrive）持续时间
    float dashBatteryTimer;  // 闪避电池（DashBattery）持续时间
    float phaseShiftTimer;   // 相位偏移（PhaseShift）持续时间
    float novaFormTimer;     // Nova形态（NovaForm）持续时间
    float speedCoilTimer;    // 速度线圈（SpeedCoil）持续时间
    float attackModuleTimer; // 攻击模块（AttackBoost）持续时间

    // ============================================================
    // 闪避状态
    // ============================================================
    float dashCooldown;    // 闪避冷却计时器（2秒）
    bool dashing;          // 是否正在闪避（闪避中无敌+高速移动）
    float dashTimer;       // 闪避持续时间（0.15秒）
    sf::Vector2f dashDir;   // 闪避方向（WASD输入方向或朝向方向）

    // ============================================================
    // 动画与反馈
    // ============================================================
    float mechRotationAngle;    // 战甲朝向角度（用于渲染，弧度转角度）
    float weaponRecoilTimer;    // 武器后座力计时器（射击时枪管后退动画）

    // ============================================================
    // 连击系统
    // ============================================================
    int comboCount;        // 当前连击数（连续命中同一目标时+1）
    float comboTimer;      // 连击超时计时器（2秒内不命中则重置）
    int lastHitTargetId;   // 上次命中的目标ID（用于判断是否连续命中同一目标）

    // ============================================================
    // 减速Debuff（玩家对Boss施加的减速效果）
    // ============================================================
    int slowStacks;   // 减速层数（每层10%减速，上限3层=30%减速）
    float slowTimer;  // 减速持续时间（2秒）
};