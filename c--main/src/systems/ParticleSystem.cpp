// ============================================================
// ParticleSystem.cpp - 粒子系统实现
// ============================================================
// 职责：管理游戏中的视觉效果粒子（爆炸、闪光、轨迹等）
// 使用对象池模式，预分配PARTICLE_POOL_SIZE个粒子
// 渲染使用sf::VertexArray + 加法混合模式实现发光效果
// ============================================================

#include "ParticleSystem.h"
#include "core/Config.h"
#include <cmath>  // cosf, sinf, sqrtf
#include <cstdlib> // rand(), RAND_MAX
#include <cstdint> // uint8_t

// ============================================================
// 构造函数
// ============================================================
// 预分配PARTICLE_POOL_SIZE个粒子到池中
ParticleSystem::ParticleSystem() {
    particles.resize(PARTICLE_POOL_SIZE);
}

// ============================================================
// spawnExplosion - 爆炸效果
// ============================================================
// 从position向四周发射count个粒子
// 每个粒子有随机方向、随机速度（80~330像素/秒）、随机寿命（0.3~0.8秒）
void ParticleSystem::spawnExplosion(sf::Vector2f position, sf::Color color, int count) {
    int spawned = 0;
    for (auto& p : particles) {
        if (spawned >= count) break;       // 达到目标数量则停止
        if (p.active) continue;            // 跳过已激活的粒子

        p.position = position;              // 设置位置
        // 随机角度（0~2π）
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        // 随机速度（80~330像素/秒）
        float speed = 80.f + static_cast<float>(rand()) / RAND_MAX * 250.f;
        p.velocity = sf::Vector2f(cosf(angle) * speed, sinf(angle) * speed);
        p.color = color;                    // 设置颜色
        // 随机寿命（0.3~0.8秒）
        p.maxLifetime = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.5f;
        p.lifetime = p.maxLifetime;        // 当前寿命等于最大寿命
        p.active = true;                    // 激活
        spawned++;                           // 已生成数量+1
    }
}

// ============================================================
// spawnHitSpark - 命中火花
// ============================================================
// 从position发射5~10个快速粒子
// 用于攻击命中的瞬间反馈
void ParticleSystem::spawnHitSpark(sf::Vector2f position, sf::Color color) {
    int count = 5 + rand() % 6;            // 5~10个粒子
    for (auto& p : particles) {
        if (count <= 0) break;             // 达到目标数量则停止
        if (p.active) continue;            // 跳过已激活的粒子

        p.position = position;              // 设置位置
        float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
        // 较高速度（150~350像素/秒）
        float speed = 150.f + static_cast<float>(rand()) / RAND_MAX * 200.f;
        p.velocity = sf::Vector2f(cosf(angle) * speed, sinf(angle) * speed);
        p.color = color;                    // 设置颜色
        // 较短寿命（0.1~0.3秒）
        p.maxLifetime = 0.1f + static_cast<float>(rand()) / RAND_MAX * 0.2f;
        p.lifetime = p.maxLifetime;
        p.active = true;
        count--;
    }
}

// ============================================================
// spawnOrbitParticles - 环绕粒子
// ============================================================
// 在center周围以orbitRadius为半径生成count个粒子
// 粒子沿切线方向缓慢移动，产生轨道效果
// 用于道具周围的环绕效果
void ParticleSystem::spawnOrbitParticles(sf::Vector2f center, sf::Color color, int count, float orbitRadius) {
    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;        // 找空闲粒子

            // 均匀分布在圆周上
            float angle = 6.28318f * i / count; // i/count * 2π
            // 位置 = 中心 + 方向 × 半径
            p.position = center + sf::Vector2f(std::cosf(angle) * orbitRadius, std::sinf(angle) * orbitRadius);
            // 速度 = 垂直方向 × 80（切线速度）
            p.velocity = sf::Vector2f(-std::sinf(angle) * 80.f, std::cosf(angle) * 80.f);
            p.color = color;
            p.maxLifetime = 1.5f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break; // 只找一颗粒子就跳出内循环
        }
    }
}

// ============================================================
// spawnLightning - 闪电效果
// ============================================================
// 从from到to生成折线形粒子序列，模拟闪电
// 将线段分成8段，每段有随机垂直偏移
void ParticleSystem::spawnLightning(sf::Vector2f from, sf::Vector2f to, sf::Color color) {
    sf::Vector2f dir = to - from;         // 方向向量
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y); // 长度
    if (len < 1.f) return;                 // 距离太短则跳过
    dir /= len;                            // 归一化
    sf::Vector2f perp(-dir.y, dir.x);     // 垂直方向向量
    int segments = 8;                     // 8段
    sf::Vector2f prev = from;
    for (int i = 1; i <= segments; ++i) {
        float t = i / static_cast<float>(segments); // 0~1进度
        sf::Vector2f base = from + dir * (len * t); // 基准点位置
        // 随机偏移（除最后一段外）
        float offset = (i == segments) ? 0.f : (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 30.f;
        sf::Vector2f curr = base + perp * offset; // 加上垂直偏移
        // 为当前点找一个空闲粒子
        for (auto& p : particles) {
            if (p.active) continue;
            p.position = curr;
            p.velocity = sf::Vector2f(0.f, 0.f); // 闪电静止不动
            p.color = color;
            p.maxLifetime = 0.15f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
        prev = curr;
    }
}

// ============================================================
// spawnNovaRing - Nova环形
// ============================================================
// 从center向外扩散的环形粒子
// 粒子沿着半径为radius的圆周均匀分布，然后向外飞散
void ParticleSystem::spawnNovaRing(sf::Vector2f center, float radius, sf::Color color, int count) {
    for (int i = 0; i < count; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            // 均匀分布在半径为radius的圆周上
            float angle = 6.28318f * i / count;
            p.position = center + sf::Vector2f(std::cosf(angle) * radius, std::sinf(angle) * radius);
            // 向外飞散（速度方向 = 半径方向）
            p.velocity = sf::Vector2f(std::cosf(angle) * 100.f, std::sinf(angle) * 100.f);
            p.color = color;
            p.maxLifetime = 0.4f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

// ============================================================
// spawnDashTrail - 冲刺轨迹
// ============================================================
// 在position生成残留粒子效果（玩家冲刺时留下的一串粒子）
// 每次生成8个小粒子，随机散开
void ParticleSystem::spawnDashTrail(sf::Vector2f position, sf::Color color) {
    for (int i = 0; i < 8; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
            float speed = 20.f + static_cast<float>(rand()) / RAND_MAX * 40.f;
            // 在position附近20像素范围内随机位置
            p.position = position + sf::Vector2f(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.f);
            p.velocity = sf::Vector2f(std::cosf(angle) * speed, std::sinf(angle) * speed);
            p.color = color;
            // 较短寿命（0.2~0.35秒）
            p.maxLifetime = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.15f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

// ============================================================
// spawnMechThrust - 机械推进器
// ============================================================
// 从position沿dir方向发射粒子（玩家冲刺时的推进器特效）
// 粒子有轻微扩散
void ParticleSystem::spawnMechThrust(sf::Vector2f position, sf::Vector2f dir, sf::Color color) {
    for (int i = 0; i < 3; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            // 随机扩散角度
            float spread = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.8f;
            float speed = 100.f + static_cast<float>(rand()) / RAND_MAX * 80.f;
            sf::Vector2f perp(-dir.y, dir.x); // 垂直方向
            p.position = position;
            // 主方向 + 垂直扩散
            p.velocity = dir * speed + perp * spread * 50.f;
            p.color = color;
            p.maxLifetime = 0.15f + static_cast<float>(rand()) / RAND_MAX * 0.1f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

// ============================================================
// spawnBulletTrail - 子弹轨迹
// ============================================================
// 在position生成单个小粒子（玩家子弹的尾迹）
// 静止不动，很快消失
void ParticleSystem::spawnBulletTrail(sf::Vector2f position, sf::Color color) {
    for (auto& p : particles) {
        if (p.active) continue;
        p.position = position;
        p.velocity = sf::Vector2f(0.f, 0.f); // 静止
        p.color = color;
        p.maxLifetime = 0.08f; // 很快消失
        p.lifetime = p.maxLifetime;
        p.active = true;
        break;
    }
}

// ============================================================
// spawnHealEffect - 治疗效果
// ============================================================
// 从position向上升起的绿色粒子（治疗时使用）
// 20个粒子向上发射，有轻微左右扩散
void ParticleSystem::spawnHealEffect(sf::Vector2f position) {
    for (int i = 0; i < 20; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            // 向上（-π/2），有±1.2弧度的扩散
            float angle = -3.14159f * 0.5f + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.2f;
            float speed = 80.f + static_cast<float>(rand()) / RAND_MAX * 60.f;
            // 在position附近30像素范围内随机
            p.position = position + sf::Vector2f(
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 30.f,
                (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 30.f);
            p.velocity = sf::Vector2f(std::cosf(angle) * speed, std::sinf(angle) * speed);
            p.color = sf::Color(100, 255, 150, 200); // 绿色带透明度
            p.maxLifetime = 0.6f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

// ============================================================
// spawnPhaseTransition - 阶段转换
// ============================================================
// 从position向四周爆发的白色粒子（Boss阶段转换时使用）
// 40个粒子，快速向四周飞散
void ParticleSystem::spawnPhaseTransition(sf::Vector2f position) {
    for (int i = 0; i < 40; ++i) {
        for (auto& p : particles) {
            if (p.active) continue;
            float angle = static_cast<float>(rand()) / RAND_MAX * 6.28318f;
            // 高速（200~500像素/秒）
            float speed = 200.f + static_cast<float>(rand()) / RAND_MAX * 300.f;
            p.position = position;
            p.velocity = sf::Vector2f(std::cosf(angle) * speed, std::sinf(angle) * speed);
            p.color = sf::Color(255, 255, 255, 220); // 白色
            p.maxLifetime = 0.4f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
            p.lifetime = p.maxLifetime;
            p.active = true;
            break;
        }
    }
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 更新所有激活粒子的位置、速度（乘以0.96衰减）、寿命
// 寿命耗尽则标记为非激活
void ParticleSystem::update(float dt) {
    for (auto& p : particles) {
        if (!p.active) continue;           // 跳过非激活粒子

        p.position += p.velocity * dt;      // 位置更新
        p.velocity *= 0.96f;               // 速度衰减（每帧×0.96）
        p.lifetime -= dt;                   // 寿命减少

        if (p.lifetime <= 0.f) {           // 寿命耗尽
            p.active = false;               // 标记为非激活，回收至对象池
        }
    }
}

// ============================================================
// render - 渲染粒子
// ============================================================
// 使用sf::VertexArray绘制所有激活粒子
// 每个粒子渲染为2个三角形构成的小方形（6个顶点）
// 透明度随寿命衰减：alpha = lifetime / maxLifetime
// 使用加法混合模式（Additive Blending）实现发光效果
// SrcAlpha × One：源颜色 × 透明度 + 目标颜色 × 1（叠加）
void ParticleSystem::render(sf::RenderWindow& window) {
    sf::VertexArray va(sf::PrimitiveType::Triangles);

    for (const auto& p : particles) {
        if (!p.active) continue;            // 跳过非激活粒子

        // 计算透明度：lifetime/maxLifetime从1衰减到0
        float alpha = p.lifetime / p.maxLifetime;
        sf::Color c = p.color;
        c.a = static_cast<std::uint8_t>(255.f * alpha); // 透明度

        float halfSize = 3.f;               // 粒子半尺寸
        sf::Vector2f pos = p.position;      // 粒子位置

        // ============================================================
        // 构建2个三角形（构成一个方形）
        // 三角形1：左上、右上、右下
        // 三角形2：左上、右下、左下
        // ============================================================
        sf::Vertex v1, v2, v3, v4, v5, v6;
        v1.position = sf::Vector2f(pos.x - halfSize, pos.y - halfSize);
        v1.color = c;
        v2.position = sf::Vector2f(pos.x + halfSize, pos.y - halfSize);
        v2.color = c;
        v3.position = sf::Vector2f(pos.x + halfSize, pos.y + halfSize);
        v3.color = c;
        v4.position = sf::Vector2f(pos.x - halfSize, pos.y - halfSize);
        v4.color = c;
        v5.position = sf::Vector2f(pos.x + halfSize, pos.y + halfSize);
        v5.color = c;
        v6.position = sf::Vector2f(pos.x - halfSize, pos.y + halfSize);
        v6.color = c;
        va.append(v1);
        va.append(v2);
        va.append(v3);
        va.append(v4);
        va.append(v5);
        va.append(v6);
    }

    // 加法混合模式：源颜色×透明度 + 目标颜色
    // 效果：多个发光粒子叠加会变得更亮
    sf::BlendMode additive(sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::One);
    window.draw(va, additive);
}