// ============================================================
// AudioSystem.h - 音频系统声明
// ============================================================
// 职责：管理游戏中的所有音频（背景音乐+音效）
// 使用SFML的音频系统，所有声音都是程序化生成的（无外部音频文件）
// 背景音乐使用Cyberpunk风格的电子音乐（BPM=128）
// 音效包括：射击、命中、拾取、受伤、冲刺、护盾、胜利、失败
// ============================================================

#pragma once

// SFML音频库（用于sf::SoundBuffer, sf::Sound, sf::Music）
#include <SFML/Audio.hpp>

// 标准字符串（用于文件路径）
#include <string>

// 标准向量（用于存储音频样本）
#include <vector>

// 标准数学库（用于sinf, cosf等）
#include <cmath>

// 标准整型（用于int16_t等）
#include <cstdint>

// ============================================================
// AudioSystem类
// ============================================================
// 单例模式：全局只有一个AudioSystem实例
// 所有音频都是程序化生成的（通过数学公式计算样本值）
// 无需外部音频文件，bg_music_loop.wav是由程序生成的临时文件
class AudioSystem {
public:
    // 构造函数：设置单例指针
    AudioSystem();
    // 析构函数：停止背景音乐
    ~AudioSystem();

    // ---- 单例访问 ----
    // 获取单例实例
    static AudioSystem* getInstance();

    // ============================================================
    // 音效播放接口（供Game.cpp或Player.cpp调用）
    // ============================================================
    void playShoot();       // 射击音效：激光啾啾声，频率下滑
    void playHit();         // 命中音效：冲击声，压过背景音乐
    void playPickup();      // 拾取音效：上升4音符叮咚声
    void playPlayerHurt();  // 玩家受伤音效：痛苦低沉的隆隆声
    void playBossHurt();    // Boss受伤音效：更重更长的隆隆声
    void playDash();        // 冲刺音效：呼啸声，音调上升
    void playOverdrive();   // 超载音效：上升和弦，略带失真
    void playShield();      // 护盾音效：高频颤动的嗡嗡声
    void playVictory();     // 胜利音效：凯旋的和弦+回声闪光
    void playGameOver();    // 失败音效：深沉的轰鸣+下降回声

    // ============================================================
    // 背景音乐控制
    // ============================================================
    // 启动背景音乐：128BPM Cyberpunk电子音乐，循环播放
    void startBackgroundMusic();
    // 停止背景音乐
    void stopBackgroundMusic();

private:
    // ---- 私有工具方法 ----
    // 确保音效已初始化（在第一次播放前初始化）
    void ensureSoundsInitialized();

    // ============================================================
    // 音频缓冲区（存储程序化生成的音效样本）
    // ============================================================
    // 每个Buffer存储一种音效的样本数据
    sf::SoundBuffer shootBuffer;       // 射击：激光啾啾声
    sf::SoundBuffer hitBuffer;         // 命中：冲击声
    sf::SoundBuffer pickupBuffer;      // 拾取：上升音符
    sf::SoundBuffer playerHurtBuffer;  // 玩家受伤：低沉隆隆
    sf::SoundBuffer bossHurtBuffer;    // Boss受伤：更重隆隆
    sf::SoundBuffer dashBuffer;        // 冲刺：呼啸声
    sf::SoundBuffer overdriveBuffer;   // 超载：和弦
    sf::SoundBuffer shieldBuffer;      // 护盾：高频嗡嗡
    sf::SoundBuffer victoryBuffer;     // 胜利：和弦
    sf::SoundBuffer gameOverBuffer;    // 失败：轰鸣

    // ---- 初始化状态 ----
    // soundsInitialized保证ensureSoundsInitialized只执行一次
    bool soundsInitialized = false;

    // ============================================================
    // 背景音乐
    // ============================================================
    // sf::Music流式播放大型音频文件（这里用于播放生成的WAV文件）
    sf::Music backgroundMusic;
    // 标记背景音乐是否正在播放
    bool musicPlaying = false;

    // ---- 单例指针 ----
    static AudioSystem* s_instance;
};