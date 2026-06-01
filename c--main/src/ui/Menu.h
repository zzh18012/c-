// ============================================================
// Menu.h - 菜单系统声明
// ============================================================
// 职责：管理游戏中的所有菜单界面（主菜单、暂停、胜利、失败）
// 支持键盘导航（WASD/方向键+Enter）和鼠标操作
// 每个菜单项使用Button组件，回调在Game.cpp中设置
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::Text, sf::RenderWindow等）
#include <SFML/Graphics.hpp>

// 标准向量（存储Button数组）
#include <vector>

// 标准字符串
#include <string>

// 可选类型（用于sf::Text的延迟构造）
#include <optional>

// 按钮组件
#include "Button.h"

// 配置文件（获取窗口尺寸、按钮尺寸等常量）
#include "core/Config.h"

// ============================================================
// 菜单模式枚举
// ============================================================
// 定义游戏中的4种菜单状态
enum class MenuMode {
    MainMenu,   // 主菜单：选择难度（Normal/Hard/Lunatic）或退出
    Pause,      // 暂停菜单：继续游戏或返回主菜单
    Victory,    // 胜利菜单：再玩一次或返回主菜单
    GameOver    // 失败菜单：重试或返回主菜单
};

// ============================================================
// Menu类
// ============================================================
// 游戏菜单系统，管理所有菜单界面
// 功能：
// 1. 显示不同模式的菜单（主菜单/暂停/胜利/失败）
// 2. 处理键盘导航（WASD/方向键+Enter）
// 3. 处理鼠标悬停和点击
// 4. 管理菜单按钮数组
// 5. 保存当前选择的难度
//
// 回调机制：Game.cpp通过getButtons()获取按钮数组，
// 然后为每个按钮设置onClick回调，实现菜单选择逻辑
class Menu {
public:
    // 构造函数：初始化菜单遮罩
    // 析构函数：默认实现
    Menu();
    ~Menu();

    // ---- 显示菜单 ----
    // mode: 菜单模式（主菜单/暂停/胜利/失败）
    // font: 游戏字体
    // diff: 默认选择的难度（仅主菜单使用）
    // 根据mode构建对应的按钮列表
    void show(MenuMode mode, const sf::Font& font, Difficulty diff = Difficulty::Normal);

    // ---- 事件处理 ----
    // event: SFML事件
    // window: 当前窗口
    // 处理键盘导航（上下键/Enter）和鼠标操作
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    // ---- 每帧更新 ----
    // dt: 时间差（秒）
    // 更新所有按钮状态
    void update(float dt);

    // ---- 渲染 ----
    // 渲染遮罩、标题、按钮
    void render(sf::RenderWindow& window) const;

    // ---- 状态查询 ----
    bool isVisible() const;          // 菜单是否可见
    void setVisible(bool v);         // 设置可见性
    MenuMode getMode() const;        // 获取当前菜单模式

    // ---- 按钮访问（供Game.cpp设置回调）----
    // 获取按钮数组，用于设置onClick回调
    std::vector<Button>& getButtons();
    Button& getButton(int index);    // 获取指定索引的按钮

    // ---- 获取选择 ----
    // 返回主菜单中用户选择的难度
    Difficulty getSelectedDifficulty() const { return selectedDifficulty; }

private:
    // ---- 私有构建方法 ----
    // 根据不同模式构建对应的按钮列表
    void buildMainMenu(const sf::Font& font);   // 主菜单：4个按钮（3难度+退出）
    void buildPauseMenu(const sf::Font& font);  // 暂停菜单：2个按钮（继续+返回）
    void buildVictoryMenu(const sf::Font& font); // 胜利菜单：2个按钮（重玩+主页）
    void buildGameOverMenu(const sf::Font& font); // 失败菜单：2个按钮（重试+主页）

    // ---- 当前状态 ----
    MenuMode mode = MenuMode::MainMenu;  // 当前菜单模式
    Difficulty selectedDifficulty = Difficulty::Normal; // 选择的难度

    // ---- 渲染元素 ----
    std::optional<sf::Text> title;       // 菜单标题文字
    std::vector<Button> buttons;          // 按钮数组

    // ---- 选择状态 ----
    int selectedIndex = 0;  // 当前选中的按钮索引（键盘导航用）

    // ---- 背景遮罩 ----
    // 半透明黑色遮罩，覆盖游戏画面
    // 除了主菜单外，其他菜单都显示遮罩
    sf::RectangleShape overlay;

    // ---- 可见性 ----
    bool visible = false;
};