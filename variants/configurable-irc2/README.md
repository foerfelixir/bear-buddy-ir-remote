# Configurable IRC2

Source-backed NEC/Samsung firmware with a 64-byte flash configuration page.

## Behavior

- Each button can independently select NEC (`proto = 0`) or Samsung32 (`proto = 1`).
- Configuration lives at `0x08000800`, the start of a 64-byte-aligned flash page.
- The code region is capped at 2048 bytes so it cannot grow into the configuration page.
- Invalid, blank, or old `IRC1` pages fall back to NEC address `0x00` with commands `152`, `31`, and `62`.
- A held button sends its full configured frame every 200 ms. This variant does not use Samsung short-repeat frames.
- Priority is SW1 > SW2 > SW3.
- The core stays awake for six seconds after reset and after release. A button press ends the window immediately; SWIO movement extends it. Configuration is re-read after every wake.

## Configuration page

All multi-byte values are little-endian.

| Offset | Size | Meaning |
|---:|---:|---|
| `0x00` | 4 | Magic bytes `IRC2` (`0x32435249`) |
| `0x04` | 4 | SW1: address, command, protocol, `0xFF` |
| `0x08` | 4 | SW2: address, command, protocol, `0xFF` |
| `0x0C` | 4 | SW3: address, command, protocol, `0xFF` |
| `0x10` | 4 | Checksum: sum of bytes `0x00..0x0F` |
| `0x14` | 44 | Unused; erased `0xFF` |

The browser codec in `web/ir.js` builds and parses this page. It is copied from the MIT-licensed [Bear-Buddy web flasher](https://github.com/foerfelixir/bear-buddy-webflasher); its license is retained beside it.

## Build

```sh
TOOLCHAIN_DIR=/path/to/xpack-riscv-none-elf-gcc-15.2.0-1/bin sh build.sh
node test_config_page.mjs
```

Set `WEBFW=/path/to/WCH_WebLink/data/www/fw` to copy the generated template into a local web-flasher asset directory. Without `WEBFW`, nothing outside this variant is modified.

Outputs:

- `build/ir_remote.bin` — 1360-byte firmware; flash at `0x08000000`.
- `build/ir_remote.hex` — Intel HEX.
- `build/ir_remote_template.bin` — 2112 bytes: 2048-byte code area padded with `0xFF`, then a blank 64-byte config page.

Checked-in firmware SHA-256:

```text
668a315d4d9c389a24133a1ace37118a01c9205a58db924070b1597f650378ec
```

## Tests

`test_config_page.mjs` checks:

- C and browser constants agree.
- Magic, offsets, checksum, and erased bytes are correct.
- The browser codec round-trips all three buttons.
- Unsupported protocols, `IRC1`, and blank pages are rejected.
- The deployed template starts with the current firmware and is exactly 2112 bytes.
