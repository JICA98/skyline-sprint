#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"
mkdir -p bin
SDL2_CFLAGS="$(sdl2-config --cflags)"
SDL2_LIBS="$(sdl2-config --libs)"
INCLUDES="-Isrc -I/home/jica/lld-bin/usr/include $SDL2_CFLAGS"
LIBS="-L/home/jica/lld-bin/usr/lib/x86_64-linux-gnu -Wl,-rpath,/home/jica/lld-bin/usr/lib/x86_64-linux-gnu -Wl,--disable-new-dtags -lSDL2_image $SDL2_LIBS"

clang++ -std=c++17 -Wall -Wextra -O2 $INCLUDES \
  tests/test_main.cpp \
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
  $LIBS -o bin/skyline_sprint_tests

SDL_VIDEODRIVER=dummy ./bin/skyline_sprint_tests
