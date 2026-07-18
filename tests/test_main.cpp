#include <iostream>
#include <cassert>
#include "util/random.h"
#include "util/logger.h"
#include "game/game_loop.h"
#include "input/input.h"
#include "game/player.h"
#include "game/world.h"
#include "game/constants.h"

// Mock SDL Renderer for tests (not rendering anything)
SDL_Renderer* mockRenderer = nullptr;

void Test_PRNG_Determinism() {
    std::cout << "[Test] Running PRNG determinism check..." << std::endl;
    Random prng(0xDEADC0DE);

    // Verify first 5 numbers from Range(0, 100)
    uint32_t v1 = prng.Range(0, 100);
    uint32_t v2 = prng.Range(0, 100);
    uint32_t v3 = prng.Range(0, 100);
    uint32_t v4 = prng.Range(0, 100);
    uint32_t v5 = prng.Range(0, 100);

    // Baseline outputs generated deterministically using PCG32
    std::cout << "PRNG outputs: " << v1 << ", " << v2 << ", " << v3 << ", " << v4 << ", " << v5 << std::endl;
    
    // Check internal state progression consistency
    Random prng2(0xDEADC0DE);
    assert(prng2.Range(0, 100) == v1);
    assert(prng2.Range(0, 100) == v2);
    assert(prng2.Range(0, 100) == v3);
    assert(prng2.Range(0, 100) == v4);
    assert(prng2.Range(0, 100) == v5);

    std::cout << "[Test] PRNG determinism check: PASSED" << std::endl;
}

void Test_GameState_Transitions() {
    std::cout << "[Test] Running GameState transitions check..." << std::endl;

    Game game;
    // Set dummy video driver to prevent SDL failures in headless environment
    SDL_SetHint(SDL_HINT_VIDEODRIVER, "dummy");
    
    // We pass nullptr renderer to Init. Since we have dummy driver and headless test doesn't draw, it should initialize
    // Logger and Input, and set state to Boot.
    // Wait, let's verify if game.Init(nullptr) fails because Render::Init fails.
    // Render::Init(nullptr) returns false (since fontTexture won't load with null renderer).
    // So we need to mock or initialize the game system manually without full renderer validation for unit tests!
    // Or we can create a mock window/renderer using SDL dummy driver!
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("Test", 0, 0, 1, 1, SDL_WINDOW_HIDDEN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    
    assert(ren != nullptr);

    bool initOk = game.Init(ren, win);
    assert(initOk);
    assert(game.GetState() == GameState::Boot);

    // Boot screen auto transitions to Title after 120 ticks
    game.RunHeadlessTicks(120);
    assert(game.GetState() == GameState::Title);

    // Select input transitions Title to Gameplay
    Input::SetHeadlessInput(InputAction::Select, true);
    game.RunFrame(ren, 1.0f / 60.0f);
    Input::ClearHeadlessInput();
    
    assert(game.GetState() == GameState::Gameplay);

    // Pause input transitions Gameplay to Pause
    Input::SetHeadlessInput(InputAction::Pause, true);
    game.RunFrame(ren, 1.0f / 60.0f);
    Input::ClearHeadlessInput();
    game.RunFrame(ren, 1.0f / 60.0f); // Release frame

    assert(game.GetState() == GameState::Pause);

    // Pause input transitions Pause back to Gameplay
    Input::SetHeadlessInput(InputAction::Pause, true);
    game.RunFrame(ren, 1.0f / 60.0f);
    Input::ClearHeadlessInput();
    game.RunFrame(ren, 1.0f / 60.0f); // Release frame

    assert(game.GetState() == GameState::Gameplay);

    game.Shutdown();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    std::cout << "[Test] GameState transitions check: PASSED" << std::endl;
}

void Test_Player_Physics_And_Collisions() {
    std::cout << "[Test] Running Player physics and collision check..." << std::endl;

    Player player;
    player.Reset(100.0f, 100.0f);
    assert(player.x == 100.0f);
    assert(player.y == 100.0f);
    assert(!player.grounded);

    // Build a mock platform at Y=200
    World world;
    Random prng(12345);
    world.Reset(12345, prng);

    // Simulate free-fall
    // Player falls 10 frames under gravity
    for (int i = 0; i < 10; ++i) {
        player.Update();
        world.ResolvePlayerCollision(player);
    }

    assert(player.y > 100.0f);
    assert(player.vy > 0.0f);

    // Now test a manual collision box
    // Position player just above a floor tile
    player.Reset(100.0f, 568.0f); // platform top is at 600.0f
    player.vy = 50.0f; // moving down
    player.Update();
    world.ResolvePlayerCollision(player);

    // Player should land exactly on top of the platform (Y = 600 - 32 = 568)
    assert(player.grounded);
    assert(player.vy == 0.0f);
    assert(player.y == 568.0f);

    // Verify coyote time: jump immediately after leaving platform
    // Move player off to the side (X = 3000, where there is a gap)
    player.x = 2000.0f;
    player.grounded = false;
    player.coyoteTicks = PhysicsTuning::COYOTE_TICKS;
    
    // Press jump button while not grounded but within coyote time
    Input::SetHeadlessInput(InputAction::Jump, true);
    Input::Update(0); // Update/cycle input state
    player.Update();
    Input::ClearHeadlessInput();
    
    // Jump should have triggered (vy = JUMP_IMPULSE)
    assert(player.vy == PhysicsTuning::JUMP_IMPULSE);

    std::cout << "[Test] Player physics and collision check: PASSED" << std::endl;
}

int main() {
    std::cout << "=== Running Skyline Sprint Unit Tests ===" << std::endl;
    Test_PRNG_Determinism();
    Test_GameState_Transitions();
    Test_Player_Physics_And_Collisions();
    std::cout << "=== All Unit Tests PASSED ===" << std::endl;
    return 0;
}
