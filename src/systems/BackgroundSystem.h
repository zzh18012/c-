#pragma once
#include <SFML/Graphics.hpp>

class BackgroundSystem {
public:
    BackgroundSystem();
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
};
