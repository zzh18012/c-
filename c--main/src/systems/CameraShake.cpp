#include "CameraShake.h"
#include <cstdlib>
#include <algorithm>

CameraShake::CameraShake()
    : intensity(0.f), duration(0.f), timer(0.f), offset(0.f, 0.f)
{
}

void CameraShake::shake(float intensity, float duration) {
    this->intensity = std::max(this->intensity, intensity);
    this->duration = std::max(this->duration, duration);
    timer = 0.f;
}

void CameraShake::update(float dt) {
    if (timer < duration) {
        timer += dt;
        float decay = 1.f - (timer / duration);
        float currentIntensity = intensity * decay;
        offset.x = (static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f) * currentIntensity;
        offset.y = (static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f) * currentIntensity;
    } else {
        offset = sf::Vector2f(0.f, 0.f);
    }
}

sf::Vector2f CameraShake::getOffset() const {
    return offset;
}
