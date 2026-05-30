#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

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

// UI - Button
constexpr float BUTTON_WIDTH = 260.f;
constexpr float BUTTON_HEIGHT = 54.f;
constexpr float BUTTON_BORDER_THICKNESS = 2.f;

// UI - Menu
constexpr float MENU_TITLE_SIZE = 48.f;
constexpr float MENU_BUTTON_SPACING = 20.f;
constexpr float MENU_OVERLAY_ALPHA = 160.f;

// UI - HUD
constexpr float HUD_PADDING = 20.f;
constexpr float HUD_BAR_HEIGHT = 22.f;
constexpr float HUD_PLAYER_BAR_WIDTH = 280.f;
constexpr float HUD_BOSS_BAR_WIDTH = 420.f;
constexpr float HUD_FONT_SIZE_SMALL = 14;
constexpr float HUD_FONT_SIZE_NORMAL = 18;
constexpr float HUD_FONT_SIZE_LARGE = 36;
constexpr float HUD_BAR_LERP_SPEED = 8.f;

// Background
constexpr int BG_AMBIENT_DOT_COUNT = 35;
constexpr float BG_GRID_ALPHA = 60.f;
constexpr float BG_DOT_MIN_RADIUS = 2.5f;
constexpr float BG_DOT_MAX_RADIUS = 5.f;
constexpr float BG_DOT_DRIFT_SPEED = 25.f;
constexpr float BG_DOT_PULSE_SPEED = 1.5f;

// === NEW PLAYER BULLET TYPES ===
constexpr float PLAYER_SPREAD_ANGLE = 30.f;
constexpr float PLAYER_PIERCING_SPEED = 1200.f;
constexpr int PLAYER_PIERCING_DAMAGE = 15;
constexpr float PLAYER_ORBITAL_SPEED = 180.f;
constexpr float PLAYER_ORBITAL_RADIUS = 60.f;
constexpr int ORBITAL_BULLET_DAMAGE = 15;
constexpr float PLAYER_CLUSTER_SPEED = 400.f;
constexpr int PLAYER_HOMING_DAMAGE = 18;
constexpr float HOMING_TURN_RATE = 0.03f;

// === NEW BOSS BULLET PATTERNS ===
constexpr float FLOWER_INTERVAL = 0.8f;
constexpr float FLOWER_DURATION = 4.0f;
constexpr int FLOWER_BURST_COUNT = 24;
constexpr int FLOWER_LAYERS = 3;
constexpr float EIGHT_DIR_INTERVAL = 0.4f;
constexpr float EIGHT_DIR_DURATION = 4.0f;
constexpr float SNAKE_INTERVAL = 0.1f;
constexpr float SNAKE_DURATION = 5.0f;
constexpr float SNAKE_SPEED = 200.f;
constexpr float SNAKE_AMPLITUDE = 80.f;
constexpr float SNAKE_FREQUENCY = 3.f;
constexpr float HOMING_ORB_INTERVAL = 0.8f;
constexpr float HOMING_ORB_DURATION = 4.0f;
constexpr float HOMING_ORB_SPEED = 120.f;
constexpr float DOUBLE_SPIRAL_INTERVAL = 0.05f;
constexpr float DOUBLE_SPIRAL_DURATION = 5.0f;
constexpr float BLADE_RING_INTERVAL = 0.15f;
constexpr float BLADE_RING_DURATION = 4.0f;
constexpr float BLADE_RING_SPEED = 250.f;
constexpr float SCREEN_WIPE_INTERVAL = 0.08f;
constexpr float SCREEN_WIPE_DURATION = 3.0f;
constexpr float SCREEN_WIPE_SPEED = 400.f;
constexpr float RADIAL_BURST_INTERVAL = 0.3f;
constexpr float RADIAL_BURST_DURATION = 4.0f;
constexpr float RANDOM_SPRAY_INTERVAL = 0.05f;
constexpr float RANDOM_SPRAY_DURATION = 3.0f;
constexpr float PREDICTIVE_INTERVAL = 1.5f;
constexpr float PREDICTIVE_DURATION = 4.0f;
constexpr float PREDICTIVE_SHOT_SPEED = 600.f;

// === NEW BOSS ATTACKS ===
constexpr float CHARGE_BEAM_WARNING = 1.5f;
constexpr float CHARGE_BEAM_DURATION = 1.0f;
constexpr int CHARGE_BEAM_DAMAGE = 50;
constexpr float TENTACLE_SWEEP_DURATION = 2.0f;
constexpr float SCREEN_LOCK_DURATION = 3.0f;
constexpr float BULLET_RAIN_FULL_INTERVAL = 0.03f;
constexpr float BULLET_RAIN_FULL_DURATION = 3.0f;
constexpr float MINION_SPAWN_INTERVAL = 2.0f;
constexpr int MINION_DAMAGE = 20;
constexpr float PHASE_TRANSITION_DURATION = 2.0f;
constexpr int MAX_MINIONS = 5;

// === NEW PLAYER FORMS ===
constexpr float PHASE_SHIFT_DURATION = 1.0f;
constexpr float PHASE_SHIFT_TRIGGER_CHANCE = 0.35f;
constexpr float NOVA_FORM_DURATION = 0.5f;
constexpr int NOVA_FORM_DAMAGE = 100;
constexpr float NOVA_FORM_RADIUS = 500.f;
constexpr float SPEED_COIL_DURATION = 8.0f;
constexpr float SPEED_COIL_MULT = 1.5f;
constexpr float ATTACK_MODULE_DURATION = 10.0f;
constexpr float ATTACK_MODULE_MULT = 1.3f;

// === PLAYER WEAPON SYSTEM ===
constexpr float PLAYER_SPREAD_COOLDOWN = 0.3f;
constexpr float PLAYER_PIERCING_COOLDOWN = 0.5f;
constexpr float PLAYER_ORBITAL_COOLDOWN = 0.1f;
constexpr float PLAYER_CLUSTER_COOLDOWN = 0.4f;
constexpr float PLAYER_HOMING_COOLDOWN = 0.6f;
constexpr int PLAYER_SPREAD_COUNT = 5;
constexpr float PLAYER_CLUSTER_BURST_COUNT = 8;

// === WEAPON SPECIAL EFFECTS ===
constexpr int COMBO_THRESHOLD = 3;
constexpr float COMBO_DAMAGE_MULT = 1.5f;
constexpr float COMBO_TIMEOUT = 2.f;
constexpr float SPREAD_BOUNCE_DAMAGE_MULT = 0.7f;
constexpr float PIERCE_CHAIN_DAMAGE_RATIO = 0.5f;
constexpr float PIERCE_CHAIN_RADIUS = 80.f;
constexpr int ORBITAL_MAX_DAMAGE = 60;
constexpr int CLUSTER_SPLIT_COUNT = 2;
constexpr float CLUSTER_SPLIT_DAMAGE_RATIO = 0.3f;
constexpr float HOMING_SLOW_STACK = 0.1f;
constexpr float HOMING_SLOW_DURATION = 2.f;
constexpr int HOMING_SLOW_MAX_STACKS = 3;

// === 难度模式 ===
enum class Difficulty { Normal, Hard, Lunatic };

// 难度配置
constexpr int HARD_BOSS_COUNT = 2;
constexpr int LUNATIC_BOSS_COUNT = 3;
constexpr float BOSS_SPAWN_HP_THRESHOLD = 0.5f;  // Boss A血量50%时触发后续BOSS入场

// Boss名称颜色（用于HUD）
const sf::Color BOSS_NAME_COLORS[3] = {
    sf::Color(255, 80, 40),    // 烈焰恶魔 - 红橙
    sf::Color(180, 80, 255),   // 虚空幽灵 - 紫蓝
    sf::Color(255, 220, 60)    // 雷霆泰坦 - 金黄
};

// Boss位置随机范围
constexpr float BOSS_SPAWN_MIN_X = 200.f;
constexpr float BOSS_SPAWN_MAX_X = WINDOW_WIDTH - 200.f;
constexpr float BOSS_SPAWN_MIN_Y = 100.f;
constexpr float BOSS_SPAWN_MAX_Y = 250.f;
