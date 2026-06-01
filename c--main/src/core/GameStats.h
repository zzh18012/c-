// ============================================================
// GameStats.h - 游戏统计数据结构
// ============================================================
// 职责：存储游戏中所有需要显示的实时数据，供HUD读取
// 这是一个纯数据结构，不包含任何逻辑，只做数据容器
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f等基础类型）
#include <string>

// 标准数组容器（用于存储多个Boss的状态）
#include <array>

// 引入Config获取MAX_BOSS_COUNT等常量
#include "core/Config.h"

// ============================================================
// 游戏结果枚举
// ============================================================
// 表示游戏的最终结果（用于结算画面）
enum class GameResult {
    Playing,   // 游戏进行中
    Victory,   // 玩家胜利（击败所有Boss）
    GameOver   // 玩家失败（HP归零）
};

// ============================================================
// Boss攻击类型枚举
// ============================================================
// 定义Boss所有可能的攻击模式（用于HUD显示当前攻击名称）
enum class BossAttackType {
    None,              // 无攻击（刚生成或空闲状态）

    // ---- 阶段1攻击（简单）----
    CircleBurst,       // 圆形爆发：向360度均匀发射多发子弹
    SpiralShot,        // 螺旋射击：持续旋转的多束子弹螺旋状射出
    AimedShot,         // 瞄准射击：朝玩家方向发射3发扩散子弹

    // ---- 阶段2新增攻击 ----
    WaveBarrage,       // 波浪弹幕：从屏幕两侧发出正弦波动的子弹
    CrossLaser,        // 十字激光：预警+实际伤害，十字形弹幕
    RainBullets,       // 子弹雨：从屏幕顶部随机位置落下的弹幕

    // ---- 阶段3新增攻击（更多样、更危险）----
    FlowerBurst,       // 花朵爆发：多层环形子弹向外扩散，呈花朵形状
    EightDirections,   // 八方向射击：同时向8个方向发射子弹
    SnakeWave,         // 蛇形波浪：左右摆动的蛇形路径弹幕
    HomingOrbs,        // 追踪球：缓慢但持续追踪玩家的球状弹幕
    DoubleSpiral,      // 双螺旋：两股同时旋转的螺旋弹
    BladeRing,         // 刀刃之环：旋转的环形刀刃弹幕
    ScreenWipe,        // 屏幕横扫：从左到右横扫的弹幕
    RadialBurst,       // 径向爆发：16发同时向周围爆发
    RandomSpray,       // 随机喷射：随机位置和方向的弹幕
    PredictiveShot,    // 预测射击：朝玩家预测位置发射的精准弹幕
    ChargeBeam,        // 蓄力光束：长时间预警+高伤害光束
    TentacleSweep,     // 触须横扫：Boss触须横向扫过屏幕
    ScreenLock,        // 屏幕锁定：画面两侧缩进，限制玩家移动
    BulletRainFull,    // 全屏弹幕雨：大量子弹从顶部落下
    SummonMinions,     // 召唤小怪：生成辅助小怪（暂未使用）
    PhaseTransition    // 阶段转换：Boss无敌+回血+瞬移
};

// ============================================================
// 单个Boss的状态
// ============================================================
// 用于HUD显示单个Boss的血量、阶段、当前攻击名
struct BossState {
    int hp = 0;               // 当前HP
    int maxHP = 0;            // 最大HP
    int phase = 1;            // 当前阶段（1/2/3）
    bool active = false;      // 是否存活
    std::string attackName;   // 当前攻击名称（用于HUD显示）
    bool laserWarning = false;   // 是否处于激光预警阶段
    bool laserActive = false;    // 是否处于激光实际伤害阶段
};

// ============================================================
// 多Boss状态
// ============================================================
// 支持最多3个Boss同时存在的数据结构（Hard=2个，Lunatic=3个）
static constexpr int MAX_BOSS_COUNT = 3;

// 多Boss统计信息容器
struct MultiBossStats {
    std::array<BossState, MAX_BOSS_COUNT> bosses;  // 最多MAX_BOSS_COUNT个Boss的状态数组
    int activeCount = 0;                          // 当前存活的Boss数量
    Difficulty difficulty = Difficulty::Normal;     // 当前难度
};

// ============================================================
// 游戏统计数据结构（主结构）
// ============================================================
// 收集了游戏的所有实时状态，Game.cpp每帧更新，HUD每帧读取
struct GameStats {
    // ---- 玩家状态 ----
    int playerHP = 100;       // 当前HP
    int playerMaxHP = 100;    // 最大HP

    // ---- 时间相关 ----
    float elapsedTime = 0.0f;   // 游戏已进行时间（秒）

    // ---- 当前Boss攻击（单Boss时使用）----
    BossAttackType currentAttack = BossAttackType::None;  // 当前攻击类型
    std::string currentAttackName = "None";               // 当前攻击名称（显示用）

    // ---- 玩家特殊状态 ----
    bool playerInvincible = false;     // 是否处于无敌状态
    float playerShieldTimer = 0.f;    // 护盾剩余时间（>0时无敌）
    float playerOverdriveTimer = 0.f;  // 火力提升剩余时间
    float playerDashBatteryTimer = 0.f;  // 闪避电池剩余时间
    bool playerIsDashing = false;       // 是否正在闪避
    float playerDashCooldown = 0.f;    // 闪避冷却时间

    // ---- 玩家倍率状态 ----
    float playerSpeedMultiplier = 1.f;       // 移动速度倍率
    float playerFireRateMultiplier = 1.f;    // 射速倍率
    float playerDamageMultiplier = 1.f;      // 伤害倍率

    // ---- 游戏效果 ----
    bool bulletTimeActive = false;   // 子弹时间是否激活

    // ---- 游戏结果 ----
    bool victory = false;   // 玩家是否胜利
    bool gameOver = false;   // 玩家是否失败

    // ---- 多Boss统计 ----
    MultiBossStats multiBossStats;  // 多Boss信息

    // ---- 构造函数 ----
    // 初始化列表确保multiBossStats的默认构造被调用
    GameStats() : multiBossStats{} {}
};