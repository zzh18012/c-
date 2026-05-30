#pragma once
#include <SFML/Graphics.hpp>
#include "GameStats.h"
#include "entities/Player.h"
#include "entities/BossManager.h"
#include "entities/Item.h"
#include "systems/BackgroundSystem.h"
#include "systems/ParticleSystem.h"
#include "systems/CameraShake.h"
#include "systems/AudioSystem.h"
#include "ui/HUD.h"
#include "ui/Menu.h"

enum class GameState { MainMenu, Playing, Paused, Victory, GameOver };

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void resetGame();
    void startGame();

    sf::RenderWindow window;
    sf::Font font;

    GameState state = GameState::MainMenu;
    GameStats gameStats;

    Player player;
    BossManager bossManager;
    std::vector<Item> items;
    float itemSpawnTimer;
    float bulletTimeTimer;
    bool healSpawnedAt75;
    bool healSpawnedAt50;
    bool healSpawnedAt25;
    sf::Clock clock;
    bool running;
    bool godModeActive = false;

    BackgroundSystem background;
    ParticleSystem particles;
    CameraShake cameraShake;
    AudioSystem audio;
    HUD hud;
    Menu menu;

    // Track previous state for effect triggers
    int prevBossPhase = 1;
    int prevPlayerHP = 0;

    Difficulty currentDifficulty = Difficulty::Normal;

    // Screen flash effect
    float screenFlashTimer = 0.f;
    sf::Color screenFlashColor = sf::Color::White;

    // Nova Ring effect
    float novaRingRadius = 0.f;
    float novaRingMaxRadius = 0.f;
    bool novaRingActive = false;
    sf::Color novaRingColor = sf::Color::White;
};
