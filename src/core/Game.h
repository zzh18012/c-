#pragma once
#include <SFML/Graphics.hpp>
#include "GameStats.h"
#include "entities/Player.h"
#include "entities/Boss.h"
#include "entities/Item.h"

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    sf::RenderWindow window;
    GameStats gameStats;
    Player player;
    Boss boss;
    std::vector<Item> items;
    float itemSpawnTimer;
    float bulletTimeTimer;
    bool healSpawnedAt75;
    bool healSpawnedAt50;
    bool healSpawnedAt25;
    sf::Clock clock;
    bool running;
    bool godModeActive = false;
};
