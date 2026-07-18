#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

if [[ ! -f src/main.cpp ]]; then
    echo "ERROR: src/main.cpp is missing." >&2
    exit 1
fi

mkdir -p bin
SDL2_CFLAGS="$(sdl2-config --cflags)"
SDL2_LIBS="$(sdl2-config --libs)"

clang++ -std=c++17 -Wall -Wextra -O2 -Isrc $SDL2_CFLAGS \
  src/main.cpp \
  src/platform/platform.cpp \
  src/util/random.cpp \
  src/util/logger.cpp \
  src/game/game_loop.cpp \
  src/audio/audio_manager.cpp \
  src/render/renderer.cpp \
  src/input/input.cpp \
  src/game/physics.cpp \
  src/game/player.cpp \
  src/game/world.cpp \
  -lSDL2_image $SDL2_LIBS -o bin/skyline_sprint

echo "Desktop build complete: bin/skyline_sprint"
