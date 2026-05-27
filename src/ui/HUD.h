#pragma once
#include <SFML/Graphics.hpp>
#include "core/GameStats.h"

class HUD {
public:
    HUD();
    void update(const GameStats& stats);
    void render(sf::RenderWindow& window);

private:
};
