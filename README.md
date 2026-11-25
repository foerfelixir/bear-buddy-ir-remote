# Bear-Buddy CH32V003 IR Remote

Source, binaries, and documentation for the three-button IR remote included with the **Box 10 — Bear-Buddy DIY Desk Companion**, an awesome hardware kit designed by **TomatoCube**.

**Status:** public. Read the [documentation site](https://foerfelixir.github.io/bear-buddy-ir-remote/) or open [`docs/index.html`](docs/index.html) locally.

## Hardware and related repositories

- [Physically traced schematic and hardware notes](https://github.com/foerfelixir/bear-buddy-hardware-notes)
- [Bear-Buddy ESP32-C3 web flasher and configurator](https://github.com/foerfelixir/bear-buddy-webflasher)

The remote uses a CH32V003J4M6. SW1, SW2, and SW3 are active-low inputs on PC4, PC2, and PC1. PA2 (TIM1_CH2N) drives the IR and indicator LEDs with a hardware-generated 38 kHz carrier. The core sleeps in standby between presses.

## Firmware variants

| Variant | Directory | Protocol and commands | Held-button behavior | Code size |
|---|---|---|---|---:|
| NEC 200 ms | [`variants/nec-repeat-200ms`](variants/nec-repeat-200ms/README.md) | NEC address `0x00`; SW1 `152`, SW2 `31`, SW3 `62` | Full frame every 200 ms | 984 B |
| Configurable IRC2 | [`variants/configurable-irc2`](variants/configurable-irc2/README.md) | NEC or Samsung per button, stored in flash | Full configured frame every 200 ms | 1360 B |
| Samsung 110 ms | [`variants/samsung-110ms`](variants/samsung-110ms/README.md) | Samsung address `0x07`; Power, Enter, and `7150` + Enter macro | SW1/SW2 short repeat every 110 ms; SW3 repeats the full macro | 1308 B |
| NEC-only backup | [`variants/neconly-backup`](variants/neconly-backup/README.md) | Historical NEC-only IRC2-era image | Binary-only snapshot; source not available | 1276 B |

The original `firmware/` and `versions/configurable-irc2/` trees were byte-identical. This repository keeps one canonical source copy of that variant rather than duplicating it.

## Build and verify

Prerequisites for the top-level build: a POSIX shell, Node.js, and an xpack RISC-V toolchain. GNU Make is optional and only needed for the per-variant Makefiles. The checked-in hashes were produced with `xpack-riscv-none-elf-gcc-15.2.0-1`.

```sh
TOOLCHAIN_DIR=/path/to/xpack-riscv-none-elf-gcc-15.2.0-1/bin sh scripts/build-all.sh
node scripts/verify-manifests.mjs
```

`CROSS` defaults to `riscv-none-elf-`. Set it to another prefix if required. The build regenerates `.bin`, `.hex`, and the configurable variant's 2112-byte template, then runs the Samsung code check and the IRC2 browser-codec compatibility test.

See [build and flash documentation](docs/build.html) for toolchain setup, flash offsets, programming-window behavior, and verification commands.

## Flash safely

1. Remove the remote's battery before connecting a programmer.
2. Use a WCH-LinkE in **WCH-LinkRV / RISC-V** mode.
3. Flash `.bin` files at `0x08000000`, or open the `.hex` file in WCH-LinkUtility.
4. For the configurable and Samsung variants, press and hold any remote button while the programmer accesses SWIO. Their six-second link window closes when idle; button activity or SWIO movement extends it.
5. The NEC 200 ms variant has a three-second awake window after reset and no post-release link window. Use a power-cycle erase if SWIO cannot attach.

Never flash the 2112-byte configurable template as though it were source code. It contains 2048 bytes of code space followed by a blank 64-byte configuration page.

## Documentation site

Open [`docs/index.html`](docs/index.html) locally, or serve the `docs` directory with any static web server. The site has no build step, external fonts, JavaScript, or third-party trackers. The public site is at [foerfelixir.github.io/bear-buddy-ir-remote](https://foerfelixir.github.io/bear-buddy-ir-remote/).

Pages:

- [Variants](docs/variants.html)
- [Build and flash](docs/build.html)
- [Hardware and traced schematic](docs/hardware.html)
- [IR protocols and configuration](docs/protocols.html)
- [Troubleshooting](docs/troubleshooting.html)
- [Credits](docs/credits.html)

## Attribution

Special thanks to **TomatoCube** for the awesome *Bear-Buddy* hardware kit, the hardware design, and the comprehensive tutorial:

[Box 10 — Bear-Buddy DIY Desk Companion](https://github.com/TomatoCube18/Docs/tree/main/Box%2010%20-%20Bear-Buddy%20DIY%20Desk%20Companion)

The three-button IR remote is part of that kit. Firmware was also informed by [wagiminator's TinyRemote](https://github.com/wagiminator/CH32V003-IR-Remote) and [cnlohr's ch32v003fun](https://github.com/cnlohr/ch32v003fun). Full credits are in [`docs/credits.md`](docs/credits.md).

## License

Firmware and project documentation are provided under **Creative Commons Attribution-ShareAlike 3.0 Unported**, matching the upstream TinyRemote work; see [`LICENSE`](LICENSE). The bundled browser codec at `variants/configurable-irc2/web/ir.js` comes from the MIT-licensed Bear-Buddy web flasher and retains its own [`LICENSE`](variants/configurable-irc2/web/LICENSE).
