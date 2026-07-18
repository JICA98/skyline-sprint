# Building Skyline Sprint

## Prerequisites
### Desktop
- Clang / GCC compiler with C++17 support
- SDL2 development libraries
- SDL2_image development libraries

### PS4 Target
- OpenOrbis PS4 Toolchain (v0.5.4 pinned)
- Ubuntu LLD 21.1.8 (`ld.lld` binary in PATH)
- `.NET Core` globalization invariant set (`export DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1`)
- `libssl.so.1.1` on library search path

## Compilation Commands

### Desktop Build
```bash
./build_desktop.sh
```

### PS4 Target Build
```bash
./build_ps4.sh
```

### Clean Build
```bash
./clean.sh
```
