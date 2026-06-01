// ============================================================
// HUD.cpp - 游戏抬头显示器实现
// ============================================================
// 职责：渲染游戏中所有HUD元素
// 每帧从GameStats读取数据，更新显示内容
// 支持平滑HP过渡动画、多Boss显示、攻击名称闪烁等效果
// ============================================================

#include "HUD.h"
#include "core/Config.h"
#include <cmath>    // sinf, fabsf
#include <sstream>  // ostringstream
#include <iomanip>  // setfill, setw
#include <cstdint>  // uint8_t

// ============================================================
// 构造函数
// ============================================================
// 初始化所有渲染元素的默认属性（尺寸、位置、颜色等）
// 注意：文字元素在init()中创建，这里只设置形状元素
HUD::HUD() : fontPtr(nullptr) {
    // ============================================================
    // 玩家HP血条
    // ============================================================
    // 背景：深色底框，青色边框
    playerBarBg.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH, HUD_BAR_HEIGHT));
    playerBarBg.setPosition(sf::Vector2f(HUD_PADDING, HUD_PADDING));
    playerBarBg.setFillColor(sf::Color(30, 30, 30, 200));
    playerBarBg.setOutlineThickness(1.f);
    playerBarBg.setOutlineColor(sf::Color(0, 180, 220));

    // 填充：根据HP比例变化颜色（高HP青绿色，低HP红色）
    playerBarFill.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH, HUD_BAR_HEIGHT));
    playerBarFill.setPosition(sf::Vector2f(HUD_PADDING, HUD_PADDING));
    playerBarFill.setFillColor(sf::Color(0, 200, 240));

    // ============================================================
    // Boss HP血条（最多3个，垂直堆叠）
    // ============================================================
    float bossBarX = WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH;
    float bossSpacing = 30.f; // 相邻血条的垂直间距

    bossBarBgs.resize(MAX_BOSS_BARS);
    bossBarFills.resize(MAX_BOSS_BARS);

    for (int i = 0; i < MAX_BOSS_BARS; ++i) {
        float yPos = HUD_PADDING + i * bossSpacing; // 每个血条的Y位置

        // 背景：深色底框，红色边框
        bossBarBgs[i].setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH, HUD_BAR_HEIGHT));
        bossBarBgs[i].setPosition(sf::Vector2f(bossBarX, yPos));
        bossBarBgs[i].setFillColor(sf::Color(30, 30, 30, 200));
        bossBarBgs[i].setOutlineThickness(1.f);
        bossBarBgs[i].setOutlineColor(sf::Color(220, 60, 60));

        // 填充：红色
        bossBarFills[i].setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH, HUD_BAR_HEIGHT));
        bossBarFills[i].setPosition(sf::Vector2f(bossBarX, yPos));
        bossBarFills[i].setFillColor(sf::Color(240, 50, 50));
    }

    // ============================================================
    // 阶段标记（单Boss时显示）
    // ============================================================
    // 显示70%和35%的位置线，提示Boss即将进入下个阶段
    float markerY = HUD_PADDING - 4.f;
    float markerH = HUD_BAR_HEIGHT + 8.f;

    // 70%标记：黄色
    phaseMarker70.setSize(sf::Vector2f(2.f, markerH));
    phaseMarker70.setPosition(sf::Vector2f(bossBarX + HUD_BOSS_BAR_WIDTH * (1.f - PHASE2_HP_RATIO), markerY));
    phaseMarker70.setFillColor(sf::Color(255, 180, 40, 150));

    // 35%标记：红色
    phaseMarker35.setSize(sf::Vector2f(2.f, markerH));
    phaseMarker35.setPosition(sf::Vector2f(bossBarX + HUD_BOSS_BAR_WIDTH * (1.f - PHASE3_HP_RATIO), markerY));
    phaseMarker35.setFillColor(sf::Color(255, 80, 40, 150));

    // ============================================================
    // 冲刺冷却圆形指示器
    // ============================================================
    // 位于屏幕底部中央，显示冲刺技能的冷却进度
    dashCooldownBg.setRadius(16.f);
    dashCooldownBg.setOrigin(sf::Vector2f(16.f, 16.f));
    dashCooldownBg.setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 36.f));
    dashCooldownBg.setFillColor(sf::Color(20, 20, 30, 180));
    dashCooldownBg.setOutlineThickness(2.f);
    dashCooldownBg.setOutlineColor(sf::Color(0, 200, 255, 100));

    dashCooldownRing.setRadius(14.f);
    dashCooldownRing.setOrigin(sf::Vector2f(14.f, 14.f));
    dashCooldownRing.setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 36.f));
    dashCooldownRing.setFillColor(sf::Color(0, 0, 0, 0)); // 透明填充
    dashCooldownRing.setOutlineThickness(3.f);
    dashCooldownRing.setOutlineColor(sf::Color(0, 220, 255, 200));
    dashCooldownRing.setPointCount(60); // 圆弧分段数

    // ============================================================
    // 道具状态图标
    // ============================================================
    // 位于屏幕左下角，3个图标水平排列
    auto makeBuffIcon = [](sf::RectangleShape& icon, sf::Color c, float x, float y) {
        icon.setSize(sf::Vector2f(20.f, 20.f));
        icon.setPosition(sf::Vector2f(x, y));
        icon.setFillColor(c);
        icon.setOutlineThickness(1.f);
        icon.setOutlineColor(sf::Color(255, 255, 255, 80));
    };

    float buffY = WINDOW_HEIGHT - 50.f;
    makeBuffIcon(shieldIcon, sf::Color(60, 140, 255), HUD_PADDING, buffY);           // 蓝色
    makeBuffIcon(overdriveIcon, sf::Color(255, 150, 30), HUD_PADDING + 120.f, buffY); // 橙色
    makeBuffIcon(dashBatteryIcon, sf::Color(0, 220, 200), HUD_PADDING + 240.f, buffY); // 青色
}

HUD::~HUD() {}

// ============================================================
// init - 初始化文字元素
// ============================================================
// font: 游戏字体
// 创建所有sf::Text对象，设置初始位置和颜色
void HUD::init(const sf::Font& font) {
    fontPtr = &font; // 保存字体指针

    // ============================================================
    // 创建所有文字元素
    // ============================================================
    playerHPText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    attackNameText.emplace(font, "", HUD_FONT_SIZE_NORMAL);
    shieldText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    overdriveText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    dashBatteryText.emplace(font, "", HUD_FONT_SIZE_SMALL);
    bulletTimeText.emplace(font, "", HUD_FONT_SIZE_NORMAL);
    timerText.emplace(font, "", HUD_FONT_SIZE_NORMAL);
    endMessageText.emplace(font, "", HUD_FONT_SIZE_LARGE);

    // Boss文字数组
    for (int i = 0; i < MAX_BOSS_BARS; ++i) {
        bossHPTexts[i].emplace(font, "", HUD_FONT_SIZE_SMALL);
        bossPhaseTexts[i].emplace(font, "", HUD_FONT_SIZE_SMALL);
        bossNameTexts[i].emplace(font, "", HUD_FONT_SIZE_SMALL);
    }

    // ============================================================
    // 设置玩家HP文字
    // ============================================================
    playerHPText->setPosition(sf::Vector2f(HUD_PADDING + 6.f, HUD_PADDING + 1.f));
    playerHPText->setFillColor(sf::Color::White);

    // ============================================================
    // 设置Boss血条文字位置和颜色
    // ============================================================
    float bossBarX = WINDOW_WIDTH - HUD_PADDING - HUD_BOSS_BAR_WIDTH;
    float bossSpacing = 30.f;

    for (int i = 0; i < MAX_BOSS_BARS; ++i) {
        float yPos = HUD_PADDING + i * bossSpacing;

        // HP文字：血条内部
        bossHPTexts[i]->setPosition(sf::Vector2f(bossBarX + 6.f, yPos + 1.f));
        bossHPTexts[i]->setFillColor(sf::Color::White);

        // 阶段文字：血条左侧
        bossPhaseTexts[i]->setPosition(sf::Vector2f(bossBarX - 40.f, yPos - 2.f));
        bossPhaseTexts[i]->setFillColor(sf::Color(255, 200, 50));

        // 名称文字：血条右侧
        bossNameTexts[i]->setPosition(sf::Vector2f(bossBarX + HUD_BOSS_BAR_WIDTH - 6.f, yPos - 2.f));
        bossNameTexts[i]->setFillColor(sf::Color(255, 80, 40));
    }

    // Boss默认名称
    bossNameTexts[0]->setString("INFerno");
    bossNameTexts[1]->setString("VOID");
    bossNameTexts[2]->setString("THUNDER");

    // ============================================================
    // 攻击名称：屏幕顶部中央
    // ============================================================
    attackNameText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, 50.f));
    attackNameText->setFillColor(sf::Color(255, 220, 100));

    // ============================================================
    // 道具计时器文字：位于对应图标旁边
    // ============================================================
    shieldText->setPosition(sf::Vector2f(HUD_PADDING + 26.f, WINDOW_HEIGHT - 52.f));
    shieldText->setFillColor(sf::Color(120, 180, 255));

    overdriveText->setPosition(sf::Vector2f(HUD_PADDING + 146.f, WINDOW_HEIGHT - 52.f));
    overdriveText->setFillColor(sf::Color(255, 180, 80));

    dashBatteryText->setPosition(sf::Vector2f(HUD_PADDING + 266.f, WINDOW_HEIGHT - 52.f));
    dashBatteryText->setFillColor(sf::Color(80, 240, 220));

    // ============================================================
    // 子弹时间提示：屏幕底部中央
    // ============================================================
    bulletTimeText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 70.f));
    bulletTimeText->setFillColor(sf::Color(200, 120, 255));

    // ============================================================
    // 游戏时间：右下角
    // ============================================================
    timerText->setPosition(sf::Vector2f(WINDOW_WIDTH - 80.f, WINDOW_HEIGHT - 36.f));
    timerText->setFillColor(sf::Color(200, 200, 200));

    // ============================================================
    // 结束消息：屏幕正中央
    // ============================================================
    endMessageText->setPosition(sf::Vector2f(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f));
    endMessageText->setFillColor(sf::Color::White);
}

// ============================================================
// update - 每帧更新
// ============================================================
// stats: 游戏统计数据（从Game.cpp传入）
// 更新所有HUD元素的显示内容
void HUD::update(const GameStats& stats) {
    if (!fontPtr) return; // 未初始化则跳过

    // 保存难度（用于后续判断）
    currentDifficulty = stats.multiBossStats.difficulty;

    // ============================================================
    // 平滑HP过渡
    // ============================================================
    // HP值不是立即跳变，而是每帧插值过渡（视觉平滑）
    float lerpSpeed = HUD_BAR_LERP_SPEED;
    displayedPlayerHP += (static_cast<float>(stats.playerHP) - displayedPlayerHP) * lerpSpeed * 0.016f;
    if (std::fabs(displayedPlayerHP - stats.playerHP) < 0.5f)
        displayedPlayerHP = static_cast<float>(stats.playerHP);

    // ============================================================
    // 玩家HP血条
    // ============================================================
    // 计算HP比例
    float playerRatio = displayedPlayerHP / stats.playerMaxHP;
    // 更新填充宽度
    playerBarFill.setSize(sf::Vector2f(HUD_PLAYER_BAR_WIDTH * playerRatio, HUD_BAR_HEIGHT));

    // HP比例决定颜色：高HP青绿色，低HP红色
    sf::Color hpColor;
    if (playerRatio > 0.5f) {
        // >50%: 从青色(0,220,240)到黄色(255,220,50)
        hpColor = sf::Color(
            static_cast<std::uint8_t>(255 * (1.f - playerRatio) * 2.f),
            220,
            static_cast<std::uint8_t>(255 * (0.5f - playerRatio) * 2.f + 100));
    } else {
        // <50%: 从黄色(255,220,50)到红色(255,50,50)
        hpColor = sf::Color(255,
            static_cast<std::uint8_t>(220 * playerRatio * 2.f),
            50);
    }
    playerBarFill.setFillColor(hpColor);

    // 更新HP文字
    std::ostringstream php;
    php << "HP " << stats.playerHP << "/" << stats.playerMaxHP;
    playerHPText->setString(php.str());

    // ============================================================
    // 玩家无敌状态闪烁效果
    // ============================================================
    if (stats.playerInvincible) {
        // 边框闪烁：sin函数产生0~1之间的周期变化
        float flash = (sinf(stats.elapsedTime * 15.f) + 1.f) * 0.5f;
        playerBarBg.setOutlineColor(sf::Color(255, 255, 255, static_cast<std::uint8_t>(100 + flash * 155)));
    } else {
        playerBarBg.setOutlineColor(sf::Color(0, 180, 220)); // 恢复正常边框
    }

    // ============================================================
    // 多Boss HP血条
    // ============================================================
    int activeCount = stats.multiBossStats.activeCount;
    activeBossCount = activeCount;

    for (int i = 0; i < activeCount; ++i) {
        const BossState& boss = stats.multiBossStats.bosses[i];

        // 平滑Boss HP过渡
        displayedBossHPs[i] += (static_cast<float>(boss.hp) - displayedBossHPs[i]) * lerpSpeed * 0.016f;
        if (std::fabs(displayedBossHPs[i] - boss.hp) < 0.5f)
            displayedBossHPs[i] = static_cast<float>(boss.hp);

        // 更新血条填充
        float bossRatio = boss.maxHP > 0 ? displayedBossHPs[i] / boss.maxHP : 0.f;
        bossBarFills[i].setSize(sf::Vector2f(HUD_BOSS_BAR_WIDTH * bossRatio, HUD_BAR_HEIGHT));

        // 更新Boss HP文字
        std::ostringstream bhp;
        bhp << "HP " << boss.hp << "/" << boss.maxHP;
        bossHPTexts[i]->setString(bhp.str());

        // 更新阶段文字
        std::ostringstream phaseStr;
        phaseStr << "P" << boss.phase;
        bossPhaseTexts[i]->setString(phaseStr.str());

        // 设置Boss名称颜色
        if (i < 3) {
            bossNameTexts[i]->setFillColor(BOSS_NAME_COLORS[i]);
        }
    }

    // 隐藏未使用的Boss血条
    for (int i = activeCount; i < MAX_BOSS_BARS; ++i) {
        bossHPTexts[i]->setString("");
        bossPhaseTexts[i]->setString("");
    }

    // ============================================================
    // 阶段标记颜色
    // ============================================================
    phaseMarker70.setFillColor(sf::Color(255, 180, 40, 150));
    phaseMarker35.setFillColor(sf::Color(255, 80, 40, 150));

    // ============================================================
    // 攻击名称
    // ============================================================
    // 检测是否有Boss处于激光预警状态
    bool anyLaserWarning = false;
    for (int i = 0; i < stats.multiBossStats.activeCount; ++i) {
        if (stats.multiBossStats.bosses[i].laserWarning) {
            anyLaserWarning = true;
            break;
        }
    }

    if (attackNameText) {
        std::string newName = stats.currentAttackName;
        // 攻击名称变化时重置闪烁计时器
        if (newName != lastAttackName) {
            lastAttackName = newName;
            attackNameText->setString(newName);
            // 重新设置origin（居中对齐）
            sf::FloatRect ab = attackNameText->getLocalBounds();
            attackNameText->setOrigin(sf::Vector2f(ab.position.x + ab.size.x / 2.f, ab.position.y));
            attackFlashTimer = 0.f;
        }
        attackFlashTimer += 0.016f;

        // 激光预警时红色闪烁
        if (anyLaserWarning) {
            float flash = (sinf(attackFlashTimer * 8.f) + 1.f) * 0.5f;
            attackNameText->setFillColor(sf::Color(255, static_cast<std::uint8_t>(60 + flash * 195), static_cast<std::uint8_t>(60 + flash * 195)));
        } else {
            attackNameText->setFillColor(sf::Color(255, 220, 100)); // 正常金黄色
        }
    }

    // ============================================================
    // 道具计时器文字
    // ============================================================
    shieldText->setString(stats.playerShieldTimer > 0.f ? fmtTime(stats.playerShieldTimer) : "");
    overdriveText->setString(stats.playerOverdriveTimer > 0.f ? fmtTime(stats.playerOverdriveTimer) : "");
    dashBatteryText->setString(stats.playerDashBatteryTimer > 0.f ? fmtTime(stats.playerDashBatteryTimer) : "");

    bulletTimeText->setString(stats.bulletTimeActive ? "BULLET TIME" : "");

    // ============================================================
    // 冲刺冷却圆环
    // ============================================================
    // 计算冷却进度：1 - 当前冷却时间/总冷却时间
    float dashRatio = 1.f - (stats.playerDashCooldown / PLAYER_DASH_COOLDOWN);
    dashRatio = std::max(0.f, std::min(1.f, dashRatio)); // 限制在[0,1]
    // 圆弧点数 = 进度 × 59 + 1（1~60点）
    int points = static_cast<int>(dashRatio * 59.f) + 1;
    dashCooldownRing.setPointCount(points);
    // 冷却完成时颜色变亮
    dashCooldownRing.setOutlineColor(
        dashRatio >= 1.f ? sf::Color(0, 255, 200, 220) : sf::Color(0, 180, 220, 120));

    // ============================================================
    // 游戏时间
    // ============================================================
    int mins = static_cast<int>(stats.elapsedTime) / 60;
    int secs = static_cast<int>(stats.elapsedTime) % 60;
    std::ostringstream timeStr;
    timeStr << std::setfill('0') << std::setw(2) << mins << ":"
            << std::setfill('0') << std::setw(2) << secs;
    timerText->setString(timeStr.str());
    // 设置origin使文字居中
    sf::FloatRect tb = timerText->getLocalBounds();
    timerText->setOrigin(sf::Vector2f(tb.position.x + tb.size.x / 2.f, 0.f));

    // ============================================================
    // 结束消息（胜利/失败）
    // ============================================================
    if (stats.victory) {
        endMessageText->setString("VICTORY!");
        endMessageText->setFillColor(sf::Color(80, 255, 120, static_cast<std::uint8_t>(endMessageAlpha)));
        endMessageAlpha = std::min(255.f, endMessageAlpha + 60.f * 0.016f); // 逐渐淡入
    } else if (stats.gameOver) {
        endMessageText->setString("GAME OVER");
        endMessageText->setFillColor(sf::Color(255, 60, 60, static_cast<std::uint8_t>(endMessageAlpha)));
        endMessageAlpha = std::min(255.f, endMessageAlpha + 60.f * 0.016f);
    } else {
        endMessageAlpha = 0.f; // 重置
    }
    // 设置文字origin到中心点
    sf::FloatRect eb = endMessageText->getLocalBounds();
    endMessageText->setOrigin(sf::Vector2f(eb.position.x + eb.size.x / 2.f, eb.position.y + eb.size.y / 2.f));
}

// ============================================================
// fmtTime - 格式化时间
// ============================================================
// seconds: 秒数
// 返回格式如 "3.5s" 的字符串
std::string HUD::fmtTime(float seconds) {
    int s = static_cast<int>(seconds);
    int tenths = static_cast<int>((seconds - s) * 10.f);
    std::ostringstream ss;
    ss << s << "." << tenths << "s";
    return ss.str();
}

// ============================================================
// render - 渲染HUD
// ============================================================
// 按层次渲染所有HUD元素
void HUD::render(sf::RenderWindow& window) {
    // ============================================================
    // 玩家HP血条
    // ============================================================
    window.draw(playerBarBg);
    window.draw(playerBarFill);
    if (playerHPText) window.draw(*playerHPText);

    // ============================================================
    // Boss HP血条（根据activeBossCount渲染）
    // ============================================================
    for (int i = 0; i < MAX_BOSS_BARS; ++i) {
        if (i < activeBossCount) {
            window.draw(bossBarBgs[i]);
            window.draw(bossBarFills[i]);
            if (bossHPTexts[i]) window.draw(*bossHPTexts[i]);
            if (bossPhaseTexts[i]) window.draw(*bossPhaseTexts[i]);
            if (bossNameTexts[i]) window.draw(*bossNameTexts[i]);
        }
    }

    // ============================================================
    // 阶段标记（单Boss时显示）
    // ============================================================
    if (activeBossCount <= 1) {
        window.draw(phaseMarker70);
        window.draw(phaseMarker35);
    }

    // ============================================================
    // 攻击名称
    // ============================================================
    if (attackNameText && attackNameText->getString().getSize() > 0) {
        window.draw(*attackNameText);
    }

    // ============================================================
    // 道具状态（仅在激活时显示）
    // ============================================================
    if (shieldText && shieldText->getString().getSize() > 0) {
        window.draw(shieldIcon);
        window.draw(*shieldText);
    }
    if (overdriveText && overdriveText->getString().getSize() > 0) {
        window.draw(overdriveIcon);
        window.draw(*overdriveText);
    }
    if (dashBatteryText && dashBatteryText->getString().getSize() > 0) {
        window.draw(dashBatteryIcon);
        window.draw(*dashBatteryText);
    }
    if (bulletTimeText && bulletTimeText->getString().getSize() > 0) {
        window.draw(*bulletTimeText);
    }

    // ============================================================
    // 冲刺冷却
    // ============================================================
    window.draw(dashCooldownBg);
    window.draw(dashCooldownRing);

    // ============================================================
    // 游戏时间
    // ============================================================
    if (timerText) window.draw(*timerText);

    // ============================================================
    // 结束消息
    // ============================================================
    if (endMessageAlpha > 0.f && endMessageText) {
        window.draw(*endMessageText);
    }
}