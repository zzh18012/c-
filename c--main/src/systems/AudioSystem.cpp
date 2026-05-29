#include "AudioSystem.h"
#include "core/Config.h"
#include <cstdlib>
#include <cstdint>

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

void AudioSystem::ensureSoundsInitialized() {
    if (soundsInitialized) return;
    soundsInitialized = true;

    // Generate shoot sound - short high-pitched laser blip
    {
        int durationMs = 80;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        for (int i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float envelope = 1.f - static_cast<float>(i) / totalSamples;
            float freq = 800.f - (400.f * static_cast<float>(i) / totalSamples);
            float sample = std::sin(2.f * 3.14159f * freq * t) * envelope;
            samples[i] = static_cast<int16_t>(sample * 16000.f);
        }
        std::vector<sf::SoundChannel> channelMap(1, sf::SoundChannel::Mono);
        bool result = shootBuffer.loadFromSamples(
            samples.data(),
            static_cast<uint64_t>(samples.size()),
            1,
            static_cast<unsigned int>(SAMPLE_RATE),
            channelMap);
        fprintf(stderr, "SHOOT BUFFER: samples=%zu, loadResult=%d, getSampleCount=%u\n",
            samples.size(), result, shootBuffer.getSampleCount());
    }

    // Hit sound - sharp percussive impact
    {
        int durationMs = 100;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        for (int i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float envelope = std::exp(-t * 50.f);
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.3f;
            float thud = std::sin(2.f * 3.14159f * 100.f * t) * 0.7f;
            float sample = (noise + thud) * envelope;
            samples[i] = static_cast<int16_t>(sample * 28000.f);
        }
        hitBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Pickup sound - ascending cheerful chime
    {
        int durationMs = 200;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        float freqs[] = { 523.f, 659.f, 784.f, 1047.f };
        int samplesPerNote = totalSamples / 4;
        for (int i = 0; i < totalSamples; i++) {
            int noteIndex = i / samplesPerNote;
            float t = static_cast<float>(i % samplesPerNote) / SAMPLE_RATE;
            float envelope = 1.f - static_cast<float>(i % samplesPerNote) / samplesPerNote;
            float freq = freqs[noteIndex % 4];
            float sample = std::sin(2.f * 3.14159f * freq * t) * envelope;
            samples[i] = static_cast<int16_t>(sample * 22000.f);
        }
        pickupBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Hurt sound - pained low thud
    {
        int durationMs = 250;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        for (int i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float envelope = std::exp(-t * 15.f);
            float rumble = std::sin(2.f * 3.14159f * 80.f * t) * 0.6f;
            float harmonic = std::sin(2.f * 3.14159f * 160.f * t) * 0.3f;
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.1f;
            float sample = (rumble + harmonic + noise) * envelope;
            samples[i] = static_cast<int16_t>(sample * 26000.f);
        }
        playerHurtBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
        bossHurtBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Dash sound - whoosh
    {
        int durationMs = 150;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        for (int i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float envelope = 1.f - static_cast<float>(i) / totalSamples;
            float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f);
            float freq = 200.f + 800.f * static_cast<float>(i) / totalSamples;
            float sweep = std::sin(2.f * 3.14159f * freq * t);
            float sample = (noise * 0.4f + sweep * 0.6f) * envelope;
            samples[i] = static_cast<int16_t>(sample * 20000.f);
        }
        dashBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Overdrive sound - powerful rising hum
    {
        int durationMs = 400;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        for (int i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float envelope = std::exp(-t * 3.f);
            float freq = 150.f + 400.f * static_cast<float>(i) / totalSamples;
            float fundamental = std::sin(2.f * 3.14159f * freq * t);
            float harmonic = std::sin(2.f * 3.14159f * freq * 2.f * t) * 0.5f;
            float sample = (fundamental + harmonic) * envelope;
            samples[i] = static_cast<int16_t>(sample * 28000.f);
        }
        overdriveBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Shield sound - shimmering protective hum
    {
        int durationMs = 300;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        for (int i = 0; i < totalSamples; i++) {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float envelope = 1.f - static_cast<float>(i) / totalSamples;
            float freq = 800.f;
            float shimmer = std::sin(2.f * 3.14159f * freq * t) * 0.5f;
            shimmer += std::sin(2.f * 3.14159f * freq * 1.5f * t) * 0.3f;
            shimmer += std::sin(2.f * 3.14159f * freq * 2.f * t) * 0.2f;
            float sample = shimmer * envelope;
            samples[i] = static_cast<int16_t>(sample * 20000.f);
        }
        shieldBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Victory sound - triumphant fanfare
    {
        int durationMs = 1000;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        float freqs[] = { 262.f, 330.f, 392.f, 523.f };
        int notesCount = 4;
        int samplesPerNote = totalSamples / notesCount;
        for (int i = 0; i < totalSamples; i++) {
            int noteIndex = i / samplesPerNote;
            float t = static_cast<float>(i % samplesPerNote) / SAMPLE_RATE;
            float baseFreq = freqs[noteIndex % notesCount];
            float envelope = std::exp(-t * 2.f) * 0.8f + 0.2f;
            float chord = std::sin(2.f * 3.14159f * baseFreq * t);
            chord += std::sin(2.f * 3.14159f * baseFreq * 2.f * t) * 0.5f;
            chord += std::sin(2.f * 3.14159f * baseFreq * 3.f * t) * 0.25f;
            samples[i] = static_cast<int16_t>(chord * envelope * 20000.f);
        }
        victoryBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }

    // Game Over sound - somber descending tone
    {
        int durationMs = 1200;
        int totalSamples = SAMPLE_RATE * durationMs / 1000;
        std::vector<int16_t> samples(totalSamples);
        float freqs[] = { 262.f, 220.f, 175.f, 131.f };
        int notesCount = 4;
        int samplesPerNote = totalSamples / notesCount;
        for (int i = 0; i < totalSamples; i++) {
            int noteIndex = i / samplesPerNote;
            float t = static_cast<float>(i % samplesPerNote) / SAMPLE_RATE;
            float baseFreq = freqs[noteIndex % notesCount];
            float noteProgress = static_cast<float>(i % samplesPerNote) / samplesPerNote;
            float envelope = (1.f - noteProgress) * std::exp(-noteProgress * 0.5f);
            float sample = std::sin(2.f * 3.14159f * baseFreq * t);
            sample += std::sin(2.f * 3.14159f * baseFreq * 0.5f * t) * 0.3f;
            samples[i] = static_cast<int16_t>(sample * envelope * 22000.f);
        }
        gameOverBuffer.loadFromSamples(samples.data(), samples.size(), 1, static_cast<unsigned int>(SAMPLE_RATE), {});
    }
}

void AudioSystem::playShoot() {
    ensureSoundsInitialized();
    fprintf(stderr, "PLAY SHOOT: buffer sampleCount=%u\n", shootBuffer.getSampleCount());
    sf::Sound sound(shootBuffer);
    sound.setVolume(60.f);
    sound.play();
    fprintf(stderr, "PLAY SHOOT: sound status=%d\n", sound.getStatus());
}

void AudioSystem::playHit() {
    ensureSoundsInitialized();
    sf::Sound sound(hitBuffer);
    sound.setVolume(80.f);
    sound.play();
}

void AudioSystem::playPickup() {
    ensureSoundsInitialized();
    sf::Sound sound(pickupBuffer);
    sound.setVolume(90.f);
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
    sound.setVolume(70.f);
    sound.play();
}

void AudioSystem::playDash() {
    ensureSoundsInitialized();
    sf::Sound sound(dashBuffer);
    sound.setVolume(70.f);
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
    sound.setVolume(80.f);
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

void AudioSystem::startBackgroundMusic() {
    // Background music requires an audio file - skip without file
}

void AudioSystem::stopBackgroundMusic() {
    if (musicPlaying) {
        backgroundMusic.stop();
        musicPlaying = false;
    }
}