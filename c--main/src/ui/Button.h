// ============================================================
// Button.h - 按钮类声明
// ============================================================
// 职责：游戏菜单中的按钮（点击触发回调）
// 支持鼠标悬停效果（颜色/边框变化）
// 使用std::function<void()>实现灵活的回调机制
// ============================================================

#pragma once

// SFML图形库（用于sf::RectangleShape, sf::Text, sf::RenderWindow等）
#include <SFML/Graphics.hpp>

// 标准函数（用于std::function<void()>回调类型）
#include <functional>

// 标准字符串
#include <string>

// 可选类型（用于sf::Text的延迟构造）
#include <optional>

// ============================================================
// Button类
// ============================================================
// 游戏菜单中的按钮组件
// 功能：
// 1. 渲染按钮背景和标签文字
// 2. 检测鼠标悬停和点击
// 3. 点击时触发回调函数
// 设计：使用std::function<void()>存储任意可调用对象
class Button {
public:
    // ---- 构造函数 ----
    // 默认构造函数：使用默认颜色（青色）
    Button();
    // 带参数的构造函数：一步完成所有初始化
    // text: 按钮上显示的文字
    // font: 字体
    // position: 按钮左上角位置
    // size: 按钮尺寸
    // callback: 点击按钮时触发的回调函数
    Button(const std::string& text, const sf::Font& font, sf::Vector2f position,
           sf::Vector2f size, std::function<void()> callback);

    // ---- 事件处理 ----
    // 处理鼠标移动和点击事件
    // event: SFML事件（鼠标移动、按钮按下）
    // window: 当前窗口（用于获取鼠标位置）
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    // ---- 手动激活 ----
    // 直接触发按钮的回调函数（不依赖鼠标事件）
    void activate();

    // ---- 选中状态 ----
    // sel: 是否被选中（选中状态改变边框样式）
    void setSelected(bool sel);

    // ---- 状态查询 ----
    // point: 测试点是否在按钮范围内
    bool contains(sf::Vector2f point) const;

    // ---- 每帧更新 ----
    // dt: 时间差（秒）
    // 根据hovered和selected状态更新按钮边框样式
    void update(float dt);

    // ---- 渲染 ----
    // 绘制按钮背景和标签文字
    void render(sf::RenderWindow& window) const;

private:
    // ---- 渲染元素 ----
    sf::RectangleShape background;  // 按钮背景矩形
    std::optional<sf::Text> label;   // 标签文字（可选，延迟构造）

    // ---- 回调 ----
    // 点击按钮时触发的函数（可以是任意可调用对象）
    std::function<void()> onClick;

    // ---- 状态 ----
    bool hovered = false;  // 鼠标是否悬停在按钮上
    bool selected = false; // 按钮是否被选中（用于键盘导航）

    // ---- 颜色配置 ----
    // normalColor: 正常/未hover状态下的边框颜色
    // hoverColor: hover或selected状态下的边框颜色
    sf::Color normalColor;
    sf::Color hoverColor;
};