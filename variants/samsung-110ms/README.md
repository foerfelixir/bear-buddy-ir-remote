# Samsung 110 ms

Source-backed Samsung32 firmware for the Bear-Buddy three-button remote. This variant was confirmed working on the physical remote.

## Behavior

All commands use Samsung address `0x07`.

| Button | MCU pin | Command | Published 32-bit code |
|---|---|---:|---:|
| SW1 | PC4 | Power `0x02` | `0xE0E040BF` |
| SW2 | PC2 | Enter `0x68` | `0xE0E016E9` |
| SW3 | PC1 | Macro `7`, `1`, `5`, `0`, Enter | see below |

SW3 macro commands:

| Key | Command | Published 32-bit code |
|---|---:|---:|
| 7 | `0x0C` | `0xE0E030CF` |
| 1 | `0x04` | `0xE0E020DF` |
| 5 | `0x09` | `0xE0E0906F` |
| 0 | `0x11` | `0xE0E08877` |
| Enter | `0x68` | `0xE0E016E9` |

- Full Samsung32 frame: 4500 µs header mark, 4500 µs header space, address twice, command, inverted command, 560 µs trailing mark.
- Bits are LSB first with 560 µs marks and 560/1690 µs spaces.
- SW1/SW2 send one full frame, then Samsung's short repeat frame every 110 ms on a fixed start-to-start raster.
- Short repeat waveform: 4500 µs mark, 4500 µs space, 560 µs mark, 560 µs space, 560 µs mark.
- SW3 always sends the complete `7 → 1 → 5 → 0 → Enter` sequence with 500 ms pauses. If held, the full sequence repeats 110 ms after it finishes.
- Priority is SW1 > SW2 > SW3.
- Six-second SWIO window after reset and release; SWIO movement extends it.

## Build and test

```sh
make CROSS=riscv-none-elf-
node check_codes.mjs
```

Outputs:

- `build/ir_remote_samsung.bin` — 1308 bytes; flash at `0x08000000`.
- `build/ir_remote_samsung.hex` — Intel HEX.

Checked-in binary SHA-256:

```text
21eef16bf18e71c84d29251de90cf4bb4ea5bd8dd8bbd29974aa84faaaaef5ad
```

`check_codes.mjs` reconstructs every transmitted 32-bit frame from the C constants and compares it with the published Samsung values above.
