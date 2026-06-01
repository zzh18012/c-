// ============================================================
// ParticleSystem.h - 粒子系统声明
// ============================================================
// 职责：管理游戏中的视觉效果粒子（爆炸、闪光、轨迹等）
// 使用对象池模式，预分配PARTICLE_POOL_SIZE个粒子
// 支持多种粒子效果：爆炸、火花、闪电、环形扩散、冲刺轨迹等
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f, sf::Color, sf::RenderWindow等）
#include <SFML/Graphics.hpp>

// 标准向量（存储粒子数组）
#include <vector>

// ============================================================
// Particle结构体
// ============================================================
// 单个粒子的数据结构
// 包含位置、速度、颜色、生命周期等属性
struct Particle {
    sf::Vector2f position;  // 粒子当前位置
    sf::Vector2f velocity;  // 粒子速度向量
    sf::Color color;       // 粒子颜色
    float lifetime;       // 剩余寿命（秒）
    float maxLifetime;     // 最大寿命（秒，用于计算透明度衰减）
    bool active = false;   // 是否激活
};

// ============================================================
// ParticleSystem类
// ============================================================
// 游戏粒子效果系统，使用对象池模式
// 预分配PARTICLE_POOL_SIZE个粒子，避免运行时内存分配
// 所有spawn方法从池中取空闲粒子，update回收过期粒子
// 渲染使用sf::VertexArray + 加法混合模式实现发光效果
class ParticleSystem {
public:
    // 构造函数：预分配粒子池（PARTICLE_POOL_SIZE个粒子）
    ParticleSystem();

    // ============================================================
    // 粒子生成方法
    // ============================================================
    // 每种方法从池中取空闲粒子，设置属性

    // 爆炸效果：从position向四周发射count个粒子
    void spawnExplosion(sf::Vector2f position, sf::Color color, int count);

    // 命中火花：从position发射少量快速粒子
    void spawnHitSpark(sf::Vector2f position, sf::Color color);

    // 环绕粒子：在center周围以orbitRadius为半径生成count个粒子
    // 粒子沿切线方向缓慢移动（轨道效果）
    void spawnOrbitParticles(sf::Vector2f center, sf::Color color, int count, float orbitRadius);

    // 闪电效果：从from到to生成折线形粒子序列
    // 每段有随机垂直偏移模拟闪电
    void spawnLightning(sf::Vector2f from, sf::Vector2f to, sf::Color color);

    // Nova环形：从center向外扩散的环形粒子
    void spawnNovaRing(sf::Vector2f center, float radius, sf::Color color, int count);

    // 冲刺轨迹：在position生成残留粒子效果
    void spawnDashTrail(sf::Vector2f position, sf::Color color);

    // 机械推进器：从position沿dir方向发射粒子（玩家冲刺时的特效）
    void spawnMechThrust(sf::Vector2f position, sf::Vector2f dir, sf::Color color);

    // 子弹轨迹：单个小粒子（用于玩家子弹尾迹）
    void spawnBulletTrail(sf::Vector2f position, sf::Color color);

    // 治疗效果：从position向上升起的绿色粒子
    void spawnHealEffect(sf::Vector2f position);

    // 阶段转换：从position向四周爆发的白色粒子
    void spawnPhaseTransition(sf::Vector2f position);

    // ============================================================
    // 每帧更新和渲染
    // ============================================================
    // dt: 时间差（秒）
    // 更新所有激活粒子的位置、速度、寿命
    void update(float dt);

    // 渲染：使用sf::VertexArray绘制所有激活粒子
    // 采用加法混合模式（Additive Blending）实现发光效果
    void render(sf::RenderWindow& window);

private:
    // ---- 粒子池 ----
    // 预分配固定数量的粒子，避免运行时new/delete
    // 每个粒子独立运行，激活状态标记
    std::vector<Particle> particles;
};