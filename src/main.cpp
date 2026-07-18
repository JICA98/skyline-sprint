#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include "platform/platform.h"
#include "game/game_loop.h"

int main(int argc, char* argv[]) {
    // Disable buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    int maxFrames = -1;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--frames" && i + 1 < argc) {
            maxFrames = std::stoi(argv[++i]);
        }
    }

    std::cout << "Starting Skyline Sprint (" << BUILD_ID << ")" << std::endl;
    std::cout << "Toolchain: " << TOOLCHAIN_ID << std::endl;
    if (maxFrames > 0) {
        std::cout << "Headless frame limit: " << maxFrames << " frames." << std::endl;
    }

    if (!Platform_Init()) {
        std::cerr << "Platform initialization failed!" << std::endl;
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        Platform_Shutdown();
        return 1;
    }
    std::cout << "[INIT] SDL initialized" << std::endl;

#ifdef PS4
    // The OpenOrbis PS4 SDL video backend exposes its actual VideoOut mode as
    // display 0. shadPS4 can expose 1280x720 even when the game's logical
    // workspace is 1920x1080.
    int physicalWidth = 1920;
    int physicalHeight = 1080;
    SDL_DisplayMode displayMode{};

    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0 &&
        displayMode.w > 0 && displayMode.h > 0) {
        physicalWidth = displayMode.w;
        physicalHeight = displayMode.h;
    } else {
        std::cout << "[PS4-FB-FINAL] SDL_GetCurrentDisplayMode failed; "
                  << "falling back to 1920x1080: " << SDL_GetError() << std::endl;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Skyline Sprint",
        0,
        0,
        physicalWidth,
        physicalHeight,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Failed to create SDL window on PS4: "
                  << SDL_GetError() << std::endl;
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    std::cout << "[PS4-FB-FINAL] Window created: pointer=" << (void*)window
              << ", origin=0,0, requested=" << physicalWidth << "x" << physicalHeight
              << std::endl;

    // IMPORTANT:
    // Bind the software renderer to the SDL_Window, not directly to an
    // SDL_Surface. With the OpenOrbis SDL port, SDL_RenderPresent() then:
    //   1. executes the queued software render commands,
    //   2. calls SDL_UpdateWindowSurface(),
    //   3. copies the surface to the registered VideoOut buffer,
    //   4. submits the flip.
    //
    // SDL_CreateSoftwareRenderer(windowSurface) creates a surface-only
    // renderer whose renderer->window is null, so its present callback cannot
    // perform the PS4 window update by itself.
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE
    );

    if (!renderer) {
        std::cerr << "Failed to create window-bound SDL software renderer on PS4: "
                  << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    SDL_RendererInfo rendererInfo{};
    if (SDL_GetRendererInfo(renderer, &rendererInfo) == 0) {
        std::cout << "[PS4-FB-FINAL] Renderer name="
                  << (rendererInfo.name ? rendererInfo.name : "(null)")
                  << ", flags=0x" << std::hex << rendererInfo.flags << std::dec
                  << std::endl;
    } else {
        std::cout << "[PS4-FB-FINAL] SDL_GetRendererInfo failed: "
                  << SDL_GetError() << std::endl;
    }

    int outputWidth = 0;
    int outputHeight = 0;
    if (SDL_GetRendererOutputSize(renderer, &outputWidth, &outputHeight) == 0) {
        std::cout << "[PS4-FB-FINAL] Renderer output="
                  << outputWidth << "x" << outputHeight << std::endl;
    } else {
        std::cout << "[PS4-FB-FINAL] SDL_GetRendererOutputSize failed: "
                  << SDL_GetError() << std::endl;
    }

    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        std::cerr << "Failed to get PS4 window surface: "
                  << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    std::cout << "[PS4-FB-FINAL] Window surface: pointer="
              << (void*)windowSurface
              << ", size=" << windowSurface->w << "x" << windowSurface->h
              << ", format="
              << SDL_GetPixelFormatName(windowSurface->format->format)
              << ", pitch=" << windowSurface->pitch << std::endl;

#else
    // Create window (1920x1080) for desktop
    SDL_Window* window = SDL_CreateWindow(
        "Skyline Sprint",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1920,
        1080,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }
    std::cout << "[INIT] Window created: pointer=" << (void*)window << ", size=1920x1080" << std::endl;

    // Create renderer (try accelerated first, fall back to software/default)
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        std::cout << "Accelerated renderer failed, falling back to default renderer..." << std::endl;
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    if (!renderer) {
        std::cerr << "Failed to create any SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }
    std::cout << "[INIT] Software renderer created: pointer=" << (void*)renderer << std::endl;
#endif

    // Initialize game
    Game game;
    if (!game.Init(renderer, window)) {
        std::cerr << "Failed to initialize game logic!" << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    std::cout << "[INIT] Textures loaded" << std::endl;
    std::cout << "[INIT] Fonts loaded" << std::endl;
    std::cout << "[INIT] Game state initialized" << std::endl;
    std::cout << "[INIT] Entering main loop" << std::endl;

    // Main loop
    bool running = true;
    uint32_t lastTicks = SDL_GetTicks();
    int frameCount = 0;

    while (running) {
        uint32_t currentTicks = SDL_GetTicks();
        float delta = static_cast<float>(currentTicks - lastTicks) / 1000.0f;
        lastTicks = currentTicks;

        // Run frame simulation and rendering
        running = game.RunFrame(renderer, delta);

        frameCount++;
        if (maxFrames > 0 && frameCount >= maxFrames) {
            std::cout << "Reached target frame count of " << maxFrames << ". Exiting loop." << std::endl;
            running = false;
        }
    }

    game.Shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    Platform_Shutdown();

    std::cout << "Skyline Sprint shut down cleanly." << std::endl;
    return 0;
}
