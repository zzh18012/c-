#include "HUD.h"
#include "core/Config.h"
#include <cmath>
#include <sstream>
#include <iomanip>

HUD::HUD() {
    // Player HP bar background
    playerBarBg.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH, HUD_BAR_HEIGHT));
    playerBarBg.setPosition(HUD_PADDING, HUD_PADDING);
    playerBarBg.setFillColor(sf::Color(30, 30, 30, 200));
    playerBarBg.setOutlineThickness(1.f);
    playerBarBg.setOutlineColor(sf::Color(0, 180, 220));

    playerBarFill.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH, HUD_BAR_HEIGHT));
    playerBarFill.setPosition(HUD_PADDING, HUD_PADDING);
    playerBarFill.setFillColor(sf::Color(0, 200, 240));

    // Boss HP bar background
    bossBarBg.setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH, HUD_BAR_HEIGHT));
    bossBarBg.setPosition(WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH, HUD_PADDING);
    bossBarBg.setFillColor(sf::Color(30, 30, 30, 200));
    bossBarBg.setOutlineThickness(1.f);
    bossBarBg.setOutlineColor(sf::Color(220, 60, 60));

    bossBarFill.setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH, HUD_BAR_HEIGHT));
    bossBarFill.setPosition(WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH, HUD_PADDING);
    bossBarFill.setFillColor(sf::Color(240, 50, 50));

    // Phase markers
    float bossBarX = WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH;
    float markerY = HUD_PADDING - 4.f;
    float markerH = HUD_BAR_HEIGHT + 8.f;

    phaseMarker70.setSize(sf::Vector2f(2.f, markerH));
    phaseMarker70.setPosition(bossBarX + HUD_BOSS_BAR_WIDTH * (1.f - PHASE2_HP_RATIO), markerY);
    phaseMarker70.setFillColor(sf::Color(255, 180, 40, 150));

    phaseMarker35.setSize(sf::Vector2f(2.f, markerH));
    phaseMarker35.setPosition(bossBarX + HUD_BOSS_BAR_WIDTH * (1.f - PHASE3_HP_RATIO), markerY);
    phaseMarker35.setFillColor(sf::Color(255, 80, 40, 150));

    // Dash cooldown circle
    dashCooldownBg.setRadius(16.f);
    dashCooldownBg.setOrigin(16.f, 16.f);
    dashCooldownBg.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 36.f);
    dashCooldownBg.setFillColor(sf::Color(20, 20, 30, 180));
    dashCooldownBg.setOutlineThickness(2.f);
    dashCooldownBg.setOutlineColor(sf::Color(0, 200, 255, 100));

    dashCooldownRing.setRadius(14.f);
    dashCooldownRing.setOrigin(14.f, 14.f);
    dashCooldownRing.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 36.f);
    dashCooldownRing.setFillColor(sf::Color::Transparent);
    dashCooldownRing.setOutlineThickness(3.f);
    dashCooldownRing.setOutlineColor(sf::Color(0, 220, 255, 200));
    dashCooldownRing.setPointCount(60);

    // Buff icons
    auto makeBuffIcon = [](sf::RectangleShape& icon, sf::Color c, float x, float y) {
        icon.setSize(sf::Vector2f(20.f, 20.f));
        icon.setPosition(x, y);
        icon.setFillColor(c);
        icon.setOutlineThickness(1.f);
        icon.setOutlineColor(sf::Color(255, 255, 255, 80));
    };

    float buffY = WINDOW_HEIGHT - 50.f;
    makeBuffIcon(shieldIcon, sf::Color(60, 140, 255), HUD_PADDING, buffY);
    makeBuffIcon(overdriveIcon, sf::Color(255, 150, 30), HUD_PADDING + 120.f, buffY);
    makeBuffIcon(dashBatteryIcon, sf::Color(0, 220, 200), HUD_PADDING + 240.f, buffY);
}

void HUD::init(const sf::Font& font) {
    auto makeText = [&](sf::Text& t, float x, float y, unsigned size) {
        t.setFont(font);
        t.setCharacterSize(size);
        t.setPosition(x, y);
    };

    makeText(playerHPText, HUD_PADDING + 6.f, HUD_PADDING + 1.f, HUD_FONT_SIZE_SMALL);
    playerHPText.setFillColor(sf::Color::White);

    float bossBarX = WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH;
    makeText(bossHPText, bossBarX + 6.f, HUD_PADDING + 1.f, HUD_FONT_SIZE_SMALL);
    bossHPText.setFillColor(sf::Color::White);

    makeText(bossPhaseText, bossBarX - 40.f, HUD_PADDING - 2.f, HUD_FONT_SIZE_SMALL);
    bossPhaseText.setFillColor(sf::Color(255, 200, 50));

    makeText(attackNameText, WINDOW_WIDTH / 2.f, 50.f, HUD_FONT_SIZE_NORMAL);
    attackNameText.setFillColor(sf::Color(255, 220, 100));

    makeText(timerText, WINDOW_WIDTH - 80.f, WINDOW_HEIGHT - 36.f, HUD_FONT_SIZE_NORMAL);
    timerText.setFillColor(sf::Color(200, 200, 200));

    float buffY = WINDOW_HEIGHT - 50.f;
    makeText(shieldText, HUD_PADDING + 26.f, buffY - 2.f, HUD_FONT_SIZE_SMALL);
    shieldText.setFillColor(sf::Color(120, 180, 255));
    makeText(overdriveText, HUD_PADDING + 146.f, buffY - 2.f, HUD_FONT_SIZE_SMALL);
    overdriveText.setFillColor(sf::Color(255, 180, 80));
    makeText(dashBatteryText, HUD_PADDING + 266.f, buffY - 2.f, HUD_FONT_SIZE_SMALL);
    dashBatteryText.setFillColor(sf::Color(80, 240, 220));

    makeText(bulletTimeText, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 70.f, HUD_FONT_SIZE_NORMAL);
    bulletTimeText.setFillColor(sf::Color(200, 120, 255));

    makeText(endMessageText, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f, HUD_FONT_SIZE_LARGE);
    endMessageText.setFillColor(sf::Color::White);
}

void HUD::update(const GameStats& stats) {
    // Smooth HP transitions
    float lerpSpeed = HUD_BAR_LERP_SPEED;
    displayedPlayerHP += (static_cast<float>(stats.playerHP) - displayedPlayerHP) * lerpSpeed * 0.016f;
    if (std::abs(displayedPlayerHP - stats.playerHP) < 0.5f)
        displayedPlayerHP = static_cast<float>(stats.playerHP);

    displayedBossHP += (static_cast<float>(stats.bossHP) - displayedBossHP) * lerpSpeed * 0.016f;
    if (std::abs(displayedBossHP - stats.bossHP) < 0.5f)
        displayedBossHP = static_cast<float>(stats.bossHP);

    // Player HP bar
    float playerRatio = displayedPlayerHP / stats.playerMaxHP;
    playerBarFill.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH * playerRatio, HUD_BAR_HEIGHT));
    // Color: green → yellow → red as HP drops
    sf::Color hpColor;
    if (playerRatio > 0.5f)
        hpColor = sf::Color(static_cast<sf::Uint8>(255 * (1.f - playerRatio) * 2.f), 220, static_cast<sf::Uint8>(255 * (0.5f - playerRatio) * 2.f + 100));
    else
        hpColor = sf::Color(255, static_cast<sf::Uint8>(220 * playerRatio * 2.f), 50);
    playerBarFill.setFillColor(hpColor);

    std::ostringstream php;
    php << "HP " << stats.playerHP << "/" << stats.playerMaxHP;
    playerHPText.setString(php.str());

    // Player HP bar background flash when invincible
    if (stats.playerInvincible) {
        float flash = (sinf(stats.elapsedTime * 15.f) + 1.f) * 0.5f;
        playerBarBg.setOutlineColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(100 + flash * 155)));
    } else {
        playerBarBg.setOutlineColor(sf::Color(0, 180, 220));
    }

    // Boss HP bar
    float bossRatio = displayedBossHP / stats.bossMaxHP;
    bossBarFill.setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH * bossRatio, HUD_BAR_HEIGHT));

    std::ostringstream bhp;
    bhp << "HP " << stats.bossHP << "/" << stats.bossMaxHP;
    bossHPText.setString(bhp.str());

    // Boss phase text
    std::ostringstream phaseStr;
    phaseStr << "Phase " << stats.bossPhase;
    bossPhaseText.setString(phaseStr.str());

    // Phase markers highlight when active phase is near
    if (stats.bossPhase == 2) {
        phaseMarker70.setFillColor(sf::Color(255, 200, 50, 220));
    }
    if (stats.bossPhase == 3) {
        phaseMarker35.setFillColor(sf::Color(255, 100, 40, 220));
    }

    // Attack name
    attackNameText.setString(stats.currentAttackName);
    attackFlashTimer += 0.016f;
    if (stats.bossLaserWarning) {
        float flash = (sinf(attackFlashTimer * 8.f) + 1.f) * 0.5f;
        attackNameText.setFillColor(sf::Color(255, static_cast<sf::Uint8>(60 + flash * 195), static_cast<sf::Uint8>(60 + flash * 195)));
    } else {
        attackNameText.setFillColor(sf::Color(255, 220, 100));
    }
    // Center the attack name text
    sf::FloatRect ab = attackNameText.getLocalBounds();
    attackNameText.setOrigin(ab.left + ab.width / 2.f, ab.top);

    // Buff timers
    shieldText.setString(stats.playerShieldTimer > 0.f ? fmtTime(stats.playerShieldTimer) : "");
    overdriveText.setString(stats.playerOverdriveTimer > 0.f ? fmtTime(stats.playerOverdriveTimer) : "");
    dashBatteryText.setString(stats.playerDashBatteryTimer > 0.f ? fmtTime(stats.playerDashBatteryTimer) : "");

    bulletTimeText.setString(stats.bulletTimeActive ? "BULLET TIME" : "");

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
    timerText.setString(timeStr.str());
    sf::FloatRect tb = timerText.getLocalBounds();
    timerText.setOrigin(tb.left + tb.width / 2.f, 0.f);

    // End message
    if (stats.victory) {
        endMessageText.setString("VICTORY!");
        endMessageText.setFillColor(sf::Color(80, 255, 120, static_cast<sf::Uint8>(endMessageAlpha)));
        endMessageAlpha = std::min(255.f, endMessageAlpha + 60.f * 0.016f);
    } else if (stats.gameOver) {
        endMessageText.setString("GAME OVER");
        endMessageText.setFillColor(sf::Color(255, 60, 60, static_cast<sf::Uint8>(endMessageAlpha)));
        endMessageAlpha = std::min(255.f, endMessageAlpha + 60.f * 0.016f);
    } else {
        endMessageAlpha = 0.f;
    }

    sf::FloatRect eb = endMessageText.getLocalBounds();
    endMessageText.setOrigin(eb.left + eb.width / 2.f, eb.top + eb.height / 2.f);
    endMessageText.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
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
    window.draw(playerHPText);

    window.draw(bossBarBg);
    window.draw(bossBarFill);
    window.draw(phaseMarker70);
    window.draw(phaseMarker35);
    window.draw(bossHPText);
    window.draw(bossPhaseText);

    // Attack name
    if (attackNameText.getString().getSize() > 0) {
        window.draw(attackNameText);
    }

    // Buffs (only when active)
    if (shieldText.getString().getSize() > 0) {
        window.draw(shieldIcon);
        window.draw(shieldText);
    }
    if (overdriveText.getString().getSize() > 0) {
        window.draw(overdriveIcon);
        window.draw(overdriveText);
    }
    if (dashBatteryText.getString().getSize() > 0) {
        window.draw(dashBatteryIcon);
        window.draw(dashBatteryText);
    }
    if (bulletTimeText.getString().getSize() > 0) {
        window.draw(bulletTimeText);
    }

    // Dash cooldown
    window.draw(dashCooldownBg);
    window.draw(dashCooldownRing);

    // Timer
    window.draw(timerText);

    // End message
    if (endMessageAlpha > 0.f) {
        window.draw(endMessageText);
    }
}
