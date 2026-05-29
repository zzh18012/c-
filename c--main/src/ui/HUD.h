#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "core/GameStats.h"

class HUD {
public:
    HUD();
    ~HUD();
    void init(const sf::Font& font);
    void update(const GameStats& stats);
    void render(sf::RenderWindow& window);

private:
    // Player HP bar
    sf::RectangleShape playerBarBg, playerBarFill;

    // Boss HP bar
    sf::RectangleShape bossBarBg, bossBarFill;
    sf::RectangleShape phaseMarker70, phaseMarker35;

    // Buff indicators
    sf::RectangleShape shieldIcon, overdriveIcon, dashBatteryIcon;

    // Dash cooldown
    sf::CircleShape dashCooldownBg, dashCooldownRing;

    // Text members (initialized in init())
    std::optional<sf::Text> playerHPText;
    std::optional<sf::Text> bossHPText;
    std::optional<sf::Text> bossPhaseText;
    std::optional<sf::Text> attackNameText;
    std::optional<sf::Text> shieldText;
    std::optional<sf::Text> overdriveText;
    std::optional<sf::Text> dashBatteryText;
    std::optional<sf::Text> bulletTimeText;
    std::optional<sf::Text> timerText;
    std::optional<sf::Text> endMessageText;

    // End message
    float endMessageAlpha = 0.f;
    float attackFlashTimer = 0.f;

    // Smooth HP values
    float displayedPlayerHP = 100.f;
    float displayedBossHP = 1000.f;

    static std::string fmtTime(float seconds);

    const sf::Font* fontPtr = nullptr;
};