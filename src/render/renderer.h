#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <string>

class Render {
public:
    static bool Init(SDL_Renderer* renderer);
    static void Shutdown();

    // Render text string using custom 8x8 bitmap font
    static void DrawText(SDL_Renderer* renderer, const std::string& text, int x, int y, int scale = 2, uint32_t colorHex = 0xF8FAFC);

    // Render color-tinted primitives in logical coordinates
    static void DrawRect(SDL_Renderer* renderer, int x, int y, int w, int h, uint32_t colorHex, bool fill = true);
    static void DrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, uint32_t colorHex);

    // Load a TGA texture from the dynamic assets path
    static SDL_Texture* LoadTexture(SDL_Renderer* renderer, const std::string& relativePath);

    // Retrieve safe area limits (5% margin inside 1920x1080)
    static SDL_Rect GetSafeArea();

private:
    static SDL_Texture* fontTexture;
};

#endif // RENDERER_H
