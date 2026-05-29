#pragma once
#include <SFML/Graphics.hpp>

class BackgroundSystem {
public:
    BackgroundSystem();
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    sf::RectangleShape bgRect;
    sf::VertexArray gridLines;
    std::vector<sf::CircleShape> ambientDots;
    std::vector<float> dotPhases;
    std::vector<sf::Vector2f> dotBasePositions;
    float timer;
};
