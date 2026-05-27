#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class Button {
public:
    Button();
    Button(const std::string& text, const sf::Font& font, sf::Vector2f position,
           sf::Vector2f size, std::function<void()> callback);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void activate();
    void setSelected(bool sel);
    bool contains(sf::Vector2f point) const;
    void update(float dt);
    void render(sf::RenderWindow& window) const;

private:
    sf::RectangleShape background;
    sf::Text label;
    std::function<void()> onClick;
    bool hovered = false;
    bool selected = false;
    sf::Color normalColor;
    sf::Color hoverColor;
};
