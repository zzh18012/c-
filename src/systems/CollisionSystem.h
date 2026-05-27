#pragma once
#include <SFML/Graphics.hpp>

class CollisionSystem {
public:
    static bool checkCircleCollision(
        const sf::Vector2f& pos1, float r1,
        const sf::Vector2f& pos2, float r2
    );
};
