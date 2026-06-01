// ============================================================
// Config.h - 游戏配置文件
// ============================================================
// 职责：定义游戏中所有常量数值（200+个）
// 所有常量使用constexpr修饰，在编译时确定，提高性能
// 使用有意义的命名规则：
//   - WINDOW_*：窗口相关
//   - PLAYER_*：玩家相关
//   - BOSS_*：Boss相关
//   - *_SPEED/*_DAMAGE/*_RADIUS：数值型属性
//   - *_DURATION/*_INTERVAL/*_COOLDOWN：时间型属性
//   - *_RATIO/*_MULT/*_THRESHOLD：比例/倍数/阈值
// ============================================================

#pragma once

// SFML头文件（Vector2f用于坐标，Color用于颜色）
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

// ============================================================
// 窗口配置
// ============================================================
constexpr int WINDOW_WIDTH = 1280;              // 窗口宽度（像素）
constexpr int WINDOW_HEIGHT = 720;             // 窗口高度（像素）
constexpr const char* WINDOW_TITLE = "Neon Bullet Arena"; // 窗口标题

// ============================================================
// 玩家基础属性
// ============================================================
constexpr float PLAYER_SPEED = 300.f;           // 玩家移动速度（像素/秒）
constexpr float PLAYER_RADIUS = 18.f;          // 玩家碰撞半径（像素，用于圆形碰撞检测）
constexpr int PLAYER_MAX_HP = 100;             // 玩家最大生命值
constexpr float PLAYER_SHOOT_COOLDOWN = 0.15f; // 射击冷却时间（秒），越小射速越快
constexpr int PLAYER_BULLET_DAMAGE = 10;       // 普通子弹伤害值
constexpr float PLAYER_BULLET_SPEED = 800.f;  // 子弹飞行速度（像素/秒）
constexpr float PLAYER_INVINCIBLE_TIME = 1.5f; // 受伤后无敌时间（秒）
constexpr float PLAYER_DASH_SPEED = 900.f;     // 闪避冲刺速度（像素/秒）
constexpr float PLAYER_DASH_DURATION = 0.15f;   // 闪避持续时间（秒）
constexpr float PLAYER_DASH_COOLDOWN = 2.0f;   // 闪避冷却时间（秒）
constexpr sf::Vector2f PLAYER_START_POS(640.f, 550.f); // 玩家初始位置（屏幕中央偏下）

// ============================================================
// Boss基础属性
// ============================================================
constexpr float BOSS_RADIUS = 55.f;            // Boss碰撞半径（像素）
constexpr int BOSS_MAX_HP = 1000;              // Boss最大生命值
constexpr sf::Vector2f BOSS_POS(640.f, 180.f); // Boss默认位置（屏幕顶部中央）

// ============================================================
// Boss阶段转换阈值
// ============================================================
// 当Boss HP低于某个百分比时进入下一阶段
constexpr float PHASE2_HP_RATIO = 0.70f;       // 进入阶段2的HP阈值（70%）
constexpr float PHASE3_HP_RATIO = 0.35f;       // 进入阶段3的HP阈值（35%）

// ============================================================
// Boss子弹属性
// ============================================================
constexpr float BOSS_BULLET_SPEED_SLOW = 180.f;   // 慢速子弹速度
constexpr float BOSS_BULLET_SPEED_NORMAL = 240.f; // 中速子弹速度
constexpr float BOSS_BULLET_SPEED_FAST = 320.f;  // 快速子弹速度
constexpr int BOSS_BULLET_DAMAGE = 15;            // Boss子弹基础伤害
constexpr float BOSS_BULLET_RADIUS = 8.f;         // Boss子弹碰撞半径
constexpr int MAX_BOSS_BULLETS = 2000;             // Boss子弹对象池最大数量

// ============================================================
// 玩家子弹属性
// ============================================================
constexpr int MAX_PLAYER_BULLETS = 200;           // 玩家子弹对象池最大数量
constexpr float PLAYER_BULLET_RADIUS = 5.f;      // 玩家子弹碰撞半径

// ============================================================
// 攻击时间参数（秒）
// ============================================================
// 圆形爆发
constexpr float CIRCLE_BURST_INTERVAL = 0.6f;    // 射击间隔
constexpr float CIRCLE_BURST_DURATION = 4.0f;     // 持续时间
constexpr int CIRCLE_BURST_COUNT = 24;           // 每次发射子弹数

// 螺旋射击
constexpr float SPIRAL_INTERVAL = 0.05f;         // 射击间隔（很短的间隔产生螺旋效果）
constexpr float SPIRAL_DURATION = 5.0f;           // 持续时间
constexpr int SPIRAL_BULLETS_PER_SHOT = 4;        // 每组子弹数

// 瞄准射击
constexpr float AIMED_INTERVAL = 0.4f;           // 射击间隔
constexpr float AIMED_DURATION = 4.0f;            // 持续时间
constexpr float AIMED_ANGLE_OFFSET = 8.f;        // 散射角度偏移（度）

// 波浪弹幕
constexpr float WAVE_INTERVAL = 0.15f;            // 射击间隔
constexpr float WAVE_DURATION = 5.0f;              // 持续时间
constexpr float WAVE_AMPLITUDE = 60.f;           // 正弦波振幅（像素）
constexpr float WAVE_FREQUENCY = 3.f;             // 正弦波频率

// 十字激光
constexpr float CROSS_LASER_WARNING_TIME = 1.0f; // 预警时间（红色警告线）
constexpr float CROSS_LASER_ACTIVE_TIME = 0.5f;   // 实际伤害时间（激光发射）
constexpr int CROSS_LASER_REPEATS = 3;            // 重复次数
constexpr float LASER_WIDTH = 40.f;                 // 激光宽度（像素）

// 子弹雨
constexpr float RAIN_INTERVAL = 0.08f;           // 射击间隔（很快产生密集效果）
constexpr float RAIN_DURATION = 5.0f;              // 持续时间

// ============================================================
// 道具系统配置
// ============================================================
constexpr int ITEM_MAX_COUNT = 2;                // 场上最大道具数量
constexpr float ITEM_SPAWN_INTERVAL = 10.f;       // 道具生成间隔（秒）
constexpr float ITEM_RADIUS = 14.f;               // 道具碰撞半径
constexpr float ITEM_LIFETIME = 8.f;              // 道具存在时间（秒），超时消失

// ============================================================
// 治疗核心道具
// ============================================================
constexpr float HEAL_CORE_RATIO = 0.25f;          // 治疗比例：恢复25%最大HP

// ============================================================
// 护盾球道具
// ============================================================
constexpr float SHIELD_ORB_DURATION = 3.f;        // 无敌护盾持续时间（秒）

// ============================================================
// 火力超载道具（Overdrive）
// ============================================================
constexpr float OVERDRIVE_DURATION = 6.f;          // 持续时间
constexpr float OVERDRIVE_FIRE_RATE_MULT = 0.5f;  // 射速倍率（0.5=时间减半=速度翻倍）
constexpr float OVERDRIVE_DAMAGE_MULT = 1.3f;      // 伤害倍率（×1.3）

// ============================================================
// 子弹时间道具（BulletTime）
// ============================================================
constexpr float BULLET_TIME_DURATION = 5.f;        // 持续时间
constexpr float BULLET_TIME_SLOW_RATIO = 0.5f;    // Boss子弹速度比例（×0.5=减速50%）

// ============================================================
// Nova炸弹道具
// ============================================================
constexpr int NOVA_BOMB_DAMAGE = 150;             // 全屏伤害值

// ============================================================
// 冲刺电池道具
// ============================================================
constexpr float DASH_BATTERY_DURATION = 4.f;       // 效果持续时间
constexpr float DASH_BATTERY_SPEED_MULT = 1.5f;  // 移动速度倍率（×1.5）

// ============================================================
// 粒子系统
// ============================================================
constexpr int PARTICLE_POOL_SIZE = 5000;           // 粒子对象池最大数量

// ============================================================
// UI - 按钮样式
// ============================================================
constexpr float BUTTON_WIDTH = 260.f;            // 按钮宽度（像素）
constexpr float BUTTON_HEIGHT = 54.f;             // 按钮高度（像素）
constexpr float BUTTON_BORDER_THICKNESS = 2.f;     // 按钮边框厚度

// ============================================================
// UI - 菜单样式
// ============================================================
constexpr float MENU_TITLE_SIZE = 48.f;           // 菜单标题字体大小
constexpr float MENU_BUTTON_SPACING = 20.f;      // 菜单按钮间距（像素）
constexpr float MENU_OVERLAY_ALPHA = 160.f;       // 菜单遮罩透明度

// ============================================================
// UI - HUD样式
// ============================================================
constexpr float HUD_PADDING = 20.f;               // HUD内边距（像素）
constexpr float HUD_BAR_HEIGHT = 22.f;            // 血条高度（像素）
constexpr float HUD_PLAYER_BAR_WIDTH = 280.f;      // 玩家血条宽度
constexpr float HUD_BOSS_BAR_WIDTH = 420.f;        // Boss血条宽度
constexpr float HUD_FONT_SIZE_SMALL = 14;         // 小号字体
constexpr float HUD_FONT_SIZE_NORMAL = 18;         // 普通字体
constexpr float HUD_FONT_SIZE_LARGE = 36;          // 大号字体
constexpr float HUD_BAR_LERP_SPEED = 8.f;          // 血条平滑过渡速度

// ============================================================
// 背景系统配置
// ============================================================
constexpr int BG_AMBIENT_DOT_COUNT = 35;          // 环境漂浮点数量
constexpr float BG_GRID_ALPHA = 60.f;              // 网格线透明度（0~255）
constexpr float BG_DOT_MIN_RADIUS = 2.5f;         // 漂浮点最小半径
constexpr float BG_DOT_MAX_RADIUS = 5.f;           // 漂浮点最大半径
constexpr float BG_DOT_DRIFT_SPEED = 25.f;         // 漂浮点向上飘动速度
constexpr float BG_DOT_PULSE_SPEED = 1.5f;         // 漂浮点脉动速度

// ============================================================
// 玩家武器变体 - 散射武器
// ============================================================
constexpr float PLAYER_SPREAD_ANGLE = 30.f;       // 散射角度（5发子弹分布在30度范围内）

// ============================================================
// 玩家武器变体 - 穿透武器
// ============================================================
constexpr float PLAYER_PIERCING_SPEED = 1200.f;   // 穿透子弹速度（很快）
constexpr int PLAYER_PIERCING_DAMAGE = 15;         // 穿透子弹伤害

// ============================================================
// 玩家武器变体 - 环绕武器
// ============================================================
constexpr float PLAYER_ORBITAL_SPEED = 180.f;     // 环绕子弹角速度（弧度/秒）
constexpr float PLAYER_ORBITAL_RADIUS = 60.f;     // 环绕半径
constexpr int ORBITAL_BULLET_DAMAGE = 15;         // 环绕子弹伤害

// ============================================================
// 玩家武器变体 - 集群武器
// ============================================================
constexpr float PLAYER_CLUSTER_SPEED = 400.f;    // 母弹飞行速度
constexpr int PLAYER_HOMING_DAMAGE = 18;          // 追踪子弹伤害

// ============================================================
// 追踪子弹参数
// ============================================================
constexpr float HOMING_TURN_RATE = 0.03f;         // 追踪转向率（每帧最大转向角度）

// ============================================================
// Boss攻击模式 - 花朵爆发
// ============================================================
constexpr float FLOWER_INTERVAL = 0.8f;          // 射击间隔
constexpr float FLOWER_DURATION = 4.0f;          // 持续时间
constexpr int FLOWER_BURST_COUNT = 24;            // 每层子弹数
constexpr int FLOWER_LAYERS = 3;                  // 层数（多层叠加如花朵）

// ============================================================
// Boss攻击模式 - 八方向射击
// ============================================================
constexpr float EIGHT_DIR_INTERVAL = 0.4f;        // 射击间隔
constexpr float EIGHT_DIR_DURATION = 4.0f;        // 持续时间

// ============================================================
// Boss攻击模式 - 蛇形波浪
// ============================================================
constexpr float SNAKE_INTERVAL = 0.1f;            // 射击间隔
constexpr float SNAKE_DURATION = 5.0f;            // 持续时间
constexpr float SNAKE_SPEED = 200.f;             // 蛇形子弹飞行速度
constexpr float SNAKE_AMPLITUDE = 80.f;           // 蛇形振幅（左右摆动范围）
constexpr float SNAKE_FREQUENCY = 3.f;            // 蛇形频率

// ============================================================
// Boss攻击模式 - 追踪球
// ============================================================
constexpr float HOMING_ORB_INTERVAL = 0.8f;      // 发射间隔
constexpr float HOMING_ORB_DURATION = 4.0f;        // 持续时间
constexpr float HOMING_ORB_SPEED = 120.f;        // 追踪球速度（较慢但持续追踪）

// ============================================================
// Boss攻击模式 - 双螺旋
// ============================================================
constexpr float DOUBLE_SPIRAL_INTERVAL = 0.05f;   // 射击间隔（很密）
constexpr float DOUBLE_SPIRAL_DURATION = 5.0f;    // 持续时间

// ============================================================
// Boss攻击模式 - 刀刃之环
// ============================================================
constexpr float BLADE_RING_INTERVAL = 0.15f;      // 射击间隔
constexpr float BLADE_RING_DURATION = 4.0f;        // 持续时间
constexpr float BLADE_RING_SPEED = 250.f;        // 刀刃飞行速度

// ============================================================
// Boss攻击模式 - 屏幕横扫
// ============================================================
constexpr float SCREEN_WIPE_INTERVAL = 0.08f;    // 射击间隔
constexpr float SCREEN_WIPE_DURATION = 3.0f;      // 持续时间
constexpr float SCREEN_WIPE_SPEED = 400.f;        // 横扫速度

// ============================================================
// Boss攻击模式 - 径向爆发
// ============================================================
constexpr float RADIAL_BURST_INTERVAL = 0.3f;     // 射击间隔
constexpr float RADIAL_BURST_DURATION = 4.0f;      // 持续时间

// ============================================================
// Boss攻击模式 - 随机喷射
// ============================================================
constexpr float RANDOM_SPRAY_INTERVAL = 0.05f;   // 射击间隔
constexpr float RANDOM_SPRAY_DURATION = 3.0f;     // 持续时间

// ============================================================
// Boss攻击模式 - 预测射击
// ============================================================
constexpr float PREDICTIVE_INTERVAL = 1.5f;       // 射击间隔
constexpr float PREDICTIVE_DURATION = 4.0f;        // 持续时间
constexpr float PREDICTIVE_SHOT_SPEED = 600.f;    // 预测子弹速度

// ============================================================
// Boss攻击模式 - 蓄力光束
// ============================================================
constexpr float CHARGE_BEAM_WARNING = 1.5f;      // 预警时间（蓄力阶段）
constexpr float CHARGE_BEAM_DURATION = 1.0f;       // 实际伤害时间
constexpr int CHARGE_BEAM_DAMAGE = 50;            // 光束伤害值

// ============================================================
// Boss攻击模式 - 触须横扫
// ============================================================
constexpr float TENTACLE_SWEEP_DURATION = 2.0f;  // 横扫持续时间

// ============================================================
// Boss攻击模式 - 屏幕锁定
// ============================================================
constexpr float SCREEN_LOCK_DURATION = 3.0f;    // 锁定持续时间

// ============================================================
// Boss攻击模式 - 全屏弹幕雨
// ============================================================
constexpr float BULLET_RAIN_FULL_INTERVAL = 0.03f; // 射击间隔（非常密集）
constexpr float BULLET_RAIN_FULL_DURATION = 3.0f;   // 持续时间

// ============================================================
// Boss召唤小怪
// ============================================================
constexpr float MINION_SPAWN_INTERVAL = 2.0f;     // 小怪生成间隔
constexpr int MINION_DAMAGE = 20;                 // 小怪碰撞伤害
constexpr float PHASE_TRANSITION_DURATION = 2.0f; // 阶段转换持续时间（无敌+回血）
constexpr int MAX_MINIONS = 5;                    // 最大小怪数量

// ============================================================
// 玩家特殊形态 - 相位偏移
// ============================================================
constexpr float PHASE_SHIFT_DURATION = 1.0f;    // 无敌持续时间
constexpr float PHASE_SHIFT_TRIGGER_CHANCE = 0.35f; // 闪避成功后触发概率

// ============================================================
// 玩家特殊形态 - Nova形态
// ============================================================
constexpr float NOVA_FORM_DURATION = 0.5f;        // 持续时间（很短但大范围）
constexpr int NOVA_FORM_DAMAGE = 100;              // 全屏伤害值
constexpr float NOVA_FORM_RADIUS = 500.f;         // 伤害范围半径

// ============================================================
// 速度线圈道具
// ============================================================
constexpr float SPEED_COIL_DURATION = 8.0f;      // 持续时间
constexpr float SPEED_COIL_MULT = 1.5f;          // 移速倍率

// ============================================================
// 攻击模块道具
// ============================================================
constexpr float ATTACK_MODULE_DURATION = 10.0f;  // 持续时间
constexpr float ATTACK_MODULE_MULT = 1.3f;        // 伤害倍率

// ============================================================
// 玩家武器冷却时间
// ============================================================
constexpr float PLAYER_SPREAD_COOLDOWN = 0.3f;    // 散射武器冷却
constexpr float PLAYER_PIERCING_COOLDOWN = 0.5f;  // 穿透武器冷却
constexpr float PLAYER_ORBITAL_COOLDOWN = 0.1f;   // 环绕武器冷却（非常短）
constexpr float PLAYER_CLUSTER_COOLDOWN = 0.4f;    // 集群武器冷却
constexpr float PLAYER_HOMING_COOLDOWN = 0.6f;    // 追踪武器冷却
constexpr int PLAYER_SPREAD_COUNT = 5;             // 散射子弹数量

// ============================================================
// 集群子弹分裂
// ============================================================
constexpr float PLAYER_CLUSTER_BURST_COUNT = 8;   // 母弹分裂成子弹数

// ============================================================
// 武器特殊效果参数
// ============================================================
constexpr int COMBO_THRESHOLD = 3;              // 连击阈值（3次以上触发加成）
constexpr float COMBO_DAMAGE_MULT = 1.5f;        // 连击伤害倍率（×1.5）
constexpr float COMBO_TIMEOUT = 2.f;             // 连击超时时间（秒）
constexpr float SPREAD_BOUNCE_DAMAGE_MULT = 0.7f; // 散射子弹反弹后伤害倍率
constexpr float PIERCE_CHAIN_DAMAGE_RATIO = 0.5f; // 穿透连锁伤害比例
constexpr float PIERCE_CHAIN_RADIUS = 80.f;       // 连锁范围半径
constexpr int ORBITAL_MAX_DAMAGE = 60;           // 环绕子弹累计伤害上限
constexpr int CLUSTER_SPLIT_COUNT = 2;           // 集群子弹每次分裂数
constexpr float CLUSTER_SPLIT_DAMAGE_RATIO = 0.3f; // 分裂子弹伤害比例

// ============================================================
// 追踪子弹减速效果
// ============================================================
constexpr float HOMING_SLOW_STACK = 0.1f;        // 每层减速百分比（10%）
constexpr float HOMING_SLOW_DURATION = 2.f;       // 减速持续时间
constexpr int HOMING_SLOW_MAX_STACKS = 3;        // 最大减速层数（3层=30%减速）

// ============================================================
// 难度枚举
// ============================================================
// 影响Boss数量和游戏挑战性
enum class Difficulty {
    Normal,   // 普通：1个Boss
    Hard,     // 困难：2个Boss
    Lunatic   // 疯狂：3个Boss（第二波在第一个Boss血量50%时触发）
};

// ============================================================
// 难度配置
// ============================================================
constexpr int HARD_BOSS_COUNT = 2;               // Hard难度Boss数量
constexpr int LUNATIC_BOSS_COUNT = 3;             // Lunatic难度Boss数量
// Lunatic难度下，当Boss A血量低于此比例时触发Boss B、C入场
constexpr float BOSS_SPAWN_HP_THRESHOLD = 0.5f; // 50%

// ============================================================
// Boss名称颜色（用于HUD显示）
// ============================================================
// 3种Boss类型对应3种颜色主题
const sf::Color BOSS_NAME_COLORS[3] = {
    sf::Color(255, 80, 40),    // Inferno（烈焰恶魔）- 红橙色调
    sf::Color(180, 80, 255),   // Void（虚空幽灵）- 紫蓝色调
    sf::Color(255, 220, 60)    // Thunder（雷霆泰坦）- 金黄色调
};

// ============================================================
// Boss生成位置随机范围
// ============================================================
// 确保Boss不会生成在屏幕边缘或太中间的位置
constexpr float BOSS_SPAWN_MIN_X = 200.f;         // X轴最小位置
constexpr float BOSS_SPAWN_MAX_X = WINDOW_WIDTH - 200.f; // X轴最大位置
constexpr float BOSS_SPAWN_MIN_Y = 100.f;         // Y轴最小位置（屏幕上方）
constexpr float BOSS_SPAWN_MAX_Y = 250.f;         // Y轴最大位置