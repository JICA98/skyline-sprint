# PS4 blank-screen fix

This source tree includes the final framebuffer correction:

- The active source is directly under `src/`; there is no nested `src/src_fixed` tree.
- The PS4 SDL window is created at `(0, 0)`, not `SDL_WINDOWPOS_UNDEFINED`.
- This matters because OpenOrbis SDL copies the software surface into VideoOut using `window->x` and `window->y` as destination offsets.
- The renderer is window-bound and software-backed.
- `SDL_RenderPresent()` flushes drawing and triggers the OpenOrbis window-framebuffer update.
- The PS4 build always deletes stale objects, `eboot.bin`, GP4 metadata, and old PKGs before compiling.
- The runtime/build marker is `SkylineSprint-0.1.2-final` and log marker is `[PS4-FB-FINAL]`.
- Package version is `1.01` to distinguish it from the old 1.00 build.
