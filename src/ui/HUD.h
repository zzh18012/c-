#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "core/GameStats.h"

class HUD {
public:
    HUD();
    void init(const sf::Font& font);
    void update(const GameStats& stats);
    void render(sf::RenderWindow& window);

private:
    // Player HP bar
    sf::RectangleShape playerBarBg, playerBarFill;
    sf::Text playerHPText;

    // Boss HP bar
    sf::RectangleShape bossBarBg, bossBarFill;
    sf::RectangleShape phaseMarker70, phaseMarker35;
    sf::Text bossHPText, bossPhaseText;

    // Attack name
    sf::Text attackNameText;
    float attackFlashTimer = 0.f;

    // Buff indicators
    sf::RectangleShape shieldIcon, overdriveIcon, dashBatteryIcon;
    sf::Text shieldText, overdriveText, dashBatteryText;
    sf::Text bulletTimeText;

    // Dash cooldown
    sf::CircleShape dashCooldownBg, dashCooldownRing;

    // Timer
    sf::Text timerText;

    // End message
    sf::Text endMessageText;
    float endMessageAlpha = 0.f;

    // Smooth HP values
    float displayedPlayerHP = 100.f;
    float displayedBossHP = 1000.f;

    static std::string fmtTime(float seconds);
};
