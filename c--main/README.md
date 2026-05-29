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

## 审计发现（已知问题）

以下为代码审计中发现的问题：

### 严重（建议优先修复）

| # | 位置 | 问题描述 |
|---|------|----------|
| 1 | `Game.cpp:416` | **轨道弹击中 Boss 后不 deactivate**。轨道弹每帧与 Boss 重叠时都会造成伤害，因为没有调用 `bullet.deactivate()`。对比 cluster 子弹（第 434 行）正确调用了 deactivate。 |
| 2 | `Game.cpp:405` | **穿透弹击中 Boss 后不 deactivate**。同样问题——穿透弹击中 Boss 后依然存活，每帧重叠都持续造成伤害。 |
| 3 | `Boss.cpp:1072` | **子弹池耗尽时静默失败**。当 2000 发子弹全部激活时，`spawnBullet()` 直接返回，不会有任何提示，弹幕在密集时会凭空消失。 |

### 中等（建议修复）

| 4 | `Player.cpp:148` | Spread 武器的射击音在内部循环中每颗子弹播放一次。5 路散弹会导致 `playShoot()` 调用 5 次，而非每轮齐射播放一次。 |
| 5 | `Player.cpp:168` | 轨道弹没有冷却时间检测。每次调用 `shoot()` 且装备轨道弹时，所有未激活的轨道弹都会生成，没有类似其他武器的限速机制。 |
| 6 | `Game.cpp:62` | `resetGame()` 不重置 `state` 成员。调用方必须在之后手动设置状态，脆弱——如果直接调用会导致状态不一致。 |
| 7 | `Game.cpp:456` | Boss 子弹碰撞循环中没有检查 `player.isDead()`。死亡判定检查之前，所有在飞弹幕都会持续对玩家造成伤害。 |
| 8 | `Config.h:186` | `NOVA_FORM_DURATION = 0.5s`，持续时间过短。60fps 下只能渲染约 29 帧，新星环效果几乎看不清就消失了。 |

### 轻微 / 设计相关

| 9 | `Game.cpp:122` | F2 热键将 float 伤害值截断为 int。Boss HP 较低时可能得到比预期更少的伤害（如 HP=10 → 截断为 4）。 |
| 10 | `Item.cpp:536` | `getLifetime()` 返回静态常量 `ITEM_LIFETIME`（8.0），而非实例成员 `lifetime`。所有道具无论各自状态如何，都按同一常量计时。 |
| 11 | `Game.cpp:463` | 相变追踪使用两个独立变量：局部 `bossPhaseBefore` 和类成员 `prevBossPhase` 各自记录同一概念，导致状态管理碎片化。 |