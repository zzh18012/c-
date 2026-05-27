#include "CameraShake.h"

CameraShake::CameraShake()
    : intensity(0.f), duration(0.f), timer(0.f), offset(0.f, 0.f)
{
}

void CameraShake::shake(float intensity, float duration) {
    (void)intensity;
    (void)duration;
}

void CameraShake::update(float dt) {
    (void)dt;
}

sf::Vector2f CameraShake::getOffset() const {
    return offset;
}
