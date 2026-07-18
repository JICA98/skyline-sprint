# Testing Skyline Sprint

Skyline Sprint includes both developer testing modes and diagnostic features:

## Automated Soak and Test Modes
- **Automatic Test Mode (`F3`)**: Runs the game automatically with simulated optimal controller inputs to verify platform compatibility and logic.
- **Stress Mode (`F4`)**: Runs game simulation at max possible speed, spawning excessive hazards and objects to verify memory boundaries and performance under peak load.

## Runtime Diagnostics
- **Compact Overlay (`F1`)**: A lightweight on-screen HUD showing current frame rates, memory overhead, and frame times.
- **Full Diagnostics (`F2`)**: Full diagnostic statistics containing system metrics, PRNG state, active chunks/objects count, and memory allocation stats.
