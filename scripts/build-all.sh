#!/bin/sh
set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
if [ -n "${TOOLCHAIN_DIR:-}" ]; then
  PATH="$TOOLCHAIN_DIR:$PATH"
fi

sh "$ROOT/scripts/build-variant.sh" "$ROOT/variants/nec-repeat-200ms" ir_remote
sh "$ROOT/scripts/build-variant.sh" "$ROOT/variants/samsung-110ms" ir_remote_samsung
sh "$ROOT/variants/configurable-irc2/build.sh"

node "$ROOT/variants/samsung-110ms/check_codes.mjs"
node "$ROOT/variants/configurable-irc2/test_config_page.mjs"
node "$ROOT/scripts/verify-manifests.mjs"
