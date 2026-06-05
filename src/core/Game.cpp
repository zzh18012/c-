#include "Game.h"
#include "Config.h"
#include "systems/CollisionSystem.h"
#include <cstdlib>
#include <cstdio>

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

    // Load font
    if (!font.loadFromFile("resources/font.ttf")) {
        font.loadFromFile("C:/Windows/Fonts/consola.ttf");
    }

    hud.init(font);
    prevBossHP = BOSS_MAX_HP;
    prevPlayerHP = PLAYER_MAX_HP;

    // Show main menu
    menu.show(MenuMode::MainMenu, font);
    menu.getButton(0) = Button("START GAME", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { startGame(); });
    menu.getButton(1) = Button("QUIT", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
}

void Game::run() {
    while (window.isOpen() && running) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;
        processEvents();
        update(dt);
        render();
    }
}

void Game::startGame() {
    state = GameState::Playing;
    menu.setVisible(false);

    bossManager.init(difficulty);
    audio.startBackgroundMusic();

    // Track primary boss for HUD / thresholds
    auto& bosses = bossManager.getBosses();
    prevBossHP = bosses[0].getHP();
    prevBossPhase = bosses[0].getPhase();
    prevPlayerHP = player.getHP();
}

void Game::resetGame() {
    player = Player();
    bossManager = BossManager();
    for (auto& item : items) item.deactivate();
    itemSpawnTimer = ITEM_SPAWN_INTERVAL;
    bulletTimeTimer = 0.f;
    healSpawnedAt75 = false;
    healSpawnedAt50 = false;
    healSpawnedAt25 = false;
    godModeActive = false;

    gameStats = GameStats();
    prevBossHP = BOSS_MAX_HP;
    prevBossPhase = 1;
    prevPlayerHP = PLAYER_MAX_HP;

    particles = ParticleSystem();
    cameraShake = CameraShake();
    audio.stopBackgroundMusic();
}

// ---- helper: get primary (first active) boss ----
static Boss& primaryBoss(BossManager& mgr) { return mgr.getBosses()[0]; }

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        switch (state) {
            case GameState::MainMenu:
                menu.handleEvent(event, window);
                break;

            case GameState::Playing:
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            state = GameState::Paused;
                            audio.stopBackgroundMusic();
                            menu.show(MenuMode::Pause, font);
                            menu.getButton(0) = Button("RESUME", font,
                                sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 320.f),
                                sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { state = GameState::Playing; menu.setVisible(false); audio.startBackgroundMusic(); });
                            menu.getButton(1) = Button("QUIT TO MENU", font,
                                sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 320.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                                sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                                    menu.getButton(0) = Button("START GAME", font,
                                        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f),
                                        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { startGame(); });
                                    menu.getButton(1) = Button("QUIT", font,
                                        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                                        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
                                });
                            break;

                        // Debug hotkeys
                        case sf::Keyboard::F1: player.heal(PLAYER_MAX_HP); break;
                        case sf::Keyboard::F2: {
                            auto& bs = bossManager.getBosses();
                            for (auto& b : bs) if (!b.isDead()) b.takeDamage(b.getHP() * 0.4f);
                            break;
                        }
                        case sf::Keyboard::F3:
                            godModeActive = !godModeActive;
                            player.setGodMode(godModeActive);
                            break;
                        case sf::Keyboard::F4: bossManager.clearAllBullets(); break;
                        case sf::Keyboard::F5: {
                            auto& bs = bossManager.getBosses();
                            for (auto& b : bs) if (!b.isDead()) b.takeDamage(b.getHP() / 2);
                            break;
                        }
                        case sf::Keyboard::F6: {
                            auto& bs = bossManager.getBosses();
                            for (auto& b : bs) if (!b.isDead()) b.takeDamage(b.getHP());
                            break;
                        }
                        default: break;
                    }
                }
                break;

            case GameState::Paused:
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                    state = GameState::Playing;
                    menu.setVisible(false);
                    audio.startBackgroundMusic();
                }
                menu.handleEvent(event, window);
                break;

            case GameState::Victory:
            case GameState::GameOver:
                menu.handleEvent(event, window);
                break;
        }
    }
}

void Game::update(float dt) {
    background.update(dt);
    particles.update(dt);
    cameraShake.update(dt);

    if (state == GameState::MainMenu || state == GameState::Paused) {
        menu.update(dt);
        return;
    }

    if (state == GameState::Victory || state == GameState::GameOver) {
        menu.update(dt);
        hud.update(gameStats);
        return;
    }

    // === Playing state ===
    gameStats.elapsedTime += dt;
    player.handleInput(dt, window);
    player.update(dt);
    bossManager.update(dt, player.getPosition());

    // Primary boss reference for phase / HP / HUD
    Boss& boss = primaryBoss(bossManager);

    int bossHPBefore = boss.getHP();
    int bossPhaseBefore = boss.getPhase();
    int playerHPBefore = player.getHP();

    // Bullet Time countdown — apply to all bosses
    if (bulletTimeTimer > 0.f) {
        bulletTimeTimer -= dt;
        if (bulletTimeTimer <= 0.f) {
            for (auto& b : bossManager.getBosses()) b.setBulletSpeedMultiplier(1.f);
        }
    }

    // Forced Heal Core spawns at HP thresholds (primary boss)
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

    if (!healSpawnedAt75 && hpRatio <= 0.75f) { healSpawnedAt75 = true; spawnHealCore(); }
    if (!healSpawnedAt50 && hpRatio <= 0.50f) { healSpawnedAt50 = true; spawnHealCore(); }
    if (!healSpawnedAt25 && hpRatio <= 0.25f) { healSpawnedAt25 = true; spawnHealCore(); }

    // Normal item spawning
    itemSpawnTimer -= dt;
    if (itemSpawnTimer <= 0.f) {
        itemSpawnTimer = ITEM_SPAWN_INTERVAL;
        int activeCount = 0;
        for (const auto& item : items) if (item.isActive()) activeCount++;
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
    for (auto& item : items) item.update(dt);

    // Player-Item collision
    for (auto& item : items) {
        if (!item.isActive()) continue;
        if (CollisionSystem::checkCircleCollision(
                player.getPosition(), player.getRadius(),
                item.getPosition(), item.getRadius())) {

            particles.spawnHitSpark(item.getPosition(), Item::getColor(item.getType()));

            switch (item.getType()) {
                case ItemType::HealCore:
                    player.heal(static_cast<int>(player.getMaxHP() * HEAL_CORE_RATIO));
                    audio.playPickup();
                    break;
                case ItemType::ShieldOrb:
                    player.activateShieldOrb();
                    audio.playShield();
                    break;
                case ItemType::Overdrive:
                    player.activateOverdrive();
                    audio.playOverdrive();
                    break;
                case ItemType::BulletTime:
                    for (auto& b : bossManager.getBosses()) b.setBulletSpeedMultiplier(BULLET_TIME_SLOW_RATIO);
                    bulletTimeTimer = BULLET_TIME_DURATION;
                    audio.playPickup();
                    break;
                case ItemType::NovaBomb:
                    bossManager.clearAllBullets();
                    for (auto& b : bossManager.getBosses()) if (!b.isDead()) b.takeDamage(NOVA_BOMB_DAMAGE);
                    particles.spawnExplosion(item.getPosition(), sf::Color(255, 220, 50), 40);
                    cameraShake.shake(10.f, 0.5f);
                    audio.playHit();
                    break;
                case ItemType::DashBattery:
                    player.activateDashBattery();
                    audio.playDash();
                    break;
            }
            item.deactivate();
        }
    }

    // Player bullets vs ALL Bosses collision
    for (auto& bossB : bossManager.getBosses()) {
        if (bossB.isDead()) continue;
        for (auto& bullet : player.getBullets()) {
            if (!bullet.isActive()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    bossB.getPosition(), bossB.getRadius())) {
                bossB.takeDamage(bullet.getDamage());
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 255, 100));
                audio.playShoot();
                bullet.deactivate();
            }
        }
    }

    // ALL Boss bullets vs Player collision
    for (auto& bossB : bossManager.getBosses()) {
        if (bossB.isDead()) continue;
        for (auto& bullet : bossB.getBullets()) {
            if (!bullet.isActive()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    player.getPosition(), player.getRadius())) {
                player.takeDamage(bullet.getDamage());
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 60, 40));
                bullet.deactivate();
            }
        }
    }

    // Boss phase change trigger (check primary)
    if (boss.getPhase() != bossPhaseBefore && boss.getPhase() > bossPhaseBefore) {
        particles.spawnExplosion(boss.getPosition(), sf::Color(255, 255, 255, 200), 25);
        cameraShake.shake(8.f, 0.4f);
        audio.playBossHurt();
    }

    // Player hit trigger
    if (player.getHP() < playerHPBefore) {
        particles.spawnHitSpark(player.getPosition(), sf::Color(255, 80, 60));
        cameraShake.shake(5.f, 0.3f);
        audio.playPlayerHurt();
    }

    // Victory check
    if (bossManager.isAllBossesDead() && state == GameState::Playing) {
        gameStats.victory = true;
        state = GameState::Victory;
        bossManager.clearAllBullets();
        particles.spawnExplosion(boss.getPosition(), sf::Color(255, 140, 30), 60);
        cameraShake.shake(15.f, 0.8f);
        audio.stopBackgroundMusic();
        audio.playVictory();
        menu.show(MenuMode::Victory, font);
        menu.getButton(0) = Button("PLAY AGAIN", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { resetGame(); startGame(); });
        menu.getButton(1) = Button("MAIN MENU", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                menu.getButton(0) = Button("START GAME", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { startGame(); });
                menu.getButton(1) = Button("QUIT", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
            });
    }

    // GameOver check
    if (player.isDead() && state == GameState::Playing) {
        gameStats.gameOver = true;
        state = GameState::GameOver;
        bossManager.clearAllBullets();
        particles.spawnExplosion(player.getPosition(), sf::Color(255, 50, 40), 30);
        cameraShake.shake(10.f, 0.6f);
        audio.stopBackgroundMusic();
        audio.playGameOver();
        menu.show(MenuMode::GameOver, font);
        menu.getButton(0) = Button("RETRY", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { resetGame(); startGame(); });
        menu.getButton(1) = Button("MAIN MENU", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                menu.getButton(0) = Button("START GAME", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { startGame(); });
                menu.getButton(1) = Button("QUIT", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 300.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
            });
    }

    // Sync GameStats from primary boss
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
    gameStats.bossLaserWarning = boss.isLaserWarning();
    gameStats.bossLaserActive = boss.isLaserActive();
    gameStats.bossActive = !boss.isDead();
    gameStats.currentAttack = boss.getCurrentAttackType();
    gameStats.currentAttackName = boss.getCurrentAttackName();

    prevBossHP = boss.getHP();
    prevBossPhase = boss.getPhase();
    prevPlayerHP = player.getHP();

    hud.update(gameStats);
}

void Game::render() {
    sf::Color bgColor(10, 10, 30);
    if (gameStats.victory) bgColor = sf::Color(10, 40, 20);
    if (gameStats.gameOver) bgColor = sf::Color(40, 10, 10);
    window.clear(bgColor);

    background.render(window);

    if (state == GameState::MainMenu || state == GameState::Paused ||
        state == GameState::Victory || state == GameState::GameOver) {
        menu.render(window);
    }

    if (state != GameState::MainMenu) {
        bossManager.render(window);
        for (const auto& item : items) item.render(window);
        player.render(window);
        particles.render(window);
        hud.render(window);
    }

    window.display();
}
