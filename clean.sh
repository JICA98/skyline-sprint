#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"
rm -rf bin dist x64
rm -f ./*.pkg package/pkg.gp4 package/eboot.bin package/sce_sys/param.sfo
find . -type f \( -name '*.o' -o -name '*.elf' -o -name '*.oelf' \) -delete
printf '%s\n' 'Clean completed.'
