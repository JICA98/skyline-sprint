#include "input.h"
#include "util/logger.h"

InputState Input::state = {};
SDL_Joystick* Input::joystick = nullptr;
bool Input::useHeadlessInput = false;
bool Input::headlessState[static_cast<int>(InputAction::Count)] = {};

void Input::Init() {
    state = {};
    joystick = nullptr;
    useHeadlessInput = false;
    for (int i = 0; i < static_cast<int>(InputAction::Count); ++i) {
        headlessState[i] = false;
    }

    // Try to open first joystick if already connected
    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
        if (joystick) {
            state.controllerConnected = true;
            Logger::Log(LogLevel::Info, "Input", "Controller initialized: " + std::string(SDL_JoystickName(joystick)));
        }
    }
}

void Input::Shutdown() {
    if (joystick) {
        SDL_JoystickClose(joystick);
        joystick = nullptr;
    }
}

void Input::Update(uint32_t currentTick) {
    (void)currentTick;

    // 1. Cycle previous states
    for (int i = 0; i < static_cast<int>(InputAction::Count); ++i) {
        state.previous[i] = state.current[i];
        if (!useHeadlessInput) {
            state.current[i] = false;
        }
    }

    // 2. Handle override for automated headless testing
    if (useHeadlessInput) {
        for (int i = 0; i < static_cast<int>(InputAction::Count); ++i) {
            state.current[i] = headlessState[i];
        }
        return;
    }

    // 3. Monitor controller connection changes
    int numJoysticks = SDL_NumJoysticks();
    if (numJoysticks > 0 && !joystick) {
        joystick = SDL_JoystickOpen(0);
        if (joystick) {
            state.controllerConnected = true;
            Logger::Log(LogLevel::Info, "Input", "Controller connected: " + std::string(SDL_JoystickName(joystick)));
        }
    } else if (numJoysticks == 0 && joystick) {
        SDL_JoystickClose(joystick);
        joystick = nullptr;
        state.controllerConnected = false;
        state.leftStickX = 0;
        state.leftStickY = 0;
        Logger::Log(LogLevel::Info, "Input", "Controller disconnected.");
    }

    // 4. Poll Keyboard
    const Uint8* kbd = SDL_GetKeyboardState(NULL);
    if (kbd[SDL_SCANCODE_LEFT] || kbd[SDL_SCANCODE_A])  state.current[static_cast<int>(InputAction::MoveLeft)]  = true;
    if (kbd[SDL_SCANCODE_RIGHT] || kbd[SDL_SCANCODE_D]) state.current[static_cast<int>(InputAction::MoveRight)] = true;
    if (kbd[SDL_SCANCODE_UP] || kbd[SDL_SCANCODE_W])    state.current[static_cast<int>(InputAction::MoveUp)]    = true;
    if (kbd[SDL_SCANCODE_DOWN] || kbd[SDL_SCANCODE_S])  state.current[static_cast<int>(InputAction::MoveDown)]  = true;
    if (kbd[SDL_SCANCODE_SPACE])                        state.current[static_cast<int>(InputAction::Jump)]      = true;
    if (kbd[SDL_SCANCODE_ESCAPE])                       state.current[static_cast<int>(InputAction::Pause)]     = true;
    if (kbd[SDL_SCANCODE_RETURN])                       state.current[static_cast<int>(InputAction::Select)]    = true;
    if (kbd[SDL_SCANCODE_BACKSPACE])                    state.current[static_cast<int>(InputAction::Back)]      = true;
    if (kbd[SDL_SCANCODE_F1])                           state.current[static_cast<int>(InputAction::F1)]        = true;
    if (kbd[SDL_SCANCODE_F2])                           state.current[static_cast<int>(InputAction::F2)]        = true;
    if (kbd[SDL_SCANCODE_F3])                           state.current[static_cast<int>(InputAction::F3)]        = true;
    if (kbd[SDL_SCANCODE_F4])                           state.current[static_cast<int>(InputAction::F4)]        = true;

    // 5. Poll Controller (if open)
    if (joystick) {
        // Retrieve axis values with deadzone filter (8000 / 32767)
        int axisX = SDL_JoystickGetAxis(joystick, 0);
        int axisY = SDL_JoystickGetAxis(joystick, 1);
        state.leftStickX = axisX;
        state.leftStickY = axisY;

        if (axisX < -8000) state.current[static_cast<int>(InputAction::MoveLeft)]  = true;
        if (axisX > 8000)  state.current[static_cast<int>(InputAction::MoveRight)] = true;
        if (axisY < -8000) state.current[static_cast<int>(InputAction::MoveUp)]    = true;
        if (axisY > 8000)  state.current[static_cast<int>(InputAction::MoveDown)]  = true;

        // Button Mapping:
        // Cross = 0 (Jump/Select), Circle = 1 (Back), Options = 9 (Pause), Directional Pad buttons
        if (SDL_JoystickGetButton(joystick, 0)) {
            state.current[static_cast<int>(InputAction::Jump)]   = true;
            state.current[static_cast<int>(InputAction::Select)] = true;
        }
        if (SDL_JoystickGetButton(joystick, 1))  state.current[static_cast<int>(InputAction::Back)]   = true;
        if (SDL_JoystickGetButton(joystick, 9))  state.current[static_cast<int>(InputAction::Pause)]  = true;
        if (SDL_JoystickGetButton(joystick, 13)) state.current[static_cast<int>(InputAction::MoveUp)]   = true; // Pad Up
        if (SDL_JoystickGetButton(joystick, 14)) state.current[static_cast<int>(InputAction::MoveDown)] = true; // Pad Down
        if (SDL_JoystickGetButton(joystick, 15)) state.current[static_cast<int>(InputAction::MoveLeft)] = true; // Pad Left
        if (SDL_JoystickGetButton(joystick, 16)) state.current[static_cast<int>(InputAction::MoveRight)] = true; // Pad Right
        if (SDL_JoystickGetButton(joystick, 17)) state.current[static_cast<int>(InputAction::F1)] = true; // Touchpad maps to F1/Diagnostics
    }
}

bool Input::IsHeld(InputAction action) {
    return state.current[static_cast<int>(action)];
}

bool Input::WasPressed(InputAction action) {
    return state.current[static_cast<int>(action)] && !state.previous[static_cast<int>(action)];
}

bool Input::WasReleased(InputAction action) {
    return !state.current[static_cast<int>(action)] && state.previous[static_cast<int>(action)];
}

void Input::SetHeadlessInput(InputAction action, bool pressed) {
    useHeadlessInput = true;
    headlessState[static_cast<int>(action)] = pressed;
}

void Input::ClearHeadlessInput() {
    useHeadlessInput = false;
    for (int i = 0; i < static_cast<int>(InputAction::Count); ++i) {
        headlessState[i] = false;
    }
}
