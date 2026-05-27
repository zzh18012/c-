#include "Menu.h"
#include "core/Config.h"

Menu::Menu() {
    overlay.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
}

void Menu::show(MenuMode mode, const sf::Font& font) {
    this->mode = mode;
    selectedIndex = 0;
    visible = true;

    switch (mode) {
        case MenuMode::MainMenu:   buildMainMenu(font); break;
        case MenuMode::Pause:      buildPauseMenu(font); break;
        case MenuMode::Victory:    buildVictoryMenu(font); break;
        case MenuMode::GameOver:   buildGameOverMenu(font); break;
    }

    if (!buttons.empty()) {
        buttons[0].setSelected(true);
    }
}

void Menu::buildMainMenu(const sf::Font& font) {
    title.setFont(font);
    title.setString("NEON BULLET ARENA");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color(0, 220, 255));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(WINDOW_WIDTH / 2.f, 160.f);

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 300.f;

    buttons.emplace_back("START GAME", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("QUIT", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
}

void Menu::buildPauseMenu(const sf::Font& font) {
    title.setFont(font);
    title.setString("PAUSED");
    title.setCharacterSize(42);
    title.setFillColor(sf::Color(255, 200, 50));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(WINDOW_WIDTH / 2.f, 200.f);

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 320.f;

    buttons.emplace_back("RESUME", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("QUIT TO MENU", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
}

void Menu::buildVictoryMenu(const sf::Font& font) {
    title.setFont(font);
    title.setString("VICTORY!");
    title.setCharacterSize(52);
    title.setFillColor(sf::Color(80, 255, 120));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(WINDOW_WIDTH / 2.f, 200.f);

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 340.f;

    buttons.emplace_back("PLAY AGAIN", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("MAIN MENU", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
}

void Menu::buildGameOverMenu(const sf::Font& font) {
    title.setFont(font);
    title.setString("GAME OVER");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color(255, 60, 60));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(WINDOW_WIDTH / 2.f, 200.f);

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 340.f;

    buttons.emplace_back("RETRY", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("MAIN MENU", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
}

void Menu::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!visible || buttons.empty()) return;

    // Keyboard navigation
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
            buttons[selectedIndex].setSelected(false);
            selectedIndex = (selectedIndex - 1 + static_cast<int>(buttons.size())) % static_cast<int>(buttons.size());
            buttons[selectedIndex].setSelected(true);
        }
        if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
            buttons[selectedIndex].setSelected(false);
            selectedIndex = (selectedIndex + 1) % static_cast<int>(buttons.size());
            buttons[selectedIndex].setSelected(true);
        }
        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
            buttons[selectedIndex].activate();
        }
    }

    // Mouse hover — update selection
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
            if (buttons[i].contains(mousePos) && i != selectedIndex) {
                buttons[selectedIndex].setSelected(false);
                selectedIndex = i;
                buttons[selectedIndex].setSelected(true);
            }
        }
    }

    // Delegate mouse events to all buttons for hover/click
    for (auto& btn : buttons) {
        btn.handleEvent(event, window);
    }
}

void Menu::update(float dt) {
    if (!visible) return;
    for (auto& btn : buttons) {
        btn.update(dt);
    }
}

void Menu::render(sf::RenderWindow& window) const {
    if (!visible) return;

    if (mode != MenuMode::MainMenu) {
        window.draw(overlay);
    }

    window.draw(title);
    for (const auto& btn : buttons) {
        btn.render(window);
    }
}

bool Menu::isVisible() const { return visible; }
void Menu::setVisible(bool v) { visible = v; }
MenuMode Menu::getMode() const { return mode; }

std::vector<Button>& Menu::getButtons() { return buttons; }
Button& Menu::getButton(int index) { return buttons[index]; }
