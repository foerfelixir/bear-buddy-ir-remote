#!/bin/sh
set -eu

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
if [ -n "${TOOLCHAIN_DIR:-}" ]; then
  PATH="$TOOLCHAIN_DIR:$PATH"
fi

sh "$ROOT/scripts/build-variant.sh" "$HERE" ir_remote
node "$ROOT/scripts/make-template.mjs" "$HERE/build/ir_remote.bin" "$HERE/build/ir_remote_template.bin" 2048 64

if [ -n "${WEBFW:-}" ]; then
  mkdir -p "$WEBFW"
  cp "$HERE/build/ir_remote_template.bin" "$WEBFW/ir_remote.bin"
  echo "installed: $WEBFW/ir_remote.bin"
fi
