# NEC 200 ms

Source-backed NEC-only firmware for the Bear-Buddy three-button remote.

## Behavior

| Button | MCU pin | NEC address | Command |
|---|---|---:|---:|
| SW1 | PC4 | `0x00` | `152` (`0x98`) |
| SW2 | PC2 | `0x00` | `31` (`0x1F`) |
| SW3 | PC1 | `0x00` | `62` (`0x3E`) |

- Full NEC frame: 9 ms mark, 4.5 ms space, address, inverted address, command, inverted command, 560 µs trailing mark.
- Bits are sent LSB first with 560 µs marks and 560/1690 µs spaces.
- While held, the full frame repeats every 200 ms.
- Priority is SW1 > SW2 > SW3.
- The core enters standby after release and wakes on a falling button edge.
- A three-second awake window after reset keeps SWIO available for initial programming. There is no post-release programming window in this variant.

## Build

```sh
make CROSS=riscv-none-elf-
```

Outputs:

- `build/ir_remote.bin` — 984 bytes; flash at `0x08000000`.
- `build/ir_remote.hex` — Intel HEX for WCH-LinkUtility.

Checked-in binary SHA-256:

```text
f4d67cd728ed617ac83db7d72a747ce6f9394f0c1eee1924151c9892a6e7659d
```

## Source map

- `src/main.c` — GPIO, TIM1 carrier, SysTick timing, EXTI wake, standby, and NEC transmission.
- `src/start.S` — reset vector, stack, BSS clear, and data copy.
- `src/link.ld` — 16 KiB flash / 2 KiB RAM layout.
- `Makefile` — RV32EC / ILP32E freestanding build.
