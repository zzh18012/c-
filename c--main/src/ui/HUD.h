#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include <vector>
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

    // Boss HP bars (max 3)
    static constexpr int MAX_BOSS_BARS = 3;
    std::vector<sf::RectangleShape> bossBarBgs;
    std::vector<sf::RectangleShape> bossBarFills;
    std::array<std::optional<sf::Text>, MAX_BOSS_BARS> bossHPTexts;
    std::array<std::optional<sf::Text>, MAX_BOSS_BARS> bossPhaseTexts;
    std::array<std::optional<sf::Text>, MAX_BOSS_BARS> bossNameTexts;

    // Phase markers (only for single boss view)
    sf::RectangleShape phaseMarker70, phaseMarker35;

    // Buff indicators
    sf::RectangleShape shieldIcon, overdriveIcon, dashBatteryIcon;

    // Dash cooldown
    sf::CircleShape dashCooldownBg, dashCooldownRing;

    // Text members (initialized in init())
    std::optional<sf::Text> playerHPText;
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
    std::string lastAttackName;

    // Smooth HP values
    float displayedPlayerHP = 100.f;
    std::array<float, MAX_BOSS_BARS> displayedBossHPs;

    // Difficulty
    Difficulty currentDifficulty = Difficulty::Normal;

    // Active boss count for rendering
    int activeBossCount = 0;

    static std::string fmtTime(float seconds);

    const sf::Font* fontPtr = nullptr;
};