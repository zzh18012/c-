#pragma once
#include <string>

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

struct GameStats {
    int playerHP = 100;
    int playerMaxHP = 100;
    int bossHP = 1000;
    int bossMaxHP = 1000;
    int bossPhase = 1;
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
    bool bossLaserWarning = false;
    bool bossLaserActive = false;
    bool bossActive = true;
    bool victory = false;
    bool gameOver = false;
};
