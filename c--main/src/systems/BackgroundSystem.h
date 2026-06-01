// ============================================================
// BackgroundSystem.h - 背景系统声明
// ============================================================
// 职责：渲染游戏的背景（纯色背景 + 透视网格 + 漂浮环境点）
// 背景是静态的，但环境点会缓慢飘动和脉动
// 透视网格模拟3D地面效果，增强空间感
// ============================================================

#pragma once

// SFML图形库（用于sf::RectangleShape, sf::VertexArray, sf::CircleShape等）
#include <SFML/Graphics.hpp>

// 标准向量（存储环境点数组）
#include <vector>

// ============================================================
// BackgroundSystem类
// ============================================================
// 游戏背景渲染器，由Game.cpp每帧调用render方法
// 构成：
// 1. 纯色背景：深蓝色(8, 8, 24)
// 2. 透视网格：18条水平线 + 24条垂直线，汇聚于消失点
// 3. 漂浮环境点：BG_AMBIENT_DOT_COUNT个圆点，随时间飘动和脉动
class BackgroundSystem {
public:
    // 构造函数：初始化背景矩形、网格线、环境点
    BackgroundSystem();

    // ---- 每帧更新 ----
    // dt: 时间差（秒）
    // 更新所有环境点的位置（飘动）和透明度（脉动）
    void update(float dt);

    // ---- 渲染 ----
    // 按顺序绘制：背景 → 网格线 → 环境点
    void render(sf::RenderWindow& window);

private:
    // ---- 背景矩形 ----
    // 覆盖整个窗口的纯色矩形，深蓝色
    sf::RectangleShape bgRect;

    // ---- 透视网格 ----
    // 使用sf::VertexArray绘制多条线段
    // 水平线18条，垂直线24条，共36条线（72个顶点）
    sf::VertexArray gridLines;

    // ---- 环境漂浮点 ----
    // 若干小圆点，随机颜色，在屏幕上缓慢飘动
    std::vector<sf::CircleShape> ambientDots;       // 圆点形状数组
    std::vector<float> dotPhases;                   // 每个点的相位偏移（用于错开脉动）
    std::vector<sf::Vector2f> dotBasePositions;    // 每个点的初始位置（用于计算飘动）

    // ---- 时间相关 ----
    float timer;  // 总运行时间（秒），用于计算环境点位置
};