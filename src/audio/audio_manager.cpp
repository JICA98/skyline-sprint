#include "audio_manager.h"
#include "util/logger.h"
#include "platform/platform.h"
#include <cstring>
#include <cstdlib>

SDL_AudioDeviceID AudioManager::audioDevice = 0;
SDL_AudioSpec AudioManager::deviceSpec;
bool AudioManager::initialized = false;
bool AudioManager::muted = false;

AudioManager::SoundAsset AudioManager::jumpSound = { nullptr, 0 };
AudioManager::SoundAsset AudioManager::pickupSound = { nullptr, 0 };
AudioManager::SoundAsset AudioManager::failureSound = { nullptr, 0 };

AudioManager::ActiveSound AudioManager::activeSounds[AudioManager::MAX_ACTIVE_SOUNDS];

static bool LoadAndConvertWAV(const std::string& path, SDL_AudioSpec& deviceSpec, Uint8*& outBuffer, Uint32& outLength) {
    SDL_AudioSpec wavSpec;
    Uint8* wavBuffer = nullptr;
    Uint32 wavLength = 0;

    if (!SDL_LoadWAV(path.c_str(), &wavSpec, &wavBuffer, &wavLength)) {
        Logger::Log(LogLevel::Warning, "Audio", "Failed to load WAV file: " + path);
        return false;
    }

    // Build converter from WAV format to opened device format
    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt, wavSpec.format, wavSpec.channels, wavSpec.freq,
                          deviceSpec.format, deviceSpec.channels, deviceSpec.freq) < 0) {
        Logger::Log(LogLevel::Warning, "Audio", "Failed to build audio converter for: " + path);
        SDL_FreeWAV(wavBuffer);
        return false;
    }

    cvt.len = wavLength;
    cvt.buf = (Uint8*)malloc(cvt.len * cvt.len_mult);
    if (!cvt.buf) {
        SDL_FreeWAV(wavBuffer);
        return false;
    }

    memcpy(cvt.buf, wavBuffer, wavLength);
    SDL_FreeWAV(wavBuffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        Logger::Log(LogLevel::Warning, "Audio", "Failed to convert audio for: " + path);
        free(cvt.buf);
        return false;
    }

    outBuffer = cvt.buf;
    outLength = cvt.len_cvt;
    return true;
}

bool AudioManager::Init() {
#ifdef PS4
    initialized = false;
    Logger::Log(LogLevel::Info, "Audio", "Diagnostic bypass: Audio disabled on PS4.");
    return true;
#endif
    if (initialized) return true;

    // Zero out active sounds list
    for (int i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
        activeSounds[i].active = false;
        activeSounds[i].pos = nullptr;
        activeSounds[i].remaining = 0;
    }

    // Initialize SDL Audio Subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        Logger::Log(LogLevel::Error, "Audio", "Failed to initialize SDL Audio Subsystem: " + std::string(SDL_GetError()));
        return false;
    }

    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 22050;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1; // Mono
    desiredSpec.samples = 1024;
    desiredSpec.callback = AudioCallback;
    desiredSpec.userdata = nullptr;

    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &deviceSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (audioDevice == 0) {
        Logger::Log(LogLevel::Error, "Audio", "Failed to open SDL Audio Device: " + std::string(SDL_GetError()));
        return false;
    }

    // Load and convert WAV assets
    bool success = true;
    std::string jumpPath = Platform_GetAssetPath("assets/audio/jump.wav");
    std::string pickupPath = Platform_GetAssetPath("assets/audio/pickup.wav");
    std::string failurePath = Platform_GetAssetPath("assets/audio/failure.wav");

    Logger::Log(LogLevel::Info, "Audio", "Loading audio asset from path: " + jumpPath);
    success &= LoadAndConvertWAV(jumpPath, deviceSpec, jumpSound.buffer, jumpSound.length);

    Logger::Log(LogLevel::Info, "Audio", "Loading audio asset from path: " + pickupPath);
    success &= LoadAndConvertWAV(pickupPath, deviceSpec, pickupSound.buffer, pickupSound.length);

    Logger::Log(LogLevel::Info, "Audio", "Loading audio asset from path: " + failurePath);
    success &= LoadAndConvertWAV(failurePath, deviceSpec, failureSound.buffer, failureSound.length);

    if (!success) {
        Logger::Log(LogLevel::Warning, "Audio", "Failed to load/convert one or more WAV assets. Continuing silently.");
        if (jumpSound.buffer) { free(jumpSound.buffer); jumpSound.buffer = nullptr; }
        if (pickupSound.buffer) { free(pickupSound.buffer); pickupSound.buffer = nullptr; }
        if (failureSound.buffer) { free(failureSound.buffer); failureSound.buffer = nullptr; }
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
        return false;
    }

    // Start playing audio (unpause)
    SDL_PauseAudioDevice(audioDevice, 0);

    initialized = true;
    Logger::Log(LogLevel::Info, "Audio", "Audio subsystem initialized successfully.");
    return true;
}

void AudioManager::Shutdown() {
    if (!initialized) return;

    if (audioDevice != 0) {
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_CloseAudioDevice(audioDevice);
        audioDevice = 0;
    }

    if (jumpSound.buffer) { free(jumpSound.buffer); jumpSound.buffer = nullptr; }
    if (pickupSound.buffer) { free(pickupSound.buffer); pickupSound.buffer = nullptr; }
    if (failureSound.buffer) { free(failureSound.buffer); failureSound.buffer = nullptr; }

    initialized = false;
    Logger::Log(LogLevel::Info, "Audio", "Audio subsystem shut down.");
}

void AudioManager::PlaySound(SoundType type) {
    if (!initialized || muted || audioDevice == 0) return;

    SoundAsset* asset = nullptr;
    switch (type) {
        case SoundType::Jump:    asset = &jumpSound; break;
        case SoundType::Pickup:  asset = &pickupSound; break;
        case SoundType::Failure: asset = &failureSound; break;
        case SoundType::Landing: asset = &jumpSound; break;
        case SoundType::Menu:    asset = &pickupSound; break;
    }

    if (!asset || !asset->buffer) return;

    // Find an empty slot in activeSounds
    SDL_LockAudioDevice(audioDevice);
    bool slotted = false;
    for (int i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
        if (!activeSounds[i].active) {
            activeSounds[i].pos = asset->buffer;
            activeSounds[i].remaining = asset->length;
            activeSounds[i].active = true;
            slotted = true;
            break;
        }
    }
    SDL_UnlockAudioDevice(audioDevice);

    if (!slotted) {
        Logger::Log(LogLevel::Warning, "Audio", "No free audio channel slots available.");
    }
}

void AudioManager::SetMute(bool mute) {
    muted = mute;
    Logger::Log(LogLevel::Info, "Audio", "Audio mute state set to: " + std::string(muted ? "MUTED" : "UNMUTED"));
}

bool AudioManager::IsMuted() {
    return muted;
}

bool AudioManager::IsActive() {
    return initialized && (audioDevice != 0);
}

void AudioManager::AudioCallback(void* userdata, Uint8* stream, int len) {
    memset(stream, 0, len);

    for (int i = 0; i < MAX_ACTIVE_SOUNDS; ++i) {
        if (!activeSounds[i].active) continue;

        Uint32 mixLen = (activeSounds[i].remaining > (Uint32)len) ? (Uint32)len : activeSounds[i].remaining;
        SDL_MixAudioFormat(stream, activeSounds[i].pos, deviceSpec.format, mixLen, SDL_MIX_MAXVOLUME);

        activeSounds[i].pos += mixLen;
        activeSounds[i].remaining -= mixLen;

        if (activeSounds[i].remaining == 0) {
            activeSounds[i].active = false;
        }
    }
}
