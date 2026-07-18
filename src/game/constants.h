#ifndef CONSTANTS_H
#define CONSTANTS_H

// Screen and resolution parameters
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

// Centralized physics tuning parameters
namespace PhysicsTuning {
    constexpr float DT = 1.0f / 60.0f; // 60 Hz fixed timestep simulation
    constexpr float GRAVITY = 2000.0f; // px/s^2
    constexpr float MAX_FALL_SPEED = 1200.0f; // px/s

    constexpr float MAX_RUN_SPEED = 500.0f; // px/s
    constexpr float ACCEL_GROUND = 2500.0f; // px/s^2
    constexpr float DECEL_GROUND = 3500.0f; // px/s^2

    constexpr float ACCEL_AIR = 1500.0f; // px/s^2
    constexpr float DECEL_AIR = 1000.0f; // px/s^2

    constexpr float JUMP_IMPULSE = -800.0f; // px/s (upward is negative Y)
    constexpr float JUMP_CUTOFF_IMPULSE = -300.0f; // variable jump height cutoff
    constexpr float DOUBLE_JUMP_IMPULSE = -700.0f;

    constexpr float COYOTE_DURATION = 0.10f; // coyote jump window
    constexpr int COYOTE_TICKS = 6; // 6 ticks @ 60Hz

    constexpr int JUMP_BUFFER_TICKS = 6; // 6 ticks @ 60Hz input buffering

    constexpr float KILL_PLANE_Y = 1000.0f; // drop below this and die
}

#endif // CONSTANTS_H
