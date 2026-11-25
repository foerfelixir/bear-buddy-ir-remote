# NEC-only backup (binary only)

Historical NEC-only snapshot preserved from the IRC2 development period.

**Source status:** the exact source tree for this binary was not retained. These files are preserved byte-for-byte and are not reproducible from the other variants in this repository.

## Artifacts

| File | Size | Purpose |
|---|---:|---|
| `ir_remote.bin` | 1276 B | Firmware image; flash at `0x08000000` |
| `ir_remote.hex` | 3605 B | Intel HEX image |
| `ir_remote_template.bin` | 2112 B | Firmware plus blank 64-byte config page |

SHA-256 values are recorded in [`../../manifests/SHA256SUMS`](../../manifests/SHA256SUMS).

Use this only when the exact historical image is required. For maintained source, use [`../nec-repeat-200ms`](../nec-repeat-200ms/README.md) or [`../configurable-irc2`](../configurable-irc2/README.md).
