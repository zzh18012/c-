// ============================================================
// Game.cpp - 游戏主控制器
// ============================================================
// 职责：游戏主循环、控制所有子系统的协调、碰撞检测、游戏状态切换
// 游戏循环：processEvents() → update(dt) → render()
// ============================================================

#include "Game.h"
#include "Config.h"
#include "systems/CollisionSystem.h"
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cmath>

// ============================================================
// 构造函数
// ============================================================
// 初始化游戏窗口、菜单、HUD、粒子系统等所有子系统
// ============================================================
Game::Game()
    // 初始化SFML窗口：1280×720分辨率，标题"Neon Bullet Arena"，60fps限制
    : window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), WINDOW_TITLE)
    , running(true)                          // 游戏是否继续运行
    , itemSpawnTimer(ITEM_SPAWN_INTERVAL)    // 道具生成计时器（初始为间隔时间，立即可生成）
    , bulletTimeTimer(0.f)                  // 子弹时间效果计时器（初始为0，未激活）
    , healSpawnedAt75(false)                // 75%血量补血是否已生成（防止重复生成）
    , healSpawnedAt50(false)                // 50%血量补血是否已生成
    , healSpawnedAt25(false)                // 25%血量补血是否已生成
    // novaRingMaxRadius：对角线长度，用于NovaBomb扩散圆环的最大半径
    , novaRingMaxRadius(sqrt(WINDOW_WIDTH*WINDOW_WIDTH + WINDOW_HEIGHT*WINDOW_HEIGHT))
{
    // 限制窗口最大帧率为60fps，避免CPU过度消耗
    window.setFramerateLimit(60);

    // 将items数组预分配到最大道具数（ITEM_MAX_COUNT = 2）
    items.resize(ITEM_MAX_COUNT);

    // ---- 加载字体 ----
    // 优先尝试Windows系统字体（Consolas等宽字体，适合游戏UI）
    if (!font.openFromFile("C:/Windows/Fonts/consola.ttf")) {
        // 如果系统字体加载失败，则使用项目resources目录下的字体文件
        (void)font.openFromFile("resources/font.ttf");
    }

    // 初始化HUD（传入字体，HUD会创建内部文字对象）
    hud.init(font);

    // 设置道具的粒子系统引用（道具激活时需要产生粒子特效）
    Item::setParticleSystem(&particles);

    // 记录玩家初始HP，用于后续判断玩家是否掉血（触发受击特效）
    prevPlayerHP = player.getMaxHP();

    // ---- 显示主菜单 ----
    menu.show(MenuMode::MainMenu, font);

    // 在主菜单上动态创建4个按钮（NORMAL / HARD / LUNATIC / QUIT）
    // 每个按钮绑定一个lambda回调，点击时设置难度并开始游戏
    menu.getButton(0) = Button("NORMAL", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),         // 居中，y=240
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() { currentDifficulty = Difficulty::Normal; startGame(); });   // 回调：设为Normal难度并开始
    menu.getButton(1) = Button("HARD", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
    menu.getButton(2) = Button("LUNATIC", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
    menu.getButton(3) = Button("QUIT", font,
        sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
        sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
        [this]() { window.close(); });   // 回调：关闭窗口，程序退出
}

// ============================================================
// 游戏主循环
// ============================================================
// 标准SFML游戏循环：每帧获取delta时间，处理事件，更新，渲染
// ============================================================
void Game::run() {
    // 窗口打开且running=true时持续循环
    while (window.isOpen() && running) {
        // restart()返回距上次调用经过的时间（秒），并重置时钟
        float dt = clock.restart().asSeconds();

        // 防止delta过大（游戏卡顿时超过0.1秒则截断到0.1秒）
        // 避免物理计算出现异常（如穿墙、子弹速度异常）
        if (dt > 0.1f) dt = 0.1f;

        // 处理输入事件（键盘、鼠标、窗口关闭等）
        processEvents();
        // 更新游戏逻辑（移动、碰撞、状态切换）
        update(dt);
        // 渲染画面（背景→实体→粒子→HUD→菜单）
        render();
    }
}

// ============================================================
// 开始游戏
// ============================================================
// 从主菜单点击"开始"后调用，初始化Boss管理器、播放背景音乐
// ============================================================
void Game::startGame() {
    state = GameState::Playing;          // 切换到游戏中状态
    menu.setVisible(false);             // 隐藏菜单（不再响应菜单按钮）
    bossManager.init(currentDifficulty); // 根据难度初始化Boss（Normal=1个，Hard=2个，Lunatic=3个）
    prevPlayerHP = player.getHP();       // 重置玩家HP追踪值
    audio.startBackgroundMusic();        // 开始播放背景音乐
}

// ============================================================
// 重置游戏
// ============================================================
// 回到主菜单或重新开始时调用，清除所有状态，重新创建干净的游戏对象
// ============================================================
void Game::resetGame() {
    // 重新创建Player对象（调用默认构造函数，位置/血量等恢复初始值）
    player = Player();
    // 重新创建BossManager（清空所有Boss）
    bossManager = BossManager();
    // 遍历所有道具，逐个deactivate（设为非活跃，不显示）
    for (auto& item : items) item.deactivate();
    // 重置道具生成计时器
    itemSpawnTimer = ITEM_SPAWN_INTERVAL;
    // 重置子弹时间
    bulletTimeTimer = 0.f;
    // 重置Boss血量节点触发标志（下次游戏可以再次触发）
    healSpawnedAt75 = false;
    healSpawnedAt50 = false;
    healSpawnedAt25 = false;
    // 关闭无敌模式
    godModeActive = false;

    // 重置游戏统计信息
    gameStats = GameStats();
    // 重置Boss阶段追踪
    prevBossPhase = 1;
    // 重置玩家HP追踪
    prevPlayerHP = player.getMaxHP();

    // 清空粒子系统和相机震动（清除残留特效）
    particles = ParticleSystem();
    cameraShake = CameraShake();
}

// ============================================================
// 事件处理
// ============================================================
// 处理所有SFML事件：窗口关闭、键盘输入、鼠标点击
// 根据当前游戏状态（主菜单/游戏中/暂停/胜利/失败）分发到不同处理器
// ============================================================
void Game::processEvents() {
    // 遍历所有待处理的事件（while auto event = ... 是SFML 3的写法）
    while (auto event = window.pollEvent()) {
        // ---- 窗口关闭事件 ----
        if (event->getIf<sf::Event::Closed>()) {
            window.close();  // 关闭窗口，游戏循环结束
            return;          // 跳过本帧剩余事件
        }

        // ---- 根据游戏状态分发事件 ----
        switch (state) {
        // === 主菜单状态 ===
        case GameState::MainMenu:
            // 主菜单只响应按钮点击和QUIT按钮的关闭窗口
            menu.handleEvent(*event, window);
            break;

        // === 游戏中状态 ===
        case GameState::Playing:
            // 只处理键盘按下事件
            if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyEvent->code) {
                    case sf::Keyboard::Key::Escape:
                        // ESC暂停游戏
                        state = GameState::Paused;
                        // 显示暂停菜单
                        menu.show(MenuMode::Pause, font);
                        // 动态创建RESUME和QUIT TO MENU按钮
                        menu.getButton(0) = Button("RESUME", font,
                            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 320.f),
                            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                            [this]() { state = GameState::Playing; menu.setVisible(false); });
                        menu.getButton(1) = Button("QUIT TO MENU", font,
                            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 320.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                            // 点击QUIT TO MENU：重置游戏→回到主菜单→重新创建主菜单按钮
                            [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                                menu.getButton(0) = Button("NORMAL", font,
                                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
                                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                                    [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
                                menu.getButton(1) = Button("HARD", font,
                                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                                    [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
                                menu.getButton(2) = Button("LUNATIC", font,
                                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                                    [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
                                menu.getButton(3) = Button("QUIT", font,
                                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                                    [this]() { window.close(); });
                            });
                        break;

                    // ---- 调试快捷键 ----
                    case sf::Keyboard::Key::F1:
                        // F1：瞬间满血（调试用）
                        player.heal(PLAYER_MAX_HP);
                        break;
                    case sf::Keyboard::Key::F3:
                        // F3：开关无敌模式
                        godModeActive = !godModeActive;
                        player.setGodMode(godModeActive);
                        break;
                    default:
                        break;
                }
            }
            break;

        // === 暂停状态 ===
        case GameState::Paused:
            // ESC再次按下则解除暂停
            if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    state = GameState::Playing;
                    menu.setVisible(false);  // 隐藏暂停菜单
                }
            }
            // 暂停时仍然响应菜单按钮（RESUME / QUIT TO MENU）
            menu.handleEvent(*event, window);
            break;

        // === 胜利/失败状态 ===
        case GameState::Victory:
        case GameState::GameOver:
            // 这两个状态下只响应菜单按钮（PLAY AGAIN / MAIN MENU）
            menu.handleEvent(*event, window);
            break;
        }
    }
}

// ============================================================
// 游戏逻辑更新（每帧调用）
// ============================================================
// dt：距上一帧的时间（秒），用于帧率无关的运动计算
// ============================================================
void Game::update(float dt) {
    // ---- 始终更新的系统（不受游戏状态影响）----
    background.update(dt);      // 背景星空/网格动画持续播放
    particles.update(dt);       // 粒子特效持续更新（爆炸、拖尾等）
    cameraShake.update(dt);    // 相机震动持续衰减

    // ---- 非游戏状态直接返回（不更新游戏逻辑）----
    if (state == GameState::MainMenu || state == GameState::Paused) {
        menu.update(dt);  // 菜单动画仍然更新
        return;
    }
    if (state == GameState::Victory || state == GameState::GameOver) {
        menu.update(dt);           // 胜利/失败菜单动画
        hud.update(gameStats);     // HUD也继续更新（显示结算信息）
        return;
    }

    // === 以下为游戏中状态（Playing）===

    // 累加游戏已运行时间（用于游戏统计）
    gameStats.elapsedTime += dt;

    // ---- 玩家输入与移动 ----
    player.handleInput(dt, window);  // 检测键盘输入（WASD/射击/闪避）
    player.update(dt);              // 更新玩家位置、射击冷却、状态效果

    // ---- Boss管理更新 ----
    bossManager.update(dt, player.getPosition());  // 更新所有Boss的位置、攻击、入场动画

    // ---- 追踪玩家HP变化（用于触发受击特效）----
    int playerHPBefore = player.getHP();  // 在碰撞检测前记录，用于后面判断是否掉血

    // ---- 子弹时间效果 ----
    if (bulletTimeTimer > 0.f) {
        bulletTimeTimer -= dt;  // 倒计时
        if (bulletTimeTimer <= 0.f) {
            // 子弹时间结束时，清空所有Boss子弹
            bossManager.clearAllBullets();
        }
    }

    // ---- 屏幕闪烁效果 ----
    if (screenFlashTimer > 0.f) {
        screenFlashTimer -= dt;  // 倒计时消退
    }

    // ---- Overdrive时的轻微屏幕震动 ----
    if (player.getOverdriveTimer() > 0.f) {
        static float shakeAccum = 0.f;  // 累积时间（静态变量，跨帧保留）
        shakeAccum += dt;
        if (shakeAccum >= 0.5f) {      // 每0.5秒触发一次轻微震动
            shakeAccum = 0.f;
            cameraShake.shake(2.f, 0.15f);  // 强度2，持续0.15秒
        }
    }

    // ---- Nova Bomb/NovaCore的扩散圆环效果 ----
    if (novaRingActive) {
        // 圆环半径每帧扩大（1200像素/秒）
        novaRingRadius += dt * 1200.f;
        // 在当前位置生成粒子（圆环形状）
        particles.spawnNovaRing(player.getPosition(), novaRingRadius, novaRingColor, 8);
        // 圆环半径超过屏幕对角线时结束
        if (novaRingRadius >= novaRingMaxRadius) {
            novaRingActive = false;
            novaRingRadius = 0.f;
        }
    }

    // ---- 强制生成回血道具（当Boss血量低于特定阈值）----
    // 找到当前HP比例最低的Boss
    float lowestHPRatio = 1.f;
    for (auto& b : bossManager.getBosses()) {
        if (!b.isDead()) {  // 只看存活的Boss
            float ratio = static_cast<float>(b.getHP()) / b.getMaxHP();
            if (ratio < lowestHPRatio) lowestHPRatio = ratio;
        }
    }

    // 定义一个生成HealCore的函数（检查空闲道具槽，生成在随机位置）
    auto spawnHealCore = [&]() {
        for (auto& item : items) {
            if (!item.isActive()) {  // 找到第一个非活跃的道具槽
                // 在屏幕内随机生成位置（留出边距防止贴边）
                float x = 80.f + static_cast<float>(rand() % (WINDOW_WIDTH - 160));
                float y = 80.f + static_cast<float>(rand() % (WINDOW_HEIGHT - 160));
                item.spawn(ItemType::HealCore, sf::Vector2f(x, y));  // 生成HealCore道具
                return;
            }
        }
    };

    // 检测三个阈值，逐个触发（每个阈值只触发一次）
    if (!healSpawnedAt75 && lowestHPRatio <= 0.75f) {
        healSpawnedAt75 = true;
        spawnHealCore();
    }
    if (!healSpawnedAt50 && lowestHPRatio <= 0.50f) {
        healSpawnedAt50 = true;
        spawnHealCore();
    }
    if (!healSpawnedAt25 && lowestHPRatio <= 0.25f) {
        healSpawnedAt25 = true;
        spawnHealCore();
    }

    // ---- 常规道具生成（定时器驱动）----
    itemSpawnTimer -= dt;
    if (itemSpawnTimer <= 0.f) {
        // 重置计时器
        itemSpawnTimer = ITEM_SPAWN_INTERVAL;  // 10秒间隔
        // 统计当前活跃道具数量
        int activeCount = 0;
        for (const auto& item : items) if (item.isActive()) activeCount++;
        // 如果道具未满上限，则生成新道具
        if (activeCount < ITEM_MAX_COUNT) {
            // 找到当前所有Boss中的最高阶段（阶段越高，道具类型越强）
            int maxPhase = 1;
            for (auto& b : bossManager.getBosses()) {
                if (!b.isDead() && b.getPhase() > maxPhase) maxPhase = b.getPhase();
            }
            // 遍历道具槽，找到第一个空闲的生成
            for (auto& item : items) {
                if (!item.isActive()) {
                    int phase = maxPhase;
                    ItemType type;
                    // 根据阶段决定道具类型（阶段1/2/3各有不同的道具池）
                    if (phase == 1) {
                        int r = rand() % 5;
                        if (r == 0) type = ItemType::HealCore;
                        else if (r == 1) type = ItemType::DashBattery;
                        else if (r == 2) type = ItemType::WeaponEssence;
                        else if (r == 3) type = ItemType::SpeedCoil;
                        else type = ItemType::AttackModule;
                    } else if (phase == 2) {
                        int r = rand() % 5;
                        if (r == 0) type = ItemType::ShieldOrb;
                        else if (r == 1) type = ItemType::Overdrive;
                        else if (r == 2) type = ItemType::PhaseCrystal;
                        else if (r == 3) type = ItemType::WeaponEssence;
                        else type = ItemType::NovaCore;
                    } else {
                        int r = rand() % 5;
                        if (r == 0) type = ItemType::BulletTime;
                        else if (r == 1) type = ItemType::NovaBomb;
                        else if (r == 2) type = ItemType::PhaseCrystal;
                        else if (r == 3) type = ItemType::NovaCore;
                        else type = ItemType::WeaponEssence;
                    }
                    // 随机位置生成道具
                    float x = 80.f + static_cast<float>(rand() % (WINDOW_WIDTH - 160));
                    float y = 80.f + static_cast<float>(rand() % (WINDOW_HEIGHT - 160));
                    item.spawn(type, sf::Vector2f(x, y));
                    break;  // 只生成一个，跳出循环
                }
            }
        }
    }

    // ---- 更新所有道具（存活/消失动画）----
    for (auto& item : items) item.update(dt);

    // ============================================================
    // 道具碰撞检测（玩家碰到道具）
    // ============================================================
    for (auto& item : items) {
        if (!item.isActive()) continue;  // 非活跃道具跳过

        // 圆形碰撞检测：玩家和道具都是圆形
        if (CollisionSystem::checkCircleCollision(
                player.getPosition(), player.getRadius(),
                item.getPosition(), item.getRadius())) {

            // 生成拾取粒子特效（闪烁火花）
            particles.spawnHitSpark(item.getPosition(), Item::getColor(item.getType()));

            // ---- 根据道具类型触发不同效果 ----
            switch (item.getType()) {
                case ItemType::HealCore:
                    // 回血25%，屏幕闪绿，生成治疗粒子和爆炸特效
                    player.heal(static_cast<int>(player.getMaxHP() * HEAL_CORE_RATIO));
                    screenFlashTimer = 0.2f;  // 屏幕闪绿色
                    screenFlashColor = sf::Color(100, 255, 150, 120);
                    particles.spawnHealEffect(player.getPosition());
                    particles.spawnExplosion(player.getPosition(), sf::Color(60, 255, 100), 20);
                    break;

                case ItemType::ShieldOrb:
                    // 激活护盾（3秒内无敌），屏幕闪蓝
                    player.activateShieldOrb();
                    screenFlashTimer = 0.15f;
                    screenFlashColor = sf::Color(60, 140, 255, 100);
                    particles.spawnExplosion(player.getPosition(), sf::Color(60, 140, 255), 15);
                    break;

                case ItemType::Overdrive:
                    // 火力全开（射速提升，伤害提升），屏幕闪橙
                    player.activateOverdrive();
                    screenFlashTimer = 0.25f;
                    screenFlashColor = sf::Color(255, 100, 40, 130);
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 100, 40), 25);
                    break;

                case ItemType::BulletTime:
                    // 清空所有Boss子弹，触发子弹时间效果（5秒）
                    bossManager.clearAllBullets();
                    bulletTimeTimer = BULLET_TIME_DURATION;
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(180, 60, 255, 110);
                    particles.spawnExplosion(player.getPosition(), sf::Color(180, 60, 255), 20);
                    break;

                case ItemType::NovaBomb:
                    // 清空Boss子弹+对所有Boss造成150伤害+扩散圆环特效
                    bossManager.clearAllBullets();
                    for (auto& boss : bossManager.getBosses()) {
                        if (!boss.isDead()) boss.takeDamage(NOVA_BOMB_DAMAGE);
                    }
                    novaRingActive = true;         // 开启圆环动画
                    novaRingRadius = 0.f;
                    novaRingColor = sf::Color(255, 255, 100);  // 金黄色圆环
                    particles.spawnExplosion(item.getPosition(), sf::Color(255, 220, 50), 40);
                    cameraShake.shake(12.f, 0.6f);  // 剧烈屏幕震动
                    break;

                case ItemType::DashBattery:
                    // 激活闪避强化（4秒内闪避速度×1.5），闪电+爆炸特效
                    player.activateDashBattery();
                    screenFlashTimer = 0.15f;
                    screenFlashColor = sf::Color(255, 255, 60, 100);
                    particles.spawnLightning(player.getPosition(), player.getPosition() + sf::Vector2f(30.f, -30.f), sf::Color(255, 255, 60));
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 255, 60), 15);
                    break;

                case ItemType::WeaponEssence:
                    // 切换到下一种武器（Normal→Spread→Piercing→Orbital→Cluster→Homing→Normal循环）
                    player.cycleWeapon();
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(200, 100, 255, 120);
                    particles.spawnExplosion(player.getPosition(), sf::Color(200, 100, 255), 20);
                    break;

                case ItemType::PhaseCrystal:
                    // 触发相位偏移（无敌+穿怪），屏幕闪紫
                    player.activatePhaseShift();
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(150, 50, 200, 120);
                    particles.spawnExplosion(player.getPosition(), sf::Color(150, 50, 200), 25);
                    break;

                case ItemType::SpeedCoil:
                    // 速度强化（8秒内移动速度×1.5），屏幕闪青
                    player.activateSpeedCoil();
                    screenFlashTimer = 0.15f;
                    screenFlashColor = sf::Color(100, 255, 200, 100);
                    particles.spawnExplosion(player.getPosition(), sf::Color(100, 255, 200), 15);
                    break;

                case ItemType::AttackModule:
                    // 攻击强化（10秒内伤害×1.3），屏幕闪红
                    player.activateAttackBoost();
                    screenFlashTimer = 0.2f;
                    screenFlashColor = sf::Color(255, 80, 80, 120);
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 80, 80), 20);
                    break;

                case ItemType::NovaCore:
                    // 最强道具：清子弹+高伤害+大圆环特效+剧烈震动
                    player.activateNovaForm();
                    bossManager.clearAllBullets();
                    for (auto& boss : bossManager.getBosses()) {
                        if (!boss.isDead()) boss.takeDamage(NOVA_FORM_DAMAGE);
                    }
                    novaRingActive = true;
                    novaRingRadius = 0.f;
                    novaRingColor = sf::Color(255, 150, 50);  // 橙红色圆环
                    particles.spawnExplosion(player.getPosition(), sf::Color(255, 150, 50), 50);
                    cameraShake.shake(15.f, 0.8f);
                    screenFlashTimer = 0.4f;
                    screenFlashColor = sf::Color(255, 200, 100, 150);
                    break;
            }

            // 拾取后道具消失
            item.deactivate();
        }
    }

    // ============================================================
    // 玩家子弹 vs Boss 碰撞检测（6种不同子弹类型分别检测）
    // ============================================================

    // 1. 普通子弹（玩家默认武器）
    for (auto& bullet : player.getBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            // 圆形碰撞检测
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                // 命中后消耗一次连击次数
                player.registerHit(0);
                // 计算伤害（基础伤害 × Combo加成）
                int dmg = static_cast<int>(bullet.getDamage() * player.getComboDamageMultiplier());
                boss.takeDamage(dmg);  // Boss扣血
                audio.playHit();        // 播放命中音效
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 255, 100));  // 黄色火花
                bullet.deactivate();     // 子弹消失（不穿透）
                break;  // 这发子弹已经命中，跳出Boss循环
            }
        }
    }

    // 2. 散射子弹（Spread）
    for (auto& bullet : player.getSpreadBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                // 散射子弹有0.7的伤害系数，+ Combo加成
                boss.takeDamage(bullet.getDamage() * player.getComboDamageMultiplier());
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(0, 200, 255));  // 青色火花
                bullet.deactivate();
                break;
            }
        }
    }

    // 3. 穿透子弹（Piercing）- 不消失，可穿透多个Boss
    for (auto& bullet : player.getPiercingBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage());  // 无Combo加成
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(100, 255, 200));  // 青绿色火花
                // 注意：不deactivate()，子弹继续飞，穿透下一个目标
            }
        }
    }

    // 4. 环绕子弹（Orbital）- 统计累积伤害后一次性结算
    {
        int totalOrbitalDamage = 0;  // 统计所有命中子弹的累积伤害
        for (auto& bullet : player.getOrbitalBullets()) {
            if (!bullet.isActive()) continue;
            for (auto& boss : bossManager.getBosses()) {
                if (boss.isDead()) continue;
                if (CollisionSystem::checkCircleCollision(
                        bullet.getPosition(), bullet.getRadius(),
                        boss.getPosition(), boss.getRadius())) {
                    // 每次碰撞累积伤害，并加上旋转圈数加成（每圈+5伤害）
                    totalOrbitalDamage += bullet.getDamage();
                    totalOrbitalDamage += bullet.getOrbitCount() * 5;
                    bullet.resetOrbitCount();   // 重置圈数计数
                    bullet.deactivate();        // 碰撞后消失
                    break;  // 这发子弹命中后只计算一次
                }
            }
        }
        // 如果有累积伤害，对第一个存活的Boss造成总伤害（一次性爆发）
        if (totalOrbitalDamage > 0) {
            for (auto& boss : bossManager.getBosses()) {
                if (!boss.isDead()) {
                    boss.takeDamage(totalOrbitalDamage);
                    break;
                }
            }
            audio.playHit();
        }
    }

    // 5. 集群子弹（Cluster）- 爆炸性伤害，命中后消失
    for (auto& bullet : player.getClusterBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage());
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 150, 50));  // 橙色火花
                bullet.deactivate();
                break;
            }
        }
    }

    // 6. 追踪子弹（Homing）- 命中后给Boss叠加减速Debuff
    for (auto& bullet : player.getHomingBullets()) {
        if (!bullet.isActive()) continue;
        for (auto& boss : bossManager.getBosses()) {
            if (boss.isDead()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    boss.getPosition(), boss.getRadius())) {
                boss.takeDamage(bullet.getDamage());
                boss.applySlow(1);    // 叠加1层减速（每层10%减速，上限3层）
                audio.playHit();
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 100, 200));  // 粉红色火花
                bullet.deactivate();
                break;
            }
        }
    }

    // ============================================================
    // Boss子弹 vs 玩家 碰撞检测
    // ============================================================
    for (auto& boss : bossManager.getBosses()) {
        if (boss.isDead()) continue;  // 死亡的Boss不产子弹，跳过
        for (auto& bullet : boss.getBullets()) {
            if (!bullet.isActive()) continue;
            if (CollisionSystem::checkCircleCollision(
                    bullet.getPosition(), bullet.getRadius(),
                    player.getPosition(), player.getRadius())) {
                player.takeDamage(bullet.getDamage());  // 玩家扣血
                particles.spawnHitSpark(bullet.getPosition(), sf::Color(255, 60, 40));  // 红色火花
                bullet.deactivate();  // 子弹消失
            }
        }
    }

    // ============================================================
    // Boss阶段切换触发特效
    // ============================================================
    for (auto& boss : bossManager.getBosses()) {
        if (boss.isDead()) continue;
        int currentPhase = boss.getPhase();
        // 当Boss阶段比之前记录的高时（发生阶段切换）
        if (currentPhase > prevBossPhase) {
            // 阶段切换时触发：相位过渡粒子特效 + 白色爆炸 + 屏幕震动 + 闪白
            particles.spawnPhaseTransition(boss.getPosition());
            particles.spawnExplosion(boss.getPosition(), sf::Color(255, 255, 255, 200), 25);
            cameraShake.shake(8.f, 0.4f);
            screenFlashTimer = 0.3f;
            screenFlashColor = sf::Color(255, 200, 220, 150);
            prevBossPhase = currentPhase;  // 更新追踪值
            break;  // 只处理一次（防止多个Boss同时切换阶段时重复触发）
        }
    }

    // ============================================================
    // 玩家受击触发特效（HP下降时）
    // ============================================================
    if (player.getHP() < playerHPBefore) {
        // 玩家掉血：红色火花粒子 + 中等屏幕震动
        particles.spawnHitSpark(player.getPosition(), sf::Color(255, 80, 60));
        cameraShake.shake(5.f, 0.3f);
    }

    // ============================================================
    // 胜利检测：所有Boss死亡
    // ============================================================
    if (bossManager.isAllBossesDead() && state == GameState::Playing) {
        gameStats.victory = true;           // 标记胜利
        state = GameState::Victory;         // 切换到胜利状态
        bossManager.clearAllBullets();      // 清空所有残留子弹
        // 在第一个Boss位置生成大型爆炸特效
        for (auto& boss : bossManager.getBosses()) {
            if (!boss.isDead()) {
                particles.spawnExplosion(boss.getPosition(), sf::Color(255, 140, 30), 60);
                break;
            }
        }
        cameraShake.shake(15.f, 0.8f);       // 剧烈震动
        audio.stopBackgroundMusic();         // 停止背景音乐
        audio.playVictory();                // 播放胜利音效
        // 显示胜利菜单
        menu.show(MenuMode::Victory, font);
        menu.getButton(0) = Button("PLAY AGAIN", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this]() { resetGame(); startGame(); });  // 重置后立即开始
        menu.getButton(1) = Button("MAIN MENU", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            // 回到主菜单，重新创建主菜单按钮
            [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                menu.getButton(0) = Button("NORMAL", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
                menu.getButton(1) = Button("HARD", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
                menu.getButton(2) = Button("LUNATIC", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
                menu.getButton(3) = Button("QUIT", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { window.close(); });
            });
    }

    // ============================================================
    // 失败检测：玩家死亡
    // ============================================================
    if (player.isDead() && state == GameState::Playing) {
        gameStats.gameOver = true;          // 标记失败
        state = GameState::GameOver;        // 切换到失败状态
        bossManager.clearAllBullets();      // 清空所有子弹
        particles.spawnExplosion(player.getPosition(), sf::Color(255, 50, 40), 30);  // 玩家位置爆炸
        cameraShake.shake(10.f, 0.6f);      // 中等震动
        audio.stopBackgroundMusic();        // 停止背景音乐
        audio.playGameOver();               // 播放失败音效
        menu.show(MenuMode::GameOver, font);
        menu.getButton(0) = Button("RETRY", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this]() { resetGame(); startGame(); });
        menu.getButton(1) = Button("MAIN MENU", font,
            sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 340.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
            sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
            [this]() { resetGame(); state = GameState::MainMenu; menu.show(MenuMode::MainMenu, font);
                menu.getButton(0) = Button("NORMAL", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { currentDifficulty = Difficulty::Normal; startGame(); });
                menu.getButton(1) = Button("HARD", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + BUTTON_HEIGHT + MENU_BUTTON_SPACING),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { currentDifficulty = Difficulty::Hard; startGame(); });
                menu.getButton(2) = Button("LUNATIC", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 2 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { currentDifficulty = Difficulty::Lunatic; startGame(); });
                menu.getButton(3) = Button("QUIT", font,
                    sf::Vector2f(WINDOW_WIDTH / 2.f - BUTTON_WIDTH / 2.f, 240.f + 3 * (BUTTON_HEIGHT + MENU_BUTTON_SPACING)),
                    sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT),
                    [this]() { window.close(); });
            });
    }

    // ============================================================
    // 同步GameStats（将所有游戏状态写入HUD显示数据）
    // ============================================================
    // 玩家状态
    gameStats.playerHP = player.getHP();
    gameStats.playerMaxHP = player.getMaxHP();
    gameStats.playerInvincible = player.isInvincible();
    gameStats.playerShieldTimer = player.getShieldTimer();
    gameStats.playerOverdriveTimer = player.getOverdriveTimer();
    gameStats.playerDashBatteryTimer = player.getDashBatteryTimer();
    gameStats.playerIsDashing = player.isDashing();
    gameStats.playerDashCooldown = player.getDashCooldown();
    gameStats.playerSpeedMultiplier = player.getSpeedMultiplier();
    gameStats.playerFireRateMultiplier = player.getFireRateMultiplier();
    gameStats.playerDamageMultiplier = player.getDamageMultiplier();
    gameStats.bulletTimeActive = bulletTimeTimer > 0.f;

    // 多Boss状态（最多MAX_BOSS_COUNT个Boss）
    gameStats.multiBossStats.activeCount = bossManager.getActiveBossCount();
    int idx = 0;
    for (auto& boss : bossManager.getBosses()) {
        // 只记录存活的Boss
        if (!boss.isDead() && idx < MAX_BOSS_COUNT) {
            gameStats.multiBossStats.bosses[idx].hp = boss.getHP();
            gameStats.multiBossStats.bosses[idx].maxHP = boss.getMaxHP();
            gameStats.multiBossStats.bosses[idx].phase = boss.getPhase();
            gameStats.multiBossStats.bosses[idx].active = true;
            gameStats.multiBossStats.bosses[idx].attackName = boss.getCurrentAttackName();
            gameStats.multiBossStats.bosses[idx].laserWarning = boss.isLaserWarning();
            gameStats.multiBossStats.bosses[idx].laserActive = boss.isLaserActive();
            idx++;
        }
    }
    gameStats.multiBossStats.difficulty = currentDifficulty;

    // 更新玩家HP追踪（用于下一帧判断是否掉血）
    prevPlayerHP = player.getHP();

    // 更新HUD（传入最新GameStats，HUD会更新显示内容）
    hud.update(gameStats);
}

// ============================================================
// 游戏渲染（每帧调用）
// ============================================================
// 渲染顺序很重要：先画的在上层，被后画的覆盖
// 层级：背景 → Boss → 道具 → 玩家 → 粒子 → HUD → 菜单 → 屏幕特效
// ============================================================
void Game::render() {
    // ---- 设置背景颜色 ----
    sf::Color bgColor(10, 10, 30);       // 默认：深蓝色（太空感）
    if (gameStats.victory) bgColor = sf::Color(10, 40, 20);   // 胜利：深绿色
    if (gameStats.gameOver) bgColor = sf::Color(40, 10, 10);  // 失败：深红色
    window.clear(bgColor);             // 清空上一帧画面

    // ---- Layer 1: 背景（星空/网格）----
    background.render(window);

    // ---- Layer 2: 相机震动 ----
    // 通过修改SFML的View（视角）实现屏幕震动效果
    sf::View originalView = window.getView();  // 保存原始视角
    sf::View shakeView = originalView;          // 创建震动视角副本
    // 将视角中心偏移相机震动量（cameraShake.getOffset()每帧衰减）
    shakeView.setCenter(sf::Vector2f(WINDOW_WIDTH / 2.f + cameraShake.getOffset().x,
                                     WINDOW_HEIGHT / 2.f + cameraShake.getOffset().y));
    window.setView(shakeView);                  // 应用震动视角，所有物体随震动

    // ---- Layer 3: 游戏实体（都在震动视角下）----
    bossManager.render(window);    // 渲染所有Boss和它们的子弹
    for (const auto& item : items) {
        item.render(window);       // 渲染所有道具
    }
    player.render(window);         // 渲染玩家

    // ---- Layer 4: 粒子特效 ----
    // 粒子系统使用additive blend（叠加混合），让发光效果更亮
    particles.render(window);

    // ---- Nova Ring扩散圆环特效 ----
    if (novaRingActive) {
        // 计算透明度（圆环越大，越透明）
        float alpha = std::fmaxf(0.f, 1.f - novaRingRadius / novaRingMaxRadius);
        // 创建圆环Shape（外圈有outline，内圈透明）
        sf::CircleShape novaRing(novaRingRadius);
        novaRing.setOrigin(sf::Vector2f(novaRingRadius, novaRingRadius));
        novaRing.setPosition(player.getPosition());
        novaRing.setFillColor(sf::Color(255, 255, 200, static_cast<std::uint8_t>(alpha * 30.f)));   // 淡黄色填充
        novaRing.setOutlineColor(sf::Color(255, 255, 100, static_cast<std::uint8_t>(alpha * 200.f))); // 金黄色边框
        novaRing.setOutlineThickness(4.f);
        window.draw(novaRing);
    }

    // ---- 恢复视角 ----
    // 所有需要不受震动影响的元素（如HUD）都要在setView(originalView)之后绘制
    window.setView(originalView);

    // ---- Layer 5: HUD（不受震动影响）----
    hud.render(window);

    // ---- Layer 6: 菜单（最上层）----
    menu.render(window);

    // ---- 屏幕闪烁特效 ----
    if (screenFlashTimer > 0.f) {
        // 计算透明度（screenFlashTimer从0.25→0，逐渐消失）
        float alpha = screenFlashTimer / 0.25f;
        if (alpha > 1.f) alpha = 1.f;
        // 创建覆盖整个屏幕的半透明矩形
        sf::RectangleShape flash(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        flash.setPosition(sf::Vector2f(0.f, 0.f));
        sf::Color c = screenFlashColor;  // screenFlashColor是各种颜色的（绿/蓝/橙/紫等）
        c.a = static_cast<std::uint8_t>(c.a * alpha);  // 乘以透明度，淡出效果
        flash.setFillColor(c);
        window.draw(flash);  // 绘制在所有内容之上
    }

    // ---- 子弹时间特效（紫色暗角 vignette）----
    if (bulletTimeTimer > 0.f) {
        // 透明度随时间减少（从0.3→0）
        float alpha = 0.3f * (bulletTimeTimer / BULLET_TIME_DURATION);
        if (alpha > 0.3f) alpha = 0.3f;
        // 用四个矩形遮住屏幕四角，形成暗角效果（紫色）
        sf::RectangleShape vigTop(sf::Vector2f(WINDOW_WIDTH, 100.f));         // 顶部遮罩
        vigTop.setPosition(sf::Vector2f(0.f, 0.f));
        vigTop.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        sf::RectangleShape vigBottom(sf::Vector2f(WINDOW_WIDTH, 100.f));    // 底部遮罩
        vigBottom.setPosition(sf::Vector2f(0.f, WINDOW_HEIGHT - 100.f));
        vigBottom.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        sf::RectangleShape vigLeft(sf::Vector2f(100.f, WINDOW_HEIGHT - 200.f));  // 左侧遮罩（中间部分）
        vigLeft.setPosition(sf::Vector2f(0.f, 100.f));
        vigLeft.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        sf::RectangleShape vigRight(sf::Vector2f(100.f, WINDOW_HEIGHT - 200.f)); // 右侧遮罩
        vigRight.setPosition(sf::Vector2f(WINDOW_WIDTH - 100.f, 100.f));
        vigRight.setFillColor(sf::Color(30, 0, 50, static_cast<std::uint8_t>(alpha * 255)));
        window.draw(vigTop);
        window.draw(vigBottom);
        window.draw(vigLeft);
        window.draw(vigRight);
    }

    // ---- 将画面输出到显示器 ----
    window.display();
}