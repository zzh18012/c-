// ============================================================
// CameraShake.h - 相机抖动系统声明
// ============================================================
// 职责：提供屏幕震动效果（用于攻击命中、阶段转换等事件）
// 效果是让整个画面产生随机偏移，模拟真实感
// 由Game.cpp每帧调用update，并根据offset调整视图
// ============================================================

#pragma once

// SFML图形库（用于sf::Vector2f）
#include <SFML/Graphics.hpp>

// ============================================================
// CameraShake类
// ============================================================
// 相机抖动系统，使用随机偏移模拟震动效果
// 工作原理：
// 1. 调用shake(intensity, duration)启动抖动
// 2. 每帧update计算新的随机偏移
// 3. 偏移量随时间衰减（decay = 1 - timer/duration）
// 4. Game.cpp获取offset并应用到视图
class CameraShake {
public:
    // 构造函数：初始化为不抖动状态（intensity=0, duration=0, offset=(0,0)）
    CameraShake();

    // ---- 触发抖动 ----
    // intensity: 抖动强度（像素为单位，最大偏移量）
    // duration: 抖动持续时间（秒）
    // 如果已有抖动在进行，新调用会取两者的较大值
    void shake(float intensity, float duration);

    // ---- 每帧更新 ----
    // dt: 时间差（秒）
    // 计算新的随机偏移：offset = random(-intensity, +intensity) * decay
    void update(float dt);

    // ---- 获取偏移 ----
    // 返回当前帧的抖动偏移量
    // Game.cpp在渲染前将此偏移应用到视图
    sf::Vector2f getOffset() const;

private:
    // ---- 抖动参数 ----
    float intensity;  // 抖动强度（最大偏移量，像素）
    float duration;  // 抖动持续时间（秒）
    float timer;    // 已过去的时间（秒）

    // ---- 当前偏移 ----
    // 每帧更新的随机偏移量
    // 用于Game.cpp设置视图偏移：window.setView(view.getCenter() + offset)
    sf::Vector2f offset;
};