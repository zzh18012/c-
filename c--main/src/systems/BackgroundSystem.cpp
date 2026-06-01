// ============================================================
// BackgroundSystem.cpp - 背景系统实现
// ============================================================
// 职责：渲染游戏的背景（纯色背景 + 透视网格 + 漂浮环境点）
// 透视网格模拟3D地面效果，消失点位于屏幕顶部中央
// 环境点随时间向上飘动并有脉动效果
// ============================================================

#include "BackgroundSystem.h"
#include "core/Config.h"
#include <cstdlib>  // rand(), RAND_MAX
#include <cmath>   // sinf, fmodf
#include <cstdint> // uint8_t

// ============================================================
// 构造函数
// ============================================================
// 初始化所有背景元素：
// 1. 深蓝色背景矩形
// 2. 18条水平透视网格线（近密远疏）
// 3. 24条垂直透视网格线（汇聚于消失点）
// 4. BG_AMBIENT_DOT_COUNT个随机环境点
BackgroundSystem::BackgroundSystem()
    : timer(0.f)
{
    // ---- 深蓝色背景矩形 ----
    bgRect.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    bgRect.setFillColor(sf::Color(8, 8, 24)); // 深蓝色，几乎是黑色

    // ============================================================
    // 透视网格初始化
    // ============================================================
    // 使用sf::VertexArray绘制线段
    // sf::PrimitiveType::Lines：每2个顶点构成一条线段
    const int hLines = 18;  // 水平线数量
    const int vLines = 24; // 垂直线数量
    const int totalVertices = (hLines + vLines) * 2; // 总顶点数（每线2个顶点）
    gridLines.setPrimitiveType(sf::PrimitiveType::Lines);
    gridLines.resize(totalVertices);

    // 网格颜色：蓝灰色，半透明
    sf::Color gridColor(50, 160, 240, static_cast<std::uint8_t>(BG_GRID_ALPHA));
    // 地平线Y坐标：屏幕顶部10%处
    float horizonY = WINDOW_HEIGHT * 0.1f;

    // ============================================================
    // 水平线（近密远疏的透视效果）
    // ============================================================
    // 使用立方插值使线条间距随接近地平线而减小
    for (int i = 0; i < hLines; ++i) {
        // t从0到1表示从地平线到屏幕底部
        float t = static_cast<float>(i) / (hLines - 1);
        // y = horizonY + (屏幕高度 - horizonY) × t³
        // t³使接近地平线的线条更密集（透视效果）
        float y = horizonY + (WINDOW_HEIGHT - horizonY) * (t * t * t);

        // 靠近地平线的线条更透明（0.3 + 0.7*t）
        sf::Color lineColor = gridColor;
        lineColor.a = static_cast<std::uint8_t>(gridColor.a * (0.3f + 0.7f * t));

        // 两个顶点：屏幕左侧(0,y)到屏幕右侧(WINDOW_WIDTH,y)
        gridLines[i * 2].position = sf::Vector2f(0.f, y);
        gridLines[i * 2].color = lineColor;
        gridLines[i * 2 + 1].position = sf::Vector2f(WINDOW_WIDTH, y);
        gridLines[i * 2 + 1].color = lineColor;
    }

    // ============================================================
    // 垂直线（汇聚于消失点）
    // ============================================================
    // 消失点位于屏幕顶部中央
    float vanishX = WINDOW_WIDTH * 0.5f;
    float vanishY = horizonY;
    int vOffset = hLines * 2; // 垂直线在vertexArray中的起始索引

    for (int i = 0; i < vLines; ++i) {
        // t从0到1表示从左到右
        float t = static_cast<float>(i) / (vLines - 1);

        // 顶部X坐标：在消失点附近小范围偏移
        float topX = vanishX + (t - 0.5f) * WINDOW_WIDTH * 0.08f;
        // 底部X坐标：屏幕底部时展开到更大范围
        float botX = vanishX + (t - 0.5f) * WINDOW_WIDTH * 1.3f;

        sf::Color lineColor = gridColor;
        lineColor.a = static_cast<std::uint8_t>(gridColor.a * 0.6f); // 60%透明度

        // 从消失点(topX, vanishY)到底部(botX, WINDOW_HEIGHT)
        gridLines[vOffset + i * 2].position = sf::Vector2f(topX, vanishY);
        gridLines[vOffset + i * 2].color = lineColor;
        gridLines[vOffset + i * 2 + 1].position = sf::Vector2f(botX, WINDOW_HEIGHT);
        gridLines[vOffset + i * 2 + 1].color = lineColor;
    }

    // ============================================================
    // 环境漂浮点初始化
    // ============================================================
    ambientDots.resize(BG_AMBIENT_DOT_COUNT);
    dotPhases.resize(BG_AMBIENT_DOT_COUNT);
    dotBasePositions.resize(BG_AMBIENT_DOT_COUNT);

    // 预定义颜色数组：青色、粉红、绿色、金色、紫色
    sf::Color dotColors[] = {
        sf::Color(0, 220, 255),     // 青色
        sf::Color(255, 80, 180),    // 粉红
        sf::Color(80, 255, 120),    // 绿色
        sf::Color(255, 220, 60),    // 金色
        sf::Color(160, 100, 255),   // 紫色
    };

    for (int i = 0; i < BG_AMBIENT_DOT_COUNT; ++i) {
        // 随机半径：BG_DOT_MIN_RADIUS ~ BG_DOT_MAX_RADIUS
        float radius = BG_DOT_MIN_RADIUS + static_cast<float>(rand()) / RAND_MAX * (BG_DOT_MAX_RADIUS - BG_DOT_MIN_RADIUS);
        ambientDots[i].setRadius(radius);
        ambientDots[i].setOrigin(sf::Vector2f(radius, radius)); // 以圆心为基准点

        // 随机颜色（从预定义数组中选）
        ambientDots[i].setFillColor(dotColors[rand() % 5]);

        // 随机初始位置
        float x = static_cast<float>(rand() % WINDOW_WIDTH);
        float y = static_cast<float>(rand() % WINDOW_HEIGHT);
        dotBasePositions[i] = sf::Vector2f(x, y);
        ambientDots[i].setPosition(sf::Vector2f(x, y));

        // 随机相位（使每个点的脉动和飘动错开）
        dotPhases[i] = static_cast<float>(rand()) / RAND_MAX * 6.28318f; // 0 ~ 2π
    }
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 更新内容：
// 1. timer累加dt
// 2. 每个环境点：向上飘动 + 水平正弦摆动 + 透明度脉动
void BackgroundSystem::update(float dt) {
    timer += dt; // 累计时间

    for (int i = 0; i < BG_AMBIENT_DOT_COUNT; ++i) {
        // ---- 向上飘动 ----
        // newY = baseY - speed * time，然后取模WINDOW_HEIGHT实现循环
        float newY = dotBasePositions[i].y - timer * BG_DOT_DRIFT_SPEED;
        newY = fmodf(newY + WINDOW_HEIGHT, WINDOW_HEIGHT); // 确保在[0, WINDOW_HEIGHT)范围内

        // ---- 水平正弦摆动 ----
        // sin(time * 0.3 + phase)产生-1到1之间的周期性摆动
        float newX = dotBasePositions[i].x + std::sin(timer * 0.3f + dotPhases[i]) * 20.f;

        // 更新位置
        ambientDots[i].setPosition(sf::Vector2f(newX, newY));

        // ---- 透明度脉动 ----
        // sin(time * speed + phase)产生-1到1，再映射到[0,1]
        float pulse = (std::sin(timer * BG_DOT_PULSE_SPEED + dotPhases[i]) + 1.f) * 0.5f;
        // 透明度范围：80 + pulse * 40 = 80 ~ 120
        sf::Color c = ambientDots[i].getFillColor();
        c.a = static_cast<std::uint8_t>(80 + pulse * 120);
        ambientDots[i].setFillColor(c);
    }
}

// ============================================================
// render - 渲染背景
// ============================================================
// 按顺序绘制：
// 1. 纯色背景矩形（最底层）
// 2. 透视网格线（覆盖在背景上）
// 3. 环境漂浮点（最顶层）
void BackgroundSystem::render(sf::RenderWindow& window) {
    window.draw(bgRect);     // 绘制纯色背景
    window.draw(gridLines); // 绘制透视网格
    for (const auto& dot : ambientDots) {
        window.draw(dot);   // 绘制每个环境点
    }
}