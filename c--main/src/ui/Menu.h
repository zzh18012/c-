#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <optional>
#include "Button.h"

enum class MenuMode { MainMenu, Pause, Victory, GameOver };

class Menu {
public:
    Menu();
    ~Menu();

    void show(MenuMode mode, const sf::Font& font);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt);
    void render(sf::RenderWindow& window) const;

    bool isVisible() const;
    void setVisible(bool v);
    MenuMode getMode() const;

    std::vector<Button>& getButtons();
    Button& getButton(int index);

private:
    void buildMainMenu(const sf::Font& font);
    void buildPauseMenu(const sf::Font& font);
    void buildVictoryMenu(const sf::Font& font);
    void buildGameOverMenu(const sf::Font& font);

    MenuMode mode = MenuMode::MainMenu;
    std::optional<sf::Text> title;
    std::vector<Button> buttons;
    int selectedIndex = 0;
    sf::RectangleShape overlay;
    bool visible = false;
};