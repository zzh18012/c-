// ============================================================
// Menu.cpp - 菜单系统实现
// ============================================================
// 职责：管理游戏中的所有菜单界面
// 支持键盘导航（WASD/方向键+Enter）和鼠标操作
// 回调机制：Game.cpp通过getButtons()获取按钮并设置onClick
// ============================================================

#include "Menu.h"
#include "core/Config.h"

// ============================================================
// 构造函数
// ============================================================
// 初始化菜单遮罩：覆盖整个窗口，半透明黑色
Menu::Menu() {
    overlay.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 160)); // 160/255 ≈ 63%透明度
}

Menu::~Menu() {}

// ============================================================
// show - 显示菜单
// ============================================================
// mode: 菜单模式
// font: 游戏字体
// diff: 默认选择的难度
// 重置状态，然后根据mode构建对应的按钮列表
void Menu::show(MenuMode mode, const sf::Font& font, Difficulty diff) {
    this->mode = mode;                           // 设置菜单模式
    selectedDifficulty = diff;                   // 保存默认难度
    selectedIndex = 0;                           // 重置选择索引
    visible = true;                              // 显示菜单

    // 根据模式构建对应的按钮列表
    switch (mode) {
        case MenuMode::MainMenu:   buildMainMenu(font); break;
        case MenuMode::Pause:      buildPauseMenu(font); break;
        case MenuMode::Victory:    buildVictoryMenu(font); break;
        case MenuMode::GameOver:   buildGameOverMenu(font); break;
    }

    // 默认选中第一个按钮
    if (!buttons.empty()) {
        buttons[0].setSelected(true);
    }
}

// ============================================================
// buildMainMenu - 构建主菜单
// ============================================================
// 主菜单包含：
// 1. 标题 "NEON BULLET ARENA"
// 2. 3个难度按钮：NORMAL / HARD / LUNATIC
// 3. 退出按钮：QUIT
void Menu::buildMainMenu(const sf::Font& font) {
    // ---- 标题 ----
    title.emplace(font, "", 48);                         // 创建文字
    title->setString("NEON BULLET ARENA");                // 设置内容
    title->setFillColor(sf::Color(0, 220, 255));          // 青色
    // 设置origin到中心（居中对齐）
    sf::FloatRect tb = title->getLocalBounds();
    title->setOrigin(sf::Vector2f(tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f));
    title->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, 140.f)); // 屏幕顶部

    // ---- 按钮 ----
    buttons.clear();                                             // 清空旧按钮
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;       // 居中X坐标
    float startY = 240.f;                                        // 第一个按钮Y

    buttons.emplace_back("NORMAL", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);     // 难度按钮1

    buttons.emplace_back("HARD", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);      // 难度按钮2

    buttons.emplace_back("LUNATIC", font,
        sf::Vector2f(btnX, startY + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);      // 难度按钮3

    buttons.emplace_back("QUIT", font,
        sf::Vector2f(btnX, startY + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);      // 退出按钮
}

// ============================================================
// buildPauseMenu - 构建暂停菜单
// ============================================================
// 暂停菜单包含：
// 1. 标题 "PAUSED"
// 2. 继续游戏按钮：RESUME
// 3. 返回主菜单按钮：QUIT TO MENU
void Menu::buildPauseMenu(const sf::Font& font) {
    title.emplace(font, "", 42);
    title->setString("PAUSED");
    title->setFillColor(sf::Color(255, 200, 50));          // 金黄色
    sf::FloatRect tb = title->getLocalBounds();
    title->setOrigin(sf::Vector2f(tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f));
    title->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, 200.f));

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 320.f;

    buttons.emplace_back("RESUME", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);  // 继续按钮

    buttons.emplace_back("QUIT TO MENU", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);  // 返回主菜单
}

// ============================================================
// buildVictoryMenu - 构建胜利菜单
// ============================================================
// 胜利菜单包含：
// 1. 标题 "VICTORY!"
// 2. 再玩一次按钮：PLAY AGAIN
// 3. 返回主菜单按钮：MAIN MENU
void Menu::buildVictoryMenu(const sf::Font& font) {
    title.emplace(font, "", 52);
    title->setString("VICTORY!");
    title->setFillColor(sf::Color(80, 255, 120));          // 绿色
    sf::FloatRect tb = title->getLocalBounds();
    title->setOrigin(sf::Vector2f(tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f));
    title->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, 200.f));

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 340.f;

    buttons.emplace_back("PLAY AGAIN", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr); // 再玩按钮

    buttons.emplace_back("MAIN MENU", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);  // 主菜单
}

// ============================================================
// buildGameOverMenu - 构建失败菜单
// ============================================================
// 失败菜单包含：
// 1. 标题 "GAME OVER"
// 2. 重试按钮：RETRY
// 3. 返回主菜单按钮：MAIN MENU
void Menu::buildGameOverMenu(const sf::Font& font) {
    title.emplace(font, "", 48);
    title->setString("GAME OVER");
    title->setFillColor(sf::Color(255, 60, 60));            // 红色
    sf::FloatRect tb = title->getLocalBounds();
    title->setOrigin(sf::Vector2f(tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f));
    title->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, 200.f));

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 340.f;

    buttons.emplace_back("RETRY", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);  // 重试按钮

    buttons.emplace_back("MAIN MENU", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);  // 主菜单
}

// ============================================================
// handleEvent - 处理事件
// ============================================================
// event: SFML事件
// window: 当前窗口
// 处理键盘导航和鼠标操作
void Menu::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!visible || buttons.empty()) return; // 不可见或无按钮则跳过

    // ============================================================
    // 键盘导航
    // ============================================================
    if (auto keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        // 上/W键：选择上一个按钮
        if (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::W) {
            buttons[selectedIndex].setSelected(false); // 取消当前选中
            // (selectedIndex - 1 + size) % size 实现环绕（倒数第一→最后）
            selectedIndex = (selectedIndex - 1 + static_cast<int>(buttons.size())) % static_cast<int>(buttons.size());
            buttons[selectedIndex].setSelected(true);  // 选中新的
        }
        // 下/S键：选择下一个按钮
        if (keyEvent->code == sf::Keyboard::Key::Down || keyEvent->code == sf::Keyboard::Key::S) {
            buttons[selectedIndex].setSelected(false);
            selectedIndex = (selectedIndex + 1) % static_cast<int>(buttons.size());
            buttons[selectedIndex].setSelected(true);
        }
        // Enter/Space：激活当前选中按钮
        if (keyEvent->code == sf::Keyboard::Key::Enter || keyEvent->code == sf::Keyboard::Key::Space) {
            buttons[selectedIndex].activate();
        }
    }

    // ============================================================
    // 鼠标悬停检测
    // ============================================================
    if (event.getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition());
        // 检查鼠标是否移动到某个按钮上
        for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
            if (buttons[i].contains(mousePos) && i != selectedIndex) {
                buttons[selectedIndex].setSelected(false); // 取消当前选中
                selectedIndex = i;                         // 更新索引
                buttons[selectedIndex].setSelected(true);  // 选中新的
            }
        }
    }

    // ============================================================
    // 委托鼠标事件给所有按钮（处理点击）
    // ============================================================
    // 遍历所有按钮，让它们处理自己的鼠标事件
    for (auto& btn : buttons) {
        btn.handleEvent(event, window);
    }
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 更新所有按钮状态（边框颜色等）
void Menu::update(float dt) {
    if (!visible) return; // 不可见则跳过
    for (auto& btn : buttons) {
        btn.update(dt);   // 更新按钮状态
    }
}

// ============================================================
// render - 渲染菜单
// ============================================================
// 渲染顺序：遮罩 → 标题 → 按钮
void Menu::render(sf::RenderWindow& window) const {
    if (!visible) return; // 不可见则跳过

    // 非主菜单显示遮罩（主菜单不需要，因为游戏还没开始）
    if (mode != MenuMode::MainMenu) {
        window.draw(overlay);
    }

    // 渲染标题
    if (title) {
        window.draw(*title);
    }

    // 渲染所有按钮
    for (const auto& btn : buttons) {
        btn.render(window);
    }
}

// ============================================================
// 状态查询和修改方法
// ============================================================
bool Menu::isVisible() const { return visible; }
void Menu::setVisible(bool v) { visible = v; }
MenuMode Menu::getMode() const { return mode; }

// ============================================================
// 按钮访问（供Game.cpp设置回调）
// ============================================================
// 获取按钮数组
std::vector<Button>& Menu::getButtons() { return buttons; }
// 获取指定索引的按钮
Button& Menu::getButton(int index) { return buttons[index]; }