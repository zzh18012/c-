#include "AudioSystem.h"
#include "core/Config.h"
#include <cstdlib>
#include <cstdint>
#include <ctime>
#include <cmath>

static const int SAMPLE_RATE = 44100;

AudioSystem* AudioSystem::s_instance = nullptr;

AudioSystem::AudioSystem() {
    s_instance = this;
}

AudioSystem::~AudioSystem() {
    stopBackgroundMusic();
}

AudioSystem* AudioSystem::getInstance() {
    return s_instance;
}

//------------------------------------------------------------
// Cyberpunk background music generator
//------------------------------------------------------------
static void generateBgMusic(float* buffer, int numSamples, float bpm) {
    float beatLen = 60.f / bpm;
    float barLen = beatLen * 4.f;

    for (int i = 0; i < numSamples; i++) {
        float t = static_cast<float>(i) / SAMPLE_RATE;
        float out = 0.f;

        float barPos = fmodf(t, barLen) / barLen;
        float beatPos = fmodf(t, beatLen) / beatLen;

        // Sub bass on beats 1 & 3
        float subEnv = 0.f;
        if (barPos < 0.25f) subEnv = 1.f - barPos * 4.f;
        else if (barPos > 0.5f && barPos < 0.75f) subEnv = 1.f - (barPos - 0.5f) * 4.f;
        out += sinf(2.f * 3.14159f * 55.f * t) * subEnv * 0.4f;

        // Kick on every beat
        float kickEnv = expf(-beatPos * 40.f);
        out += sinf(2.f * 3.14159f * 120.f * t * (1.f - beatPos * 0.3f)) * kickEnv * 0.5f;

        // Hi-hat click every half-beat
        float hihatPhase = fmodf(t, beatLen * 0.5f) / (beatLen * 0.5f);
        float hihatEnv = expf(-hihatPhase * 80.f);
        float hihatClick = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * hihatEnv;
        out += hihatClick * 0.08f;

        // Saw arp lead: E minor pentatonic
        float melNotes[] = { 82.41f, 98.0f, 110.0f, 123.47f, 146.83f, 164.81f, 196.0f, 220.0f };
        float arpPeriod = beatLen * 0.5f;
        int arpIdx = static_cast<int>(fmodf(t / arpPeriod, 8.f));
        float arpFreq = melNotes[arpIdx];
        float arpPhase = fmodf(t, arpPeriod) / arpPeriod;
        float arpEnv = (1.f - arpPhase) * expf(-arpPhase * 3.f);
        float saw = 0.f;
        for (int h = 1; h <= 6; h++) saw += sinf(2.f * 3.14159f * arpFreq * h * t) * (1.f / h);
        out += saw * arpEnv * 0.15f;

        // Tritone shimmer
        float shimmerEnv = (barPos > 0.1f && barPos < 0.9f) ? 0.12f : 0.f;
        out += sinf(2.f * 3.14159f * 55.f * 1.06f * t) * 0.3f * shimmerEnv;
        out += sinf(2.f * 3.14159f * 55.f * 1.06f * 1.5f * t) * 0.15f * shimmerEnv;

        buffer[i] = out;
    }
}

//------------------------------------------------------------
void AudioSystem::ensureSoundsInitialized() {
    if (soundsInitialized) return;
    soundsInitialized = true;

    // SHOOT — laser blip, high freq sweep down
    {
        int N = SAMPLE_RATE * 80 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = (1.f - static_cast<float>(i) / N);
            env = env * env;
            float freq = 1200.f - 700.f * static_cast<float>(i) / N;
            float s = sinf(2.f * 3.14159f * freq * t);
            for (int h = 2; h <= 4; h++) s += sinf(2.f * 3.14159f * freq * h * t) * (1.f / (h * 2.f));
            buf[i] = static_cast<int16_t>(s * env * 18000.f);
        }
        shootBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // HIT — punchy impact that cuts through music:
    // Sharp click → deep thud with freq sweep → metallic ring → noise
    {
        int N = SAMPLE_RATE * 250 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float p = static_cast<float>(i) / N;

            // Click transient — 2kHz + 3.5kHz sharp attack to cut through
            float clickEnv = expf(-t * 120.f);
            float click = sinf(2.f * 3.14159f * 2000.f * t) * clickEnv * 0.5f;
            click += sinf(2.f * 3.14159f * 3500.f * t) * expf(-t * 180.f) * 0.3f;

            // Main thud — 120Hz dropping to 50Hz
            float thudFreq = 120.f - 70.f * p;
            float thudEnv = expf(-t * 20.f);
            float thud = sinf(2.f * 3.14159f * thudFreq * t) * thudEnv * 1.0f;
            thud += sinf(2.f * 3.14159f * thudFreq * 2.f * t) * thudEnv * 0.4f;

            // Mid punch at 200Hz
            float midEnv = expf(-t * 30.f);
            float mid = sinf(2.f * 3.14159f * 200.f * t) * midEnv * 0.6f;

            // Metallic ring at 800Hz
            float ringEnv = expf(-t * 50.f);
            float ring = sinf(2.f * 3.14159f * 800.f * t) * ringEnv * 0.4f;

            // Noise crack
            float noiseEnv = expf(-t * 40.f);
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * noiseEnv * 0.4f;

            float sample = click + thud + mid + ring + noise;
            buf[i] = static_cast<int16_t>(sample * 32000.f);
        }
        hitBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // PICKUP — ascending 4-note chime
    {
        int N = SAMPLE_RATE * 250 / 1000;
        std::vector<int16_t> buf(N);
        float freqs[] = { 523.f, 659.f, 784.f, 1047.f };
        int noteLen = N / 4;
        for (int i = 0; i < N; i++) {
            int ni = i / noteLen;
            float lt = static_cast<float>(i % noteLen) / SAMPLE_RATE;
            float env = (1.f - static_cast<float>(i % noteLen) / noteLen);
            env = env * env;
            float s = sinf(2.f * 3.14159f * freqs[ni] * lt);
            s += sinf(2.f * 3.14159f * freqs[ni] * 2.f * lt) * 0.3f;
            buf[i] = static_cast<int16_t>(s * env * 22000.f);
        }
        pickupBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // PLAYER HURT — pained low rumble
    {
        int N = SAMPLE_RATE * 300 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = expf(-t * 12.f);
            float rumble = sinf(2.f * 3.14159f * 70.f * t) * 0.6f;
            rumble += sinf(2.f * 3.14159f * 140.f * t) * 0.3f;
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
            buf[i] = static_cast<int16_t>((rumble + noise) * env * 26000.f);
        }
        playerHurtBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // BOSS HURT — heavier longer rumble
    {
        int N = SAMPLE_RATE * 350 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = expf(-t * 8.f);
            float rumble = sinf(2.f * 3.14159f * 55.f * t) * 0.5f;
            rumble += sinf(2.f * 3.14159f * 110.f * t) * 0.35f;
            rumble += sinf(2.f * 3.14159f * 220.f * t) * 0.15f;
            buf[i] = static_cast<int16_t>(rumble * env * 28000.f);
        }
        bossHurtBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // DASH — whoosh with pitch rise
    {
        int N = SAMPLE_RATE * 180 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float p = static_cast<float>(i) / N;
            float env = 1.f - p;
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f);
            float sweepFreq = 300.f + 1500.f * p;
            float sweep = sinf(2.f * 3.14159f * sweepFreq * t);
            float s = (noise * 0.3f + sweep * 0.7f) * env;
            buf[i] = static_cast<int16_t>(s * 20000.f);
        }
        dashBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // OVERDRIVE — rising power chord with distortion
    {
        int N = SAMPLE_RATE * 500 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float env = expf(-t * 6.f);
            float freq = 110.f + 200.f * static_cast<float>(i) / N;
            float chord = sinf(2.f * 3.14159f * freq * t);
            chord += sinf(2.f * 3.14159f * freq * 1.5f * t) * 0.7f;
            chord += sinf(2.f * 3.14159f * freq * 2.f * t) * 0.4f;
            chord = tanhf(chord * 1.5f);
            for (int h = 3; h <= 5; h++) chord += sinf(2.f * 3.14159f * freq * h * t) * (0.3f / h);
            buf[i] = static_cast<int16_t>(chord * env * 24000.f);
        }
        overdriveBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // SHIELD — shimmering high-freq hum
    {
        int N = SAMPLE_RATE * 350 / 1000;
        std::vector<int16_t> buf(N);
        for (int i = 0; i < N; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float p = static_cast<float>(i) / N;
            float env = (1.f - p) * (1.f - p);
            float base = 900.f;
            float s = sinf(2.f * 3.14159f * base * t) * 0.5f;
            s += sinf(2.f * 3.14159f * base * 1.5f * t) * 0.3f;
            s += sinf(2.f * 3.14159f * base * 2.f * t) * 0.2f;
            s += sinf(2.f * 3.14159f * base * 3.f * t) * 0.1f;
            s *= 0.8f + 0.2f * sinf(2.f * 3.14159f * 8.f * t);
            buf[i] = static_cast<int16_t>(s * env * 20000.f);
        }
        shieldBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // VICTORY — triumphant fanfare:
    // Bright chord stab + echo shimmer + harmonic richness
    {
        int N = SAMPLE_RATE * 2000 / 1000;
        std::vector<int16_t> buf(N);
        // C major chord: C3, E3, G3
        float chordFreqs[] = { 130.81f, 164.81f, 196.0f };
        int noteLen = N / 2;
        for (int i = 0; i < N; i++) {
            int ni = i / noteLen;
            float lt = static_cast<float>(i % noteLen) / SAMPLE_RATE;
            float noteP = static_cast<float>(i % noteLen) / noteLen;

            // Chord stab with attack
            float attackEnv = expf(-lt * 4.f);
            float chord = 0.f;
            for (int f = 0; f < 3; f++) {
                chord += sinf(2.f * 3.14159f * chordFreqs[f] * lt) * 0.5f;
                chord += sinf(2.f * 3.14159f * chordFreqs[f] * 2.f * lt) * 0.3f;
                chord += sinf(2.f * 3.14159f * chordFreqs[f] * 3.f * lt) * 0.15f;
            }

            // Brightness shimmer on top
            float shimmer = sinf(2.f * 3.14159f * 1047.f * lt) * 0.15f * attackEnv;

            // Rising pitch sweep for second note
            float sweep = (ni == 1) ? sinf(2.f * 3.14159f * (262.f + 200.f * noteP) * lt) * 0.4f : 0.f;

            float env = (1.f - noteP) * expf(-noteP * 1.2f);
            float s = (chord + shimmer + sweep) * env;
            buf[i] = static_cast<int16_t>(s * 28000.f);
        }
        victoryBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }

    // GAME OVER — deep boom + descending echo:
    // Low-frequency collapse with reverb tail
    {
        int N = SAMPLE_RATE * 2500 / 1000;
        std::vector<int16_t> buf(N);
        float freqs[] = { 130.f, 100.f, 75.f, 55.f };
        int noteLen = N / 4;
        for (int i = 0; i < N; i++) {
            int ni = i / noteLen;
            float lt = static_cast<float>(i % noteLen) / SAMPLE_RATE;
            float noteP = static_cast<float>(i % noteLen) / noteLen;

            // Core tone — deep with harmonic layers
            float tone = sinf(2.f * 3.14159f * freqs[ni] * lt);
            tone += sinf(2.f * 3.14159f * freqs[ni] * 0.5f * lt) * 0.5f; // sub octave

            // Long reverb tail
            float env = (1.f - noteP) * expf(-noteP * 0.8f) * expf(-lt * 2.f);

            // Final boom on last note
            float boom = (ni == 3) ? sinf(2.f * 3.14159f * 30.f * lt) * expf(-lt * 8.f) * 0.6f : 0.f;

            float s = (tone + boom) * env;
            buf[i] = static_cast<int16_t>(s * 30000.f);
        }
        gameOverBuffer.loadFromSamples(buf.data(), buf.size(), 1, SAMPLE_RATE,
            std::vector<sf::SoundChannel>(1, sf::SoundChannel::Mono));
    }
}

//------------------------------------------------------------
void AudioSystem::startBackgroundMusic() {
    if (musicPlaying) return;

    const float BPM = 128.f;
    float barLen = 240.f / BPM;
    int bars = 8;
    int loopSamples = static_cast<int>(SAMPLE_RATE * barLen * bars);
    int totalSamples = loopSamples * 4;

    std::vector<int16_t> stereoLoop(totalSamples * 2);
    std::vector<float> chunk(loopSamples);

    for (int rep = 0; rep < 4; rep++) {
        generateBgMusic(chunk.data(), loopSamples, BPM);
        for (int i = 0; i < loopSamples; i++) {
            float s = chunk[i] * 1.5f;
            if (s > 1.f) s = 1.f;
            if (s < -1.f) s = -1.f;
            stereoLoop[(rep * loopSamples + i) * 2] = static_cast<int16_t>(s * 22000.f);
            stereoLoop[(rep * loopSamples + i) * 2 + 1] = static_cast<int16_t>(s * 22000.f);
        }
    }

    // Save to a temporary WAV file then load via sf::Music
    {
        std::vector<sf::SoundChannel> chan(2);
        chan[0] = sf::SoundChannel::FrontLeft;
        chan[1] = sf::SoundChannel::FrontRight;
        sf::SoundBuffer tempBuf;
        tempBuf.loadFromSamples(stereoLoop.data(), stereoLoop.size(), 2, SAMPLE_RATE,
            std::vector<sf::SoundChannel>{sf::SoundChannel::FrontLeft, sf::SoundChannel::FrontRight});
        tempBuf.saveToFile("bg_music_loop.wav");
    }

    backgroundMusic.openFromFile("bg_music_loop.wav");
    backgroundMusic.setLooping(true);
    backgroundMusic.setVolume(45.f);
    backgroundMusic.play();
    musicPlaying = true;
}

void AudioSystem::stopBackgroundMusic() {
    if (musicPlaying) {
        backgroundMusic.stop();
        musicPlaying = false;
    }
}

//------------------------------------------------------------
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