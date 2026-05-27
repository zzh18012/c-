#include "Game.h"
#include "Config.h"
#include "systems/CollisionSystem.h"
#include <cstdlib>

Game::Game()
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE)
    , running(true)
    , itemSpawnTimer(ITEM_SPAWN_INTERVAL)
    , bulletTimeTimer(0.f)
    , healSpawnedAt75(false)
    , healSpawnedAt50(false)
    , healSpawnedAt25(false)
{
    window.setFramerateLimit(60);
    items.resize(ITEM_MAX_COUNT);
}

void Game::run() {
    while (window.isOpen() && running) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Escape:
                    window.close();
                    break;

                // Debug hotkeys
                case sf::Keyboard::F1:
                    player.heal(PLAYER_MAX_HP);
                    break;
                case sf::Keyboard::F2:
                    boss.takeDamage(boss.getHP() * 0.4f);
                    break;
                case sf::Keyboard::F3:
                    godModeActive = !godModeActive;
                    player.setGodMode(godModeActive);
                    break;
                case sf::Keyboard::F4:
                    boss.clearAllBullets();
                    break;
                case sf::Keyboard::F5:
                    boss.takeDamage(boss.getHP() / 2);
                    break;
                case sf::Keyboard::F6:
                    boss.takeDamage(boss.getHP());
                    break;
                default:
                    break;
            }
        }
    }
}

void Game::update(float dt) {
    if (gameStats.victory || gameStats.gameOver) return;

    gameStats.elapsedTime += dt;
    player.handleInput(dt, window);
    player.update(dt);
    boss.update(dt, player.getPosition());

    // Bullet Time countdown
    if (bulletTimeTimer > 0.f) {
        bulletTimeTimer -= dt;
        if (bulletTimeTimer <= 0.f) {
            boss.setBulletSpeedMultiplier(1.f);
        }
    }

    // Forced Heal Core spawns at HP thresholds (75%, 50%, 25%)
    float hpRatio = static_cast<float>(boss.getHP()) / boss.getMaxHP();
    auto spawnHealCore = [&]() {
        for (auto& item : items) {
            if (!item.isActive()) {
                float x = 80.f + static_cast<float>(rand() % (WINDOW_WIDTH - 160));
                float y = 80.f + static_cast<float>(rand() % (WINDOW_HEIGHT - 160));
                item.spawn(ItemType::HealCore, sf::Vector2f(x, y));
                return;
            }
        }
    };

    if (!healSpawnedAt75 && hpRatio <= 0.75f) {
        healSpawnedAt75 = true;
        spawnHealCore();
    }
    if (!healSpawnedAt50 && hpRatio <= 0.50f) {
        healSpawnedAt50 = true;
        spawnHealCore();
    }
    if (!healSpawnedAt25 && hpRatio <= 0.25f) {
        healSpawnedAt25 = true;
        spawnHealCore();
    }

    // Normal item spawning (every 10s, max ITEM_MAX_COUNT on field)
    itemSpawnTimer -= dt;
    if (itemSpawnTimer <= 0.f) {
        itemSpawnTimer = ITEM_SPAWN_INTERVAL;

        int activeCount = 0;
        for (const auto& item : items) {
            if (item.isActive()) activeCount++;
        }
        if (activeCount < ITEM_MAX_COUNT) {
            for (auto& item : items) {
                if (!item.isActive()) {
                    int phase = boss.getPhase();
                    ItemType type;
                    if (phase == 1) {
                        type = (rand() % 2 == 0) ? ItemType::HealCore : ItemType::DashBattery;
                    } else if (phase == 2) {
                        type = (rand() % 2 == 0) ? ItemType::ShieldOrb : ItemType::Overdrive;
                    } else {
                        type = (rand() % 2 == 0) ? ItemType::BulletTime : ItemType::NovaBomb;
                    }
                    float x = 80.f + static_cast<float>(rand() % (WINDOW_WIDTH - 160));
                    float y = 80.f + static_cast<float>(rand() % (WINDOW_HEIGHT - 160));
                    item.spawn(type, sf::Vector2f(x, y));
                    break;
                }
            }
        }
    }

    // Update items
    for (auto& item : items) {
        item.update(dt);
    }

    // Player-Item collision
    for (auto& item : items) {
        if (!item.isActive()) continue;
        if (CollisionSystem::checkCircleCollision(
                player.getPosition(), player.getRadius(),
                item.getPosition(), item.getRadius())) {
            switch (item.getType()) {
                case ItemType::HealCore:
                    player.heal(static_cast<int>(player.getMaxHP() * HEAL_CORE_RATIO));
                    break;
                case ItemType::ShieldOrb:
                    player.activateShieldOrb();
                    break;
                case ItemType::Overdrive:
                    player.activateOverdrive();
                    break;
                case ItemType::BulletTime:
                    boss.setBulletSpeedMultiplier(BULLET_TIME_SLOW_RATIO);
                    bulletTimeTimer = BULLET_TIME_DURATION;
                    break;
                case ItemType::NovaBomb:
                    boss.clearAllBullets();
                    boss.takeDamage(NOVA_BOMB_DAMAGE);
                    break;
                case ItemType::DashBattery:
                    player.activateDashBattery();
                    break;
            }
            item.deactivate();
        }
    }

    // Player bullets vs Boss collision
    for (auto& bullet : player.getBullets()) {
        if (!bullet.isActive()) continue;
        if (CollisionSystem::checkCircleCollision(
                bullet.getPosition(), bullet.getRadius(),
                boss.getPosition(), boss.getRadius())) {
            boss.takeDamage(bullet.getDamage());
            bullet.deactivate();
        }
    }

    // Boss bullets vs Player collision
    for (auto& bullet : boss.getBullets()) {
        if (!bullet.isActive()) continue;
        if (CollisionSystem::checkCircleCollision(
                bullet.getPosition(), bullet.getRadius(),
                player.getPosition(), player.getRadius())) {
            player.takeDamage(bullet.getDamage());
            bullet.deactivate();
        }
    }

    // Victory / GameOver check
    if (boss.isDead()) {
        gameStats.victory = true;
        boss.clearAllBullets();
    }
    if (player.isDead()) {
        gameStats.gameOver = true;
        boss.clearAllBullets();
    }

    // Sync GameStats
    gameStats.playerHP = player.getHP();
    gameStats.playerMaxHP = player.getMaxHP();
    gameStats.playerInvincible = player.isInvincible();
    gameStats.playerShieldTimer = player.getShieldTimer();
    gameStats.playerOverdriveTimer = player.getOverdriveTimer();
    gameStats.playerDashBatteryTimer = player.getDashBatteryTimer();
    gameStats.playerIsDashing = player.isDashing();
    gameStats.playerDashCooldown = player.getDashCooldown();
    gameStats.playerSpeedMultiplier = player.getSpeedMultiplier();
    gameStats.playerFireRateMultiplier = player.getFireRateMultiplier();
    gameStats.playerDamageMultiplier = player.getDamageMultiplier();
    gameStats.bulletTimeActive = bulletTimeTimer > 0.f;
    gameStats.bossHP = boss.getHP();
    gameStats.bossMaxHP = boss.getMaxHP();
    gameStats.bossPhase = boss.getPhase();
    gameStats.bossActive = !boss.isDead();
    gameStats.currentAttack = boss.getCurrentAttackType();
    gameStats.currentAttackName = boss.getCurrentAttackName();
}

void Game::render() {
    sf::Color bgColor(10, 10, 30);
    if (gameStats.victory) bgColor = sf::Color(10, 40, 20);
    if (gameStats.gameOver) bgColor = sf::Color(40, 10, 10);
    window.clear(bgColor);

    boss.render(window);
    for (const auto& item : items) {
        item.render(window);
    }
    player.render(window);

    window.display();
}
