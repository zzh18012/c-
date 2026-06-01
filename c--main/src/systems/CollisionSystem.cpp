// ============================================================
// CollisionSystem.cpp - 碰撞检测系统实现
// ============================================================
// 职责：提供游戏中的碰撞检测功能
// 碰撞检测基于圆形碰撞盒（circle-circle）
// 优化：比较距离的平方避免开方运算（性能优化）
// ============================================================

#include "CollisionSystem.h"
#include <cmath> // 未使用但保留（原本可能用std::sqrt）

// ============================================================
// checkCircleCollision - 圆形碰撞检测
// ============================================================
// 圆形碰撞检测原理：
// 两圆相交 ⟺ 圆心距离 < 半径之和
// 即：distance(pos1, pos2) < r1 + r2
//
// 优化：不计算实际距离（需要sqrt），而是比较距离的平方
// distance² < (r1 + r2)²
// (dx*dx + dy*dy) < (r1+r2)*(r1+r2)
//
// 数学：
// dx = pos1.x - pos2.x
// dy = pos1.y - pos2.y
// distSq = dx*dx + dy*dy
// radiusSum = r1 + r2
// collision = distSq <= radiusSum * radiusSum
bool CollisionSystem::checkCircleCollision(
    const sf::Vector2f& pos1, float r1,
    const sf::Vector2f& pos2, float r2
) {
    float dx = pos1.x - pos2.x;      // X轴距离差
    float dy = pos1.y - pos2.y;      // Y轴距离差
    float distSq = dx * dx + dy * dy; // 距离的平方（避免开方）
    float radiusSum = r1 + r2;        // 半径之和
    return distSq <= radiusSum * radiusSum; // 比较平方值
}