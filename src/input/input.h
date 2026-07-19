#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>
#include <cstdint>

enum class InputAction {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Jump,
    Pause,
    Select,
    Back,
    F1,
    F2,
    F3,
    F4,
    Count
};

struct InputState {
    bool current[static_cast<int>(InputAction::Count)];
    bool previous[static_cast<int>(InputAction::Count)];
    int leftStickX;
    int leftStickY;
    bool controllerConnected;
};

class Input {
public:
    static void Init();
    static void Shutdown();
    static void Update(uint32_t currentTick);

    // Unified action checks
    static bool IsHeld(InputAction action);
    static bool WasPressed(InputAction action);
    static bool WasReleased(InputAction action);

    // Stick values
    static int GetLeftStickX() { return state.leftStickX; }
    static int GetLeftStickY() { return state.leftStickY; }
    static bool IsControllerConnected() { return state.controllerConnected; }

    // Test overrides
    static void SetHeadlessInput(InputAction action, bool pressed);
    static void ClearHeadlessInput();

private:
    static InputState state;
    static SDL_Joystick* joystick;
    static bool useHeadlessInput;
    static bool headlessState[static_cast<int>(InputAction::Count)];
    static uint32_t lastConnectionPollTick;
};

#endif // INPUT_H
