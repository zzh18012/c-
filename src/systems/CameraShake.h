#pragma once
#include <SFML/Graphics.hpp>

class CameraShake {
public:
    CameraShake();
    void shake(float intensity, float duration);
    void update(float dt);
    sf::Vector2f getOffset() const;

private:
    float intensity;
    float duration;
    float timer;
    sf::Vector2f offset;
};
