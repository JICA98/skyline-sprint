#include "player.h"
#include "constants.h"
#include "input/input.h"
#include "util/logger.h"
#include "audio/audio_manager.h"
#include <math.h>
#include <iostream>

using namespace PhysicsTuning;

Player::Player() {
    Reset(100.0f, 500.0f);
}

void Player::Reset(float startX, float startY) {
    x = startX;
    y = startY;
    prevX = startX;
    prevY = startY;
    vx = 0.0f;
    vy = 0.0f;
    w = 32.0f;
    h = 32.0f;
    grounded = false;
    facingRight = true;
    alive = true;
    justJumped = false;
    justDoubleJumped = false;
    coyoteTicks = 0;
    jumpBufferTicks = 0;
    canDoubleJump = true;
    animTime = 0.0f;
}

void Player::Update() {
    if (!alive) return;

    prevX = x;
    prevY = y;

    // 1. Process timers
    if (grounded) {
        coyoteTicks = COYOTE_TICKS;
        canDoubleJump = true;
    } else if (coyoteTicks > 0) {
        coyoteTicks--;
    }

    if (jumpBufferTicks > 0) {
        jumpBufferTicks--;
    }

    if (Input::WasPressed(InputAction::Jump)) {
        jumpBufferTicks = JUMP_BUFFER_TICKS;
    }

    // 2. Horizontal movement physics
    float targetVx = 0.0f;
    if (Input::IsHeld(InputAction::MoveLeft)) {
        targetVx = -MAX_RUN_SPEED;
        facingRight = false;
    } else if (Input::IsHeld(InputAction::MoveRight)) {
        targetVx = MAX_RUN_SPEED;
        facingRight = true;
    }

    float accel = grounded ? ACCEL_GROUND : ACCEL_AIR;
    float decel = grounded ? DECEL_GROUND : DECEL_AIR;

    if (targetVx != 0.0f) {
        // Accelerate
        if (vx < targetVx) {
            vx = std::min(targetVx, vx + accel * DT);
        } else if (vx > targetVx) {
            vx = std::max(targetVx, vx - accel * DT);
        }
    } else {
        // Decelerate
        if (vx > 0.0f) {
            vx = std::max(0.0f, vx - decel * DT);
        } else if (vx < 0.0f) {
            vx = std::min(0.0f, vx + decel * DT);
        }
    }

    // 3. Gravity
    vy += GRAVITY * DT;
    if (vy > MAX_FALL_SPEED) {
        vy = MAX_FALL_SPEED;
    }

    // 4. Resolve Jumps
    if (jumpBufferTicks > 0) {
        if (coyoteTicks > 0) {
            // Normal ground jump
            vy = JUMP_IMPULSE;
            coyoteTicks = 0;
            jumpBufferTicks = 0;
            grounded = false;
            justJumped = true;
            AudioManager::PlaySound(SoundType::Jump);
            Logger::Log(LogLevel::Debug, "Physics", "Ground jump executed.");
        } else if (canDoubleJump) {
            // Double jump in mid-air
            vy = DOUBLE_JUMP_IMPULSE;
            canDoubleJump = false;
            jumpBufferTicks = 0;
            justDoubleJumped = true;
            AudioManager::PlaySound(SoundType::Jump);
            Logger::Log(LogLevel::Debug, "Physics", "Double jump executed.");
        }
    }

    // 5. Variable Jump Height Cutoff (if button released early)
    if (Input::WasReleased(InputAction::Jump) || !Input::IsHeld(InputAction::Jump)) {
        if (vy < JUMP_CUTOFF_IMPULSE) {
            vy = JUMP_CUTOFF_IMPULSE;
        }
    }

    // 6. Animation ticker
    if (grounded && vx != 0.0f) {
        animTime += DT * 10.0f;
    } else {
        animTime = 0.0f;
    }

    // 7. Death / Kill plane check
    if (y > KILL_PLANE_Y) {
        alive = false;
        Logger::Log(LogLevel::Info, "Player", "Player fell below kill plane.");
    }
}

AABB Player::GetAABB() const {
    return { x, y, w, h };
}

AABB Player::GetPrevAABB() const {
    return { prevX, prevY, w, h };
}

void Player::Render(SDL_Renderer* renderer, SDL_Texture* spriteTexture, float cameraX, float cameraY) {
    if (!alive) return;

    // Draw legs bobbing based on animTime
    int legOffset = static_cast<int>(sinf(animTime)) * 2;
    SDL_Rect dstRect = {
        static_cast<int>(x - cameraX - 16.0f),
        static_cast<int>(y - cameraY - 16.0f) + legOffset,
        64,
        64
    };

    // Flip horizontal if facing left
    SDL_RendererFlip flip = facingRight ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
    SDL_RenderCopyEx(renderer, spriteTexture, NULL, &dstRect, 0.0, NULL, flip);
}
