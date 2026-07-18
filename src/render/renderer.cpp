#include "renderer.h"
#include "platform/platform.h"
#include "util/logger.h"
#include <SDL2/SDL_image.h>
#include <iostream>

SDL_Texture* Render::fontTexture = nullptr;

bool Render::Init(SDL_Renderer* renderer) {
    // 1. Force 1920x1080 logical size to handle scale & letterbox automatically
    SDL_RenderSetLogicalSize(renderer, 1920, 1080);

    // 2. Load font texture
    fontTexture = LoadTexture(renderer, "assets/images/font.tga");
    if (!fontTexture) {
        Logger::Log(LogLevel::Error, "Renderer", "Failed to load custom font texture!");
        return false;
    }

    Logger::Log(LogLevel::Info, "Renderer", "Logical size set to 1920x1080. Font texture loaded.");
    return true;
}

void Render::Shutdown() {
    if (fontTexture) {
        SDL_DestroyTexture(fontTexture);
        fontTexture = nullptr;
    }
}

void Render::DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y, int scale, uint32_t colorHex) {
    if (!fontTexture) return;

    // Apply color tinting
    Uint8 r = (colorHex >> 16) & 0xFF;
    Uint8 g = (colorHex >> 8) & 0xFF;
    Uint8 b = colorHex & 0xFF;
    SDL_SetTextureColorMod(fontTexture, r, g, b);

    int curX = x;
    int curY = y;

    for (char c : text) {
        if (c == '\n') {
            curX = x;
            curY += 8 * scale;
            continue;
        }

        // Font contains characters in range [32, 127]
        if (c < 32 || c > 127) {
            c = '?'; // Fallback
        }

        int idx = static_cast<int>(c) - 32;
        int col = idx % 16;
        int row = idx / 16;

        SDL_Rect srcRect = { col * 8, row * 8, 8, 8 };
        SDL_Rect dstRect = { curX, curY, 8 * scale, 8 * scale };

        SDL_RenderCopy(renderer, fontTexture, &srcRect, &dstRect);
        curX += 8 * scale;
    }
}

void Render::DrawRect(SDL_Renderer* renderer, int x, int y, int w, int h, uint32_t colorHex, bool fill) {
    Uint8 r = (colorHex >> 16) & 0xFF;
    Uint8 g = (colorHex >> 8) & 0xFF;
    Uint8 b = colorHex & 0xFF;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    SDL_Rect rect = { x, y, w, h };
    if (fill) {
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void Render::DrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, uint32_t colorHex) {
    Uint8 r = (colorHex >> 16) & 0xFF;
    Uint8 g = (colorHex >> 8) & 0xFF;
    Uint8 b = colorHex & 0xFF;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

SDL_Texture* Render::LoadTexture(SDL_Renderer* renderer, const std::string& relativePath) {
    std::string path = Platform_GetAssetPath(relativePath);
    Logger::Log(LogLevel::Info, "Renderer", "Loading texture from path: " + path);
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        std::string errStr = "Failed to load image at: " + path + ". Error: " + IMG_GetError();
        Logger::Log(LogLevel::Error, "Renderer", errStr);
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        Logger::Log(LogLevel::Error, "Renderer", "Failed to create texture from surface for: " + path);
        return nullptr;
    }

    return texture;
}

SDL_Rect Render::GetSafeArea() {
    // 5% margin inside 1920x1080 coordinates
    // X: 96, Y: 54, W: 1728, H: 972
    return { 96, 54, 1728, 972 };
}
