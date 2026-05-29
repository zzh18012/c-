#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    static AudioSystem* getInstance();

    void playShoot();
    void playHit();
    void playPickup();
    void playPlayerHurt();
    void playBossHurt();
    void playDash();
    void playOverdrive();
    void playShield();
    void playVictory();
    void playGameOver();
    void startBackgroundMusic();
    void stopBackgroundMusic();

private:
    void ensureSoundsInitialized();

    sf::SoundBuffer shootBuffer;
    sf::SoundBuffer hitBuffer;
    sf::SoundBuffer pickupBuffer;
    sf::SoundBuffer playerHurtBuffer;
    sf::SoundBuffer bossHurtBuffer;
    sf::SoundBuffer dashBuffer;
    sf::SoundBuffer overdriveBuffer;
    sf::SoundBuffer shieldBuffer;
    sf::SoundBuffer victoryBuffer;
    sf::SoundBuffer gameOverBuffer;

    bool soundsInitialized = false;

    sf::Music backgroundMusic;
    bool musicPlaying = false;

    static AudioSystem* s_instance;
};