#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

: "${OO_PS4_TOOLCHAIN:=$ROOT_DIR/OpenOrbis-PS4-Toolchain}"
export OO_PS4_TOOLCHAIN
export DOTNET_SYSTEM_GLOBALIZATION_INVARIANT=1

if [[ ! -d "$OO_PS4_TOOLCHAIN" ]]; then
    echo "ERROR: OO_PS4_TOOLCHAIN does not exist: $OO_PS4_TOOLCHAIN" >&2
    echo "Set it before building, for example:" >&2
    echo "  export OO_PS4_TOOLCHAIN=/path/to/OpenOrbis-PS4-Toolchain" >&2
    exit 1
fi

if [[ ! -f src/main.cpp || ! -f src/game/game_loop.cpp ]]; then
    echo "ERROR: flattened source tree is missing under src/." >&2
    exit 1
fi

if ! grep -q 'SkylineSprint-0.1.2-final' src/platform/platform.cpp; then
    echo "ERROR: final framebuffer build marker is missing." >&2
    exit 1
fi

# Prefer the toolchain's bundled compiler/tools when available.
if [[ -d "$OO_PS4_TOOLCHAIN/bin/linux" ]]; then
    export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"
fi

# Ensure local LLVM lld-bin path is searched for ld.lld
if [[ -d "/home/jica/lld-bin/usr/bin" ]]; then
    export PATH="/home/jica/lld-bin/usr/bin:$PATH"
fi

# Ensure libssl.so.1.1 is found for .NET Core PkgTool.Core
if [[ -d "/home/jica/lld-bin/usr/lib/x86_64-linux-gnu" ]]; then
    export LD_LIBRARY_PATH="/home/jica/lld-bin/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}"
fi

# Ensure required redistributable runtime modules are staged.
mkdir -p package/sce_module package/sce_sys/about
for module in libc.prx libSceFios2.prx; do
    if [[ ! -f "package/sce_module/$module" ]]; then
        candidate="$(find "$OO_PS4_TOOLCHAIN" -type f -name "$module" -print -quit || true)"
        if [[ -z "$candidate" ]]; then
            echo "ERROR: Could not locate $module in the OpenOrbis toolchain." >&2
            exit 1
        fi
        cp "$candidate" "package/sce_module/$module"
    fi
done

if [[ ! -f package/sce_sys/about/right.sprx ]]; then
    candidate="$(find "$OO_PS4_TOOLCHAIN" -type f -name right.sprx -print -quit || true)"
    if [[ -z "$candidate" ]]; then
        echo "ERROR: Could not locate right.sprx in the OpenOrbis toolchain." >&2
        exit 1
    fi
    cp "$candidate" package/sce_sys/about/right.sprx
fi

echo "Building Skyline Sprint final framebuffer build..."
echo "Source marker: SkylineSprint-0.1.2-final"
echo "Toolchain: $OO_PS4_TOOLCHAIN"

# Never reuse an old eboot.bin or package.
make clean
rm -f package/eboot.bin package/pkg.gp4 ./*.pkg
rm -rf dist x64

make -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)" all

if [[ ! -s package/eboot.bin ]]; then
    echo "ERROR: package/eboot.bin was not generated." >&2
    exit 1
fi

if command -v strings >/dev/null 2>&1; then
    if ! strings package/eboot.bin | grep 'SkylineSprint-0.1.2-final' >/dev/null; then
        echo "ERROR: generated eboot.bin does not contain the final build marker." >&2
        exit 1
    fi
fi

pkg="$(find dist -maxdepth 1 -type f -name '*.pkg' -print -quit || true)"
if [[ -z "$pkg" || ! -s "$pkg" ]]; then
    echo "ERROR: final PKG was not generated." >&2
    exit 1
fi

sha256sum package/eboot.bin "$pkg" | tee dist/SHA256SUMS

echo "Build complete:"
echo "  $ROOT_DIR/package/eboot.bin"
echo "  $ROOT_DIR/$pkg"
echo "Expected runtime marker: [PS4-FB-FINAL]"
