// ============================================================
// Boss.h - Boss类声明
// ============================================================
// 职责：Boss的AI攻击模式、状态管理、渲染外观、入场动画
// Boss是一个外星机械生物，有3种类型（Inferno/Void/Thunder），3个阶段
// 每阶段有独特的攻击模式和外观颜色
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::Color等基础类型）
#include <SFML/Graphics.hpp>

// 标准字符串（用于当前攻击名称显示）
#include <string>

// 标准向量（用于子弹数组）
#include <vector>

// 游戏统计数据（用于HUD显示的信息传递）
#include "core/GameStats.h"

// Boss子弹类型
#include "BossBullet.h"

// ============================================================
// Boss类型枚举
// ============================================================
// 三种Boss各有独特的颜色主题和光环特效
enum class BossType {
    Inferno,  // 火焰型：橙红色主题，热血激进
    Void,     // 虚空型：紫蓝色主题，神秘诡异
    Thunder   // 雷电型：金黄色主题，庄严震撼
};

// ============================================================
// Boss类
// ============================================================
// Boss是游戏中的敌人，有HP、阶段、攻击模式
// 管理20+种攻击模式，每种模式有独立的计时器和执行逻辑
// 同时管理Boss自己的子弹对象池（2000发）
class Boss {
public:
    // 构造函数：初始化位置、血量、攻击模式、动画计时器
    Boss();

    // ---- 核心方法 ----
    // 每帧更新：阶段检测、攻击模式切换、具体攻击执行、子弹更新
    void update(float dt, const sf::Vector2f& playerPosition);
    // 渲染：Boss子弹→特效→触须→身体→眼睛→激光预警→触须横扫→屏幕锁定→入场特效→光环
    void render(sf::RenderWindow& window) const;

    // ============================================================
    // 状态查询接口
    // ============================================================
    sf::Vector2f getPosition() const;    // 获取Boss坐标（用于碰撞检测）
    void setPosition(const sf::Vector2f& pos);  // 设置Boss坐标（同时重置入场动画起点/终点）
    float getRadius() const;             // 获取碰撞半径（BOSS_RADIUS=55）
    int getHP() const;                  // 获取当前HP
    int getMaxHP() const;               // 获取最大HP
    int getPhase() const;               // 获取当前阶段（1/2/3）
    bool isDead() const;                // 是否死亡（HP<=0）
    std::string getCurrentAttackName() const;  // 获取当前攻击名称（供HUD显示）
    BossAttackType getCurrentAttackType() const;  // 获取当前攻击类型（供游戏逻辑判断）

    // ---- 伤害 ----
    void takeDamage(int damage);  // 受到伤害（PhaseTransition时无敌）

    // ============================================================
    // 子弹管理接口
    // ============================================================
    std::vector<BossBullet>& getBullets();  // 获取子弹数组引用（供Game.cpp碰撞检测）
    void spawnBullet(sf::Vector2f pos, sf::Vector2f dir, float speed, int dmg);  // 生成一颗子弹
    void clearAllBullets();                 // 清空所有子弹（BulletTime道具、NovaBomb等）
    void setBulletSpeedMultiplier(float mult);  // 设置子弹速度倍率（玩家技能效果）
    float getBulletSpeedMultiplier() const;
    void applySlow(int stacks);              // 施加减速Debuff（玩家Homing子弹效果）
    float getSlowMultiplier() const;        // 获取减速倍率（影响子弹速度）

    // ============================================================
    // 特殊攻击状态查询（供Game.cpp检测并触发对应特效）
    // ============================================================
    bool isLaserWarning() const;     // 十字激光预警阶段
    bool isLaserActive() const;      // 十字激光实际伤害阶段
    bool isChargeBeamWarning() const;  // 蓄力光束预警阶段
    bool isChargeBeamActive() const;   // 蓄力光束实际伤害阶段
    bool isScreenLocking() const;     // 屏幕锁定攻击中
    bool isPhaseTransitioning() const;  // 阶段转换中（无敌+回血）
    sf::Vector2f getChargeBeamTarget() const;  // 蓄力光束追踪的目标位置
    float getTentacleSweepY() const;   // 触须横扫攻击的Y坐标
    float getScreenLockProgress() const;  // 屏幕锁定进度（0~1）

    // ============================================================
    // Boss类型和入场动画
    // ============================================================
    void setBossType(BossType type);     // 设置Boss类型（影响外观颜色和光环）
    BossType getBossType() const;        // 获取Boss类型

    // 入场动画控制（从屏幕下方飘入）
    void setEntranceAnimation(float progress);  // 设置入场进度（0~1），自动计算位置插值
    float getEntranceAnimation() const;          // 获取当前入场进度
    bool isEntranceComplete() const;             // 入场是否完成（progress>=1）
    void setEntranceStartPos(const sf::Vector2f& pos);  // 设置入场起点
    void setEntranceTargetPos(const sf::Vector2f& pos);  // 设置入场终点

    // ============================================================
    // 光环渲染方法（每种Boss类型有独特的光环）
    // ============================================================
    void renderInfernoAura(sf::RenderWindow& window) const;   // 火焰光环：8个橙色粒子+红色光晕
    void renderVoidAura(sf::RenderWindow& window) const;       // 虚空光环：12个紫色粒子+紫色光晕
    void renderThunderAura(sf::RenderWindow& window) const;  // 雷电光环：6个金黄粒子+闪电装饰+金色光晕

    // 入场特效（不同类型有不同的入场动画）
    void renderEntranceEffect(sf::RenderWindow& window) const;

private:
    // ---- 攻击AI ----
    void updateAttackPattern(float dt, const sf::Vector2f& playerPos);  // 更新攻击模式逻辑
    void switchToNextAttack();  // 切换到下一个攻击（循环播放攻击序列）
    void resetNewAttackTimers();  // 重置所有攻击的计时器（切换攻击时调用）

    // ---- 20+种具体攻击执行 ----
    // 每种攻击有独立的execute方法
    void executeCircleBurst(float dt);         // 圆形爆发：均布24发子弹
    void executeSpiralShot(float dt);          // 螺旋射击：旋转多束子弹
    void executeAimedShot(float dt, const sf::Vector2f& playerPos);  // 瞄准射击：朝玩家3发扩散
    void executeWaveBarrage(float dt);          // 波浪弹幕：从两侧发出正弦波子弹
    void executeCrossLaser(float dt, const sf::Vector2f& playerPos);  // 十字激光：预警+实际伤害
    void executeRainBullets(float dt);          // 子弹雨：随机位置下落
    void executeFlowerBurst(float dt);          // 花朵爆发：多层环形子弹
    void executeEightDirections(float dt);      // 八方向射击
    void executeSnakeWave(float dt);            // 蛇形波浪：左右摆动前进
    void executeHomingOrbs(float dt, const sf::Vector2f& playerPos);  // 追踪球
    void executeDoubleSpiral(float dt);         // 双螺旋：两股螺旋
    void executeBladeRing(float dt);            // 刀刃之环：旋转环形弹幕
    void executeScreenWipe(float dt);           // 屏幕横扫：从左到右
    void executeRadialBurst(float dt);          // 径向爆发：同时16发
    void executeRandomSpray(float dt);          // 随机喷射
    void executePredictiveShot(float dt, const sf::Vector2f& playerPos);  // 预测射击
    void executeChargeBeam(float dt, const sf::Vector2f& playerPos);  // 蓄力光束
    void executeTentacleSweep(float dt, const sf::Vector2f& playerPos);  // 触须横扫
    void executeScreenLock(float dt);            // 屏幕锁定
    void executeBulletRainFull(float dt);       // 全屏弹幕雨
    void executePhaseTransition(float dt);      // 阶段转换（瞬移+回血）

    // ---- 渲染方法 ----
    void renderAlienBody(sf::RenderWindow& window) const;     // 渲染外星身体（多层圆形+凸起）
    void renderAlienEye(sf::RenderWindow& window) const;    // 渲染外星眼睛（跟随玩家）
    void renderAlienTentacles(sf::RenderWindow& window) const;  // 渲染触须（分段矩形）
    void renderAlienEffects(sf::RenderWindow& window) const;   // 渲染特效（环绕粒子/光晕）
    void renderChargeBeamWarning(sf::RenderWindow& window) const;  // 渲染蓄力光束预警
    void renderTentacleSweep(sf::RenderWindow& window) const;   // 渲染触须横扫
    void renderScreenLock(sf::RenderWindow& window) const;     // 渲染屏幕锁定

    // ============================================================
    // 成员变量
    // ============================================================

    // ---- 位置 ----
    sf::Vector2f position;          // 当前坐标
    sf::Vector2f originalPosition;   // 默认坐标（用于阶段转换后恢复）

    // ---- 子弹对象池 ----
    std::vector<BossBullet> bullets;  // Boss所有子弹（最多MAX_BOSS_BULLETS=2000发）

    // ---- 血量与阶段 ----
    int hp;         // 当前HP
    int maxHP;      // 最大HP
    int phase;      // 当前阶段（1/2/3，由HP百分比决定）

    // ---- 子弹速度倍率（玩家技能影响）----
    float bulletSpeedMultiplier = 1.f;  // 玩家Overdrive时Boss子弹加速

    // ---- 攻击状态 ----
    BossAttackType currentAttack;    // 当前攻击类型
    std::string currentAttackName;    // 当前攻击名称（显示用）
    float attackTimer;              // 当前攻击已持续时间（到达duration后切换）
    float fireTimer;                // 射击间隔计时器（每interval秒射一次）
    int attackIndex;                // 当前攻击在序列中的索引（用于切换）

    // ---- 各种攻击的专用计时器 ----
    // 每个攻击模式有自己独立的计时器，避免相互干扰
    float spiralAngle;              // 螺旋射击：当前角度（每帧累加形成旋转效果）
    int crossLaserStep;             // 十字激光：当前步骤（0=预警，1=实际伤害，2=结束）
    int crossLaserCount;            // 十字激光：已执行次数（重复3次）
    float crossLaserTimer;          // 十字激光：步骤内计时器

    // 新攻击模式的计时器
    float flowerTimer;              // 花朵爆发
    float eightDirTimer;            // 八方向射击
    float snakeTimer;               // 蛇形波浪
    float homingOrbTimer;           // 追踪球
    float doubleSpiralTimer;         // 双螺旋
    float bladeRingTimer;           // 刀刃之环
    float screenWipeTimer;          // 屏幕横扫
    float radialBurstTimer;         // 径向爆发
    float randomSprayTimer;         // 随机喷射
    float predictiveTimer;          // 预测射击
    float chargeBeamStep;           // 蓄力光束步骤（0=预警，1=实际伤害）
    float chargeBeamTimer;          // 蓄力光束计时器
    float tentacleSweepTimer;       // 触须横扫计时器
    float screenLockTimer;          // 屏幕锁定计时器
    float bulletRainFullTimer;      // 全屏弹幕雨计时器
    float phaseTransitionTimer;     // 阶段转换计时器

    // ---- 特殊攻击的动态数据 ----
    sf::Vector2f chargeBeamTarget;  // 蓄力光束追踪的目标坐标
    float tentacleSweepY;          // 触须横扫的当前Y坐标（跟随玩家）
    float screenLockProgress;       // 屏幕锁定进度（用于渲染墙面缩进）

    // ---- 减速Debuff ----
    int slowStacks;   // 减速层数（每层10%减速，上限3层）
    float slowTimer;  // 减速持续时间

    // ============================================================
    // 动画状态
    // ============================================================
    float pulseTimer;       // 脉动计时器（控制身体脉动效果）
    float deformTimer;      // 变形计时器（控制身体微小抖动）
    float eyeTrackAngle;   // 眼睛追踪角度（朝向玩家方向）

    // ============================================================
    // Boss类型和入场动画
    // ============================================================
    BossType bossType;  // Boss类型（Inferno/Void/Thunder）
    float entranceAnimationProgress;  // 入场动画进度（0~1）
    bool entranceAnimationActive;      // 入场动画是否激活
    sf::Vector2f entranceStartPos;     // 入场起点（屏幕下方某处）
    sf::Vector2f entranceTargetPos;     // 入场终点（最终位置）
};