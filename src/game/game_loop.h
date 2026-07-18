#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include <SDL2/SDL.h>
#include "util/random.h"
#include "util/logger.h"
#include "player.h"
#include "world.h"

enum class GameState {
    Boot,
    Title,
    Gameplay,
    Pause,
    Failure,
    Settings,
    TestModes,
    Diagnostics,
    Credits,
    FatalError
};

class Game {
public:
    Game();
    ~Game();

    bool Init(SDL_Renderer* renderer, SDL_Window* window);
    void Shutdown();

    // Run one main loop frame. Returns false if the game should exit.
    bool RunFrame(SDL_Renderer* renderer, float frameDelta);

    // Headless simulation entry point for automated testing
    void RunHeadlessTicks(uint32_t ticks);

    GameState GetState() const { return currentState; }
    void ChangeState(GameState newState);

    uint32_t GetTickCount() const { return tickCount; }
    uint32_t GetSeed() const { return currentSeed; }
    std::string GetStateName(GameState state) const;

private:
    void UpdateSimulation();
    void RenderScene(SDL_Renderer* renderer);

    GameState currentState;
    uint32_t tickCount;
    uint32_t currentSeed;
    Random prng;

    float accumulator;
    const float dt = 1.0f / 60.0f; // 60 Hz fixed timestep (0.0166667s)
    const float maxFrameTime = 0.25f; // Clamp excessive frame deltas to avoid spiral of death

    // Gameplay Systems
    Player player;
    World world;
    float cameraX;
    int score;
    int highScore;
    float distanceTraveled;
    float multiplier;
    int shardsCollected;
    uint32_t activeSeed;

    // Menu and UI State
    int menuIndex;
    int menuRepeatTimer;

    // Settings Parameters
    bool settingAudio;
    bool settingScreenShake;
    int settingParticles; // 0=LOW, 1=MED, 2=HIGH
    bool settingShowDiagnostics;
    int settingCharacter; // 0=Runner, 1=Spacesuit

    // Test Modes State
    bool testModeAuto;
    bool testModeStress;
    uint32_t testSeed;
    int testSeedIndex;

    // Particle System
    struct Particle {
        float x, y;
        float vx, vy;
        float life;
        float maxLife;
        uint32_t color;
        bool active;
    };
    static const int MAX_PARTICLES = 256;
    Particle particles[MAX_PARTICLES];
    void SpawnParticle(float x, float y, float vx, float vy, float life, uint32_t color);
    void UpdateParticles(float deltaTime);
    void RenderParticles(SDL_Renderer* renderer, float cameraX);
    void SpawnJumpTrail(float x, float y);
    void SpawnLandingBurst(float x, float y);
    void SpawnPickupBurst(float x, float y);

    // Screen Shake state
    float shakeTime;
    float shakeIntensity;
    float shakeOffsetX;
    float shakeOffsetY;
    void TriggerScreenShake(float duration, float intensity);
    void UpdateScreenShake(float deltaTime);

    // Sprite Textures
    SDL_Texture* runnerIdleTex;
    SDL_Texture* runnerRunTex;
    SDL_Texture* runnerJumpTex;
    SDL_Texture* spacesuitIdleTex;
    SDL_Texture* spacesuitRunTex;
    SDL_Texture* spacesuitJumpTex;
    SDL_Texture* platformTex;
    SDL_Texture* warningTex;
    SDL_Texture* shardTex;
    SDL_Texture* enemyTex;

    SDL_Window* window;
};

#endif // GAME_LOOP_H
