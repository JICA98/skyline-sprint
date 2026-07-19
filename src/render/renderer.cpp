#include "renderer.h"
#include "platform/platform.h"
#include "util/logger.h"
#include <SDL2/SDL_image.h>
#include <algorithm>

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


SDL_Texture* Render::LoadPlatformTexture(SDL_Renderer* renderer,
                                         const std::string& relativePath) {
    const std::string path = Platform_GetAssetPath(relativePath);
    Logger::Log(LogLevel::Info, "Renderer",
                "Loading and trimming platform artwork: " + path);

    SDL_Surface* loaded = IMG_Load(path.c_str());
    if (!loaded) {
        Logger::Log(LogLevel::Error, "Renderer",
                    "Failed to load platform image at: " + path +
                    ". Error: " + IMG_GetError());
        return nullptr;
    }

    SDL_Surface* rgba = SDL_ConvertSurfaceFormat(
        loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);

    if (!rgba) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("Failed to convert platform image: ") +
                    SDL_GetError());
        return nullptr;
    }

    // Updated platform art may be exported on a large opaque black canvas.
    // Treat transparent and almost-black pixels as empty only while finding
    // the outside bounds. Dark pixels inside the discovered sprite remain.
    auto pixelIsContent = [rgba](int x, int y) -> bool {
        const Uint8* row = static_cast<const Uint8*>(rgba->pixels) +
                           y * rgba->pitch;
        const Uint32 pixel = *reinterpret_cast<const Uint32*>(
            row + x * rgba->format->BytesPerPixel);

        Uint8 r = 0, g = 0, b = 0, a = 0;
        SDL_GetRGBA(pixel, rgba->format, &r, &g, &b, &a);
        if (a < 16) {
            return false;
        }

        const int brightest = std::max(static_cast<int>(r),
                              std::max(static_cast<int>(g),
                                       static_cast<int>(b)));
        return brightest > 24;
    };

    const int minRowContent = std::max(4, rgba->w / 64);
    const int minColumnContent = std::max(4, rgba->h / 64);

    int top = 0;
    for (; top < rgba->h; ++top) {
        int count = 0;
        for (int x = 0; x < rgba->w; ++x) {
            if (pixelIsContent(x, top) && ++count >= minRowContent) {
                break;
            }
        }
        if (count >= minRowContent) {
            break;
        }
    }

    int bottom = rgba->h - 1;
    for (; bottom >= top; --bottom) {
        int count = 0;
        for (int x = 0; x < rgba->w; ++x) {
            if (pixelIsContent(x, bottom) && ++count >= minRowContent) {
                break;
            }
        }
        if (count >= minRowContent) {
            break;
        }
    }

    int left = 0;
    for (; left < rgba->w; ++left) {
        int count = 0;
        for (int y = top; y <= bottom; ++y) {
            if (pixelIsContent(left, y) && ++count >= minColumnContent) {
                break;
            }
        }
        if (count >= minColumnContent) {
            break;
        }
    }

    int right = rgba->w - 1;
    for (; right >= left; --right) {
        int count = 0;
        for (int y = top; y <= bottom; ++y) {
            if (pixelIsContent(right, y) && ++count >= minColumnContent) {
                break;
            }
        }
        if (count >= minColumnContent) {
            break;
        }
    }

    // Fall back to the full image if no meaningful bounds were detected.
    if (top >= rgba->h || bottom < top || left >= rgba->w || right < left) {
        top = 0;
        bottom = rgba->h - 1;
        left = 0;
        right = rgba->w - 1;
    }

    const int cropW = right - left + 1;
    const int cropH = bottom - top + 1;
    SDL_Surface* cropped = SDL_CreateRGBSurfaceWithFormat(
        0, cropW, cropH, 32, SDL_PIXELFORMAT_RGBA32);

    if (!cropped) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("Failed to allocate cropped platform surface: ") +
                    SDL_GetError());
        SDL_FreeSurface(rgba);
        return nullptr;
    }

    SDL_SetSurfaceBlendMode(rgba, SDL_BLENDMODE_NONE);
    SDL_Rect source = { left, top, cropW, cropH };
    SDL_Rect destination = { 0, 0, cropW, cropH };
    if (SDL_BlitSurface(rgba, &source, cropped, &destination) != 0) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("Failed to crop platform surface: ") +
                    SDL_GetError());
        SDL_FreeSurface(cropped);
        SDL_FreeSurface(rgba);
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, cropped);
    SDL_FreeSurface(cropped);
    SDL_FreeSurface(rgba);

    if (!texture) {
        Logger::Log(LogLevel::Error, "Renderer",
                    std::string("Failed to create cropped platform texture: ") +
                    SDL_GetError());
        return nullptr;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    Logger::Log(LogLevel::Info, "Renderer",
                "Platform artwork trimmed to " +
                std::to_string(cropW) + "x" + std::to_string(cropH) +
                " from bounds x=" + std::to_string(left) +
                ".." + std::to_string(right) +
                ", y=" + std::to_string(top) +
                ".." + std::to_string(bottom));
    return texture;
}

SDL_Rect Render::GetSafeArea() {
    return { 96, 54, 1728, 972 };
}
