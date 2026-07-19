#include "renderer.h"
#include "platform/platform.h"
#include "util/logger.h"
#include <SDL2/SDL_image.h>

SDL_Texture* Render::fontTexture = nullptr;
uint32_t Render::lastFontColor = 0xFFFFFFFFu;

bool Render::Init(SDL_Renderer* renderer) {
    if (SDL_RenderSetLogicalSize(renderer, 1920, 1080) != 0) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("SDL_RenderSetLogicalSize failed: ") + SDL_GetError());
        return false;
    }

    fontTexture = LoadTexture(renderer, "assets/images/font.tga", true);
    if (!fontTexture) {
        Logger::Log(LogLevel::Error, "Renderer", "Failed to load custom font texture!");
        return false;
    }

    lastFontColor = 0xFFFFFFFFu;
    Logger::Log(LogLevel::Info, "Renderer",
                "Logical size set to 1920x1080; optimized text path active.");
    return true;
}

void Render::Shutdown() {
    if (fontTexture) {
        SDL_DestroyTexture(fontTexture);
        fontTexture = nullptr;
    }
    lastFontColor = 0xFFFFFFFFu;
}

void Render::DrawText(SDL_Renderer* renderer, const char* text, int x, int y,
                      int scale, uint32_t colorHex) {
    if (!fontTexture || !text || scale <= 0) {
        return;
    }

    if (lastFontColor != colorHex) {
        const Uint8 r = static_cast<Uint8>((colorHex >> 16) & 0xFF);
        const Uint8 g = static_cast<Uint8>((colorHex >> 8) & 0xFF);
        const Uint8 b = static_cast<Uint8>(colorHex & 0xFF);
        SDL_SetTextureColorMod(fontTexture, r, g, b);
        lastFontColor = colorHex;
    }

    int curX = x;
    int curY = y;
    const int glyphSize = 8 * scale;

    for (const char* p = text; *p != '\0'; ++p) {
        unsigned char c = static_cast<unsigned char>(*p);

        if (c == '\n') {
            curX = x;
            curY += glyphSize;
            continue;
        }

        if (c < 32 || c > 127) {
            c = '?';
        }

        const int idx = static_cast<int>(c) - 32;
        SDL_Rect srcRect = { (idx % 16) * 8, (idx / 16) * 8, 8, 8 };
        SDL_Rect dstRect = { curX, curY, glyphSize, glyphSize };
        SDL_RenderCopy(renderer, fontTexture, &srcRect, &dstRect);
        curX += glyphSize;
    }
}

void Render::DrawRect(SDL_Renderer* renderer, int x, int y, int w, int h,
                      uint32_t colorHex, bool fill) {
    const Uint8 r = static_cast<Uint8>((colorHex >> 16) & 0xFF);
    const Uint8 g = static_cast<Uint8>((colorHex >> 8) & 0xFF);
    const Uint8 b = static_cast<Uint8>(colorHex & 0xFF);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    SDL_Rect rect = { x, y, w, h };
    if (fill) {
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void Render::DrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
                      uint32_t colorHex) {
    const Uint8 r = static_cast<Uint8>((colorHex >> 16) & 0xFF);
    const Uint8 g = static_cast<Uint8>((colorHex >> 8) & 0xFF);
    const Uint8 b = static_cast<Uint8>(colorHex & 0xFF);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

SDL_Texture* Render::LoadTexture(SDL_Renderer* renderer,
                                 const std::string& relativePath,
                                 bool useAlpha) {
    const std::string path = Platform_GetAssetPath(relativePath);
    Logger::Log(LogLevel::Info, "Renderer", "Loading texture from path: " + path);

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        Logger::Log(LogLevel::Error, "Renderer",
                    "Failed to load image at: " + path + ". Error: " + IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        Logger::Log(LogLevel::Error, "Renderer",
                    "Failed to create texture from surface for: " + path);
        return nullptr;
    }

    SDL_SetTextureBlendMode(texture, useAlpha ? SDL_BLENDMODE_BLEND
                                              : SDL_BLENDMODE_NONE);
    return texture;
}


SDL_Texture* Render::LoadTiledTexture(SDL_Renderer* renderer,
                                      const std::string& relativePath,
                                      int width, int height) {
    if (width <= 0 || height <= 0) {
        return nullptr;
    }

    const std::string path = Platform_GetAssetPath(relativePath);
    Logger::Log(LogLevel::Info, "Renderer",
                "Building cached tiled texture from: " + path);

    SDL_Surface* tile = IMG_Load(path.c_str());
    if (!tile) {
        Logger::Log(LogLevel::Error, "Renderer",
                    "Failed to load tile image at: " + path +
                    ". Error: " + IMG_GetError());
        return nullptr;
    }

    SDL_Surface* tiled = SDL_CreateRGBSurfaceWithFormat(
        0, width, height, tile->format->BitsPerPixel,
        tile->format->format);
    if (!tiled) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("Failed to create tiled surface: ") +
                    SDL_GetError());
        SDL_FreeSurface(tile);
        return nullptr;
    }

    const bool tileHasAlpha = tile->format->Amask != 0;
    // Disable blending while constructing the cache so source alpha values are
    // copied rather than composited into the cache.
    SDL_SetSurfaceBlendMode(tile, SDL_BLENDMODE_NONE);
    for (int y = 0; y < height; y += tile->h) {
        for (int x = 0; x < width; x += tile->w) {
            SDL_Rect dst = { x, y, tile->w, tile->h };
            if (SDL_BlitSurface(tile, nullptr, tiled, &dst) != 0) {
                Logger::Log(LogLevel::Error, "Renderer",
                            std::string("Failed while building tiled surface: ") +
                            SDL_GetError());
                SDL_FreeSurface(tiled);
                SDL_FreeSurface(tile);
                return nullptr;
            }
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tiled);
    SDL_FreeSurface(tiled);
    SDL_FreeSurface(tile);

    if (!texture) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("Failed to create cached tiled texture: ") +
                    SDL_GetError());
        return nullptr;
    }

    SDL_SetTextureBlendMode(texture, tileHasAlpha ? SDL_BLENDMODE_BLEND
                                                   : SDL_BLENDMODE_NONE);
    return texture;
}

SDL_Rect Render::GetSafeArea() {
    return { 96, 54, 1728, 972 };
}
