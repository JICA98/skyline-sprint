# OpenOrbis PS4 Toolchain Baseline

## Toolchain Identification
* **OpenOrbis Release Tag**: `v0.5.4`
* **Submodule Commit Hash**: `0a1aaf9dd4a92695538bdeb09fb056d06dd11725`
* **Host Operating System**: Linux x86_64 (Ubuntu)
* **Compiler**: Clang version 21.1.8
* **Linker**: LLVM LLD 21.1.8 (compatible with GNU linkers)
* **Python version**: Python 3.14.4
* **OpenOrbis Path (`OO_PS4_TOOLCHAIN`)**: `/home/jica/repo/skyline-sprint/OpenOrbis-PS4-Toolchain`

## Development Build Dependencies
To run the `.pkg` building tools on modern Linux hosts, the following configurations and dependencies are required:
* **System.Globalization.Invariant**: `.NET Core` globalization must be disabled via `export DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1`.
* **libssl.so.1.1**: `PkgTool.Core` requires `libssl.so.1.1`. Downloaded and extracted dynamically to a local folder `/home/jica/lld-bin/usr/lib/x86_64-linux-gnu` and set via `LD_LIBRARY_PATH`.

## Build Process & Commands
The verified compilation pipeline to create a PS4 homebrew package:

### 1. Compile C/C++ Source Files
```bash
clang++ --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c \
  -isysroot $(OO_PS4_TOOLCHAIN) \
  -isystem $(OO_PS4_TOOLCHAIN)/include \
  -isystem $(OO_PS4_TOOLCHAIN)/include/c++/v1 \
  -o obj/main.o src/main.cpp
```

### 2. Link objects to ELF
```bash
ld.lld -m elf_x86_64 -pie --script $(OO_PS4_TOOLCHAIN)/link.x --eh-frame-hdr \
  -L$(OO_PS4_TOOLCHAIN)/lib \
  -lc -lkernel -lc++ -lSceUserService -lSceVideoOut -lSceAudioOut -lScePad -lSceSysmodule -lSceFreeType -lSDL2 -lSDL2_image \
  $(OO_PS4_TOOLCHAIN)/lib/crt1.o \
  obj/*.o -o bin/game.elf
```

### 3. Generate Fake SELF (`eboot.bin`)
```bash
$(OO_PS4_TOOLCHAIN)/bin/linux/create-fself \
  -in=bin/game.elf \
  -out=bin/game.oelf \
  --eboot "eboot.bin" \
  --paid 0x3800000000000011
```

### 4. Construct SFO Package Metadata
`PkgTool.Core` initializes and sets metadata parameters in `sce_sys/param.sfo`:
```bash
PkgTool.Core sfo_new sce_sys/param.sfo
PkgTool.Core sfo_setentry sce_sys/param.sfo APP_TYPE --type Integer --maxsize 4 --value 1
PkgTool.Core sfo_setentry sce_sys/param.sfo APP_VER --type Utf8 --maxsize 8 --value '1.00'
PkgTool.Core sfo_setentry sce_sys/param.sfo CATEGORY --type Utf8 --maxsize 4 --value 'gd'
PkgTool.Core sfo_setentry sce_sys/param.sfo CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
PkgTool.Core sfo_setentry sce_sys/param.sfo TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
PkgTool.Core sfo_setentry sce_sys/param.sfo TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
```

### 5. Build GP4 Project Configuration
```bash
$(OO_PS4_TOOLCHAIN)/bin/linux/create-gp4 \
  -out pkg.gp4 \
  --content-id=$(CONTENT_ID) \
  --files "eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png sce_module/libSceFios2.prx sce_module/libc.prx assets/..."
```

### 6. Build final `.pkg` package
```bash
$(OO_PS4_TOOLCHAIN)/bin/linux/PkgTool.Core pkg_build pkg.gp4 .
```

## Platform & Controller APIs
### Required Modules
On start, the PS4 game must load system modules using `sceSysmoduleLoadModule`:
* **FreeType**: `ORBIS_SYSMODULE_FREETYPE_OL` (loaded to prevent unresolved symbol crashes for text rendering)

### System Paths
* All assets must be loaded relative to `/app0/` (e.g. `/app0/assets/fonts/VeraMono.ttf`), which is where the package contents are mounted at runtime.

### Controller Mapping
The controller is initialized using the standard SDL Joystick API:
```cpp
SDL_Joystick* controller = SDL_JoystickOpen(0);
```
Joypad button events map to indices:
* `PAD_BUTTON_CROSS` = 0
* `PAD_BUTTON_CIRCLE` = 1
* `PAD_BUTTON_SQUARE` = 2
* `PAD_BUTTON_TRIANGLE` = 3
* `PAD_BUTTON_L1` = 4
* `PAD_BUTTON_R1` = 5
* `PAD_BUTTON_OPTIONS` = 9
* `PAD_BUTTON_L3` = 11
* `PAD_BUTTON_R3` = 12
* `PAD_BUTTON_UP` = 13
* `PAD_BUTTON_DOWN` = 14
* `PAD_BUTTON_LEFT` = 15
* `PAD_BUTTON_RIGHT` = 16
* `PAD_BUTTON_TOUCH_PAD` = 17
* `PAD_BUTTON_L2` = 18
* `PAD_BUTTON_R2` = 19
