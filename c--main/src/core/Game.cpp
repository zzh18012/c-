#include "Game.h"
#include "Config.h"
#include "systems/CollisionSystem.h"
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cmath>

Game::Game()
    : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), WINDOW_TITLE)
    , running(true)
    , itemSpawnTimer(ITEM_SPAWN_INTERVAL)
    , bulletTimeTimer(0.f)
    , healSpawnedAt75(false)
    , healSpawnedAt50(false)
    , healSpawnedAt25(false)
    , novaRingMaxRadius(sqrt(WINDOW_WIDTH*WINDOW_WIDTH + WINDOW_HEIGHT*WINDOW_HEIGHT))
{
    window.setFramerateLimit(60);
    items.resize(ITEM_MAX_COUNT);

    // Load font - try Windows font first, then fallback
    if (!font.openFromFile("C:/Windows/Fonts/consola.ttf")) {
        (void)font.openFromFile("resources/font.ttf");
    }

    hud.init(font);
    Item::setParticleSystem(&particles);
    prevPlayerHP = player.getMaxHP();

    // Show main menu
    menu.show(MenuMode::MainMenu, font);
    menu.getButton(0) = Button("NORMAL", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
    menu.getButton(1) = Button("HARD", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
    menu.getButton(2) = Button("LUNATIC", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
    menu.getButton(3) = Button("QUIT", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
}

void Game::run() {
    while (window.isOpen() && running) {
        float dt = clock.restart().asSeconds();
        // Cap delta to avoid spiral of death
        if (dt > 0.1f) dt = 0.1f;

        processEvents();
        update(dt);
        render();
    }
}

void Game::startGame() {
    state = GameState::Playing;
    menu.setVisible(false);
    bossManager.init(currentDifficulty);
    prevPlayerHP = player.getHP();
    audio.startBackgroundMusic();
}

void Game::resetGame() {
    // Recreate player
    player = Player();
    // Reset boss manager
    bossManager = BossManager();
    // Reset items
    for (auto& item : items) item.deactivate();
    itemSpawnTimer = ITEM_SPAWN_INTERVAL;
    bulletTimeTimer = 0.f;
    healSpawnedAt75 = false;
    healSpawnedAt50 = false;
    healSpawnedAt25 = false;
    godModeActive = false;

    // Reset GameStats
    gameStats = GameStats();
    prevBossPhase = 1;
    prevPlayerHP = player.getMaxHP();

    // Clear particles
    particles = ParticleSystem();
    cameraShake = CameraShake();
}

void Game::processEvents() {
    while (auto event = window.pollEvent()) {
        if (event->getIf<sf::Event::Closed>()) {
            window.close();
        }

        // State-dependent event handling
        switch (state) {
            case GameState::MainMenu:
                menu.handleEvent(*event, window);
                break;

            case GameState::Playing:
                if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    switch (keyEvent->code) {
                        case sf::Keyboard::Key::Escape:
                            state = GameState::Paused;
                            menu.show(MenuMode::Pause, font);
                            menu.getButton(0) = Button("RESUME", font,
                                sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 320.f),
                                sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { state = GameState::Playing; menu.setVisible(false); });
                            menu.getButton(1) = Button("QUIT TO MENU", font,
                                sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 320.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                                sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                                    menu.getButton(0) = Button("NORMAL", font,
                                        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
                                        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
                                    menu.getButton(1) = Button("HARD", font,
                                        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                                        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
                                    menu.getButton(2) = Button("LUNATIC", font,
                                        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                                        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
                                    menu.getButton(3) = Button("QUIT", font,
                                        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                                        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
                                });
                            break;

                        // Debug hotkeys
                        case sf::Keyboard::Key::F1: player.heal(PLAYER_MAX_HP); break;
                        case sf::Keyboard::Key::F3:
                            godModeActive = !godModeActive;
                            player.setGodMode(godModeActive);
                            break;
                        default: break;
                    }
                }
                break;

            case GameState::Paused:
                if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyEvent->code == sf::Keyboard::Key::Escape) {
                        state = GameState::Playing;
                        menu.setVisible(false);
                    }
                }
                menu.handleEvent(*event, window);
                break;

            case GameState::Victory:
            case GameState::GameOver:
                menu.handleEvent(*event, window);
                break;
        }
    }
}

void Game::update(float dt) {
    // Always update background, particles, camera shake
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

    // === Playing state: run game logic ===

    gameStats.elapsedTime += dt;
    player.handleInput(dt, window);
    player.update(dt);
    bossManager.update(dt, player.getPosition());

    // Track previous state for effect triggers
    int playerHPBefore = player.getHP();

    // Bullet Time countdown
    if (bulletTimeTimer > 0.f) {
        bulletTimeTimer -= dt;
        if (bulletTimeTimer <= 0.f) {
            bossManager.clearAllBullets();
        }
    }

    // Screen flash countdown
    if (screenFlashTimer > 0.f) {
        screenFlashTimer -= dt;
    }

    // Overdrive subtle screen shake
    if (player.getOverdriveTimer() > 0.f) {
        static float shakeAccum = 0.f;
        shakeAccum += dt;
        if (shakeAccum >= 0.5f) {
            shakeAccum = 0.f;
            cameraShake.shake(2.f, 0.15f);
        }
    }

    // Nova ring expansion
    if (novaRingActive) {
        novaRingRadius += dt * 1200.f;
        particles.spawnNovaRing(player.getPosition(), novaRingRadius, novaRingColor, 8);
        if (novaRingRadius >= novaRingMaxRadius) {
            novaRingActive = false;
            novaRingRadius = 0.f;
        }
    }

    // Forced Heal Core spawns at HP thresholds (check lowest HP boss)
    float lowestHPRatio = 1.f;
    for (auto& b : bossManager.getBosses()) {
        if (!b.isDead()) {
            float ratio = static_cast<float>(b.getHP()) / b.getMaxHP();
            if (ratio < lowestHPRatio) lowestHPRatio = ratio;
        }
    }
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

    if (!healSpawnedAt75 && lowestHPRatio <= 0.75f) {
        healSpawnedAt75 = true;
        spawnHealCore();
    }
    if (!healSpawnedAt50 && lowestHPRatio <= 0.50f) {
        healSpawnedAt50 = true;
        spawnHealCore();
    }
    if (!healSpawnedAt25 && lowestHPRatio <= 0.25f) {
        healSpawnedAt25 = true;
        spawnHealCore();
    }

    // Normal item spawning
    itemSpawnTimer -= dt;
    if (itemSpawnTimer <= 0.f) {
        itemSpawnTimer = ITEM_SPAWN_INTERVAL;
        int activeCount = 0;
        for (const auto& item : items) if (item.isActive()) activeCount++;
        if (activeCount < ITEM_MAX_COUNT) {
            // Get max phase among alive bosses
            int maxPhase = 1;
            for (auto& b : bossManager.getBosses()) {
                if (!b.isDead() && b.getPhase() > maxPhase) maxPhase = b.getPhase();
            }
            for (auto& item : items) {
                if (!item.isActive()) {
                    int phase = maxPhase;
                    ItemType type;
                    if (phase == 1) {
                        int r = rand() % 5;
                        if (r == 0) type = ItemType::HealCore;
                        else if (r == 1) type = ItemType::DashBattery;
                        else if (r == 2) type = ItemType::WeaponEssence;
                        else if (r == 3) type = ItemType::SpeedCoil;
                        else type = ItemType::AttackModule;
                    } else if (phase == 2) {
                        int r = rand() % 5;
                        if (r == 0) type = ItemType::ShieldOrb;
                        else if (r == 1) type = ItemType::Overdrive;
                        else if (r == 2) type = ItemType::PhaseCrystal;
                        else if (r == 3) type = ItemType::WeaponEssence;
                        else type = ItemType::NovaCore;
                    } else {
                        int r = rand() % 5;
                        if (r == 0) type = ItemType::BulletTime;
                        else if (r == 1) type = ItemType::NovaBomb;
                        else if (r == 2) type = ItemType::PhaseCrystal;
                        else if (r == 3) type = ItemType::NovaCore;
                        else type = ItemType::WeaponEssence;
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

            // Spawn pickup spark
            particles.spawnHitSpark(item.getPosition(), Item::getColor(item.getType()));

            switch (item.getType()) {
                case ItemType::HealCore:
                    player.heal(static_cast<int>(player.getMaxHP() * HEAL_CORE_RATIO));
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(100, 255, 150, 120);
                    particles.spawnHealEffect(player.getPosition());
                    particles.spawnExplosion(player.getPosition(), sf::Color(60, 255, 100), 20);
                    break;
                case ItemType::ShieldOrb:
                    player.activateShieldOrb();
                    screenFlashTimer = 0.15f;
                    screenFlashColor = sf::Color(60, 140, 255, 100);
                    particles.spawnExplosion(player.getPosition(), sf::Color(60, 140, 255), 15);
                    break;
                case ItemType::Overdrive:
                    player.activateOverdrive();
                    screenFlashTimer = 0.25f;
                    screenFlashColor = sf::Color(255, 100, 40, 130);
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 100, 40), 25);
                    break;
                case ItemType::BulletTime:
                    bossManager.clearAllBullets();
                    bulletTimeTimer = BULLET_TIME_DURATION;
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(180, 60, 255, 110);
                    particles.spawnExplosion(player.getPosition(), sf::Color(180, 60, 255), 20);
                    break;
                case ItemType::NovaBomb:
                    bossManager.clearAllBullets();
                    for (auto& boss : bossManager.getBosses()) {
                        if (!boss.isDead()) boss.takeDamage(NOVA_BOMB_DAMAGE);
                    }
                    novaRingActive = true;
                    novaRingRadius = 0.f;
                    novaRingColor = sf::Color(255, 255, 100);
                    particles.spawnExplosion(item.getPosition(), sf::Color(255, 220, 50), 40);
                    cameraShake.shake(12.f, 0.6f);
                    break;
                case ItemType::DashBattery:
                    player.activateDashBattery();
                    screenFlashTimer = 0.15f;
                    screenFlashColor = sf::Color(255, 255, 60, 100);
                    particles.spawnLightning(player.getPosition(), player.getPosition() + sf::Vector2f(30.f, -30.f), sf::Color(255, 255, 60));
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 255, 60), 15);
                    break;
                case ItemType::WeaponEssence:
                    player.cycleWeapon();
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(200, 100, 255, 120);
                    particles.spawnExplosion(player.getPosition(), sf::Color(200, 100, 255), 20);
                    break;
                case ItemType::PhaseCrystal:
                    player.activatePhaseShift();
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(150, 50, 200, 120);
                    particles.spawnExplosion(player.getPosition(), sf::Color(150, 50, 200), 25);
                    break;
                case ItemType::SpeedCoil:
                    player.activateSpeedCoil();
                    screenFlashTimer = 0.15f;
                    screenFlashColor = sf::Color(100, 255, 200, 100);
                    particles.spawnExplosion(player.getPosition(), sf::Color(100, 255, 200), 15);
                    break;
                case ItemType::AttackModule:
                    player.activateAttackBoost();
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(255, 80, 80, 120);
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 80, 80), 20);
                    break;
                case ItemType::NovaCore:
                    player.activateNovaForm();
                    bossManager.clearAllBullets();
                    for (auto& boss : bossManager.getBosses()) {
                        if (!boss.isDead()) boss.takeDamage(NOVA_FORM_DAMAGE);
                    }
                    novaRingActive = true;
                    novaRingRadius = 0.f;
                    novaRingColor = sf::Color(255, 150, 50);
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 150, 50), 50);
                    cameraShake.shake(15.f, 0.8f);
                    screenFlashTimer = 0.4f;
                    screenFlashColor = sf::Color(255, 200, 100, 150);
                    break;
            }
            item.deactivate();
        }
    }

    // Player bullets vs Boss collision
    for (auto& bullet : player.getBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                player.registerHit(0);
                int dmg = static_cast<int>(bullet.getDamage() * player.getComboDamageMultiplier());
                boss.takeDamage(dmg);
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 255, 100));
                bullet.deactivate();
                break;
            }
        }
    }

    // Player spread bullets vs Boss collision
    for (auto& bullet : player.getSpreadBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage() * player.getComboDamageMultiplier());
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(0, 200, 255));
                bullet.deactivate();
                break;
            }
        }
    }

    // Player piercing bullets vs Boss collision
    for (auto& bullet : player.getPiercingBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage());
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(100, 255, 200));
            }
        }
    }

    // Player orbital bullets vs Boss collision (accumulated burst damage)
    {
        int totalOrbitalDamage = 0;
        for (auto& bullet : player.getOrbitalBullets()) {
            if (!bullet.isActive()) continue;
            for (auto& boss : bossManager.getBosses()) {
                if (boss.isDead()) continue;
                if (CollisionSystem::checkCircleCollision(
                        bullet.getPosition(), bullet.getRadius(),
                        boss.getPosition(), boss.getRadius())) {
                    totalOrbitalDamage += bullet.getDamage();
                    totalOrbitalDamage += bullet.getOrbitCount() * 5;
                    bullet.resetOrbitCount();
                    bullet.deactivate();
                    break;
                }
            }
        }
        if (totalOrbitalDamage > 0) {
            for (auto& boss : bossManager.getBosses()) {
                if (!boss.isDead()) {
                    boss.takeDamage(totalOrbitalDamage);
                    break;
                }
            }
            audio.playHit();
        }
    }

    // Player cluster bullets vs Boss collision
    for (auto& bullet : player.getClusterBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage());
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 150, 50));
                bullet.deactivate();
                break;
            }
        }
    }

    // Player homing bullets vs Boss collision
    for (auto& bullet : player.getHomingBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage());
                boss.applySlow(1);
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 100, 200));
                bullet.deactivate();
                break;
            }
        }
    }

    // Boss bullets vs Player collision
    for (auto& boss : bossManager.getBosses()) {
        if (boss.isDead()) continue;
        for (auto& bullet : boss.getBullets()) {
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

    // Boss phase change trigger
    for (auto& boss : bossManager.getBosses()) {
        if (boss.isDead()) continue;
        int currentPhase = boss.getPhase();
        if (currentPhase > prevBossPhase) {
            particles.spawnPhaseTransition(boss.getPosition());
            particles.spawnExplosion(boss.getPosition(), sf::Color(255, 255, 255, 200), 25);
            cameraShake.shake(8.f, 0.4f);
            screenFlashTimer = 0.3f;
            screenFlashColor = sf::Color(255, 200, 220, 150);
            prevBossPhase = currentPhase;
            break;
        }
    }

    // Player hit trigger
    if (player.getHP() < playerHPBefore) {
        particles.spawnHitSpark(player.getPosition(), sf::Color(255, 80, 60));
        cameraShake.shake(5.f, 0.3f);
    }

    // Victory / GameOver check
    if (bossManager.isAllBossesDead() && state == GameState::Playing) {
        gameStats.victory = true;
        state = GameState::Victory;
        bossManager.clearAllBullets();
        for (auto& boss : bossManager.getBosses()) {
            if (!boss.isDead()) {
                particles.spawnExplosion(boss.getPosition(), sf::Color(255, 140, 30), 60);
                break;
            }
        }
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
                menu.getButton(0) = Button("NORMAL", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
                menu.getButton(1) = Button("HARD", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
                menu.getButton(2) = Button("LUNATIC", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
                menu.getButton(3) = Button("QUIT", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
            });
    }
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
                menu.getButton(0) = Button("NORMAL", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
                menu.getButton(1) = Button("HARD", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
                menu.getButton(2) = Button("LUNATIC", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
                menu.getButton(3) = Button("QUIT", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), [this]() { window.close(); });
            });
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

    // Multi-boss stats sync
    gameStats.multiBossStats.activeCount = bossManager.getActiveBossCount();
    int idx = 0;
    for (auto& boss : bossManager.getBosses()) {
        if (!boss.isDead() && idx < MAX_BOSS_COUNT) {
            gameStats.multiBossStats.bosses[idx].hp = boss.getHP();
            gameStats.multiBossStats.bosses[idx].maxHP = boss.getMaxHP();
            gameStats.multiBossStats.bosses[idx].phase = boss.getPhase();
            gameStats.multiBossStats.bosses[idx].active = true;
            gameStats.multiBossStats.bosses[idx].attackName = boss.getCurrentAttackName();
            gameStats.multiBossStats.bosses[idx].laserWarning = boss.isLaserWarning();
            gameStats.multiBossStats.bosses[idx].laserActive = boss.isLaserActive();
            idx++;
        }
    }
    gameStats.multiBossStats.difficulty = currentDifficulty;

    // Update previous trackers
    prevPlayerHP = player.getHP();

    // Update HUD
    hud.update(gameStats);
}

void Game::render() {
    sf::Color bgColor(10, 10, 30);
    if (gameStats.victory) bgColor = sf::Color(10, 40, 20);
    if (gameStats.gameOver) bgColor = sf::Color(40, 10, 10);
    window.clear(bgColor);

    // Layer 1: Background
    background.render(window);

    // Layer 2: Apply camera shake via sf::View
    sf::View originalView = window.getView();
    sf::View shakeView = originalView;
    shakeView.setCenter(sf::Vector2f(WINDOW_WIDTH / 2.f + cameraShake.getOffset().x,
                        WINDOW_HEIGHT / 2.f + cameraShake.getOffset().y));
    window.setView(shakeView);

    // Layer 3: Game entities
    bossManager.render(window);
    for (const auto& item : items) {
        item.render(window);
    }
    player.render(window);

    // Layer 4: Particles (with additive blend for glow)
    particles.render(window);

    // Nova Ring effect
    if (novaRingActive) {
        float alpha = std::fmaxf(0.f, 1.f - novaRingRadius / novaRingMaxRadius);
        sf::CircleShape novaRing(novaRingRadius);
        novaRing.setOrigin(sf::Vector2f(novaRingRadius, novaRingRadius));
        novaRing.setPosition(player.getPosition());
        novaRing.setFillColor(sf::Color(255, 255, 200, static_cast<std::uint8_t>(alpha * 30.f)));
        novaRing.setOutlineColor(sf::Color(255, 255, 100, static_cast<std::uint8_t>(alpha * 200.f)));
        novaRing.setOutlineThickness(4.f);
        window.draw(novaRing);
    }

    // Restore view
    window.setView(originalView);

    // Layer 5: HUD
    hud.render(window);

    // Layer 6: Menu (topmost)
    menu.render(window);

    // Screen flash overlay
    if (screenFlashTimer > 0.f) {
        float alpha = screenFlashTimer / 0.25f;
        if (alpha > 1.f) alpha = 1.f;
        sf::RectangleShape flash(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        flash.setPosition(sf::Vector2f(0.f, 0.f));
        sf::Color c = screenFlashColor;
        c.a = static_cast<std::uint8_t>(c.a * alpha);
        flash.setFillColor(c);
        window.draw(flash);
    }

    // Bullet Time vignette (purple darkening at edges)
    if (bulletTimeTimer > 0.f) {
        float alpha = 0.3f * (bulletTimeTimer / BULLET_TIME_DURATION);
        if (alpha > 0.3f) alpha = 0.3f;
        // Create vignette with 4 corner rectangles
        sf::RectangleShape vigTop(sf::Vector2f(WINDOW_WIDTH, 100.f));
        vigTop.setPosition(sf::Vector2f(0.f, 0.f));
        vigTop.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        sf::RectangleShape vigBottom(sf::Vector2f(WINDOW_WIDTH, 100.f));
        vigBottom.setPosition(sf::Vector2f(0.f, WINDOW_HEIGHT - 100.f));
        vigBottom.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        sf::RectangleShape vigLeft(sf::Vector2f(100.f, WINDOW_HEIGHT - 200.f));
        vigLeft.setPosition(sf::Vector2f(0.f, 100.f));
        vigLeft.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        sf::RectangleShape vigRight(sf::Vector2f(100.f, WINDOW_HEIGHT - 200.f));
        vigRight.setPosition(sf::Vector2f(WINDOW_WIDTH - 100.f, 100.f));
        vigRight.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        window.draw(vigTop);
        window.draw(vigBottom);
        window.draw(vigLeft);
        window.draw(vigRight);
    }

    window.display();
}
