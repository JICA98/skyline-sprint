#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include "platform/platform.h"
#include "game/game_loop.h"

namespace {
constexpr double TARGET_FRAME_SECONDS = 1.0 / 60.0;

void PaceFrame(Uint64 frameStart, Uint64 frequency) {
#ifdef PS4
    if (frequency == 0) {
        return;
    }

    double elapsed = static_cast<double>(SDL_GetPerformanceCounter() - frameStart) /
                     static_cast<double>(frequency);
    double remaining = TARGET_FRAME_SECONDS - elapsed;

    // VideoOut normally blocks on the flip. This fallback only engages when
    // the host returns early, preventing an uncapped loop and uneven deltas.
    if (remaining > 0.002) {
        const Uint32 sleepMs =
            static_cast<Uint32>((remaining - 0.001) * 1000.0);
        if (sleepMs > 0) {
            SDL_Delay(sleepMs);
        }
    }
#else
    (void)frameStart;
    (void)frequency;
#endif
}
}

int main(int argc, char* argv[]) {
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

    // Nearest-neighbour scaling is substantially cheaper for the PS4 software
    // renderer and matches the retro pixel-art presentation.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        Platform_Shutdown();
        return 1;
    }
    std::cout << "[INIT] SDL initialized" << std::endl;

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

#ifdef PS4
    int physicalWidth = 1920;
    int physicalHeight = 1080;
    SDL_DisplayMode displayMode{};

    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0 &&
        displayMode.w > 0 && displayMode.h > 0) {
        physicalWidth = displayMode.w;
        physicalHeight = displayMode.h;
    } else {
        std::cout << "[PS4-PERF] SDL_GetCurrentDisplayMode failed; "
                  << "falling back to 1920x1080: " << SDL_GetError() << std::endl;
    }

    window = SDL_CreateWindow(
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

    SDL_Surface* windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        std::cerr << "Failed to get PS4 window surface: "
                  << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    // Render at half the physical width and height, then perform one nearest
    // upscale into VideoOut. This cuts software-rendering pixel work to 25%.
    // 1280x720 hosts use 640x360; 1920x1080 hosts use 960x540.
    const int internalWidth = (physicalWidth >= 1280) ? physicalWidth / 2 : physicalWidth;
    const int internalHeight = (physicalHeight >= 720) ? physicalHeight / 2 : physicalHeight;

    SDL_Surface* renderSurface = SDL_CreateRGBSurfaceWithFormat(
        0,
        internalWidth,
        internalHeight,
        32,
        windowSurface->format->format
    );

    if (!renderSurface) {
        std::cerr << "Failed to create PS4 low-resolution render surface: "
                  << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    renderer = SDL_CreateSoftwareRenderer(renderSurface);
    if (!renderer) {
        std::cerr << "Failed to create PS4 software renderer: "
                  << SDL_GetError() << std::endl;
        SDL_FreeSurface(renderSurface);
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    std::cout << "[PS4-PERF] VideoOut=" << physicalWidth << "x" << physicalHeight
              << ", internal=" << internalWidth << "x" << internalHeight
              << ", format=" << SDL_GetPixelFormatName(windowSurface->format->format)
              << std::endl;
#else
    window = SDL_CreateWindow(
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

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    if (!renderer) {
        std::cerr << "Failed to create any SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }
#endif

    Game game;
    if (!game.Init(renderer, window)) {
        std::cerr << "Failed to initialize game logic!" << std::endl;
        SDL_DestroyRenderer(renderer);
#ifdef PS4
        SDL_FreeSurface(renderSurface);
#endif
        SDL_DestroyWindow(window);
        SDL_Quit();
        Platform_Shutdown();
        return 1;
    }

    std::cout << "[INIT] Game initialized; entering main loop" << std::endl;

    bool running = true;
    Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
    if (performanceFrequency == 0) {
        performanceFrequency = 1000;
    }
    Uint64 lastCounter = SDL_GetPerformanceCounter();
    int frameCount = 0;

    while (running) {
        const Uint64 frameStart = SDL_GetPerformanceCounter();
        float delta = static_cast<float>(
            static_cast<double>(frameStart - lastCounter) /
            static_cast<double>(performanceFrequency)
        );
        lastCounter = frameStart;

        running = game.RunFrame(renderer, delta);

#ifdef PS4
        // Flush software commands into the small internal surface.
        SDL_RenderPresent(renderer);

        // One optimized nearest-neighbour upscale and one VideoOut submission.
        if (SDL_BlitScaled(renderSurface, nullptr, windowSurface, nullptr) != 0) {
            std::cerr << "[PS4-PERF] SDL_BlitScaled failed: "
                      << SDL_GetError() << std::endl;
            running = false;
        } else if (SDL_UpdateWindowSurface(window) != 0) {
            std::cerr << "[PS4-PERF] SDL_UpdateWindowSurface failed: "
                      << SDL_GetError() << std::endl;
            running = false;
        }
#else
        SDL_RenderPresent(renderer);
#endif

        ++frameCount;
        if (maxFrames > 0 && frameCount >= maxFrames) {
            running = false;
        }

        if (running) {
            PaceFrame(frameStart, performanceFrequency);
        }
    }

    game.Shutdown();
    SDL_DestroyRenderer(renderer);
#ifdef PS4
    SDL_FreeSurface(renderSurface);
#endif
    SDL_DestroyWindow(window);
    SDL_Quit();
    Platform_Shutdown();

    std::cout << "Skyline Sprint shut down cleanly." << std::endl;
    return 0;
}
