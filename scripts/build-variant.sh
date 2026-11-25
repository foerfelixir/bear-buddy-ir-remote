#!/bin/sh
set -eu

if [ "$#" -ne 2 ]; then
  echo "usage: build-variant.sh <variant-directory> <target-name>" >&2
  exit 1
fi

VARIANT=$(cd "$1" && pwd)
TARGET=$2
CROSS=${CROSS:-riscv-none-elf-}
CC=${CROSS}gcc
OBJCOPY=${CROSS}objcopy
SIZE=${CROSS}size
command -v "$CC" >/dev/null 2>&1 || CC="${CC}.exe"
command -v "$OBJCOPY" >/dev/null 2>&1 || OBJCOPY="${OBJCOPY}.exe"
command -v "$SIZE" >/dev/null 2>&1 || SIZE="${SIZE}.exe"
ARCH="-march=rv32ec -mabi=ilp32e"
CFLAGS="$ARCH -Os -g -ffreestanding -fno-builtin -fno-common -fno-stack-protector -msmall-data-limit=0 -Wall -Wextra"
LDFLAGS="$ARCH -nostdlib -nostartfiles -T src/link.ld -Wl,--gc-sections -Wl,--build-id=none -Wl,-Map=build/$TARGET.map"

cd "$VARIANT"
mkdir -p build
"$CC" $ARCH -c src/start.S -o build/start.o
"$CC" $CFLAGS -c src/main.c -o build/main.o
"$CC" $LDFLAGS build/start.o build/main.o -lgcc -o "build/$TARGET.elf"
"$SIZE" "build/$TARGET.elf"
"$OBJCOPY" -O binary "build/$TARGET.elf" "build/$TARGET.bin"
"$OBJCOPY" -O ihex "build/$TARGET.elf" "build/$TARGET.hex"
