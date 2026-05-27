#include "BackgroundSystem.h"
#include "core/Config.h"
#include <cstdlib>
#include <cmath>

BackgroundSystem::BackgroundSystem()
    : timer(0.f)
{
    // Dark arena background
    bgRect.setSize(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    bgRect.setFillColor(sf::Color(8, 8, 24));

    // Perspective grid (horizontal + vertical lines forming a floor grid)
    const int hLines = 18;
    const int vLines = 24;
    const int totalVertices = (hLines + vLines) * 2;
    gridLines.setPrimitiveType(sf::Lines);
    gridLines.resize(totalVertices);

    sf::Color gridColor(50, 160, 240, static_cast<sf::Uint8>(BG_GRID_ALPHA));
    float horizonY = WINDOW_HEIGHT * 0.1f; // horizon near top

    // Horizontal lines — perspective: closer together near horizon
    for (int i = 0; i < hLines; ++i) {
        float t = static_cast<float>(i) / (hLines - 1);
        float y = horizonY + (WINDOW_HEIGHT - horizonY) * (t * t * t); // cubic for stronger perspective
        sf::Color lineColor = gridColor;
        // Lines near horizon are more transparent
        lineColor.a = static_cast<sf::Uint8>(gridColor.a * (0.3f + 0.7f * t));
        gridLines[i * 2].position = sf::Vector2f(0.f, y);
        gridLines[i * 2].color = lineColor;
        gridLines[i * 2 + 1].position = sf::Vector2f(WINDOW_WIDTH, y);
        gridLines[i * 2 + 1].color = lineColor;
    }

    // Vertical lines — converge toward center-top (vanishing point)
    float vanishX = WINDOW_WIDTH * 0.5f;
    float vanishY = horizonY;
    int vOffset = hLines * 2;
    for (int i = 0; i < vLines; ++i) {
        float t = static_cast<float>(i) / (vLines - 1); // 0..1 left to right
        float topX = vanishX + (t - 0.5f) * WINDOW_WIDTH * 0.08f; // narrow at top
        float botX = vanishX + (t - 0.5f) * WINDOW_WIDTH * 1.3f;  // wide at bottom
        sf::Color lineColor = gridColor;
        lineColor.a = static_cast<sf::Uint8>(gridColor.a * 0.6f);
        gridLines[vOffset + i * 2].position = sf::Vector2f(topX, vanishY);
        gridLines[vOffset + i * 2].color = lineColor;
        gridLines[vOffset + i * 2 + 1].position = sf::Vector2f(botX, WINDOW_HEIGHT);
        gridLines[vOffset + i * 2 + 1].color = lineColor;
    }

    // Floating ambient dots
    ambientDots.resize(BG_AMBIENT_DOT_COUNT);
    dotPhases.resize(BG_AMBIENT_DOT_COUNT);
    dotBasePositions.resize(BG_AMBIENT_DOT_COUNT);

    for (int i = 0; i < BG_AMBIENT_DOT_COUNT; ++i) {
        float radius = BG_DOT_MIN_RADIUS + static_cast<float>(rand()) / RAND_MAX * (BG_DOT_MAX_RADIUS - BG_DOT_MIN_RADIUS);
        ambientDots[i].setRadius(radius);
        ambientDots[i].setOrigin(radius, radius);

        sf::Color dotColors[] = {
            sf::Color(0, 220, 255),     // cyan
            sf::Color(255, 80, 180),    // pink
            sf::Color(80, 255, 120),    // green
            sf::Color(255, 220, 60),    // gold
            sf::Color(160, 100, 255),   // purple
        };
        ambientDots[i].setFillColor(dotColors[rand() % 5]);

        float x = static_cast<float>(rand() % WINDOW_WIDTH);
        float y = static_cast<float>(rand() % WINDOW_HEIGHT);
        dotBasePositions[i] = sf::Vector2f(x, y);
        ambientDots[i].setPosition(x, y);
        dotPhases[i] = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
    }
}

void BackgroundSystem::update(float dt) {
    timer += dt;

    for (int i = 0; i < BG_AMBIENT_DOT_COUNT; ++i) {
        float newY = dotBasePositions[i].y - timer * BG_DOT_DRIFT_SPEED;
        newY = fmodf(newY + WINDOW_HEIGHT, WINDOW_HEIGHT);
        ambientDots[i].setPosition(dotBasePositions[i].x, newY);

        float pulse = (sinf(timer * BG_DOT_PULSE_SPEED + dotPhases[i]) + 1.f) * 0.5f;
        sf::Color c = ambientDots[i].getFillColor();
        c.a = static_cast<sf::Uint8>(80 + pulse * 120);
        ambientDots[i].setFillColor(c);
    }
}

void BackgroundSystem::render(sf::RenderWindow& window) {
    window.draw(bgRect);
    window.draw(gridLines);
    for (const auto& dot : ambientDots) {
        window.draw(dot);
    }
}
