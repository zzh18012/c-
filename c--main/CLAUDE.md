# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**Neon Bullet Arena** — A 2D action/bullet-hell game built with SFML 3.
Location: `C:\Users\zzhn2\Desktop\C++work\c--main`
Executable: `c--main\build-ninja\NeonBulletArena.exe`

## Build & Run

```bash
# Build (from c--main directory)
cmake -B build-ninja -G Ninja
cmake --build build-ninja

# Run
./build-ninja/NeonBulletArena.exe
```

## Architecture

**Game loop** (`src/core/Game.cpp`): Standard SFML loop — `processEvents() → update(dt) → render()` with delta-time capped at 0.1s.

**State machine**: `enum class GameState` — `MainMenu → Playing → Paused / Victory / GameOver`.

**Central coordinator pattern**: `Game` owns all subsystems and entities, orchestrates collision detection, phase transitions, and win/lose conditions. Other classes are largely stateless utilities.

**Entity-component separation**:
- Entities: `Player`, `Boss`, `BossBullet`, `PlayerBullet*` (6 subtypes), `Item` (12 types)
- Systems: `CollisionSystem` (circle-circle), `BackgroundSystem`, `CameraShake`, `ParticleSystem`, `AudioSystem`
- UI: `Menu`, `Button`, `HUD`

**Key design patterns**:
- Object pool for bullets (`BossBullet` max 2000) and particles (5000)
- Singleton for `AudioSystem` (procedural sound generation, no audio files)
- `Config.h` — ~200 `constexpr` constants for all gameplay values
- `GameStats` struct passed to HUD for read-only state display
- `Button::onClick` uses `std::function<void()>` callbacks

**Bullet types** (`PlayerBullet*`): `Normal`, `Spread` (5-way), `Piercing` (no-hit-deact), `Orbital` (orbits player), `Cluster` (explodes), `Homing` (tracks boss).

**Boss phases**: 3 phases triggered at 70% and 35% HP. Phase transition spawns heal-core items, particle burst, camera shake, screen flash. Each phase has its own attack pattern pool (20+ attacks total).

**Audio**: All sounds generated procedurally via `AudioSystem` — no external audio files needed. Background music slot is a placeholder.

**Rendering**: Additive blending on particles for glow effect. Camera shake offsets the entire view each frame.