# Neon Bullet Arena

一款基于 SFML 3 的 2D 动作/弹幕游戏。击败外星 Boss，通过三个难度递增的阶段，收集道具强化自己，掌握六种不同的武器类型。

## 构建与运行

```bash
# 在 c--main 目录下
cmake -B build-ninja -G Ninja
cmake --build build-ninja

# 运行
./build-ninja/NeonBulletArena.exe
```

字体回退机制：优先尝试 `C:/Windows/Fonts/consola.ttf`，失败则使用 `resources/font.ttf`。

## 操作说明

| 按键 | 功能 |
|------|------|
| WASD | 移动 |
| 鼠标 | 瞄准 |
| 鼠标左键 | 射击 |
| LShift | 冲刺 |
| Q | 切换武器 |
| Escape | 暂停 |
| F1–F6 | 调试键（回血、伤害 Boss、无敌、清子弹、HP 减半、秒杀 Boss） |

## 玩家武器

| 武器 | 说明 |
|------|------|
| Normal | 标准子弹 |
| Spread | 5 路散弹 |
| Piercing | 穿透敌人（击中后不消失） |
| Orbital | 4 颗子弹环绕玩家 |
| Cluster | 撞击时爆炸 |
| Homing | 自动追踪 Boss |

## Boss 阶段

- **阶段 1**（HP 100%–70%）：圆形爆发、瞄准射击、螺旋弹幕、花瓣爆发、八向射击
- **阶段 2**（HP 70%–35%）：新增波动弹幕、十字激光、双螺旋、环形刀刃、预判射击、蓄力光束
- **阶段 3**（HP 35%–0%）：新增蛇形波动、追踪球、屏幕横扫、径向爆发、随机喷射、触手横扫、屏幕锁定、弹幕暴雨、相位跃迁

## 道具

| 道具 | 效果 |
|------|------|
| Heal Core | 恢复 25% 最大生命值 |
| Shield Orb | 3 秒无敌护盾 |
| Overdrive | 射速 +100%，伤害 +30%，持续 6 秒 |
| Bullet Time | Boss 子弹速度降低 50%，持续 5 秒 |
| Nova Bomb | 清除所有 Boss 弹幕 + 造成 150 伤害 |
| Dash Battery | 重置冲刺冷却 + 1.5 倍移速，持续 4 秒 |
| Weapon Essence | 切换主副武器形态 |
| Phase Crystal | 1 秒无敌相位位移 |
| Speed Coil | 移速 +50%，持续 8 秒 |
| Attack Module | 伤害 +30%，持续 10 秒 |
| Nova Core | 全屏新星冲击波 + 对 Boss 造成 100 伤害 |

## 架构设计

- **游戏循环**：`processEvents() → update(dt) → render()`，Delta-time 上限 0.1 秒
- **状态机**：`MainMenu → Playing → Paused / Victory / GameOver`
- **中央协调器**：`Game` 类拥有所有子系统与实体，协调碰撞检测、阶段转换和胜负判定
- **对象池**：Boss 子弹最多 2000 发，玩家子弹最多 200 发
- **单例模式**：`AudioSystem` 以程序化方式生成所有音效（无需外部音频文件）
- **配置常量**：`Config.h` 中约 200 个 `constexpr` 常量管理所有游戏数值

## 视觉效果

- 粒子使用加法混合（Additive Blend）实现发光感
- 相机震动偏移整个 View 取代局部抖动
- Nova Form 触发时有扩散光环效果
- Bullet Time 触发时屏幕边缘出现紫色暗角（vignette）