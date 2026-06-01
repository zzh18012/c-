// ============================================================
// Button.cpp - 按钮类实现
// ============================================================
// 职责：游戏菜单中的按钮（点击触发回调）
// 支持鼠标悬停效果（颜色/边框变化）
// 使用std::function<void()>实现灵活的回调机制
// ============================================================

#include "Button.h"

// ============================================================
// 默认构造函数
// ============================================================
// 使用默认颜色：青色
Button::Button()
    : normalColor(0, 200, 255, 200)
    , hoverColor(180, 240, 255, 255)
{
}

// ============================================================
// 带参数的构造函数
// ============================================================
// 一步完成所有初始化：位置、尺寸、背景、标签、回调
Button::Button(const std::string& text, const sf::Font& font, sf::Vector2f position,
               sf::Vector2f size, std::function<void()> callback)
    : onClick(callback)
    , normalColor(0, 200, 255, 180)
    , hoverColor(150, 230, 255, 255)
{
    // ---- 设置背景矩形 ----
    background.setPosition(position);              // 位置
    background.setSize(size);                       // 尺寸
    background.setFillColor(sf::Color(20, 20, 40, 200)); // 深蓝色半透明填充
    background.setOutlineThickness(2.f);            // 边框厚度
    background.setOutlineColor(normalColor);        // 边框颜色（青色）

    // ---- 创建标签文字 ----
    // 使用emplace延迟构造，避免未使用的Text占用内存
    label.emplace(font, text, 22);                  // 字体、大小22
    label->setFillColor(sf::Color::White);        // 白色文字

    // ---- 设置文字居中 ----
    // 计算文字边界，设置origin到中心点
    sf::FloatRect textBounds = label->getLocalBounds();
    label->setOrigin(sf::Vector2f(textBounds.position.x + textBounds.size.x / 2.f,
                    textBounds.position.y + textBounds.size.y / 2.f));
    // 设置文字位置到按钮中心
    label->setPosition(sf::Vector2f(position.x + size.x / 2.f, position.y + size.y / 2.f));
}

// ============================================================
// handleEvent - 处理鼠标事件
// ============================================================
// event: SFML事件
// window: 当前窗口
// 处理两种事件：
// 1. MouseMoved: 更新hovered状态
// 2. MouseButtonPressed（左键）: 检测点击并触发回调
void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    // 将鼠标位置转换为窗口坐标（考虑视图变换）
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition());

    // ---- 鼠标移动事件 ----
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseMoved>()) {
        (void)mouseEvent; // 消除未使用警告
        hovered = contains(mousePos); // 检测鼠标是否在按钮内
    }

    // ---- 鼠标按钮按下事件 ----
    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        // 左键 + 在按钮内 + 有回调 → 触发回调
        if (mouseEvent->button == sf::Mouse::Button::Left && contains(mousePos) && onClick) {
            onClick(); // 调用存储的回调函数
        }
    }
}

// ============================================================
// activate - 手动激活
// ============================================================
// 直接触发按钮的回调函数（不依赖鼠标事件）
// 用于键盘导航或程序化触发
void Button::activate() {
    if (onClick) onClick(); // 如果有回调则调用
}

// ============================================================
// setSelected - 设置选中状态
// ============================================================
// sel: 是否被选中
// 选中状态会影响update中的边框样式
void Button::setSelected(bool sel) {
    selected = sel;
}

// ============================================================
// contains - 检测点是否在按钮内
// ============================================================
// point: 测试点（窗口坐标）
// 使用sf::RectangleShape的getGlobalBounds().contains检测
bool Button::contains(sf::Vector2f point) const {
    return background.getGlobalBounds().contains(point);
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 根据hovered和selected状态更新边框样式
// hover或selected时：边框变亮(hoverColor)、变粗(3px)
// 正常状态：边框为normalColor、正常粗细(2px)
void Button::update(float dt) {
    (void)dt; // 消除未使用警告

    if (hovered || selected) {
        background.setOutlineColor(hoverColor);    // 边框变亮
        background.setOutlineThickness(3.f);        // 边框变粗
    } else {
        background.setOutlineColor(normalColor);    // 边框恢复正常
        background.setOutlineThickness(2.f);        // 边框恢复正常粗细
    }
}

// ============================================================
// render - 渲染按钮
// ============================================================
// 绘制顺序：背景 → 标签文字
void Button::render(sf::RenderWindow& window) const {
    window.draw(background); // 绘制背景矩形
    if (label) {             // 如果有标签则绘制
        window.draw(*label);
    }
}