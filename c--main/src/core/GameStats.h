#pragma once
#include <string>
#include <array>
#include "core/Config.h"

enum class GameResult {
    Playing,
    Victory,
    GameOver
};

enum class BossAttackType {
    None,
    CircleBurst,
    SpiralShot,
    AimedShot,
    WaveBarrage,
    CrossLaser,
    RainBullets,
    FlowerBurst,
    EightDirections,
    SnakeWave,
    HomingOrbs,
    DoubleSpiral,
    BladeRing,
    ScreenWipe,
    RadialBurst,
    RandomSpray,
    PredictiveShot,
    ChargeBeam,
    TentacleSweep,
    ScreenLock,
    BulletRainFull,
    SummonMinions,
    PhaseTransition
};

// Single boss state
struct BossState {
    int hp = 0;
    int maxHP = 0;
    int phase = 1;
    bool active = false;
    std::string attackName;
    bool laserWarning = false;
    bool laserActive = false;
};

// Multi-boss state (max 3)
static constexpr int MAX_BOSS_COUNT = 3;
struct MultiBossStats {
    std::array<BossState, MAX_BOSS_COUNT> bosses;
    int activeCount = 0;
    Difficulty difficulty = Difficulty::Normal;
};

struct GameStats {
    int playerHP = 100;
    int playerMaxHP = 100;
    float elapsedTime = 0.0f;
    BossAttackType currentAttack = BossAttackType::None;
    std::string currentAttackName = "None";
    bool playerInvincible = false;
    float playerShieldTimer = 0.f;
    float playerOverdriveTimer = 0.f;
    float playerDashBatteryTimer = 0.f;
    bool playerIsDashing = false;
    float playerDashCooldown = 0.f;
    float playerSpeedMultiplier = 1.f;
    float playerFireRateMultiplier = 1.f;
    float playerDamageMultiplier = 1.f;
    bool bulletTimeActive = false;
    bool victory = false;
    bool gameOver = false;

    // Multi-boss state
    MultiBossStats multiBossStats;

    GameStats() : multiBossStats{} {}
};
