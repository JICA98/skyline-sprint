#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <string>

class Render {
public:
    static bool Init(SDL_Renderer* renderer);
    static void Shutdown();

    // const char* overload avoids temporary std::string allocations for literals.
    static void DrawText(SDL_Renderer* renderer, const char* text, int x, int y,
                         int scale = 2, uint32_t colorHex = 0xF8FAFC);
    static void DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y,
                         int scale = 2, uint32_t colorHex = 0xF8FAFC) {
        DrawText(renderer, text.c_str(), x, y, scale, colorHex);
    }

    static void DrawRect(SDL_Renderer* renderer, int x, int y, int w, int h,
                         uint32_t colorHex, bool fill = true);
    static void DrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
                         uint32_t colorHex);

    static SDL_Texture* LoadTexture(SDL_Renderer* renderer,
                                    const std::string& relativePath,
                                    bool useAlpha = true);

    // Builds a large repeated texture once at startup. This replaces hundreds
    // of per-frame tile draw calls with one SDL_RenderCopy per platform.
    static SDL_Texture* LoadTiledTexture(SDL_Renderer* renderer,
                                         const std::string& relativePath,
                                         int width, int height);

    static SDL_Rect GetSafeArea();

private:
    static SDL_Texture* fontTexture;
    static uint32_t lastFontColor;
};

#endif // RENDERER_H
