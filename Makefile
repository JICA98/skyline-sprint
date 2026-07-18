# Package metadata.
TITLE       := Skyline Sprint
VERSION     := 1.01
TITLE_ID    := SKYS00001
CONTENT_ID  := IV0000-SKYS00001_00-SKYLINEGP4000000

# Root vars
TOOLCHAIN   := $(OO_PS4_TOOLCHAIN)
INTDIR      := x64/Debug
STAGEDIR    := package
PKG_TOOL    := LD_LIBRARY_PATH=/home/jica/lld-bin/usr/lib/x86_64-linux-gnu:$$LD_LIBRARY_PATH $(TOOLCHAIN)/bin/linux/PkgTool.Core

# Define libraries to link
LIBS        := -lc -lkernel -lc++ -lSceUserService -lSceVideoOut -lSceAudioOut -lScePad -lSceSysmodule -lSceFreeType -lSDL2 -lSDL2_image

# Find all sources
CPPFILES    := src/main.cpp src/platform/platform.cpp src/util/random.cpp src/util/logger.cpp src/game/game_loop.cpp src/audio/audio_manager.cpp src/render/renderer.cpp src/input/input.cpp src/game/physics.cpp src/game/player.cpp src/game/world.cpp
OBJS        := $(patsubst src/%.cpp, $(INTDIR)/src/%.o, $(CPPFILES))

# Compiler and Linker flags
CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c -Isrc -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -DPS4
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1 -std=gnu++17
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

# Setup compiler and tools
CC          := clang
CCX         := clang++
LD          := ld.lld

# Asset wildcard
ASSET_FILES := $(wildcard $(STAGEDIR)/assets/**/*)

all: $(CONTENT_ID).pkg

$(CONTENT_ID).pkg: $(STAGEDIR)/pkg.gp4
	$(PKG_TOOL) pkg_build $< .
	mkdir -p dist
	mv $(CONTENT_ID).pkg dist/ || true

$(STAGEDIR)/pkg.gp4: $(STAGEDIR)/eboot.bin $(STAGEDIR)/sce_sys/about/right.sprx $(STAGEDIR)/sce_sys/param.sfo $(STAGEDIR)/sce_sys/icon0.png $(STAGEDIR)/sce_module/libc.prx $(STAGEDIR)/sce_module/libSceFios2.prx $(ASSET_FILES)
	cd $(STAGEDIR) && $(TOOLCHAIN)/bin/linux/create-gp4 -out pkg.gp4 --content-id=$(CONTENT_ID) --files "eboot.bin sce_sys/about/right.sprx sce_sys/param.sfo sce_sys/icon0.png sce_module/libc.prx sce_module/libSceFios2.prx $(patsubst $(STAGEDIR)/%,%,$(ASSET_FILES))"

$(STAGEDIR)/sce_sys/param.sfo: Makefile
	mkdir -p $(STAGEDIR)/sce_sys
	$(PKG_TOOL) sfo_new $@
	$(PKG_TOOL) sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(PKG_TOOL) sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(PKG_TOOL) sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value 'gd'
	$(PKG_TOOL) sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(PKG_TOOL) sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(PKG_TOOL) sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(PKG_TOOL) sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(PKG_TOOL) sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

$(STAGEDIR)/eboot.bin: $(INTDIR)/skyline_sprint.elf
	$(TOOLCHAIN)/bin/linux/create-fself -in=$< -out=$(INTDIR)/skyline_sprint.oelf --eboot "$@" --paid 0x3800000000000011

$(INTDIR)/skyline_sprint.elf: $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

$(INTDIR)/src/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -o $@ $<

clean:
	rm -rf $(INTDIR) $(STAGEDIR)/pkg.gp4 $(STAGEDIR)/eboot.bin $(STAGEDIR)/sce_sys/param.sfo *.pkg dist
