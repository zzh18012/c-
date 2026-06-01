// ============================================================
// BossManager.cpp - Boss管理器实现
// ============================================================
// 职责：管理多个Boss的生成、更新、状态跟踪、入场动画协调
// 支持多Boss（Normal=1个，Hard=2个，Lunatic=3个）
// Lunatic难度下，当第一个Boss血量低于阈值时会触发第二波入场
// ============================================================

#include "BossManager.h"
#include "core/Config.h"
#include <cstdlib>  // std::rand(), std::srand()

// ============================================================
// 构造函数
// ============================================================
// 初始化列表：activeBossCount=0, difficulty=Normal, secondWaveSpawned=false, secondWaveTriggered=false
// 循环初始化所有Boss为非激活状态、入场进度为0
BossManager::BossManager()
    : activeBossCount(0), difficulty(Difficulty::Normal)
    , secondWaveSpawned(false), secondWaveTriggered(false) {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        bossActive[i] = false;   // 所有Boss标记为非激活
        spawnProgress[i] = 0.f;  // 入场进度清零
    }
}

BossManager::~BossManager() {} // 默认析构，无需手动释放资源

// ============================================================
// init - 初始化Boss
// ============================================================
// difficulty: 游戏难度（决定Boss数量）
// 初始化流程：
// 1. 设置难度、重置第二波状态、重启入场计时器
// 2. 根据难度确定Boss数量（Normal=1, Hard=HARD_BOSS_COUNT, Lunatic=LUNATIC_BOSS_COUNT）
// 3. 重置所有Boss的激活状态和入场进度
// 4. 为每个Boss设置类型、位置、入场动画起点/终点
void BossManager::init(Difficulty diff) {
    difficulty = diff;                               // 保存难度
    secondWaveSpawned = false;                       // 重置第二波状态
    secondWaveTriggered = false;                    // 重置第二波触发标志
    spawnTimer.restart();                            // 重置入场计时器

    // ---- 确定Boss数量 ----
    int bossCount = 1;                               // 默认1个（Normal难度）
    if (diff == Difficulty::Hard) bossCount = HARD_BOSS_COUNT;      // Hard难度
    else if (diff == Difficulty::Lunatic) bossCount = LUNATIC_BOSS_COUNT; // Lunatic难度
    activeBossCount = bossCount;                     // 保存激活数量

    // ---- 重置所有Boss状态 ----
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        bossActive[i] = false;    // 标记为非激活
        spawnProgress[i] = 0.f;   // 入场进度归零
    }

    // ---- 设置Boss类型映射 ----
    // 三种Boss类型：Inferno（火）, Void（虚空）, Thunder（雷电）
    // 按索引分配，多Boss时类型不同增加多样性
    BossType types[3] = { BossType::Inferno, BossType::Void, BossType::Thunder };

    // ---- 初始化每个Boss ----
    for (int i = 0; i < bossCount; ++i) {
        sf::Vector2f pos = getRandomSpawnPosition(i); // 获取随机位置
        bosses[i].setPosition(pos);                    // 设置Boss位置
        bosses[i].setBossType(types[i]);               // 设置Boss类型（影响外观颜色）
        bosses[i].setEntranceAnimation(0.f);            // 启动入场动画（从0开始）
        // 设置入场起点：屏幕下方100像素（从屏幕外飘入）
        bosses[i].setEntranceStartPos(sf::Vector2f(pos.x, WINDOW_HEIGHT + 100.f));
        bosses[i].setEntranceTargetPos(pos);           // 设置入场终点（目标位置）
        bossActive[i] = true;                          // 标记为激活
    }
}

// ============================================================
// getRandomSpawnPosition - 获取随机生成位置
// ============================================================
// bossIndex: Boss在数组中的索引（0, 1, 2）
// 单Boss时：在屏幕中上方附近随机偏移
// 多Boss时：沿X轴均匀分布，每个Boss占一个区段
sf::Vector2f BossManager::getRandomSpawnPosition(int bossIndex) const {
    // ---- 单Boss时 ----
    if (activeBossCount <= 1) {
        // 在屏幕中上方小范围随机偏移
        float offsetX = static_cast<float>((std::rand() % 100) - 50); // [-50, +50]
        float offsetY = static_cast<float>((std::rand() % 50) - 25);  // [-25, +25]
        // 计算中点位置并限制在合法范围内
        float baseX = (BOSS_SPAWN_MIN_X + BOSS_SPAWN_MAX_X) * 0.5f;
        return sf::Vector2f(
            std::max(BOSS_SPAWN_MIN_X, std::min(BOSS_SPAWN_MAX_X, baseX + offsetX)),
            BOSS_SPAWN_MIN_Y + offsetY
        );
    }

    // ---- 多Boss时 ----
    // 沿X轴均匀分布，计算每个Boss的基础X坐标
    float baseX = BOSS_SPAWN_MIN_X + (BOSS_SPAWN_MAX_X - BOSS_SPAWN_MIN_X) *
                  static_cast<float>(bossIndex) / static_cast<float>(activeBossCount - 1);
    float offsetX = static_cast<float>((std::rand() % 100) - 50); // [-50, +50]
    float offsetY = static_cast<float>((std::rand() % 50) - 25);  // [-25, +25]
    return sf::Vector2f(
        std::max(BOSS_SPAWN_MIN_X, std::min(BOSS_SPAWN_MAX_X, baseX + offsetX)),
        BOSS_SPAWN_MIN_Y + offsetY
    );
}

// ============================================================
// spawnBoss - 生成单个Boss
// ============================================================
// index: Boss在数组中的索引
// pos: 生成位置（屏幕坐标系）
// 注意：只是设置位置和激活状态，不设置类型和入场动画（init统一处理）
void BossManager::spawnBoss(int index, sf::Vector2f pos) {
    bosses[index].setPosition(pos);   // 设置位置
    bossActive[index] = true;        // 标记为激活
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// playerPosition: 玩家当前位置（Boss AI攻击瞄准用）
// 更新流程：
// 1. 更新入场动画进度（每帧增加0.5×dt，进度从0到1需要2秒）
// 2. Lunatic难度下检测第二波触发条件
// 3. 更新所有存活的Boss（AI、攻击、子弹）
// 4. 检测Boss死亡并标记为非激活
void BossManager::update(float dt, const sf::Vector2f& playerPosition) {
    // ---- 第1步：更新入场动画进度 ----
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && spawnProgress[i] < 1.f) {
            // 入场进度每帧增加0.5×dt（2秒完成入场：1/0.5=2秒）
            spawnProgress[i] += dt * 0.5f;
            if (spawnProgress[i] > 1.f) spawnProgress[i] = 1.f; // 上限为1
            bosses[i].setEntranceAnimation(spawnProgress[i]);   // 通知Boss更新位置插值
        }
    }

    // ---- 第2步：Lunatic难度第二波检测 ----
    // 条件：第一个Boss血量低于阈值 或 第一个Boss已死亡
    if (difficulty == Difficulty::Lunatic && !secondWaveTriggered && !secondWaveSpawned) {
        bool hpBelowThreshold = bosses[0].getHP() <= bosses[0].getMaxHP() * BOSS_SPAWN_HP_THRESHOLD;
        bool bossDiedAfterThreshold = !bosses[0].isDead() || hpBelowThreshold;
        if (bossDiedAfterThreshold) {
            secondWaveTriggered = true; // 标记为已触发
            // 生成剩余Boss
            for (int i = 1; i < MAX_BOSS_COUNT; ++i) {
                if (!bossActive[i]) {
                    sf::Vector2f pos = getRandomSpawnPosition(i);
                    spawnBoss(i, pos);
                }
            }
            secondWaveSpawned = true; // 标记为已生成
        }
    }

    // ---- 第3步：更新所有存活Boss ----
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].update(dt, playerPosition); // 更新Boss AI/攻击/子弹
            if (bosses[i].isDead()) {              // Boss死亡检测
                bossActive[i] = false;             // 标记为非激活
            }
        }
    }
}

// ============================================================
// render - 渲染所有存活Boss
// ============================================================
// 遍历所有Boss，渲染激活的Boss
// 每个Boss.render()会调用其子弹渲染（Boss子弹→特效→身体→眼睛→光环）
void BossManager::render(sf::RenderWindow& window) const {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].render(window);
        }
    }
}

// ============================================================
// getActiveBossCount - 获取存活Boss数量
// ============================================================
// 遍历所有Boss，统计激活且未死亡的数量
int BossManager::getActiveBossCount() const {
    int count = 0;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) count++;
    }
    return count;
}

// ============================================================
// isAllBossesDead - 是否所有Boss都已死亡
// ============================================================
// 用于判断游戏胜利条件
bool BossManager::isAllBossesDead() const {
    return getActiveBossCount() == 0;
}

// ============================================================
// shouldSpawnNextWave - 是否应该生成下一波
// ============================================================
// 用于Lunatic难度游戏开始时第一波就生成所有Boss
bool BossManager::shouldSpawnNextWave() const {
    return secondWaveTriggered && !secondWaveSpawned;
}

// ============================================================
// onBossDefeated - Boss被击败时调用
// ============================================================
// index: 被击败Boss的索引
void BossManager::onBossDefeated(int index) {
    bossActive[index] = false;
}

// ============================================================
// triggerSecondWave - 手动触发第二波
// ============================================================
// 用于游戏中途需要刷新Boss的情况
void BossManager::triggerSecondWave() {
    if (!secondWaveSpawned) {                    // 避免重复触发
        secondWaveTriggered = true;
        for (int i = 1; i < MAX_BOSS_COUNT; ++i) {
            if (!bossActive[i]) {               // 只生成未激活的Boss
                sf::Vector2f pos = getRandomSpawnPosition(i);
                spawnBoss(i, pos);
            }
        }
        secondWaveSpawned = true;
    }
}

// ============================================================
// clearAllBullets - 清除所有Boss子弹
// ============================================================
// 用于道具效果：
// - BulletTime（子弹时间）：减速所有Boss子弹
// - NovaBomb（Nova炸弹）：清除所有Boss子弹造成大爆炸
void BossManager::clearAllBullets() {
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i]) {
            bosses[i].clearAllBullets();
        }
    }
}

// ============================================================
// getAllBossPositions - 获取所有存活Boss位置
// ============================================================
// 返回一个向量，包含所有存活Boss的屏幕坐标
// 用于Game.cpp的碰撞检测（玩家子弹vs Boss）
std::vector<sf::Vector2f> BossManager::getAllBossPositions() const {
    std::vector<sf::Vector2f> positions;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) {
            positions.push_back(bosses[i].getPosition());
        }
    }
    return positions;
}

// ============================================================
// getAllBossRadii - 获取所有存活Boss半径
// ============================================================
// 返回一个向量，包含所有存活Boss的碰撞半径
// 用于Game.cpp的碰撞检测（圆形碰撞检测）
std::vector<float> BossManager::getAllBossRadii() const {
    std::vector<float> radii;
    for (int i = 0; i < MAX_BOSS_COUNT; ++i) {
        if (bossActive[i] && !bosses[i].isDead()) {
            radii.push_back(bosses[i].getRadius());
        }
    }
    return radii;
}