// ============================================================
// Game.h - 游戏主控制器声明
// ============================================================
// 职责：游戏主循环、控制所有子系统的协调、碰撞检测、游戏状态切换
// 游戏循环：processEvents() → update(dt) → render()
// ============================================================

#pragma once

// SFML图形库，提供了窗口、渲染、字体等基础功能
#include <SFML/Graphics.hpp>

// 游戏统计数据结构（包含玩家HP、Boss信息、难度等，用于HUD显示）
#include "GameStats.h"

// 玩家类（包含移动、射击、技能等逻辑）
#include "entities/Player.h"

// Boss管理器（管理多个Boss的初始化、更新、状态查询）
#include "entities/BossManager.h"

// 道具类（包含道具的类型、位置、生成等逻辑）
#include "entities/Item.h"

// 背景系统（星空/网格动画）
#include "systems/BackgroundSystem.h"

// 粒子系统（爆炸、闪电、治疗特效等）
#include "systems/ParticleSystem.h"

// 相机震动系统（屏幕抖动效果）
#include "systems/CameraShake.h"

// 音频系统（背景音乐、游戏音效的播放控制）
#include "systems/AudioSystem.h"

// HUD（抬头显示：玩家血条、Boss血条、道具图标等）
#include "ui/HUD.h"

// 菜单系统（主菜单、暂停菜单、胜利/失败菜单）
#include "ui/Menu.h"

// ============================================================
// 游戏状态枚举
// ============================================================
// 游戏在任意时刻处于以下状态之一，状态决定渲染和更新的行为
enum class GameState {
    MainMenu,  // 主菜单（显示难度选择按钮）
    Playing,   // 游戏中（玩家控制、Boss攻击、碰撞检测）
    Paused,    // 暂停（游戏暂停，ESC解除）
    Victory,   // 胜利（所有Boss死亡，显示胜利菜单）
    GameOver   // 失败（玩家死亡，显示失败菜单）
};

// ============================================================
// Game类声明
// ============================================================
// Game是整个游戏的核心，协调所有子系统：
// 窗口、玩家、Boss、道具、背景、粒子、音频、HUD、菜单
class Game {
public:
    // 构造函数：初始化所有子系统，创建窗口，加载字体，显示主菜单
    Game();

    // 启动游戏主循环（内部调用run()后，程序持续运行直到窗口关闭）
    void run();

private:
    // ---- 核心游戏循环方法 ----
    // 处理所有SFML事件（键盘、鼠标、窗口关闭）
    void processEvents();
    // 更新游戏逻辑（每帧调用，dt为帧间隔时间）
    void update(float dt);
    // 渲染游戏画面（每帧调用，按层级顺序绘制所有元素）
    void render();

    // 重置游戏到初始状态（回到主菜单或重新开始时调用）
    void resetGame();

    // 开始游戏（从主菜单点击开始后调用，初始化Boss管理器）
    void startGame();

    // ============================================================
    // 成员变量
    // ============================================================

    // ---- 窗口与字体 ----
    sf::RenderWindow window;  // SFML渲染窗口，所有游戏内容都画在这个窗口里
    sf::Font font;            // 全局字体（用于HUD、菜单的文字显示）

    // ---- 游戏状态 ----
    GameState state = GameState::MainMenu;  // 当前游戏状态（初始为主菜单）
    GameStats gameStats;                    // 游戏统计数据（实时更新，供HUD读取）

    // ---- 游戏实体 ----
    Player player;                          // 玩家对象（单例，贯穿整个游戏）
    BossManager bossManager;                 // Boss管理器（管理最多3个Boss）
    std::vector<Item> items;                // 道具数组（最多ITEM_MAX_COUNT=2个道具）

    // ---- 计时器 ----
    float itemSpawnTimer;                   // 道具生成计时器（每10秒生成一个）
    float bulletTimeTimer;                  // 子弹时间效果计时器（>0.f时子弹时间激活）

    // ---- 补血道具触发标志 ----
    // 防止在同一个血量阈值重复生成补血道具
    bool healSpawnedAt75;                   // Boss血量≤75%时已生成补血
    bool healSpawnedAt50;                   // Boss血量≤50%时已生成补血
    bool healSpawnedAt25;                   // Boss血量≤25%时已生成补血

    // ---- 时钟 ----
    sf::Clock clock;                        // 游戏时钟（用于计算delta time，帧率无关的运动）

    // ---- 运行状态 ----
    bool running;                           // 游戏是否继续运行（false时退出游戏循环）

    // ---- 调试模式 ----
    bool godModeActive = false;             // 无敌模式开关（按F3开启，测试用）

    // ============================================================
    // 子系统（各负责一个独立的功能模块）
    // ============================================================
    BackgroundSystem background;            // 背景系统（星空、网格的渲染和动画）
    ParticleSystem particles;               // 粒子系统（爆炸、火花、治疗特效）
    CameraShake cameraShake;                // 相机震动（屏幕抖动的衰减控制）
    AudioSystem audio;                     // 音频系统（背景音乐、游戏音效）
    HUD hud;                               // HUD（抬头显示：血条、技能冷却、分数等）
    Menu menu;                             // 菜单系统（主菜单、暂停、胜利、失败的菜单UI）

    // ============================================================
    // 状态追踪（用于触发特效）
    // ============================================================
    int prevBossPhase = 1;                 // 上一帧的Boss阶段（用于检测阶段切换）
    int prevPlayerHP = 0;                  // 上一帧的玩家HP（用于检测玩家掉血）

    // ============================================================
    // 难度设置
    // ============================================================
    Difficulty currentDifficulty = Difficulty::Normal;  // 当前难度（Normal/Hard/Lunatic）

    // ============================================================
    // 屏幕闪烁特效
    // ============================================================
    float screenFlashTimer = 0.f;          // 闪烁倒计时（>0时显示屏幕闪烁）
    sf::Color screenFlashColor = sf::Color::White;  // 闪烁颜色（绿/蓝/橙/紫/白等）

    // ============================================================
    // Nova扩散圆环特效（NovaBomb/NovaCore使用）
    // ============================================================
    float novaRingRadius = 0.f;           // 当前圆环半径（每帧扩大）
    float novaRingMaxRadius = 0.f;         // 最大半径（屏幕对角线长度，圆环到达后结束）
    bool novaRingActive = false;          // 是否处于Nova圆环激活状态
    sf::Color novaRingColor = sf::Color::White;  // 圆环颜色（金黄/橙红）
};