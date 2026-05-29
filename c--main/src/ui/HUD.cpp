#include "HUD.h"
#include "core/Config.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cstdint>

HUD::HUD() : fontPtr(nullptr) {
    // Player HP bar background
    playerBarBg.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH, HUD_BAR_HEIGHT));
    playerBarBg.setPosition(sf::Vector2f(HUD_PADDING, HUD_PADDING));
    playerBarBg.setFillColor(sf::Color(30, 30, 30, 200));
    playerBarBg.setOutlineThickness(1.f);
    playerBarBg.setOutlineColor(sf::Color(0, 180, 220));

    playerBarFill.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH, HUD_BAR_HEIGHT));
    playerBarFill.setPosition(sf::Vector2f(HUD_PADDING, HUD_PADDING));
    playerBarFill.setFillColor(sf::Color(0, 200, 240));

    // Boss HP bar background
    bossBarBg.setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH, HUD_BAR_HEIGHT));
    bossBarBg.setPosition(sf::Vector2f(WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH, HUD_PADDING));
    bossBarBg.setFillColor(sf::Color(30, 30, 30, 200));
    bossBarBg.setOutlineThickness(1.f);
    bossBarBg.setOutlineColor(sf::Color(220, 60, 60));

    bossBarFill.setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH, HUD_BAR_HEIGHT));
    bossBarFill.setPosition(sf::Vector2f(WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH, HUD_PADDING));
    bossBarFill.setFillColor(sf::Color(240, 50, 50));

    // Phase markers
    float bossBarX = WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH;
    float markerY = HUD_PADDING - 4.f;
    float markerH = HUD_BAR_HEIGHT + 8.f;

    phaseMarker70.setSize(sf::Vector2f(2.f, markerH));
    phaseMarker70.setPosition(sf::Vector2f(bossBarX + HUD_BOSS_BAR_WIDTH * (1.f - PHASE2_HP_RATIO), markerY));
    phaseMarker70.setFillColor(sf::Color(255, 180, 40, 150));

    phaseMarker35.setSize(sf::Vector2f(2.f, markerH));
    phaseMarker35.setPosition(sf::Vector2f(bossBarX + HUD_BOSS_BAR_WIDTH * (1.f - PHASE3_HP_RATIO), markerY));
    phaseMarker35.setFillColor(sf::Color(255, 80, 40, 150));

    // Dash cooldown circle
    dashCooldownBg.setRadius(16.f);
    dashCooldownBg.setOrigin(sf::Vector2f(16.f, 16.f));
    dashCooldownBg.setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 36.f));
    dashCooldownBg.setFillColor(sf::Color(20, 20, 30, 180));
    dashCooldownBg.setOutlineThickness(2.f);
    dashCooldownBg.setOutlineColor(sf::Color(0, 200, 255, 100));

    dashCooldownRing.setRadius(14.f);
    dashCooldownRing.setOrigin(sf::Vector2f(14.f, 14.f));
    dashCooldownRing.setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 36.f));
    dashCooldownRing.setFillColor(sf::Color(0, 0, 0, 0));
    dashCooldownRing.setOutlineThickness(3.f);
    dashCooldownRing.setOutlineColor(sf::Color(0, 220, 255, 200));
    dashCooldownRing.setPointCount(60);

    // Buff icons
    auto makeBuffIcon = [](sf::RectangleShape& icon, sf::Color c, float x, float y) {
        icon.setSize(sf::Vector2f(20.f, 20.f));
        icon.setPosition(sf::Vector2f(x, y));
        icon.setFillColor(c);
        icon.setOutlineThickness(1.f);
        icon.setOutlineColor(sf::Color(255, 255, 255, 80));
    };

    float buffY = WINDOW_HEIGHT - 50.f;
    makeBuffIcon(shieldIcon, sf::Color(60, 140, 255), HUD_PADDING, buffY);
    makeBuffIcon(overdriveIcon, sf::Color(255, 150, 30), HUD_PADDING + 120.f, buffY);
    makeBuffIcon(dashBatteryIcon, sf::Color(0, 220, 200), HUD_PADDING + 240.f, buffY);
}

HUD::~HUD() {}

void HUD::init(const sf::Font& font) {
    fontPtr = &font;

    // Initialize all text objects with the font
    playerHPText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    bossHPText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    bossPhaseText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    attackNameText.emplace(font, "", HUD_FONT_SIZE_NORMAL);
    shieldText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    overdriveText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    dashBatteryText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    bulletTimeText.emplace(font, "", HUD_FONT_SIZE_NORMAL);
    timerText.emplace(font, "", HUD_FONT_SIZE_NORMAL);
    endMessageText.emplace(font, "", HUD_FONT_SIZE_LARGE);

    // Set initial positions and colors
    playerHPText->setPosition(sf::Vector2f(HUD_PADDING + 6.f, HUD_PADDING + 1.f));
    playerHPText->setFillColor(sf::Color::White);

    float bossBarX = WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH;
    bossHPText->setPosition(sf::Vector2f(bossBarX + 6.f, HUD_PADDING + 1.f));
    bossHPText->setFillColor(sf::Color::White);

    bossPhaseText->setPosition(sf::Vector2f(bossBarX - 40.f, HUD_PADDING - 2.f));
    bossPhaseText->setFillColor(sf::Color(255, 200, 50));

    attackNameText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, 50.f));
    attackNameText->setFillColor(sf::Color(255, 220, 100));

    shieldText->setPosition(sf::Vector2f(HUD_PADDING + 26.f, WINDOW_HEIGHT - 52.f));
    shieldText->setFillColor(sf::Color(120, 180, 255));

    overdriveText->setPosition(sf::Vector2f(HUD_PADDING + 146.f, WINDOW_HEIGHT - 52.f));
    overdriveText->setFillColor(sf::Color(255, 180, 80));

    dashBatteryText->setPosition(sf::Vector2f(HUD_PADDING + 266.f, WINDOW_HEIGHT - 52.f));
    dashBatteryText->setFillColor(sf::Color(80, 240, 220));

    bulletTimeText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 70.f));
    bulletTimeText->setFillColor(sf::Color(200, 120, 255));

    timerText->setPosition(sf::Vector2f(WINDOW_WIDTH - 80.f, WINDOW_HEIGHT - 36.f));
    timerText->setFillColor(sf::Color(200, 200, 200));

    endMessageText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f));
    endMessageText->setFillColor(sf::Color::White);
}

void HUD::update(const GameStats& stats) {
    if (!fontPtr) return;

    // Smooth HP transitions
    float lerpSpeed = HUD_BAR_LERP_SPEED;
    displayedPlayerHP += (static_cast<float>(stats.playerHP) - displayedPlayerHP) * lerpSpeed * 0.016f;
    if (std::fabs(displayedPlayerHP - stats.playerHP) < 0.5f)
        displayedPlayerHP = static_cast<float>(stats.playerHP);

    displayedBossHP += (static_cast<float>(stats.bossHP) - displayedBossHP) * lerpSpeed * 0.016f;
    if (std::fabs(displayedBossHP - stats.bossHP) < 0.5f)
        displayedBossHP = static_cast<float>(stats.bossHP);

    // Player HP bar
    float playerRatio = displayedPlayerHP / stats.playerMaxHP;
    playerBarFill.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH * playerRatio, HUD_BAR_HEIGHT));
    sf::Color hpColor;
    if (playerRatio > 0.5f)
        hpColor = sf::Color(static_cast<std::uint8_t>(255 * (1.f - playerRatio) * 2.f), 220, static_cast<std::uint8_t>(255 * (0.5f - playerRatio) * 2.f + 100));
    else
        hpColor = sf::Color(255, static_cast<std::uint8_t>(220 * playerRatio * 2.f), 50);
    playerBarFill.setFillColor(hpColor);

    std::ostringstream php;
    php << "HP " << stats.playerHP << "/" << stats.playerMaxHP;
    playerHPText->setString(php.str());

    // Player HP bar background flash when invincible
    if (stats.playerInvincible) {
        float flash = (sinf(stats.elapsedTime * 15.f) + 1.f) * 0.5f;
        playerBarBg.setOutlineColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(100 + flash * 155)));
    } else {
        playerBarBg.setOutlineColor(sf::Color(0, 180, 220));
    }

    // Boss HP bar
    float bossRatio = displayedBossHP / stats.bossMaxHP;
    bossBarFill.setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH * bossRatio, HUD_BAR_HEIGHT));

    std::ostringstream bhp;
    bhp << "HP " << stats.bossHP << "/" << stats.bossMaxHP;
    bossHPText->setString(bhp.str());

    // Boss phase text
    std::ostringstream phaseStr;
    phaseStr << "Phase " << stats.bossPhase;
    bossPhaseText->setString(phaseStr.str());

    // Phase markers highlight when active phase is near
    if (stats.bossPhase == 2) {
        phaseMarker70.setFillColor(sf::Color(255, 200, 50, 220));
    }
    if (stats.bossPhase == 3) {
        phaseMarker35.setFillColor(sf::Color(255, 100, 40, 220));
    }

    // Attack name
    attackNameText->setString(stats.currentAttackName);
    attackFlashTimer += 0.016f;
    if (stats.bossLaserWarning) {
        float flash = (sinf(attackFlashTimer * 8.f) + 1.f) * 0.5f;
        attackNameText->setFillColor(sf::Color(255, static_cast<std::uint8_t>(60 + flash * 195), static_cast<std::uint8_t>(60 + flash * 195)));
    } else {
        attackNameText->setFillColor(sf::Color(255, 220, 100));
    }
    // Center the attack name text
    sf::FloatRect ab = attackNameText->getLocalBounds();
    attackNameText->setOrigin(sf::Vector2f(ab.position.x + ab.size.x / 2.f, ab.position.y));

    // Buff timers
    shieldText->setString(stats.playerShieldTimer > 0.f ? fmtTime(stats.playerShieldTimer) : "");
    overdriveText->setString(stats.playerOverdriveTimer > 0.f ? fmtTime(stats.playerOverdriveTimer) : "");
    dashBatteryText->setString(stats.playerDashBatteryTimer > 0.f ? fmtTime(stats.playerDashBatteryTimer) : "");

    bulletTimeText->setString(stats.bulletTimeActive ? "BULLET TIME" : "");

    // Dash cooldown ring
    float dashRatio = 1.f - (stats.playerDashCooldown / PLAYER_DASH_COOLDOWN);
    dashCooldownRing.setOutlineColor(
        dashRatio >= 1.f ? sf::Color(0, 255, 200, 220) : sf::Color(0, 180, 220, 120));

    // Timer
    int mins = static_cast<int>(stats.elapsedTime) / 60;
    int secs = static_cast<int>(stats.elapsedTime) % 60;
    std::ostringstream timeStr;
    timeStr << std::setfill('0') << std::setw(2) << mins << ":"
            << std::setfill('0') << std::setw(2) << secs;
    timerText->setString(timeStr.str());
    sf::FloatRect tb = timerText->getLocalBounds();
    timerText->setOrigin(sf::Vector2f(tb.position.x + tb.size.x / 2.f, 0.f));

    // End message
    if (stats.victory) {
        endMessageText->setString("VICTORY!");
        endMessageText->setFillColor(sf::Color(80, 255, 120, static_cast<std::uint8_t>(endMessageAlpha)));
        endMessageAlpha = std::min(255.f, endMessageAlpha + 60.f * 0.016f);
    } else if (stats.gameOver) {
        endMessageText->setString("GAME OVER");
        endMessageText->setFillColor(sf::Color(255, 60, 60, static_cast<std::uint8_t>(endMessageAlpha)));
        endMessageAlpha = std::min(255.f, endMessageAlpha + 60.f * 0.016f);
    } else {
        endMessageAlpha = 0.f;
    }
    sf::FloatRect eb = endMessageText->getLocalBounds();
    endMessageText->setOrigin(sf::Vector2f(eb.position.x + eb.size.x / 2.f, eb.position.y + eb.size.y / 2.f));
}

std::string HUD::fmtTime(float seconds) {
    int s = static_cast<int>(seconds);
    int tenths = static_cast<int>((seconds - s) * 10.f);
    std::ostringstream ss;
    ss << s << "." << tenths << "s";
    return ss.str();
}

void HUD::render(sf::RenderWindow& window) {
    // HP bars
    window.draw(playerBarBg);
    window.draw(playerBarFill);
    if (playerHPText) window.draw(*playerHPText);

    window.draw(bossBarBg);
    window.draw(bossBarFill);
    window.draw(phaseMarker70);
    window.draw(phaseMarker35);
    if (bossHPText) window.draw(*bossHPText);
    if (bossPhaseText) window.draw(*bossPhaseText);

    // Attack name
    if (attackNameText && attackNameText->getString().getSize() > 0) {
        window.draw(*attackNameText);
    }

    // Buffs (only when active)
    if (shieldText && shieldText->getString().getSize() > 0) {
        window.draw(shieldIcon);
        window.draw(*shieldText);
    }
    if (overdriveText && overdriveText->getString().getSize() > 0) {
        window.draw(overdriveIcon);
        window.draw(*overdriveText);
    }
    if (dashBatteryText && dashBatteryText->getString().getSize() > 0) {
        window.draw(dashBatteryIcon);
        window.draw(*dashBatteryText);
    }
    if (bulletTimeText && bulletTimeText->getString().getSize() > 0) {
        window.draw(*bulletTimeText);
    }

    // Dash cooldown
    window.draw(dashCooldownBg);
    window.draw(dashCooldownRing);

    // Timer
    if (timerText) window.draw(*timerText);

    // End message
    if (endMessageAlpha > 0.f && endMessageText) {
        window.draw(*endMessageText);
    }
}