# Architecture of Skyline Sprint

## Core Architecture Layout
Skyline Sprint separates platform code, rendering logic, and deterministic gameplay simulation:

- **Platform Interface (`src/platform/`)**: Abstracts window management, input polling, sound playback, and diagnostics between Desktop SDL2 and PS4 OpenOrbis.
- **Gameplay Simulation (`src/game/`)**: Timestep-driven deterministic execution of runner logic.
- **Renderer (`src/render/`)**: Basic abstractions over SDL2 software/hardware rendering.
- **Input System (`src/input/`)**: Unified keyboard and PS4 Controller mapper.
- **Util (`src/util/`)**: PRNG, mathematical utils, and memory pools.
