#include "CollisionSystem.h"
#include <cmath>

bool CollisionSystem::checkCircleCollision(
    const sf::Vector2f& pos1, float r1,
    const sf::Vector2f& pos2, float r2
) {
    float dx = pos1.x - pos2.x;
    float dy = pos1.y - pos2.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = r1 + r2;
    return distSq <= radiusSum * radiusSum;
}
