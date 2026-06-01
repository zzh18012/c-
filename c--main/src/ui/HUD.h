// ============================================================
// HUD.h - 游戏抬头显示器声明
// ============================================================
// 职责：渲染游戏中所有HUD元素（玩家HP、Boss HP、攻击名称、道具状态等）
// HUD从GameStats结构读取只读数据，不修改游戏状态
// 支持多Boss显示（最多3个），平滑HP过渡动画
// ============================================================

#pragma once

// SFML图形库（用于sf::RectangleShape, sf::Text, sf::RenderWindow等）
#include <SFML/Graphics.hpp>

// 游戏统计数据（从Game.cpp读取，只读）
#include "core/GameStats.h"

// 标准字符串（用于字符串操作）
#include <string>

// 可选类型（用于sf::Text的延迟构造）
#include <optional>

// 标准数组（用于多Boss状态）
#include <array>

// 标准向量（用于Boss血条数组）
#include <vector>

// ============================================================
// HUD类
// ============================================================
// 游戏抬头显示器，负责渲染：
// 1. 玩家HP血条（左上角）
// 2. Boss HP血条（右上角，最多3个，垂直堆叠）
// 3. 当前攻击名称（顶部中央）
// 4. 道具状态图标和计时器（左下角）
// 5. 冲刺冷却指示器（底部中央）
// 6. 游戏时间（右下角）
// 7. 胜利/失败消息（屏幕中央，淡入效果）
//
// 设计：Game.cpp每帧创建GameStats并传递给HUD，HUD只读不写
class HUD {
public:
    // 构造函数：初始化所有渲染元素的默认属性
    HUD();
    // 析构函数：默认实现
    ~HUD();

    // ---- 初始化 ----
    // font: 游戏字体
    // 创建所有sf::Text对象，设置初始位置和颜色
    void init(const sf::Font& font);

    // ---- 每帧更新 ----
    // stats: 游戏统计数据（从Game.cpp传入）
    // 更新所有HUD元素的显示内容
    // 包括：HP条平滑过渡、攻击名称变化、道具计时器、游戏时间等
    void update(const GameStats& stats);

    // ---- 渲染 ----
    // 渲染所有HUD元素到窗口
    void render(sf::RenderWindow& window);

private:
    // ---- 格式化工具 ----
    // seconds: 秒数
    // 返回格式如 "3.5s" 的字符串
    static std::string fmtTime(float seconds);

    // ============================================================
    // 玩家HP血条
    // ============================================================
    sf::RectangleShape playerBarBg;     // 玩家血条背景（深色底框）
    sf::RectangleShape playerBarFill;    // 玩家血条填充（渐变色）
    std::optional<sf::Text> playerHPText; // 玩家HP文字 "HP 85/100"

    // ============================================================
    // Boss HP血条（最多3个，垂直堆叠）
    // ============================================================
    static constexpr int MAX_BOSS_BARS = 3; // 最大Boss血条数量
    std::vector<sf::RectangleShape> bossBarBgs;      // Boss血条背景数组
    std::vector<sf::RectangleShape> bossBarFills;     // Boss血条填充数组
    std::array<std::optional<sf::Text>, MAX_BOSS_BARS> bossHPTexts;    // Boss HP文字数组
    std::array<std::optional<sf::Text>, MAX_BOSS_BARS> bossPhaseTexts; // Boss阶段文字数组 "P1/P2/P3"
    std::array<std::optional<sf::Text>, MAX_BOSS_BARS> bossNameTexts;  // Boss名称文字数组

    // ============================================================
    // 阶段标记（单Boss时显示）
    // ============================================================
    // 70%和35%HP位置处的标记线，提示即将进入下个阶段
    sf::RectangleShape phaseMarker70;  // 70%阶段标记（黄色）
    sf::RectangleShape phaseMarker35;  // 35%阶段标记（红色）

    // ============================================================
    // 道具状态图标
    // ============================================================
    // 3个图标：护盾（蓝色）、超载（橙色）、冲刺电池（青色）
    sf::RectangleShape shieldIcon;
    sf::RectangleShape overdriveIcon;
    sf::RectangleShape dashBatteryIcon;

    // ============================================================
    // 冲刺冷却指示器
    // ============================================================
    // 圆形指示器，显示冲刺技能冷却进度
    sf::CircleShape dashCooldownBg;   // 背景圆
    sf::CircleShape dashCooldownRing; // 进度环（用多边形模拟圆弧）

    // ============================================================
    // 文字元素
    // ============================================================
    std::optional<sf::Text> attackNameText;      // 当前攻击名称（屏幕中央顶部）
    std::optional<sf::Text> shieldText;         // 护盾计时器
    std::optional<sf::Text> overdriveText;      // 超载计时器
    std::optional<sf::Text> dashBatteryText;    // 冲刺电池计时器
    std::optional<sf::Text> bulletTimeText;      // 子弹时间提示
    std::optional<sf::Text> timerText;           // 游戏时间 MM:SS
    std::optional<sf::Text> endMessageText;      // 胜利/失败消息

    // ============================================================
    // 状态变量
    // ============================================================
    float endMessageAlpha = 0.f;          // 结束消息透明度（淡入效果）
    float attackFlashTimer = 0.f;        // 攻击名称闪烁计时器
    std::string lastAttackName;           // 上次的攻击名称（检测变化）

    // ============================================================
    // 平滑过渡
    // ============================================================
    // HP值不是立即跳变，而是平滑插值过渡
    float displayedPlayerHP = 100.f;     // 显示的玩家HP（平滑值）
    std::array<float, MAX_BOSS_BARS> displayedBossHPs; // 显示的BossHP数组

    // ============================================================
    // 游戏难度
    // ============================================================
    Difficulty currentDifficulty = Difficulty::Normal;

    // ============================================================
    // 当前活跃Boss数量
    // ============================================================
    // 用于render中决定渲染多少个Boss血条
    int activeBossCount = 0;

    // ============================================================
    // 字体指针
    // ============================================================
    // 指向init中设置的字体，避免每次创建Text时重复传递
    const sf::Font* fontPtr = nullptr;
};