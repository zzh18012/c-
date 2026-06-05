// ============================================================
// BossManager.h - Boss管理器声明
// ============================================================
// 职责：管理多个Boss的生成、更新、状态跟踪、入场动画协调
// 支持多Boss（Normal=1个，Hard=2个，Lunatic=3个）
// Lunatic难度下，当第一个Boss血量低于阈值时会触发第二波入场
// ============================================================

#pragma once

// Boss实体类
#include "Boss.h"

// 配置文件（获取MAX_BOSS_COUNT、HARD_BOSS_COUNT、LUNATIC_BOSS_COUNT等常量）
#include "core/Config.h"

// 标准数组容器（存储固定数量的Boss对象）
#include <array>

// 标准向量（用于返回位置/半径数组给Game.cpp）
#include <vector>

// ============================================================
// BossManager类
// ============================================================
// 游戏中的Boss管理器，负责：
// 1. 根据难度初始化不同数量的Boss（Normal=1，Hard=2，Lunatic=3）
// 2. 管理每个Boss的入场动画（从屏幕下方飘入）
// 3. Lunatic难度下监控第一Boss血量，触发第二波Boss入场
// 4. 提供Boss数组给Game.cpp进行碰撞检测
// 5. 清除所有Boss子弹（道具效果：BulletTime、NovaBomb）
class BossManager {
public:
    // 构造函数：初始化所有Boss为非激活状态、入场进度为0
    // 析构函数：默认实现即可（无动态内存）
    BossManager();
    ~BossManager();

    // ---- 初始化 ----
    // 根据难度初始化Boss：设置Boss类型、随机位置、入场动画起点/终点
    // difficulty: 游戏难度（影响Boss数量）
    void init(Difficulty difficulty);

    // ---- 每帧更新 ----
    // dt: 时间差（秒）
    // playerPosition: 玩家当前位置（用于Boss AI攻击瞄准）
    // 更新内容：入场动画进度 → 第二波触发检测 → 所有存活Boss的AI/攻击/子弹
    void update(float dt, const sf::Vector2f& playerPosition);

    // ---- 渲染 ----
    // 渲染所有存活的Boss（Boss.render会调用其子弹渲染）
    void render(sf::RenderWindow& window) const;

    // ============================================================
    // 状态查询接口（供Game.cpp判断游戏胜利条件）
    // ============================================================
    int getActiveBossCount() const;      // 获取当前存活的Boss数量
    bool isAllBossesDead() const;        // 是否所有Boss都已死亡（胜利条件）
    bool shouldSpawnNextWave() const;    // 是否应该生成下一波（Lunatic难度）
    void onBossDefeated(int index);      // Boss被击败时调用（标记为非激活）
    void triggerSecondWave();            // 手动触发第二波Boss入场

    // ============================================================
    // Boss数组访问接口（供Game.cpp碰撞检测）
    // ============================================================
    // 获取Boss数组引用（用于玩家子弹与Boss的碰撞检测）
    std::array<Boss, MAX_BOSS_COUNT>& getBosses() { return bosses; }
    const std::array<Boss, MAX_BOSS_COUNT>& getBosses() const { return bosses; }

    // ============================================================
    // 子弹管理
    // ============================================================
    // 清除所有Boss发射的子弹（BulletTime道具减速子弹、NovaBomb大爆炸）
    void clearAllBullets();

    // 获取所有存活Boss的位置和半径（用于Game.cpp碰撞检测）
    std::vector<sf::Vector2f> getAllBossPositions() const;
    std::vector<float> getAllBossRadii() const;

private:
    // ---- Boss数组 ----
    // 使用std::array固定大小数组，避免动态内存分配
    // 每个元素是一个完整的Boss对象（包含所有AI、渲染、入场动画逻辑）
    std::array<Boss, MAX_BOSS_COUNT> bosses;

    // ---- Boss激活状态数组 ----
    // 与bosses数组一一对应，标记每个Boss是否活跃
    // 比isDead()更精确，可以区分"未生成"和"已死亡但还未被标记"
    std::array<bool, MAX_BOSS_COUNT> bossActive;

    // ---- 游戏状态 ----
    int activeBossCount;              // 当前激活的Boss数量（初始化时设置）
    Difficulty difficulty;           // 当前游戏难度
    bool secondWaveSpawned;           // 第二波是否已生成（避免重复触发）
    bool secondWaveTriggered;         // 第二波触发条件是否已满足

    // ---- 入场动画相关 ----
    sf::Clock spawnTimer;             // 入场计时器（用于控制第二波触发时间）
    // 每个Boss的入场动画进度（0~1，0=开始，1=完成）
    // 在update中每帧递增，到1则入场完成
    std::array<float, MAX_BOSS_COUNT> spawnProgress;

    // ---- 私有辅助方法 ----
    // 生成单个Boss：设置位置、标记为激活
    void spawnBoss(int index, sf::Vector2f pos);
    // 获取随机生成位置（根据Boss索引和当前激活数量决定）
    // 单Boss时位置居中，多Boss时沿X轴均匀分布
    sf::Vector2f getRandomSpawnPosition(int bossIndex) const;
};