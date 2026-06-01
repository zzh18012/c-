// ============================================================
// AudioSystem.cpp - 音频系统实现
// ============================================================
// 职责：管理游戏中的所有音频（背景音乐+音效）
// 所有声音都是程序化生成的（通过数学公式计算音频样本）
// 使用单例模式，采样率固定为44100Hz
// ============================================================

#include "AudioSystem.h"
#include "core/Config.h"
#include <cstdlib>  // rand(), RAND_MAX
#include <cstdint> // int16_t
#include <ctime>   // time()（未使用但可能用于随机种子）
#include <cmath>   // sinf, expf, fmodf, tanhf

// ============================================================
// 常量定义
// ============================================================
// 采样率：44100Hz（CD音质标准）
static const int SAMPLE_RATE = 44100;

// ============================================================
// 单例初始化
// ============================================================
AudioSystem* AudioSystem::s_instance = nullptr;

// ============================================================
// 构造函数
// ============================================================
// 设置单例指针指向自身
AudioSystem::AudioSystem() {
    s_instance = this;
}

// ============================================================
// 析构函数
// ============================================================
// 停止背景音乐（如果正在播放）
AudioSystem::~AudioSystem() {
    stopBackgroundMusic();
}

// ============================================================
// getInstance - 获取单例实例
// ============================================================
AudioSystem* AudioSystem::getInstance() {
    return s_instance;
}

// ============================================================
// generateBgMusic - 生成Cyberpunk背景音乐
// ============================================================
// buffer: 输出样本缓冲区
// numSamples: 样本数量
// bpm: 每分钟节拍数（128BPM）
// 音乐构成：
// 1. 子低音（55Hz）：在第1和第3拍发声
// 2. 踢鼓（120Hz→50Hz下滑）：每拍发声
// 3. 高帽点击声（噪声）：每半拍发声
// 4. 锯齿波合成旋律：E小调五声音阶的琶音
// 5. 三全音闪烁：增加科幻感
static void generateBgMusic(float* buffer, int numSamples, float bpm) {
    float beatLen = 60.f / bpm;    // 每拍时长（秒）= 60/128 ≈ 0.47秒
    float barLen = beatLen * 4.f; // 每小节时长 = 4拍 ≈ 1.875秒

    for (int i = 0; i < numSamples; i++) {
        float t = static_cast<float>(i) / SAMPLE_RATE; // 当前时间（秒）
        float out = 0.f;

        // ---- 子低音（55Hz）----
        // 在第1拍和第3拍发声，形成节奏感
        float barPos = fmodf(t, barLen) / barLen; // 在小节中的位置（0~1）
        float beatPos = fmodf(t, beatLen) / beatLen; // 在拍中的位置（0~1）
        float subEnv = 0.f;
        if (barPos < 0.25f) subEnv = 1.f - barPos * 4.f; // 第1拍：逐渐衰减
        else if (barPos > 0.5f && barPos < 0.75f) subEnv = 1.f - (barPos - 0.5f) * 4.f; // 第3拍
        out += sinf(2.f * 3.14159f * 55.f * t) * subEnv * 0.4f; // 55Hz正弦波 × 包络

        // ---- 踢鼓 ----
        // 每拍发声，频率从120Hz下滑到50Hz
        float kickEnv = expf(-beatPos * 40.f); // 指数衰减包络
        // 频率随时间下滑：120Hz → (1 - beatPos*0.3)倍
        out += sinf(2.f * 3.14159f * 120.f * t * (1.f - beatPos * 0.3f)) * kickEnv * 0.5f;

        // ---- 高帽点击 ----
        // 每半拍发声，使用噪声模拟真实高帽
        float hihatPhase = fmodf(t, beatLen * 0.5f) / (beatLen * 0.5f); // 半拍位置
        float hihatEnv = expf(-hihatPhase * 80.f); // 快速衰减
        // 随机噪声 × 包络
        float hihatClick = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * hihatEnv;
        out += hihatClick * 0.08f;

        // ---- 锯齿波旋律（E小调五声音阶）----
        // 频率数组：E3, G3, A3, B3, D4, E4, G4, A4
        float melNotes[] = { 82.41f, 98.0f, 110.0f, 123.47f, 146.83f, 164.81f, 196.0f, 220.0f };
        float arpPeriod = beatLen * 0.5f; // 琶音周期 = 半拍
        int arpIdx = static_cast<int>(fmodf(t / arpPeriod, 8.f)); // 8个音符循环
        float arpFreq = melNotes[arpIdx]; // 当前音符频率
        float arpPhase = fmodf(t, arpPeriod) / arpPeriod; // 在当前音符中的位置
        // 锯齿波 = 基频 + 1/2谐波 + 1/3谐波 + ...（谐波衰减1/h）
        float arpEnv = (1.f - arpPhase) * expf(-arpPhase * 3.f); // 音符包络
        float saw = 0.f;
        for (int h = 1; h <= 6; h++) saw += sinf(2.f * 3.14159f * arpFreq * h * t) * (1.f / h);
        out += saw * arpEnv * 0.15f;

        // ---- 三全音闪烁 ----
        // 增加科幻/赛博朋克氛围
        float shimmerEnv = (barPos > 0.1f && barPos < 0.9f) ? 0.12f : 0.f;
        // 55Hz × 1.06 ≈ 58.3Hz（三全音）和 55Hz × 1.06 × 1.5
        out += sinf(2.f * 3.14159f * 55.f * 1.06f * t) * 0.3f * shimmerEnv;
        out += sinf(2.f * 3.14159f * 55.f * 1.06f * 1.5f * t) * 0.15f * shimmerEnv;

        buffer[i] = out; // 存入缓冲区
    }
}

// ============================================================
// ensureSoundsInitialized - 确保音效已初始化
// ============================================================
// 第一次播放音效前调用，生成所有音效的样本数据
// 使用sf::SoundBuffer的loadFromSamples加载样本数据
// 每种音效都经过精心设计，包含多个层次（瞬态、主体、衰减）
void AudioSystem::ensureSoundsInitialized() {
    if (soundsInitialized) return; // 避免重复初始化
    soundsInitialized = true;

    // ============================================================
    // SHOOT - 射击音效
    // ============================================================
    // 激光啾啾声：高频 sweeps down
    // 频率从1200Hz滑落到500Hz，持续80ms
    {
        int N = SAMPLE_RATE * 80 / 1000; // 80ms样本数
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = (1.f - static_cast<float>(i) / N);  // 线性衰减
            env = env * env;                                 // 平方衰减（更自然）
            float freq = 1200.f - 700.f * static_cast<float>(i) / N; // 频率下滑
            float s = sinf(2.f * 3.14159f * freq * t);     // 正弦波
            // 添加谐波增加丰富度
            for (int h = 2; h <= 4; h++) s += sinf(2.f * 3.14159f * freq * h * t) * (1.f / (h * 2.f));
            buf[i] = static_cast<int16_t>(s * env * 18000.f);
        }
        shootBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // HIT - 命中音效
    // ============================================================
    // 冲击声，能在背景音乐中突出：瞬态点击 → 深沉撞击 → 金属环 → 噪声
    // 总时长250ms
    {
        int N = SAMPLE_RATE * 250 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float p = static_cast<float>(i) / N; // 0~1进度

            // 点击瞬态：2kHz + 3.5kHz尖锐攻击，压过音乐
            float clickEnv = expf(-t * 120.f);
            float click = sinf(2.f * 3.14159f * 2000.f * t) * clickEnv * 0.5f;
            click += sinf(2.f * 3.14159f * 3500.f * t) * expf(-t * 180.f) * 0.3f;

            // 主撞击：120Hz→50Hz下滑
            float thudFreq = 120.f - 70.f * p;
            float thudEnv = expf(-t * 20.f);
            float thud = sinf(2.f * 3.14159f * thudFreq * t) * thudEnv * 1.0f;
            thud += sinf(2.f * 3.14159f * thudFreq * 2.f * t) * thudEnv * 0.4f; // 2次谐波

            // 中频打击：200Hz
            float midEnv = expf(-t * 30.f);
            float mid = sinf(2.f * 3.14159f * 200.f * t) * midEnv * 0.6f;

            // 金属环：800Hz
            float ringEnv = expf(-t * 50.f);
            float ring = sinf(2.f * 3.14159f * 800.f * t) * ringEnv * 0.4f;

            // 噪声裂响
            float noiseEnv = expf(-t * 40.f);
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * noiseEnv * 0.4f;

            float sample = click + thud + mid + ring + noise;
            buf[i] = static_cast<int16_t>(sample * 32000.f);
        }
        hitBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // PICKUP - 拾取音效
    // ============================================================
    // 上升4音符叮咚声：C5 → E5 → G5 → C6
    // 音符频率：523.25, 659.25, 783.99, 1046.50 Hz
    {
        int N = SAMPLE_RATE * 250 / 1000;
        std::vector<int16_t> buf(N);
        float freqs[] = { 523.f, 659.f, 784.f, 1047.f };
        int noteLen = N / 4; // 每个音符占1/4时长
        for (int i = 0; i < N; i++) {
            int ni = i / noteLen; // 当前音符索引（0~3）
            float lt = static_cast<float>(i % noteLen) / SAMPLE_RATE; // 在音符内的时间
            float env = (1.f - static_cast<float>(i % noteLen) / noteLen);
            env = env * env; // 平方衰减
            float s = sinf(2.f * 3.14159f * freqs[ni] * lt); // 基频
            s += sinf(2.f * 3.14159f * freqs[ni] * 2.f * lt) * 0.3f; // 2次谐波
            buf[i] = static_cast<int16_t>(s * env * 22000.f);
        }
        pickupBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // PLAYER HURT - 玩家受伤音效
    // ============================================================
    // 痛苦低沉的隆隆声：70Hz基础 + 噪声
    // 时长300ms
    {
        int N = SAMPLE_RATE * 300 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = expf(-t * 12.f); // 指数衰减
            float rumble = sinf(2.f * 3.14159f * 70.f * t) * 0.6f; // 70Hz基础音
            rumble += sinf(2.f * 3.14159f * 140.f * t) * 0.3f; // 2次谐波
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f; // 少量噪声
            buf[i] = static_cast<int16_t>((rumble + noise) * env * 26000.f);
        }
        playerHurtBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // BOSS HURT - Boss受伤音效
    // ============================================================
    // 比玩家受伤更重、更长：55Hz基础 + 更多谐波
    // 时长350ms
    {
        int N = SAMPLE_RATE * 350 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = expf(-t * 8.f); // 较慢衰减
            float rumble = sinf(2.f * 3.14159f * 55.f * t) * 0.5f; // 55Hz（更低沉）
            rumble += sinf(2.f * 3.14159f * 110.f * t) * 0.35f; // 2次谐波
            rumble += sinf(2.f * 3.14159f * 220.f * t) * 0.15f; // 4次谐波（更丰富）
            buf[i] = static_cast<int16_t>(rumble * env * 28000.f);
        }
        bossHurtBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // DASH - 冲刺音效
    // ============================================================
    // 呼啸声，音调上升：噪声 + 频率扫描
    // 时长180ms
    {
        int N = SAMPLE_RATE * 180 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float p = static_cast<float>(i) / N; // 0~1进度
            float env = 1.f - p; // 线性衰减
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f); // 噪声
            float sweepFreq = 300.f + 1500.f * p; // 频率从300Hz滑到1800Hz
            float sweep = sinf(2.f * 3.14159f * sweepFreq * t); // 扫描正弦波
            float s = (noise * 0.3f + sweep * 0.7f) * env; // 混合 + 包络
            buf[i] = static_cast<int16_t>(s * 20000.f);
        }
        dashBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // OVERDRIVE - 超载音效
    // ============================================================
    // 上升和弦，略带失真
    // 频率从110Hz上升到310Hz，持续500ms
    {
        int N = SAMPLE_RATE * 500 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = expf(-t * 6.f); // 指数衰减
            float freq = 110.f + 200.f * static_cast<float>(i) / N; // 频率上升
            // 和弦：基频 + 1.5倍（纯五度）+ 2倍（八度）
            float chord = sinf(2.f * 3.14159f * freq * t);
            chord += sinf(2.f * 3.14159f * freq * 1.5f * t) * 0.7f;
            chord += sinf(2.f * 3.14159f * freq * 2.f * t) * 0.4f;
            chord = tanhf(chord * 1.5f); // 双曲正切失真
            // 添加谐波
            for (int h = 3; h <= 5; h++) chord += sinf(2.f * 3.14159f * freq * h * t) * (0.3f / h);
            buf[i] = static_cast<int16_t>(chord * env * 24000.f);
        }
        overdriveBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // SHIELD - 护盾音效
    // ============================================================
    // 高频颤动的嗡嗡声：900Hz基础 + 泛音
    // 时长350ms
    {
        int N = SAMPLE_RATE * 350 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float p = static_cast<float>(i) / N;
            float env = (1.f - p) * (1.f - p); // 平方衰减
            float base = 900.f; // 基础频率
            float s = sinf(2.f * 3.14159f * base * t) * 0.5f;
            s += sinf(2.f * 3.14159f * base * 1.5f * t) * 0.3f; // 1.5倍
            s += sinf(2.f * 3.14159f * base * 2.f * t) * 0.2f; // 2倍
            s += sinf(2.f * 3.14159f * base * 3.f * t) * 0.1f; // 3倍
            s *= 0.8f + 0.2f * sinf(2.f * 3.14159f * 8.f * t); // 8Hz颤动调制
            buf[i] = static_cast<int16_t>(s * env * 20000.f);
        }
        shieldBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // VICTORY - 胜利音效
    // ============================================================
    // 凯旋和弦 + 回声闪光 + 谐波丰富度
    // C大三和弦：C3, E3, G3 (130.81, 164.81, 196.0 Hz)
    // 时长2000ms（2秒）
    {
        int N = SAMPLE_RATE * 2000 / 1000;
        std::vector<int16_t> buf(N);
        float chordFreqs[] = { 130.81f, 164.81f, 196.0f };
        int noteLen = N / 2; // 2个音符
        for (int i = 0; i < N; i++) {
            int ni = i / noteLen; // 当前音符索引
            float lt = static_cast<float>(i % noteLen) / SAMPLE_RATE; // 在音符内时间
            float noteP = static_cast<float>(i % noteLen) / noteLen; // 音符进度

            // 和弦打击：基频 + 2次谐波 + 3次谐波
            float attackEnv = expf(-lt * 4.f);
            float chord = 0.f;
            for (int f = 0; f < 3; f++) {
                chord += sinf(2.f * 3.14159f * chordFreqs[f] * lt) * 0.5f;
                chord += sinf(2.f * 3.14159f * chordFreqs[f] * 2.f * lt) * 0.3f;
                chord += sinf(2.f * 3.14159f * chordFreqs[f] * 3.f * lt) * 0.15f;
            }

            // 明亮闪光在和弦上方
            float shimmer = sinf(2.f * 3.14159f * 1047.f * lt) * 0.15f * attackEnv;

            // 第二个音符有上升音调扫描
            float sweep = (ni == 1) ? sinf(2.f * 3.14159f * (262.f + 200.f * noteP) * lt) * 0.4f : 0.f;

            float env = (1.f - noteP) * expf(-noteP * 1.2f);
            float s = (chord + shimmer + sweep) * env;
            buf[i] = static_cast<int16_t>(s * 28000.f);
        }
        victoryBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // ============================================================
    // GAME OVER - 失败音效
    // ============================================================
    // 深沉的轰鸣 + 下降回声
    // 频率序列：130, 100, 75, 55 Hz（下降）
    // 时长2500ms（2.5秒）
    {
        int N = SAMPLE_RATE * 2500 / 1000;
        std::vector<int16_t> buf(N);
        float freqs[] = { 130.f, 100.f, 75.f, 55.f };
        int noteLen = N / 4; // 4个音符
        for (int i = 0; i < N; i++) {
            int ni = i / noteLen; // 当前音符索引
            float lt = static_cast<float>(i % noteLen) / SAMPLE_RATE;
            float noteP = static_cast<float>(i % noteLen) / noteLen;

            // 核心音调：基频 + 次八度（0.5倍）增加厚度
            float tone = sinf(2.f * 3.14159f * freqs[ni] * lt);
            tone += sinf(2.f * 3.14159f * freqs[ni] * 0.5f * lt) * 0.5f;

            // 长的混响尾音
            float env = (1.f - noteP) * expf(-noteP * 0.8f) * expf(-lt * 2.f);

            // 最后一个音符有额外低频轰鸣
            float boom = (ni == 3) ? sinf(2.f * 3.14159f * 30.f * lt) * expf(-lt * 8.f) * 0.6f : 0.f;

            float s = (tone + boom) * env;
            buf[i] = static_cast<int16_t>(s * 30000.f);
        }
        gameOverBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }
}

// ============================================================
// startBackgroundMusic - 启动背景音乐
// ============================================================
// 生成4×8小节的音乐循环（32小节），保存为WAV文件，然后通过sf::Music播放
void AudioSystem::startBackgroundMusic() {
    if (musicPlaying) return; // 避免重复启动

    const float BPM = 128.f; // 节拍速度
    float barLen = 240.f / BPM; // 每小节时长 = 240/128 ≈ 1.875秒
    int bars = 8; // 每次生成8小节
    int loopSamples = static_cast<int>(SAMPLE_RATE * barLen * bars); // 单次样本数
    int totalSamples = loopSamples * 4; // 总样本数（4次 = 32小节）

    std::vector<int16_t> stereoLoop(totalSamples * 2); // 立体声：2通道
    std::vector<float> chunk(loopSamples); // 单次生成buffer

    // 生成4次（立体声复制）
    for (int rep = 0; rep < 4; rep++) {
        generateBgMusic(chunk.data(), loopSamples, BPM);
        for (int i = 0; i < loopSamples; i++) {
            float s = chunk[i] * 1.5f;
            if (s > 1.f) s = 1.f;   // 限幅防止爆音
            if (s < -1.f) s = -1.f;
            // 左右声道相同（单声道复制成立体声）
            stereoLoop[(rep * loopSamples + i) * 2] = static_cast<int16_t>(s * 22000.f);
            stereoLoop[(rep * loopSamples + i) * 2 + 1] = static_cast<int16_t>(s * 22000.f);
        }
    }

    // 生成临时WAV文件
    {
        std::vector<sf::SoundChannel> chan(2);
        chan[0] = sf::SoundChannel::FrontLeft;
        chan[1] = sf::SoundChannel::FrontRight;
        sf::SoundBuffer tempBuf;
        tempBuf.loadFromSamples(stereoLoop.data(), stereoLoop.size(), 2, SAMPLE_RATE,
            std::vector<sf::SoundChannel>{sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight});
        tempBuf.saveToFile("bg_music_loop.wav"); // 保存到文件供sf::Music加载
    }

    // 使用sf::Music流式播放（不会把整个文件加载到内存）
    backgroundMusic.openFromFile("bg_music_loop.wav");
    backgroundMusic.setLooping(true); // 循环播放
    backgroundMusic.setVolume(45.f); // 音量45%
    backgroundMusic.play();
    musicPlaying = true;
}

// ============================================================
// stopBackgroundMusic - 停止背景音乐
// ============================================================
void AudioSystem::stopBackgroundMusic() {
    if (musicPlaying) {
        backgroundMusic.stop();
        musicPlaying = false;
    }
}

// ============================================================
// 音效播放方法（简洁，每次调用创建新的sf::Sound实例）
// ============================================================
void AudioSystem::playShoot() {
    ensureSoundsInitialized();
    sf::Sound sound(shootBuffer);
    sound.setVolume(65.f);
    sound.play();
}

void AudioSystem::playHit() {
    ensureSoundsInitialized();
    sf::Sound sound(hitBuffer);
    sound.setVolume(100.f);
    sound.play();
}

void AudioSystem::playPickup() {
    ensureSoundsInitialized();
    sf::Sound sound(pickupBuffer);
    sound.setVolume(85.f);
    sound.play();
}

void AudioSystem::playPlayerHurt() {
    ensureSoundsInitialized();
    sf::Sound sound(playerHurtBuffer);
    sound.setVolume(85.f);
    sound.play();
}

void AudioSystem::playBossHurt() {
    ensureSoundsInitialized();
    sf::Sound sound(bossHurtBuffer);
    sound.setVolume(75.f);
    sound.play();
}

void AudioSystem::playDash() {
    ensureSoundsInitialized();
    sf::Sound sound(dashBuffer);
    sound.setVolume(75.f);
    sound.play();
}

void AudioSystem::playOverdrive() {
    ensureSoundsInitialized();
    sf::Sound sound(overdriveBuffer);
    sound.setVolume(85.f);
    sound.play();
}

void AudioSystem::playShield() {
    ensureSoundsInitialized();
    sf::Sound sound(shieldBuffer);
    sound.setVolume(75.f);
    sound.play();
}

void AudioSystem::playVictory() {
    ensureSoundsInitialized();
    sf::Sound sound(victoryBuffer);
    sound.setVolume(100.f);
    sound.play();
}

void AudioSystem::playGameOver() {
    ensureSoundsInitialized();
    sf::Sound sound(gameOverBuffer);
    sound.setVolume(100.f);
    sound.play();
}