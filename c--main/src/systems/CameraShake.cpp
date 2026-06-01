// ============================================================
// CameraShake.cpp - 相机抖动系统实现
// ============================================================
// 职责：提供屏幕震动效果（用于攻击命中、阶段转换等事件）
// 工作原理：每帧生成随机偏移量，随时间衰减
// ============================================================

#include "CameraShake.h"
#include <cstdlib>  // rand(), RAND_MAX
#include <algorithm> // std::max

// ============================================================
// 构造函数
// ============================================================
// 初始化为不抖动状态
CameraShake::CameraShake()
    : intensity(0.f), duration(0.f), timer(0.f), offset(0.f, 0.f)
{
}

// ============================================================
// shake - 触发抖动
// ============================================================
// intensity: 抖动强度（最大偏移量，像素）
// duration: 抖动持续时间（秒）
// 如果已有抖动在进行，新调用会取两者的较大值（而非替换）
void CameraShake::shake(float intensity, float duration) {
    this->intensity = std::max(this->intensity, intensity); // 取较大值
    this->duration = std::max(this->duration, duration);   // 取较大值
    timer = 0.f; // 重置计时器
}

// ============================================================
// update - 每帧更新
// ============================================================
// dt: 时间差（秒）
// 在duration时间内计算随机偏移，超出后归零
// 偏移计算：random(-1, +1) × intensity × decay
// decay = 1 - timer/duration，随时间线性衰减
void CameraShake::update(float dt) {
    if (timer < duration) {                      // 在抖动持续时间内
        timer += dt;                             // 更新已过去时间
        float decay = 1.f - (timer / duration); // 衰减因子：从1到0
        float currentIntensity = intensity * decay; // 当前强度 = 强度 × 衰减

        // 生成随机偏移：-1到+1之间，乘以当前强度
        // rand() / RAND_MAX → 0到1
        // × 2.f → 0到2
        // - 1.f → -1到+1
        offset.x = (static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f) * currentIntensity;
        offset.y = (static_cast<float>(rand()) / RAND_MAX * 2.f - 1.f) * currentIntensity;
    } else {
        offset = sf::Vector2f(0.f, 0.f); // 抖动结束，偏移归零
    }
}

// ============================================================
// getOffset - 获取当前偏移
// ============================================================
// 返回当前帧的抖动偏移量
// Game.cpp使用：view.setCenter(view.getCenter() + offset)
sf::Vector2f CameraShake::getOffset() const {
    return offset;
}