#include "game_loop.h"
#include "render/renderer.h"
#include "input/input.h"
#include "audio/audio_manager.h"
#include <iostream>
#include <sstream>
#include <math.h>
#include <cstdio>

Game::Game() : currentState(GameState::Boot), tickCount(0), currentSeed(0), accumulator(0.0f),
               cameraX(0.0f), previousCameraX(0.0f), renderAlpha(0.0f),
               menuIndex(0), menuRepeatTimer(0), settingAudio(true), settingScreenShake(true),
               settingParticles(1), settingShowDiagnostics(false), settingCharacter(0), testModeAuto(false),
               testModeStress(false), testSeed(0xDEADC0DE), testSeedIndex(0),
               shakeTime(0.0f), shakeIntensity(0.0f), shakeOffsetX(0.0f), shakeOffsetY(0.0f) {
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        particles[i].active = false;
    }
}

Game::~Game() {
}

bool Game::Init(SDL_Renderer* renderer, SDL_Window* window) {
    this->window = window;
    std::cout << "--- Game::Init executing ---" << std::endl;
    currentSeed = 0xDEADC0DE; // Default deterministic seed
    prng.Seed(currentSeed);
    Logger::Init(currentSeed);
    Logger::Log(LogLevel::Info, "Game", "Engine booted successfully.");

    Input::Init();
    AudioManager::Init();

    if (!Render::Init(renderer)) {
        Logger::Log(LogLevel::Error, "Game", "Failed to initialize renderer helpers.");
        ChangeState(GameState::FatalError);
        return false;
    }

    // Load textures
    runnerIdleTex = Render::LoadTexture(renderer, "assets/images/runner_idle.tga");
    runnerRunTex = Render::LoadTexture(renderer, "assets/images/runner_run.tga");
    runnerJumpTex = Render::LoadTexture(renderer, "assets/images/runner_jump.tga");
    spacesuitIdleTex = Render::LoadTexture(renderer, "assets/images/spacesuit_idle.tga");
    spacesuitRunTex = Render::LoadTexture(renderer, "assets/images/spacesuit_run.tga");
    spacesuitJumpTex = Render::LoadTexture(renderer, "assets/images/spacesuit_jump.tga");
    platformTex = Render::LoadTiledTexture(renderer, "assets/images/platform.tga", 1920, 480);
    warningTex = Render::LoadTexture(renderer, "assets/images/warning.tga", false);
    shardTex = Render::LoadTexture(renderer, "assets/images/shard.tga");
    enemyTex = Render::LoadTexture(renderer, "assets/images/enemy.tga");

    if (!runnerIdleTex || !runnerRunTex || !runnerJumpTex || !spacesuitIdleTex || !spacesuitRunTex || !spacesuitJumpTex || !platformTex || !warningTex || !shardTex || !enemyTex) {
        Logger::Log(LogLevel::Error, "Game", "Failed to load gameplay textures.");
        ChangeState(GameState::FatalError);
        return false;
    }

    // Reset state variables
    score = 0;
    highScore = 0;
    distanceTraveled = 0.0f;
    multiplier = 1.0f;
    shardsCollected = 0;
    activeSeed = currentSeed;
    cameraX = 0.0f;
    previousCameraX = 0.0f;
    renderAlpha = 0.0f;
    menuIndex = 0;
    menuRepeatTimer = 0;
    player.Reset(100.0f, 500.0f);
    world.Reset(currentSeed, prng);

    ChangeState(GameState::Boot);
    return true;
}

void Game::Shutdown() {
    // Destroy textures
    if (runnerIdleTex) SDL_DestroyTexture(runnerIdleTex);
    if (runnerRunTex) SDL_DestroyTexture(runnerRunTex);
    if (runnerJumpTex) SDL_DestroyTexture(runnerJumpTex);
    if (spacesuitIdleTex) SDL_DestroyTexture(spacesuitIdleTex);
    if (spacesuitRunTex) SDL_DestroyTexture(spacesuitRunTex);
    if (spacesuitJumpTex) SDL_DestroyTexture(spacesuitJumpTex);
    if (platformTex) SDL_DestroyTexture(platformTex);
    if (warningTex) SDL_DestroyTexture(warningTex);
    if (shardTex) SDL_DestroyTexture(shardTex);
    if (enemyTex) SDL_DestroyTexture(enemyTex);

    Input::Shutdown();
    AudioManager::Shutdown();
    Render::Shutdown();
    Logger::Log(LogLevel::Info, "Game", "Engine shutting down.");
}

bool Game::RunFrame(SDL_Renderer* renderer, float frameDelta) {
    // 1. Clamp excessive frame deltas to prevent physics instability ("spiral of death")
    if (frameDelta > maxFrameTime) {
        frameDelta = maxFrameTime;
    }

    // 2. Poll platform system events (quit, window sizing, etc.)
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            Logger::Log(LogLevel::Info, "Game", "Received SDL_QUIT. Exiting.");
            return false;
        }
    }

    // 3. Update dynamic input state
    Input::Update(tickCount);

    // Get max items for active menu
    int maxItems = 0;
    if (currentState == GameState::Title)         maxItems = 5;
    else if (currentState == GameState::Pause)     maxItems = 5;
    else if (currentState == GameState::Settings)  maxItems = 7;
    else if (currentState == GameState::TestModes) maxItems = 5;

    // Handle menu D-pad Up/Down repeat scrolling
    if (maxItems > 0) {
        if (Input::IsHeld(InputAction::MoveUp)) {
            menuRepeatTimer++;
            if (menuRepeatTimer == 1 || (menuRepeatTimer > 18 && (menuRepeatTimer - 18) % 5 == 0)) {
                menuIndex--;
                if (menuIndex < 0) menuIndex = maxItems - 1;
                AudioManager::PlaySound(SoundType::Menu);
            }
        } else if (Input::IsHeld(InputAction::MoveDown)) {
            menuRepeatTimer++;
            if (menuRepeatTimer == 1 || (menuRepeatTimer > 18 && (menuRepeatTimer - 18) % 5 == 0)) {
                menuIndex++;
                if (menuIndex >= maxItems) menuIndex = 0;
                AudioManager::PlaySound(SoundType::Menu);
            }
        } else {
            menuRepeatTimer = 0;
        }
    }

    // Handle menu D-pad Left/Right toggles
    bool leftPress = Input::WasPressed(InputAction::MoveLeft);
    bool rightPress = Input::WasPressed(InputAction::MoveRight);

    if (currentState == GameState::Settings && (leftPress || rightPress)) {
        AudioManager::PlaySound(SoundType::Menu);
        if (menuIndex == 0) {
            settingAudio = !settingAudio;
            AudioManager::SetMute(!settingAudio);
        }
        else if (menuIndex == 1) settingScreenShake = !settingScreenShake;
        else if (menuIndex == 2) {
            settingParticles = leftPress ? (settingParticles + 2) % 3 : (settingParticles + 1) % 3;
        }
        else if (menuIndex == 3) settingShowDiagnostics = !settingShowDiagnostics;
    } else if (currentState == GameState::TestModes && (leftPress || rightPress)) {
        AudioManager::PlaySound(SoundType::Menu);
        if (menuIndex == 0) {
            uint32_t presetSeeds[] = { 0xDEADC0DE, 0xBEEFCAFE, 0xBAADF00D, 0xFEEDFACE, 0xCCCCCCCC };
            int numPresets = 5;
            testSeedIndex = leftPress ? (testSeedIndex + numPresets - 1) % numPresets : (testSeedIndex + 1) % numPresets;
            testSeed = presetSeeds[testSeedIndex];
        }
        else if (menuIndex == 1) testModeAuto = !testModeAuto;
        else if (menuIndex == 2) testModeStress = !testModeStress;
    }

    // Handle back action (Circle / Escape / Backspace)
    if (Input::WasPressed(InputAction::Back)) {
        if (currentState == GameState::Title) {
            Logger::Log(LogLevel::Info, "Game", "Circle/Back pressed on Title. Exiting.");
            return false;
        } else if (currentState == GameState::Gameplay) {
            ChangeState(GameState::Pause);
        } else {
            ChangeState(GameState::Title);
            menuIndex = 0;
        }
    }

    // Handle pause action (Options / Escape)
    if (Input::WasPressed(InputAction::Pause)) {
        if (currentState == GameState::Gameplay) {
            ChangeState(GameState::Pause);
            menuIndex = 0;
        } else if (currentState == GameState::Pause) {
            ChangeState(GameState::Gameplay);
        }
    }

    // Handle select action (Cross / Enter / Space)
    if (Input::WasPressed(InputAction::Select)) {
        if (currentState == GameState::Boot) {
            ChangeState(GameState::Title);
            menuIndex = 0;
        }
        else if (currentState == GameState::Title) {
            if (menuIndex == 0) {
                // Play run
                activeSeed = testSeed; // use current test seed (default DEADC0DE)
                prng.Seed(activeSeed);
                tickCount = 0;
                Logger::SetTick(0);
                score = 0;
                distanceTraveled = 0.0f;
                multiplier = 1.0f;
                shardsCollected = 0;
                cameraX = 0.0f;
                previousCameraX = 0.0f;
                renderAlpha = 0.0f;
                player.Reset(100.0f, 500.0f);
                world.Reset(activeSeed, prng);
                Logger::Log(LogLevel::Info, "Game", "Starting run with seed: " + std::to_string(activeSeed));
                ChangeState(GameState::Gameplay);
            } else if (menuIndex == 1) {
                ChangeState(GameState::TestModes);
                menuIndex = 0;
            } else if (menuIndex == 2) {
                ChangeState(GameState::Settings);
                menuIndex = 0;
            } else if (menuIndex == 3) {
                ChangeState(GameState::Diagnostics);
            } else if (menuIndex == 4) {
                ChangeState(GameState::Credits);
            }
        }
        else if (currentState == GameState::Pause) {
            if (menuIndex == 0) {
                ChangeState(GameState::Gameplay);
            } else if (menuIndex == 1) {
                // Restart run
                prng.Seed(activeSeed);
                tickCount = 0;
                Logger::SetTick(0);
                score = 0;
                distanceTraveled = 0.0f;
                multiplier = 1.0f;
                shardsCollected = 0;
                cameraX = 0.0f;
                previousCameraX = 0.0f;
                renderAlpha = 0.0f;
                player.Reset(100.0f, 500.0f);
                world.Reset(activeSeed, prng);
                Logger::Log(LogLevel::Info, "Game", "Restarting run same seed.");
                ChangeState(GameState::Gameplay);
            } else if (menuIndex == 2) {
                ChangeState(GameState::Diagnostics);
            } else if (menuIndex == 3) {
                ChangeState(GameState::Settings);
                menuIndex = 0;
            } else if (menuIndex == 4) {
                ChangeState(GameState::Title);
                menuIndex = 0;
            }
        }
        else if (currentState == GameState::Settings) {
            AudioManager::PlaySound(SoundType::Menu);
            if (menuIndex == 0) {
                settingAudio = !settingAudio;
                AudioManager::SetMute(!settingAudio);
            }
            else if (menuIndex == 1) settingScreenShake = !settingScreenShake;
            else if (menuIndex == 2) settingParticles = (settingParticles + 1) % 3;
            else if (menuIndex == 3) settingShowDiagnostics = !settingShowDiagnostics;
            else if (menuIndex == 4) {
                settingCharacter = (settingCharacter + 1) % 2; // 0: Runner, 1: Spacesuit
            }
            else if (menuIndex == 5) {
                highScore = 0;
                Logger::Log(LogLevel::Info, "Settings", "High score session reset.");
            }
            else if (menuIndex == 6) {
                ChangeState(GameState::Title);
                menuIndex = 2; // return focus to settings item
            }
        }
        else if (currentState == GameState::TestModes) {
            if (menuIndex == 1)      testModeAuto = !testModeAuto;
            else if (menuIndex == 2) testModeStress = !testModeStress;
            else if (menuIndex == 4) {
                ChangeState(GameState::Title);
                menuIndex = 1; // return focus to test modes item
            }
        }
        else if (currentState == GameState::Failure) {
            // Replay same seed on Select (Cross)
            prng.Seed(activeSeed);
            tickCount = 0;
            Logger::SetTick(0);
            score = 0;
            distanceTraveled = 0.0f;
            multiplier = 1.0f;
            shardsCollected = 0;
            cameraX = 0.0f;
            previousCameraX = 0.0f;
            renderAlpha = 0.0f;
            player.Reset(100.0f, 500.0f);
            world.Reset(activeSeed, prng);
            Logger::Log(LogLevel::Info, "Game", "Restarting run same seed: " + std::to_string(activeSeed));
            ChangeState(GameState::Gameplay);
        }
        else if (currentState == GameState::Diagnostics || currentState == GameState::Credits) {
            ChangeState(GameState::Title);
            menuIndex = 0;
        }
    }
    else if (Input::WasPressed(InputAction::Jump)) {
        if (currentState == GameState::Failure) {
            // Restart with NEW seed on Jump (Space)
            activeSeed = SDL_GetTicks();
            prng.Seed(activeSeed);
            tickCount = 0;
            Logger::SetTick(0);
            score = 0;
            distanceTraveled = 0.0f;
            multiplier = 1.0f;
            shardsCollected = 0;
            cameraX = 0.0f;
            previousCameraX = 0.0f;
            renderAlpha = 0.0f;
            player.Reset(100.0f, 500.0f);
            world.Reset(activeSeed, prng);
            Logger::Log(LogLevel::Info, "Game", "Restarting run with NEW seed: " + std::to_string(activeSeed));
            ChangeState(GameState::Gameplay);
        }
    }

    // Toggle diagnostics overlay on F1/F2 or L3 (which maps to F1/F2)
    if (Input::WasPressed(InputAction::F1) || Input::WasPressed(InputAction::F2)) {
        settingShowDiagnostics = !settingShowDiagnostics;
    }

    // 3. Update simulation with 60 Hz fixed timestep accumulator
    accumulator += frameDelta;
    while (accumulator >= dt) {
        UpdateSimulation();
        accumulator -= dt;
    }

    renderAlpha = accumulator / dt;
    if (renderAlpha < 0.0f) renderAlpha = 0.0f;
    if (renderAlpha > 1.0f) renderAlpha = 1.0f;

    // 4. Render current scene. Presentation is handled centrally in main.cpp.
    RenderScene(renderer);

    return true;
}

void Game::RunHeadlessTicks(uint32_t ticks) {
    Logger::Log(LogLevel::Info, "Game", "Running headless simulation for " + std::to_string(ticks) + " ticks.");
    for (uint32_t i = 0; i < ticks; ++i) {
        UpdateSimulation();
    }
    Logger::Log(LogLevel::Info, "Game", "Headless simulation finished.");
}

void Game::ChangeState(GameState newState) {
    std::string oldStateName = GetStateName(currentState);
    std::string newStateName = GetStateName(newState);
    currentState = newState;
    Logger::Log(LogLevel::Info, "State", "State transition: " + oldStateName + " -> " + newStateName);
}

void Game::UpdateSimulation() {
    tickCount++;
    Logger::SetTick(tickCount);

    // Boot screen auto-transitions to Title after 120 ticks (2 seconds)
    if (currentState == GameState::Boot && tickCount >= 120) {
        ChangeState(GameState::Title);
    }

    if (currentState == GameState::Gameplay) {
        previousCameraX = cameraX;
        bool wasGrounded = player.grounded;

        // Update player physics state
        player.Update();

        // Spawn jump trail particles if player jumped this tick
        if (player.justJumped) {
            SpawnJumpTrail(player.x, player.y + player.h);
            player.justJumped = false;
        }
        if (player.justDoubleJumped) {
            SpawnJumpTrail(player.x, player.y + player.h);
            player.justDoubleJumped = false;
        }

        // Resolve platform collisions
        world.ResolvePlayerCollision(player);

        // Spawn landing particles if player just landed
        if (player.grounded && !wasGrounded) {
            SpawnLandingBurst(player.x + player.w / 2.0f, player.y + player.h);
        }

        // Collect shards (adds to score and builds multiplier)
        int collectedShards = world.CheckCollectibleCollisions(player.GetAABB()) / 10;
        if (collectedShards > 0) {
            shardsCollected += collectedShards;
            multiplier = std::min(5.0f, multiplier + collectedShards * 1.0f);
            score += collectedShards * 10 * static_cast<int>(multiplier);
            SpawnPickupBurst(player.x + player.w / 2.0f, player.y + player.h / 2.0f);
            AudioManager::PlaySound(SoundType::Pickup);
        }

        // Multiplier decays over time
        multiplier = std::max(1.0f, multiplier - 0.2f * dt);

        // Distance scoring (1 point per 10 logical pixels)
        float distDiff = player.x - player.prevX;
        if (distDiff > 0.0f) {
            distanceTraveled += distDiff;
            score += static_cast<int>(distDiff / 10.0f) * static_cast<int>(multiplier);
        }

        // Update session high score
        if (score > highScore) {
            highScore = score;
        }

        // Check hazard collisions
        if (world.CheckHazardCollision(player.GetAABB())) {
            player.alive = false;
            Logger::Log(LogLevel::Info, "Gameplay", "Player collided with a hazard.");
            TriggerScreenShake(0.25f, 10.0f); // 15 frames screen shake
            AudioManager::PlaySound(SoundType::Failure);
        }

        // Handle death
        if (!player.alive) {
            ChangeState(GameState::Failure);
        }

        // Update chunks (spawn ahead, retire behind)
        world.Update(player.x, prng);

        // Update particles
        UpdateParticles(dt);

        // Update screen shake
        UpdateScreenShake(dt);

        // Update camera position
        float targetCamX = player.x - 300.0f;
        if (targetCamX > cameraX) {
            cameraX = targetCamX;
        }

        // Rebase world origin if player X goes beyond 10,000 pixels
        if (player.x > 10000.0f) {
            world.Rebase(10000.0f, player, cameraX);
            previousCameraX = cameraX;
            // Rebase active particles too
            for (int i = 0; i < MAX_PARTICLES; ++i) {
                if (particles[i].active) {
                    particles[i].x -= 10000.0f;
                }
            }
        }
    }
}

void Game::RenderScene(SDL_Renderer* renderer) {
    // Determine active texture for the player sprite based on settingCharacter and movement state
    SDL_Texture* activePlayerTex = nullptr;
    if (settingCharacter == 0) {
        if (!player.grounded) {
            activePlayerTex = runnerJumpTex;
        } else if (player.vx != 0.0f) {
            activePlayerTex = runnerRunTex;
        } else {
            activePlayerTex = runnerIdleTex;
        }
    } else {
        if (!player.grounded) {
            activePlayerTex = spacesuitJumpTex;
        } else if (player.vx != 0.0f) {
            activePlayerTex = spacesuitRunTex;
        } else {
            activePlayerTex = spacesuitIdleTex;
        }
    }

    // 1. Draw solid background based on state
    uint32_t bgHex = 0x111827; // Default Deep twilight (#111827)

    switch (currentState) {
        case GameState::Boot:        bgHex = 0x1F2937; break; // Far skyline
        case GameState::Title:       bgHex = 0x111827; break; // Deep sky
        case GameState::Gameplay:    bgHex = 0x111827; break;
        case GameState::Pause:       bgHex = 0x1F2937; break;
        case GameState::Failure:     bgHex = 0x3F1A24; break; // Very dark red twilight
        case GameState::Settings:    bgHex = 0x334155; break;
        case GameState::TestModes:   bgHex = 0x1e1b4b; break; // Dark blue/indigo
        case GameState::Diagnostics: bgHex = 0x111827; break;
        case GameState::Credits:     bgHex = 0x111827; break;
        case GameState::FatalError:  bgHex = 0x4c0519; break; // Deep crimson
    }

#ifdef PS4
    Uint8 bgR = (bgHex >> 16) & 0xFF;
    Uint8 bgG = (bgHex >> 8) & 0xFF;
    Uint8 bgB = bgHex & 0xFF;
    SDL_SetRenderDrawColor(renderer, bgR, bgG, bgB, 255);
    SDL_RenderClear(renderer);
#else
    Render::DrawRect(renderer, 0, 0, 1920, 1080, bgHex, true);
#endif

    // 2. Draw Safe Area outline when verifying UI layout (except in raw gameplay)
    SDL_Rect safe = Render::GetSafeArea();
    if (currentState == GameState::Title || currentState == GameState::Settings || currentState == GameState::TestModes) {
        // Draw thin border to verify aspect-ratio scaling
        Render::DrawRect(renderer, safe.x, safe.y, safe.w, safe.h, 0x374151, false);
    }

    // 3. Draw screen-specific elements
    switch (currentState) {
        case GameState::Boot: {
            Render::DrawText(renderer, "SKYLINE SPRINT", safe.x + 350, safe.y + 350, 8, 0x22D3EE);
            Render::DrawText(renderer, "BOOTING SYSTEM...", safe.x + 500, safe.y + 600, 3, 0xCBD5E1);
            break;
        }
        case GameState::Title: {
            // Draw title text
            Render::DrawText(renderer, "SKYLINE SPRINT", safe.x + 100, safe.y + 150, 7, 0x22D3EE);
            Render::DrawText(renderer, "Build ID: SkylineSprint-0.1.3-perf", safe.x + 100, safe.y + 220, 2, 0x94A3B8);

            // Draw controller status
            bool controllerOk = Input::IsControllerConnected();
            std::string padStr = "Pad Status: " + std::string(controllerOk ? "CONNECTED" : "NOT FOUND");
            Render::DrawText(renderer, padStr, safe.x + 100, safe.y + 260, 2, controllerOk ? 0x4ADE80 : 0xFB7185);

            // Menu choices
            std::string items[] = { "PLAY RUN", "TEST MODES", "SETTINGS", "DIAGNOSTICS", "CREDITS" };
            for (int i = 0; i < 5; ++i) {
                bool focus = (menuIndex == i);
                std::string prefix = focus ? "> " : "  ";
                uint32_t color = focus ? 0x22D3EE : 0xF8FAFC;
                Render::DrawText(renderer, prefix + items[i], safe.x + 100, safe.y + 360 + i * 80, 4, color);
            }

            // Draw a live running Pulse bobbing preview on the right
            Render::DrawRect(renderer, safe.x + 900, safe.y + 420, 700, 300, 0x1F2937, true); // Far skyline backdrop
            Render::DrawRect(renderer, safe.x + 900, safe.y + 420, 700, 10, 0x94A3B8, true); // Platform top
            
            // Render player preview
            int bobOffset = static_cast<int>(sinf(tickCount * 0.1f) * 8.0f);
            SDL_Rect pDst = { safe.x + 1200, safe.y + 420 - 64 + bobOffset, 64, 64 };
            SDL_RenderCopy(renderer, activePlayerTex, NULL, &pDst);

            // Footer
            Render::DrawText(renderer, "D-pad/Stick: Move   Cross/Enter: Select   Circle/Esc: Exit", safe.x + 100, safe.y + 850, 2, 0xCBD5E1);
            break;
        }
        case GameState::Gameplay: {
            float renderCamX = cameraX + shakeOffsetX;
            float renderCamY = shakeOffsetY;

            world.Render(renderer, platformTex, warningTex, shardTex, enemyTex, renderCamX, renderCamY);
            RenderParticles(renderer, renderCamX);
            player.Render(renderer, activePlayerTex, renderCamX, renderCamY);
            
            // Render HUD metrics
            Render::DrawText(renderer, "SCORE: " + std::to_string(score), safe.x + 40, safe.y + 40, 5, 0xFACC15);
            
            // Format multiplier to 1 decimal place
            char multBuf[16];
            snprintf(multBuf, sizeof(multBuf), "x%.1f", multiplier);
            Render::DrawText(renderer, "MULT: " + std::string(multBuf), safe.x + 600, safe.y + 40, 5, 0x22D3EE);
            
            int distMeters = static_cast<int>(distanceTraveled / 10.0f);
            Render::DrawText(renderer, "DIST: " + std::to_string(distMeters) + "m", safe.x + 1100, safe.y + 40, 5, 0xF8FAFC);
            break;
        }
        case GameState::Pause: {
            // Render underlying frozen gameplay in background
            world.Render(renderer, platformTex, warningTex, shardTex, enemyTex, cameraX);
            player.Render(renderer, activePlayerTex, cameraX, 0.0f, 1.0f);

            // Draw translucent dark overlay
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 17, 24, 39, 180);
            SDL_Rect fsRect = { 0, 0, 1920, 1080 };
            SDL_RenderFillRect(renderer, &fsRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

            Render::DrawText(renderer, "GAME PAUSED", safe.x + 500, safe.y + 200, 8, 0xA78BFA);

            std::string items[] = { "RESUME RUN", "RESTART SPRINT", "DIAGNOSTICS", "SETTINGS", "RETURN TO TITLE" };
            for (int i = 0; i < 5; ++i) {
                bool focus = (menuIndex == i);
                std::string prefix = focus ? "> " : "  ";
                uint32_t color = focus ? 0xA78BFA : 0xF8FAFC;
                Render::DrawText(renderer, prefix + items[i], safe.x + 550, safe.y + 360 + i * 70, 4, color);
            }

            Render::DrawText(renderer, "D-pad/Stick: Move   Cross/Enter: Confirm   Circle/Esc: Resume", safe.x + 400, safe.y + 800, 2, 0xCBD5E1);
            break;
        }
        case GameState::Failure: {
            Render::DrawText(renderer, "RUN FAILED", safe.x + 450, safe.y + 120, 8, 0xFB7185);

            // Stats summary layout
            Render::DrawText(renderer, "FINAL SCORE:      " + std::to_string(score), safe.x + 350, safe.y + 280, 3, 0xFACC15);
            int distMeters = static_cast<int>(distanceTraveled / 10.0f);
            Render::DrawText(renderer, "DISTANCE SPRINTED: " + std::to_string(distMeters) + "m", safe.x + 350, safe.y + 340, 3, 0xF8FAFC);
            Render::DrawText(renderer, "SHARDS COLLECTED:  " + std::to_string(shardsCollected), safe.x + 350, safe.y + 400, 3, 0x22D3EE);
            Render::DrawText(renderer, "SESSION BEST:     " + std::to_string(highScore), safe.x + 350, safe.y + 460, 3, 0x4ADE80);
            
            char seedHex[32];
            snprintf(seedHex, sizeof(seedHex), "0x%08X", activeSeed);
            Render::DrawText(renderer, "RUN SEED:         " + std::string(seedHex), safe.x + 350, safe.y + 520, 3, 0x94A3B8);

            // Interactive retry buttons
            Render::DrawText(renderer, "ENTER / CROSS: REPLAY SAME SEED", safe.x + 300, safe.y + 640, 3, 0x4ADE80);
            Render::DrawText(renderer, "SPACE / SQUARE: RESTART NEW SEED", safe.x + 300, safe.y + 700, 3, 0xFACC15);
            Render::DrawText(renderer, "ESCAPE: RETURN TO TITLE", safe.x + 300, safe.y + 760, 3, 0xFB7185);
            break;
        }
        case GameState::Settings: {
            Render::DrawText(renderer, "SETTINGS", safe.x + 100, safe.y + 150, 6, 0xA78BFA);

            std::string audioStr = std::string("AUDIO: ") + (settingAudio ? "ON" : "OFF");
            std::string shakeStr = std::string("SCREEN SHAKE: ") + (settingScreenShake ? "ON" : "OFF");
            std::string particleStr = "PARTICLES: ";
            if (settingParticles == 0)      particleStr += "LOW";
            else if (settingParticles == 1) particleStr += "MEDIUM";
            else                            particleStr += "HIGH";
            std::string diagStr = std::string("SHOW DIAGNOSTICS: ") + (settingShowDiagnostics ? "ON" : "OFF");
            std::string charStr = std::string("CHARACTER: ") + (settingCharacter == 0 ? "CYBER RUNNER" : "SPACESUIT");
            std::string resetStr = "RESET SESSION HIGHEST SCORE";
            std::string backStr = "BACK TO MENU";

            std::string items[] = { audioStr, shakeStr, particleStr, diagStr, charStr, resetStr, backStr };
            for (int i = 0; i < 7; ++i) {
                bool focus = (menuIndex == i);
                std::string prefix = focus ? "> " : "  ";
                uint32_t color = focus ? 0xA78BFA : 0xF8FAFC;
                Render::DrawText(renderer, prefix + items[i], safe.x + 100, safe.y + 260 + i * 65, 4, color);
            }

            Render::DrawText(renderer, "Use D-pad Left/Right or Cross to toggle values.", safe.x + 100, safe.y + 800, 2, 0xCBD5E1);
            Render::DrawText(renderer, "Circle / Backspace: Return", safe.x + 100, safe.y + 840, 2, 0xCBD5E1);
            break;
        }
        case GameState::TestModes: {
            Render::DrawText(renderer, "TEST MODES", safe.x + 100, safe.y + 150, 6, 0x22D3EE);

            char seedBuf[32];
            snprintf(seedBuf, sizeof(seedBuf), "0x%08X", testSeed);
            std::string seedPresetNames[] = { "Flat/Easy", "Gap-Heavy", "Obstacle-Heavy", "Vertical", "Long-run" };
            std::string seedStr = "RUN SEED: " + std::string(seedBuf) + " (" + seedPresetNames[testSeedIndex] + ")";
            std::string autoStr = std::string("AUTOMATIC BOT PLAY: ") + (testModeAuto ? "ENABLED" : "DISABLED");
            std::string stressStr = std::string("STRESS SIMULATION:  ") + (testModeStress ? "ON" : "OFF");
            std::string visualizerStr = "INPUT VISUALIZER STATE";
            std::string backStr = "BACK TO MENU";

            std::string items[] = { seedStr, autoStr, stressStr, visualizerStr, backStr };
            for (int i = 0; i < 5; ++i) {
                bool focus = (menuIndex == i);
                std::string prefix = focus ? "> " : "  ";
                uint32_t color = focus ? 0x22D3EE : 0xF8FAFC;
                Render::DrawText(renderer, prefix + items[i], safe.x + 100, safe.y + 250 + i * 70, 4, color);
            }

            // Input Visualizer drawing at bottom
            Render::DrawRect(renderer, safe.x + 100, safe.y + 600, 800, 180, 0x1F2937, true);
            Render::DrawText(renderer, "[INPUT VISUALIZER]", safe.x + 120, safe.y + 610, 2, 0x22D3EE);
            
            // Check sticks
            int stickX = Input::GetLeftStickX();
            int stickY = Input::GetLeftStickY();
            std::string axesStr = "Left Stick: X = " + std::to_string(stickX) + " | Y = " + std::to_string(stickY);
            Render::DrawText(renderer, axesStr, safe.x + 120, safe.y + 650, 2, 0xF8FAFC);

            // Check buttons held
            std::string buttons = "Held: ";
            if (Input::IsHeld(InputAction::Jump)) buttons += "Jump(Cross/Space) ";
            if (Input::IsHeld(InputAction::Pause)) buttons += "Pause(Options/Esc) ";
            if (Input::IsHeld(InputAction::MoveLeft)) buttons += "Left ";
            if (Input::IsHeld(InputAction::MoveRight)) buttons += "Right ";
            Render::DrawText(renderer, buttons, safe.x + 120, safe.y + 690, 2, 0xF8FAFC);
            
            bool isConnected = Input::IsControllerConnected();
            std::string conn = "Controller Status: " + std::string(isConnected ? "CONNECTED (Index 0)" : "NOT CONNECTED (Keyboard Active)");
            Render::DrawText(renderer, conn, safe.x + 120, safe.y + 730, 2, isConnected ? 0x4ADE80 : 0xFB7185);

            Render::DrawText(renderer, "Use D-pad Left/Right or Cross to toggle values.", safe.x + 100, safe.y + 800, 2, 0xCBD5E1);
            Render::DrawText(renderer, "Circle / Backspace: Return", safe.x + 100, safe.y + 840, 2, 0xCBD5E1);
            break;
        }
        case GameState::Diagnostics: {
            Render::DrawText(renderer, "DIAGNOSTICS & SYSTEM METRICS", safe.x + 100, safe.y + 150, 5, 0x4ADE80);

            uint32_t uptimeSecs = SDL_GetTicks() / 1000;
            Render::DrawText(renderer, "Build Identifier:   SkylineSprint-0.1.3-perf", safe.x + 100, safe.y + 240, 3, 0xF8FAFC);
            Render::DrawText(renderer, "Target Toolchain:   OpenOrbis PS4 Toolchain (v0.5.4)", safe.x + 100, safe.y + 290, 3, 0xF8FAFC);
            #ifdef PS4
            Render::DrawText(renderer, "Target Platform:    Orbis OS (PlayStation 4 Homebrew)", safe.x + 100, safe.y + 340, 3, 0xF8FAFC);
            #else
            Render::DrawText(renderer, "Target Platform:    Desktop Linux (x86_64 SDL2)", safe.x + 100, safe.y + 340, 3, 0xF8FAFC);
            #endif
            Render::DrawText(renderer, "Logical Workspace:  1920x1080 @ 60Hz Fixed Timestep", safe.x + 100, safe.y + 390, 3, 0xF8FAFC);
            Render::DrawText(renderer, "System Uptime:      " + std::to_string(uptimeSecs) + " seconds", safe.x + 100, safe.y + 440, 3, 0xF8FAFC);
            
            bool padOk = Input::IsControllerConnected();
            Render::DrawText(renderer, "Pad Subsystem:      " + std::string(padOk ? "OK (ACTIVE)" : "NOT FOUND (KEYBOARD FALLBACK)"), safe.x + 100, safe.y + 490, 3, padOk ? 0x4ADE80 : 0xFB7185);
            Render::DrawText(renderer, "Audio Subsystem:    OK (SDL MIXER fallbacks active)", safe.x + 100, safe.y + 540, 3, 0x4ADE80);
#ifdef PS4
            Render::DrawText(renderer, "Log File Path:      stdout / TTY (circular memory-backed)", safe.x + 100, safe.y + 590, 3, 0x94A3B8);
#else
            Render::DrawText(renderer, "Log File Path:      stdout / stderr (circular memory-backed)", safe.x + 100, safe.y + 590, 3, 0x94A3B8);
#endif

            Render::DrawText(renderer, "Circle / Backspace / Cross: Return to title menu", safe.x + 100, safe.y + 800, 2, 0xCBD5E1);
            break;
        }
        case GameState::Credits: {
            Render::DrawText(renderer, "CREDITS & LICENCES", safe.x + 100, safe.y + 150, 6, 0xCBD5E1);

            Render::DrawText(renderer, "Skyline Sprint is an original open-source retro title.", safe.x + 100, safe.y + 240, 3, 0xF8FAFC);
            Render::DrawText(renderer, "Created using the OpenOrbis PS4 Toolchain.", safe.x + 100, safe.y + 290, 3, 0xF8FAFC);
            
            Render::DrawText(renderer, "Libraries & Components:", safe.x + 100, safe.y + 370, 3, 0x22D3EE);
            Render::DrawText(renderer, "- Simple DirectMedia Layer (SDL2) & SDL2_image", safe.x + 120, safe.y + 420, 3, 0xCBD5E1);
            Render::DrawText(renderer, "- PCG32 Deterministic Random Number Generator", safe.x + 120, safe.y + 470, 3, 0xCBD5E1);
            Render::DrawText(renderer, "- musl C standard library for embedded platforms", safe.x + 120, safe.y + 520, 3, 0xCBD5E1);

            Render::DrawText(renderer, "Non-Affiliation Notice:", safe.x + 100, safe.y + 600, 3, 0xFB923C);
            Render::DrawText(renderer, "This software is 100% homebrew, built for research and", safe.x + 100, safe.y + 650, 3, 0xCBD5E1);
            Render::DrawText(renderer, "compatibility verification. It contains no Sony proprietary SDK,", safe.x + 100, safe.y + 700, 3, 0xCBD5E1);
            Render::DrawText(renderer, "keys, licences, or firmware. Not affiliated with Sony or Nintendo.", safe.x + 100, safe.y + 750, 3, 0xCBD5E1);

            Render::DrawText(renderer, "Press Circle / Cross / Enter: Return", safe.x + 100, safe.y + 850, 2, 0xCBD5E1);
            break;
        }
        case GameState::FatalError: {
            Render::DrawText(renderer, "FATAL ERROR", safe.x + 450, safe.y + 300, 8, 0xFB923C);
            Render::DrawText(renderer, "RENDER SYSTEM FAULT", safe.x + 450, safe.y + 500, 4, 0xF8FAFC);
            break;
        }
    }

    // 4. Render compact overlay if enabled
    if (settingShowDiagnostics && currentState != GameState::Boot && currentState != GameState::FatalError) {
        Render::DrawRect(renderer, safe.x, safe.y + 80, 820, 110, 0x111827, true);
        Render::DrawRect(renderer, safe.x, safe.y + 80, 820, 110, 0x374151, false);

        std::string line1 = "FPS: 60.0 | Ticks: " + std::to_string(tickCount) + " | Seed: ";
        char sBuf[32];
        snprintf(sBuf, sizeof(sBuf), "%08X", activeSeed);
        line1 += std::string(sBuf);
        Render::DrawText(renderer, line1, safe.x + 20, safe.y + 90, 2, 0x4ADE80);

        bool pad = Input::IsControllerConnected();
        std::string line2 = "Pad: " + std::string(pad ? "OK" : "NONE") + " | Audio: OK | Chunks: 3 | Drones: Bobbing";
        Render::DrawText(renderer, line2, safe.x + 20, safe.y + 120, 2, 0xCBD5E1);

        std::string line3 = "Mode: ";
        line3 += testModeAuto ? "AUTO-BOT " : "NORMAL-PLAY ";
        line3 += testModeStress ? "| STRESS: ON" : "| STRESS: OFF";
        Render::DrawText(renderer, line3, safe.x + 20, safe.y + 150, 2, 0x22D3EE);
    }
    // main.cpp owns the presentation path. Keeping it outside Game lets the
    // PS4 build render into a small offscreen surface and upscale only once.
}

std::string Game::GetStateName(GameState state) const {
    switch (state) {
        case GameState::Boot:        return "Boot";
        case GameState::Title:       return "Title";
        case GameState::Gameplay:    return "Gameplay";
        case GameState::Pause:       return "Pause";
        case GameState::Failure:     return "Failure";
        case GameState::Settings:    return "Settings";
        case GameState::TestModes:   return "TestModes";
        case GameState::Diagnostics: return "Diagnostics";
        case GameState::Credits:     return "Credits";
        case GameState::FatalError:  return "FatalError";
    }
    return "Unknown";
}

void Game::SpawnParticle(float x, float y, float vx, float vy, float life, uint32_t color) {
    int limit = 128;
    if (settingParticles == 0) limit = 64;
    else if (settingParticles == 2) limit = 256;

    for (int i = 0; i < limit; ++i) {
        if (!particles[i].active) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = vx;
            particles[i].vy = vy;
            particles[i].life = life;
            particles[i].maxLife = life;
            particles[i].color = color;
            particles[i].active = true;
            break;
        }
    }
}

void Game::UpdateParticles(float deltaTime) {
    int limit = 128;
    if (settingParticles == 0) limit = 64;
    else if (settingParticles == 2) limit = 256;

    for (int i = 0; i < limit; ++i) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx * deltaTime;
            particles[i].y += particles[i].vy * deltaTime;
            particles[i].life -= deltaTime;
            if (particles[i].life <= 0.0f) {
                particles[i].active = false;
            }
        }
    }
}

void Game::RenderParticles(SDL_Renderer* renderer, float cameraX) {
    int limit = 128;
    if (settingParticles == 0) limit = 64;
    else if (settingParticles == 2) limit = 256;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (int i = 0; i < limit; ++i) {
        if (!particles[i].active) {
            continue;
        }

        const float px = particles[i].x - cameraX;
        if (px < -10.0f || px > 1930.0f ||
            particles[i].y < -10.0f || particles[i].y > 1090.0f) {
            continue;
        }

        const float alphaPct = particles[i].life / particles[i].maxLife;
        const int size = 6;
        SDL_Rect dst = {
            static_cast<int>(px - size / 2),
            static_cast<int>(particles[i].y - size / 2),
            size,
            size
        };

        const uint8_t r = (particles[i].color >> 16) & 0xFF;
        const uint8_t g = (particles[i].color >> 8) & 0xFF;
        const uint8_t b = particles[i].color & 0xFF;
        const uint8_t a = static_cast<uint8_t>(alphaPct * 255.0f);

        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderFillRect(renderer, &dst);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

void Game::SpawnJumpTrail(float x, float y) {
    int count = (settingParticles == 0) ? 3 : (settingParticles == 1 ? 6 : 10);
    for (int i = 0; i < count; ++i) {
        float vx = static_cast<float>(prng.Range(-80, 20));
        float vy = static_cast<float>(prng.Range(10, 80));
        float life = static_cast<float>(prng.Range(3, 8)) * 0.1f;
        SpawnParticle(x, y, vx, vy, life, 0x22D3EE);
    }
}

void Game::SpawnLandingBurst(float x, float y) {
    int count = (settingParticles == 0) ? 4 : (settingParticles == 1 ? 8 : 15);
    for (int i = 0; i < count; ++i) {
        float vx = static_cast<float>(prng.Range(-150, 150));
        float vy = static_cast<float>(prng.Range(-60, 0));
        float life = static_cast<float>(prng.Range(4, 9)) * 0.1f;
        SpawnParticle(x, y, vx, vy, life, 0xCBD5E1);
    }
}

void Game::SpawnPickupBurst(float x, float y) {
    int count = (settingParticles == 0) ? 6 : (settingParticles == 1 ? 12 : 20);
    for (int i = 0; i < count; ++i) {
        float angle = static_cast<float>(prng.Range(0, 360)) * (3.14159f / 180.0f);
        float speed = static_cast<float>(prng.Range(100, 250));
        float vx = cosf(angle) * speed;
        float vy = sinf(angle) * speed;
        float life = static_cast<float>(prng.Range(3, 7)) * 0.1f;
        SpawnParticle(x, y, vx, vy, life, 0xFACC15);
    }
}

void Game::TriggerScreenShake(float duration, float intensity) {
    if (!settingScreenShake) return;
    shakeTime = duration;
    shakeIntensity = intensity;
}

void Game::UpdateScreenShake(float deltaTime) {
    if (shakeTime > 0.0f) {
        shakeTime -= deltaTime;
        if (shakeTime <= 0.0f) {
            shakeOffsetX = 0.0f;
            shakeOffsetY = 0.0f;
        } else {
            shakeOffsetX = static_cast<float>(prng.Range(-static_cast<int>(shakeIntensity), static_cast<int>(shakeIntensity)));
            shakeOffsetY = static_cast<float>(prng.Range(-static_cast<int>(shakeIntensity), static_cast<int>(shakeIntensity)));
        }
    } else {
        shakeOffsetX = 0.0f;
        shakeOffsetY = 0.0f;
    }
}
