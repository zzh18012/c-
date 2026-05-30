# 困难/超困难模式实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 为游戏增加困难和超困难模式，支持2-3只不同外观/攻击风格的BOSS同时战斗

**Architecture:**
- 新建 `BossManager` 类管理多个BOSS实例，支持按难度模式初始化不同组合
- 扩展 `Boss` 类添加 `BossType` 枚举，支持三种BOSS类型，每种有独立外观、光环、出场动画、弹幕
- 修改 `HUD` 支持动态多血条显示（2-3根）
- 修改 `Game` 和 `Menu` 支持难度选择和入场逻辑

**Tech Stack:** C++17, SFML 3, 无外部依赖

---

## 文件结构

```
src/
  entities/
    Boss.h / Boss.cpp          - 扩展支持3种BOSS类型
    BossManager.h / .cpp        - 新建：管理多BOSS实例
  core/
    Game.h / Game.cpp           - 修改：支持难度模式和入场逻辑
    GameStats.h                 - 修改：支持多BOSS状态
  ui/
    HUD.h / HUD.cpp             - 修改：多血条显示
    Menu.h / Menu.cpp           - 修改：难度选择菜单
  core/Config.h                 - 修改：新增难度枚举和相关配置
```

---

## 任务列表

### Task 1: 添加难度枚举和配置

**Files:**
- Modify: `src/core/Config.h`

- [ ] **Step 1: 添加难度枚举和配置常量**

在 `Config.h` 末尾添加：

```cpp
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
```

- [ ] **Step 2: 提交**

```bash
git add src/core/Config.h
git commit -m "feat: add difficulty enum and config constants"
```

---

### Task 2: 扩展GameStats支持多BOSS

**Files:**
- Modify: `src/core/GameStats.h`

- [ ] **Step 1: 添加多BOSS状态结构**

在 `GameStats.h` 中替换Boss相关字段：

```cpp
// 单个BOSS状态
struct BossState {
    int hp = 0;
    int maxHP = 0;
    int phase = 1;
    bool active = false;
    std::string attackName;
    bool laserWarning = false;
    bool laserActive = false;
};

// 多BOSS状态（最大3只）
static constexpr int MAX_BOSS_COUNT = 3;
struct MultiBossStats {
    std::array<BossState, MAX_BOSS_COUNT> bosses;
    int activeCount = 0;
    Difficulty difficulty = Difficulty::Normal;
};

// 替换原有单BOSS字段
MultiBossStats multiBossStats;
```

修改构造函数 `GameStats()` 初始化 `multiBossStats`。

- [ ] **Step 2: 提交**

```bash
git add src/core/GameStats.h
git commit -m "feat: extend GameStats for multi-boss support"
```

---

### Task 3: 创建BossManager类

**Files:**
- Create: `src/entities/BossManager.h`
- Create: `src/entities/BossManager.cpp`

- [ ] **Step 1: 创建BossManager.h**

```cpp
#pragma once
#include "Boss.h"
#include "core/Config.h"
#include <array>
#include <vector>

class BossManager {
public:
    BossManager();
    ~BossManager();

    void init(Difficulty difficulty);
    void update(float dt, const sf::Vector2f& playerPosition);
    void render(sf::RenderWindow& window) const;

    // 状态查询
    int getActiveBossCount() const;
    bool isAllBossesDead() const;
    bool shouldSpawnNextWave() const;
    void onBossDefeated(int index);
    void triggerSecondWave();

    // 获取BOSS（用于碰撞检测）
    std::array<Boss, MAX_BOSS_COUNT>& getBosses() { return bosses; }
    const std::array<Boss, MAX_BOSS_COUNT>& getBosses() const { return bosses; }

    // 清除所有BOSS弹幕
    void clearAllBullets();

    // 获取指定BOSS的位置（用于玩家弹幕碰撞）
    std::vector<sf::Vector2f> getAllBossPositions() const;
    std::vector<float> getAllBossRadii() const;

private:
    std::array<Boss, MAX_BOSS_COUNT> bosses;
    std::array<bool, MAX_BOSS_COUNT> bossActive;
    int activeBossCount;
    Difficulty difficulty;
    bool secondWaveSpawned;
    bool secondWaveTriggered;
    sf::Clock spawnTimer;
    std::array<float, MAX_BOSS_COUNT> spawnProgress;  // 入场动画进度

    void spawnBoss(int index, sf::Vector2f pos);
    sf::Vector2f getRandomSpawnPosition(int bossIndex) const;
};
```

- [ ] **Step 2: 创建BossManager.cpp**

```cpp
#include "BossManager.h"
#include "core/Config.h"
#include <cstdlib>

BossManager::BossManager()
    : activeBossCount(0), difficulty(Difficulty::Normal)
    , secondWaveSpawned(false), secondWaveTriggered(false) {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        bossActive[i] = false;
        spawnProgress[i] = 0.f;
    }
}

BossManager::~BossManager() {}

void BossManager::init(Difficulty diff) {
    difficulty = diff;
    secondWaveSpawned = false;
    secondWaveTriggered = false;
    spawnTimer.restart();

    int bossCount = (diff == Difficulty::Hard) ? HARD_BOSS_COUNT : LUNATIC_BOSS_COUNT;
    activeBossCount = bossCount;

    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        bossActive[i] = (i < bossCount);
        spawnProgress[i] = 0.f;
        if (i < bossCount) {
            sf::Vector2f pos = getRandomSpawnPosition(i);
            spawnBoss(i, pos);
        }
    }
}

sf::Vector2f BossManager::getRandomSpawnPosition(int bossIndex) const {
    float baseX = BOSS_SPAWN_MIN_X + (BOSS_SPAWN_MAX_X - BOSS_SPAWN_MIN_X) *
                  static_cast<float>(bossIndex) / static_cast<float>(activeBossCount - 1);
    float offsetX = static_cast<float>((std::rand() % 100) - 50);
    float offsetY = static_cast<float>((std::rand() % 50) - 25);
    return sf::Vector2f(
        std::max(BOSS_SPAWN_MIN_X, std::min(BOSS_SPAWN_MAX_X, baseX + offsetX)),
        BOSS_SPAWN_MIN_Y + offsetY
    );
}

void BossManager::spawnBoss(int index, sf::Vector2f pos) {
    // Boss已在构造函数中创建，只需设置初始状态
    bossActive[index] = true;
}

void BossManager::update(float dt, const sf::Vector2f& playerPosition) {
    // 更新入场动画进度
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && spawnProgress[i] < 1.f) {
            spawnProgress[i] += dt * 0.5f;  // 2秒入场动画
            if (spawnProgress[i] > 1.f) spawnProgress[i] = 1.f;
        }
    }

    // 检查第二波入场条件
    if (difficulty == Difficulty::Lunatic && !secondWaveTriggered && !secondWaveSpawned) {
        if (!bosses[0].isDead() && bosses[0].getHP() <= bosses[0].getMaxHP() * BOSS_SPAWN_HP_THRESHOLD) {
            secondWaveTriggered = true;
            // BOSS 1和2同时入场
            for (int i = 1; i < MAX_BOSS_COUNT; ++i) {
                if (!bossActive[i]) {
                    sf::Vector2f pos = getRandomSpawnPosition(i);
                    spawnBoss(i, pos);
                }
            }
            secondWaveSpawned = true;
        }
    }

    // 更新存活的BOSS
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].update(dt, playerPosition);
            if (bosses[i].isDead()) {
                bossActive[i] = false;
            }
        }
    }
}

void BossManager::render(sf::RenderWindow& window) const {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            // 入场动画：应用缩放和透明度
            float progress = spawnProgress[i];
            if (progress < 1.f) {
                // 暂时修改boss位置来实现入场效果
                // 实际上Boss类需要支持入场动画缩放
            }
            bosses[i].render(window);
        }
    }
}

int BossManager::getActiveBossCount() const {
    int count = 0;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) count++;
    }
    return count;
}

bool BossManager::isAllBossesDead() const {
    return getActiveBossCount() == 0;
}

bool BossManager::shouldSpawnNextWave() const {
    return secondWaveTriggered && !secondWaveSpawned;
}

void BossManager::onBossDefeated(int index) {
    bossActive[index] = false;
}

void BossManager::triggerSecondWave() {
    if (!secondWaveSpawned) {
        secondWaveTriggered = true;
        for (int i = 1; i < MAX_BOSS_COUNT; ++i) {
            if (!bossActive[i]) {
                sf::Vector2f pos = getRandomSpawnPosition(i);
                spawnBoss(i, pos);
            }
        }
        secondWaveSpawned = true;
    }
}

void BossManager::clearAllBullets() {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].clearAllBullets();
        }
    }
}

std::vector<sf::Vector2f> BossManager::getAllBossPositions() const {
    std::vector<sf::Vector2f> positions;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) {
            positions.push_back(bosses[i].getPosition());
        }
    }
    return positions;
}

std::vector<float> BossManager::getAllBossRadii() const {
    std::vector<float> radii;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) {
            radii.push_back(bosses[i].getRadius());
        }
    }
    return radii;
}
```

- [ ] **Step 3: 提交**

```bash
git add src/entities/BossManager.h src/entities/BossManager.cpp
git commit -m "feat: create BossManager class for multi-boss support"
```

---

### Task 4: 扩展Boss类支持三种类型

**Files:**
- Modify: `src/entities/Boss.h`
- Modify: `src/entities/Boss.cpp`

- [ ] **Step 1: 在Boss.h中添加BossType枚举和 entranceAnimationProgress**

在 `Boss.h` 开头添加：

```cpp
enum class BossType { Inferno, Void, Thunder };
```

在私有成员中添加：

```cpp
BossType bossType;
float entranceAnimationProgress;
bool entranceAnimationActive;
sf::Vector2f entranceStartPos;
sf::Vector2f entranceTargetPos;
```

添加新方法声明：

```cpp
void setBossType(BossType type);
BossType getBossType() const;
void setEntranceAnimation(float progress);
float getEntranceAnimation() const;
bool isEntranceComplete() const;

// 新增渲染方法（各BOSS独特效果）
void renderInfernoAura(sf::RenderWindow& window) const;
void renderVoidAura(sf::RenderWindow& window) const;
void renderThunderAura(sf::RenderWindow& window) const;
void renderEntranceEffect(sf::RenderWindow& window) const;
```

- [ ] **Step 2: 修改Boss.cpp构造函数**

在 `Boss::Boss()` 初始化列表添加：

```cpp
, bossType(BossType::Inferno)
, entranceAnimationProgress(0.f)
, entranceAnimationActive(true)
, entranceStartPos(BOSS_POS)
, entranceTargetPos(BOSS_POS)
```

- [ ] **Step 3: 添加BossType设置和入场动画方法**

在 `Boss.cpp` 末尾添加：

```cpp
void Boss::setBossType(BossType type) {
    bossType = type;
}

BossType Boss::getBossType() const {
    return bossType;
}

void Boss::setEntranceAnimation(float progress) {
    entranceAnimationProgress = progress;
    if (progress >= 1.f) {
        entranceAnimationActive = false;
        position = entranceTargetPos;
    } else {
        // 插值计算位置（从底部升起效果）
        float t = progress;
        position = entranceStartPos + (entranceTargetPos - entranceStartPos) * t;
    }
}

float Boss::getEntranceAnimation() const {
    return entranceAnimationProgress;
}

bool Boss::isEntranceComplete() const {
    return entranceAnimationProgress >= 1.f;
}
```

- [ ] **Step 4: 添加三种BOSS的独特光环渲染**

在 `Boss.cpp` 末尾添加：

```cpp
void Boss::renderInfernoAura(sf::RenderWindow& window) const {
    // 火焰环绕光环 - 橙红色粒子向外飘散
    sf::Vector2f pos = position;
    for (int i = 0; i < 8; ++i) {
        float angle = pulseTimer * 2.f + (PI * 2.f / 8.f) * i;
        float dist = BOSS_RADIUS * 1.8f + std::sin(pulseTimer * 3.f + i) * 15.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;

        float orbRadius = 10.f + std::sin(pulseTimer * 4.f + i) * 3.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(255, 100, 30, 180));
        window.draw(orb);
    }
    // 火焰外圈光环
    float auraRadius = BOSS_RADIUS * 2.f + std::sin(pulseTimer * 2.f) * 10.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(255, 50, 0, 30));
    window.draw(aura);
}

void Boss::renderVoidAura(sf::RenderWindow& window) const {
    // 虚空能量场 - 紫蓝色星尘粒子
    sf::Vector2f pos = position;
    for (int i = 0; i < 12; ++i) {
        float angle = pulseTimer * 0.8f + (PI * 2.f / 12.f) * i;
        float dist = BOSS_RADIUS * 1.5f + std::sin(pulseTimer * 1.5f + i * 0.3f) * 20.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;

        float orbRadius = 4.f + std::sin(pulseTimer * 2.f + i) * 2.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(150, 80, 255, 200));
        window.draw(orb);
    }
    // 暗影能量场
    float auraRadius = BOSS_RADIUS * 2.2f + std::sin(pulseTimer * 1.5f) * 15.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(80, 20, 150, 40));
    window.draw(aura);
}

void Boss::renderThunderAura(sf::RenderWindow& window) const {
    // 闪电链环绕 - 电弧跳跃
    sf::Vector2f pos = position;
    for (int i = 0; i < 6; ++i) {
        float angle = pulseTimer * 3.f + (PI * 2.f / 6.f) * i;
        float dist = BOSS_RADIUS * 1.6f + std::sin(pulseTimer * 4.f + i) * 10.f;
        float orbX = pos.x + std::cos(angle) * dist;
        float orbY = pos.y + std::sin(angle) * dist;

        float orbRadius = 8.f + std::sin(pulseTimer * 5.f + i) * 3.f;
        sf::CircleShape orb(orbRadius);
        orb.setOrigin(sf::Vector2f(orbRadius, orbRadius));
        orb.setPosition(sf::Vector2f(orbX, orbY));
        orb.setFillColor(sf::Color(255, 240, 100, 220));
        window.draw(orb);

        // 闪电链
        sf::RectangleShape lightning(sf::Vector2f(30.f, 3.f));
        lightning.setOrigin(sf::Vector2f(0.f, 1.5f));
        lightning.setPosition(sf::Vector2f(orbX, orbY));
        lightning.setRotation(sf::degrees(angle * 180.f / PI));
        lightning.setFillColor(sf::Color(255, 255, 100, 150));
        window.draw(lightning);
    }
    // 电弧外圈
    float auraRadius = BOSS_RADIUS * 2.1f + std::sin(pulseTimer * 2.5f) * 12.f;
    sf::CircleShape aura(auraRadius);
    aura.setOrigin(sf::Vector2f(auraRadius, auraRadius));
    aura.setPosition(pos);
    aura.setFillColor(sf::Color(200, 200, 50, 35));
    window.draw(aura);
}

void Boss::renderEntranceEffect(sf::RenderWindow& window) const {
    if (!entranceAnimationActive || entranceAnimationProgress >= 1.f) return;

    float progress = entranceAnimationProgress;
    sf::Vector2f pos = position;

    // 根据BOSS类型显示不同入场特效
    switch (bossType) {
        case BossType::Inferno: {
            // 火焰拖尾效果
            sf::Vector2f trailDir = entranceTargetPos - entranceStartPos;
            float trailLen = std::sqrt(trailDir.x * trailDir.x + trailDir.y * trailDir.y);
            if (trailLen > 0) trailDir /= trailLen;
            for (int i = 0; i < 5; ++i) {
                float t = 1.f - progress + i * 0.1f;
                if (t > 0 && t < 1.f) {
                    sf::Vector2f trailPos = entranceStartPos + trailDir * t * trailLen;
                    float radius = 20.f * (1.f - t) * progress;
                    sf::CircleShape trail(radius);
                    trail.setOrigin(sf::Vector2f(radius, radius));
                    trail.setPosition(trailPos);
                    trail.setFillColor(sf::Color(255, 100, 30, static_cast<std::uint8_t>(150 * (1.f - t))));
                    window.draw(trail);
                }
            }
            break;
        }
        case BossType::Void: {
            // 空间撕裂裂缝
            float crackAlpha = 200.f * progress;
            sf::RectangleShape crack(sf::Vector2f(WINDOW_WIDTH, 5.f));
            crack.setPosition(sf::Vector2f(0.f, pos.y));
            crack.setFillColor(sf::Color(180, 80, 255, static_cast<std::uint8_t>(crackAlpha)));
            window.draw(crack);
            sf::RectangleShape crack2(sf::Vector2f(WINDOW_WIDTH, 5.f));
            crack2.setPosition(sf::Vector2f(0.f, pos.y - 30.f));
            crack2.setFillColor(sf::Color(150, 60, 255, static_cast<std::uint8_t>(crackAlpha * 0.5f)));
            window.draw(crack2);
            break;
        }
        case BossType::Thunder: {
            // 脉冲冲击波
            float waveRadius = BOSS_RADIUS * (1.f + progress * 3.f);
            sf::CircleShape wave(waveRadius);
            wave.setOrigin(sf::Vector2f(waveRadius, waveRadius));
            wave.setPosition(pos);
            wave.setFillColor(sf::Color(255, 255, 100, static_cast<std::uint8_t>(100 * (1.f - progress))));
            wave.setOutlineColor(sf::Color(255, 240, 100, static_cast<std::uint8_t>(200 * (1.f - progress))));
            wave.setOutlineThickness(3.f);
            window.draw(wave);
            break;
        }
    }
}
```

- [ ] **Step 5: 修改render方法调用不同光环**

找到 `Boss::render()` 中的 `renderAlienEffects(window)` 调用，在其后添加：

```cpp
void Boss::render(sf::RenderWindow& window) const {
    for (const auto& bullet : bullets) {
        bullet.render(window);
    }

    // 入场特效
    renderEntranceEffect(window);

    // 根据BOSS类型绘制独特光环
    switch (bossType) {
        case BossType::Inferno: renderInfernoAura(window); break;
        case BossType::Void: renderVoidAura(window); break;
        case BossType::Thunder: renderThunderAura(window); break;
    }

    renderAlienEffects(window);
    // ... 其余不变
}
```

- [ ] **Step 6: 提交**

```bash
git add src/entities/Boss.h src/entities/Boss.cpp
git commit -m "feat: extend Boss with 3 types and unique aura effects"
```

---

### Task 5: 修改HUD支持多血条

**Files:**
- Modify: `src/ui/HUD.h`
- Modify: `src/ui/HUD.cpp`

- [ ] **Step 1: 修改HUD.h添加多血条支持**

将 `bossBarBg`, `bossBarFill` 改为数组：

```cpp
// Boss HP bars (max 3)
static constexpr int MAX_BOSS_BARS = 3;
std::array<sf::RectangleShape, MAX_BOSS_BARS> bossBarBgs;
std::array<sf::RectangleShape, MAX_BOSS_BARS> bossBarFills;
std::array<sf::Text, MAX_BOSS_BARS> bossHPTexts;
std::array<sf::Text, MAX_BOSS_BARS> bossPhaseTexts;
std::array<sf::Text, MAX_BOSS_BARS> bossNameTexts;
```

添加难度相关：

```cpp
Difficulty currentDifficulty = Difficulty::Normal;
```

- [ ] **Step 2: 修改HUD.cpp实现多血条**

构造函数中初始化数组，参考单血条初始化逻辑。

`update()` 方法接收 `GameStats` 中的 `multiBossStats` 并更新多血条：

```cpp
void HUD::update(const GameStats& stats) {
    // ... 玩家血条不变 ...

    // 多BOSS血条
    for (int i = 0; i < stats.multiBossStats.activeCount; ++i) {
        const auto& boss = stats.multiBossStats.bosses[i];
        float bossRatio = boss.hp / boss.maxHP;
        bossBarFills[i].setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH * bossRatio, HUD_BAR_HEIGHT));
        // 更新文字等...
    }
}
```

- [ ] **Step 3: 提交**

```bash
git add src/ui/HUD.h src/ui/HUD.cpp
git commit -m "feat: extend HUD for multi-boss HP bars"
```

---

### Task 6: 修改Menu添加难度选择

**Files:**
- Modify: `src/ui/Menu.h`
- Modify: `src/ui/Menu.cpp`

- [ ] **Step 1: 添加Difficulty参数到show方法**

```cpp
void show(MenuMode mode, const sf::Font& font, Difficulty diff = Difficulty::Normal);
```

- [ ] **Step 2: 修改buildMainMenu添加3个难度选项**

```cpp
void Menu::buildMainMenu(const sf::Font& font, Difficulty diff) {
    title.emplace(font, "", 48);
    title->setString("NEON BULLET ARENA");
    // ...

    buttons.clear();
    float btnX = WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f;
    float startY = 280.f;

    buttons.emplace_back("NORMAL", font,
        sf::Vector2f(btnX, startY),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("HARD", font,
        sf::Vector2f(btnX, startY + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("LUNATIC", font,
        sf::Vector2f(btnX, startY + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
    buttons.emplace_back("QUIT", font,
        sf::Vector2f(btnX, startY + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT), nullptr);
}
```

- [ ] **Step 3: 提交**

```bash
git add src/ui/Menu.h src/ui/Menu.cpp
git commit -m "feat: add difficulty selection to main menu"
```

---

### Task 7: 修改Game整合所有组件

**Files:**
- Modify: `src/core/Game.h`
- Modify: `src/core/Game.cpp`

- [ ] **Step 1: 修改Game.h**

替换单Boss为BossManager：

```cpp
#include "entities/BossManager.h"

class Game {
    // 替换 single Boss
    BossManager bossManager;
    Difficulty currentDifficulty = Difficulty::Normal;
    // ...
};
```

- [ ] **Step 2: 修改Game.cpp**

主要修改点：

1. **startGame()**: 根据难度初始化 `bossManager.init(currentDifficulty)`

2. **update()**: 循环遍历 `bossManager.getBosses()` 进行碰撞检测

3. **render()**: 调用 `bossManager.render(window)`

4. **victory/gameover**: 检查 `bossManager.isAllBossesDead()`

5. **Player vs Boss bullets**: 需要遍历所有存活BOSS

```cpp
// 遍历所有BOSS进行碰撞
for (auto& boss : bossManager.getBosses()) {
    if (boss.isDead()) continue;
    for (auto& bullet : player.getBullets()) {
        if (!bullet.isActive()) continue;
        if (CollisionSystem::checkCircleCollision(...)) {
            // 对当前BOSS造成伤害
        }
    }
}
```

6. **Boss bullets vs Player**: 同样需要遍历所有BOSS的弹幕

- [ ] **Step 3: 提交**

```bash
git add src/core/Game.h src/core/Game.cpp
git commit -m "feat: integrate BossManager and difficulty system into Game"
```

---

### Task 8: 测试和验证

- [ ] **Step 1: 构建项目**

```bash
cd c--main && cmake -B build-ninja -G Ninja && cmake --build build-ninja
```

- [ ] **Step 2: 运行测试**

运行游戏，验证：
1. 主菜单显示3个难度选项
2. 选择Normal：1只BOSS
3. 选择Hard：2只BOSS同时入场（不同外观/光环）
4. 选择Lunatic：1只BOSS入场，打到50%时另外2只同时入场
5. 多血条正确显示
6. 各BOSS弹幕效果不同
7. 出场动画正确播放

- [ ] **Step 3: 提交**

```bash
git add -A
git commit -m "feat: complete hard/lunatic mode implementation"
```

---

## 确认状态
实施计划完成，等待用户确认后开始执行。