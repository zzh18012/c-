#pragma once
#include <SFML/System/Vector2.hpp>

// Window
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr const char* WINDOW_TITLE = "Neon Bullet Arena";

// Player
constexpr float PLAYER_SPEED = 300.f;
constexpr float PLAYER_RADIUS = 18.f;
constexpr int PLAYER_MAX_HP = 100;
constexpr float PLAYER_SHOOT_COOLDOWN = 0.15f;
constexpr int PLAYER_BULLET_DAMAGE = 10;
constexpr float PLAYER_BULLET_SPEED = 800.f;
constexpr float PLAYER_INVINCIBLE_TIME = 1.5f;
constexpr float PLAYER_DASH_SPEED = 900.f;
constexpr float PLAYER_DASH_DURATION = 0.15f;
constexpr float PLAYER_DASH_COOLDOWN = 2.0f;
const sf::Vector2f PLAYER_START_POS(640.f, 550.f);

// Boss
constexpr float BOSS_RADIUS = 55.f;
constexpr int BOSS_MAX_HP = 1000;
const sf::Vector2f BOSS_POS(640.f, 180.f);

// Boss Phases
constexpr float PHASE2_HP_RATIO = 0.70f;
constexpr float PHASE3_HP_RATIO = 0.35f;

// Boss Bullets
constexpr float BOSS_BULLET_SPEED_SLOW = 180.f;
constexpr float BOSS_BULLET_SPEED_NORMAL = 240.f;
constexpr float BOSS_BULLET_SPEED_FAST = 320.f;
constexpr int BOSS_BULLET_DAMAGE = 15;
constexpr float BOSS_BULLET_RADIUS = 8.f;
constexpr int MAX_BOSS_BULLETS = 2000;

// Player Bullets
constexpr int MAX_PLAYER_BULLETS = 200;
constexpr float PLAYER_BULLET_RADIUS = 5.f;

// Attack timing (seconds)
constexpr float CIRCLE_BURST_INTERVAL = 0.6f;
constexpr float CIRCLE_BURST_DURATION = 4.0f;
constexpr int CIRCLE_BURST_COUNT = 24;

constexpr float SPIRAL_INTERVAL = 0.05f;
constexpr float SPIRAL_DURATION = 5.0f;
constexpr int SPIRAL_BULLETS_PER_SHOT = 4;

constexpr float AIMED_INTERVAL = 0.4f;
constexpr float AIMED_DURATION = 4.0f;
constexpr float AIMED_ANGLE_OFFSET = 8.f;

constexpr float WAVE_INTERVAL = 0.15f;
constexpr float WAVE_DURATION = 5.0f;
constexpr float WAVE_AMPLITUDE = 60.f;
constexpr float WAVE_FREQUENCY = 3.f;

constexpr float CROSS_LASER_WARNING_TIME = 1.0f;
constexpr float CROSS_LASER_ACTIVE_TIME = 0.5f;
constexpr int CROSS_LASER_REPEATS = 3;
constexpr float LASER_WIDTH = 40.f;

constexpr float RAIN_INTERVAL = 0.08f;
constexpr float RAIN_DURATION = 5.0f;

// Items
constexpr int ITEM_MAX_COUNT = 2;
constexpr float ITEM_SPAWN_INTERVAL = 10.f;
constexpr float ITEM_RADIUS = 14.f;
constexpr float ITEM_LIFETIME = 8.f;

// Heal Core
constexpr float HEAL_CORE_RATIO = 0.25f;

// Shield Orb
constexpr float SHIELD_ORB_DURATION = 3.f;

// Overdrive
constexpr float OVERDRIVE_DURATION = 6.f;
constexpr float OVERDRIVE_FIRE_RATE_MULT = 0.5f;
constexpr float OVERDRIVE_DAMAGE_MULT = 1.3f;

// Bullet Time
constexpr float BULLET_TIME_DURATION = 5.f;
constexpr float BULLET_TIME_SLOW_RATIO = 0.5f;

// Nova Bomb
constexpr int NOVA_BOMB_DAMAGE = 150;

// Dash Battery
constexpr float DASH_BATTERY_DURATION = 4.f;
constexpr float DASH_BATTERY_SPEED_MULT = 1.5f;

// Visual
constexpr int PARTICLE_POOL_SIZE = 5000;
