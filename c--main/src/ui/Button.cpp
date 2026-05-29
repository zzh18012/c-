#include "Button.h"

Button::Button()
    : normalColor(0, 200, 255, 200)
    , hoverColor(180, 240, 255, 255)
{
}

Button::Button(const std::string& text, const sf::Font& font, sf::Vector2f position,
               sf::Vector2f size, std::function<void()> callback)
    : onClick(callback)
    , normalColor(0, 200, 255, 180)
    , hoverColor(150, 230, 255, 255)
{
    background.setPosition(position);
    background.setSize(size);
    background.setFillColor(sf::Color(20, 20, 40, 200));
    background.setOutlineThickness(2.f);
    background.setOutlineColor(normalColor);

    label.emplace(font, text, 22);
    label->setFillColor(sf::Color::White);
    sf::FloatRect textBounds = label->getLocalBounds();
    label->setOrigin(sf::Vector2f(textBounds.position.x + textBounds.size.x / 2.f,
                    textBounds.position.y + textBounds.size.y / 2.f));
    label->setPosition(sf::Vector2f(position.x + size.x / 2.f, position.y + size.y / 2.f));
}

void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition());

    if (const auto* mouseEvent = event.getIf<sf::Event::MouseMoved>()) {
        (void)mouseEvent;
        hovered = contains(mousePos);
    }

    if (const auto* mouseEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseEvent->button == sf::Mouse::Button::Left && contains(mousePos) && onClick) {
            onClick();
        }
    }
}

void Button::activate() {
    if (onClick) onClick();
}

void Button::setSelected(bool sel) {
    selected = sel;
}

bool Button::contains(sf::Vector2f point) const {
    return background.getGlobalBounds().contains(point);
}

void Button::update(float dt) {
    (void)dt;

    if (hovered || selected) {
        background.setOutlineColor(hoverColor);
        background.setOutlineThickness(3.f);
    } else {
        background.setOutlineColor(normalColor);
        background.setOutlineThickness(2.f);
    }
}

void Button::render(sf::RenderWindow& window) const {
    window.draw(background);
    if (label) {
        window.draw(*label);
    }
}