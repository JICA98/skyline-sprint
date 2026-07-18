#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL2/SDL.h>
#include <string>

enum class SoundType {
    Jump,
    Pickup,
    Landing,
    Menu,
    Failure
};

class AudioManager {
public:
    static bool Init();
    static void Shutdown();
    static void PlaySound(SoundType type);
    static void SetMute(bool mute);
    static bool IsMuted();
    static bool IsActive();

private:
    struct SoundAsset {
        Uint8* buffer;
        Uint32 length;
    };

    struct ActiveSound {
        Uint8* pos;
        Uint32 remaining;
        bool active;
    };

    static void AudioCallback(void* userdata, Uint8* stream, int len);

    static SDL_AudioDeviceID audioDevice;
    static SDL_AudioSpec deviceSpec;
    static bool initialized;
    static bool muted;
    
    // Loaded sound assets
    static SoundAsset jumpSound;
    static SoundAsset pickupSound;
    static SoundAsset failureSound;

    // Active playing instances
    static const int MAX_ACTIVE_SOUNDS = 8;
    static ActiveSound activeSounds[MAX_ACTIVE_SOUNDS];
};

#endif // AUDIO_MANAGER_H
