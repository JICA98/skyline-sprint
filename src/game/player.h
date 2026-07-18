#ifndef PLAYER_H
#define PLAYER_H

#include "physics.h"
#include <SDL2/SDL.h>

class Player {
public:
    Player();
    void Reset(float startX, float startY);
    void Update();

    AABB GetAABB() const;
    AABB GetPrevAABB() const;

    float x, y;
    float prevX, prevY;
    float vx, vy;
    float w, h;

    bool grounded;
    bool facingRight;
    bool alive;
    bool justJumped;
    bool justDoubleJumped;

    // Timers in ticks
    int coyoteTicks;
    int jumpBufferTicks;
    bool canDoubleJump;

    float animTime;

    void Render(SDL_Renderer* renderer, SDL_Texture* spriteTexture, float cameraX, float cameraY = 0.0f);
};

#endif // PLAYER_H
