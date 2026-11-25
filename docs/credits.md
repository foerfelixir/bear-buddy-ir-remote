# Credits

Bear-Buddy is an awesome hardware kit designed by **TomatoCube**. The three-button CH32V003 IR remote documented in this repository is part of that kit.

Thanks to **TomatoCube** for the hardware design and detailed tutorial:

- [Box 10 — Bear-Buddy DIY Desk Companion](https://github.com/TomatoCube18/Docs/tree/main/Box%2010%20-%20Bear-Buddy%20DIY%20Desk%20Companion)
- [Bear-Buddy controller-board reference](https://github.com/TomatoCube18/DIY_Bear-Buddy_ControllerBoard)

## Firmware lineage

The NEC firmware derives from [wagiminator's TinyRemote for CH32V003](https://github.com/wagiminator/CH32V003-IR-Remote). The configurable and Samsung variants are adaptations of that firmware. The project also credits [cnlohr's ch32v003fun](https://github.com/cnlohr/ch32v003fun) for CH32V003 tooling and runtime techniques.

Firmware-derived work remains under **Creative Commons Attribution-ShareAlike 3.0 Unported**, with the upstream notice retained in [`LICENSE`](../LICENSE). The browser IRC2 codec in `variants/configurable-irc2/web/ir.js` originated in the MIT-licensed Bear-Buddy web flasher and keeps its separate license notice.

## Hardware evidence

The schematic was reverse-engineered by physically tracing the PCB lines on the actual remote. It is not an official TomatoCube schematic. The original PDF and its checksum are preserved in the private [Bear-Buddy hardware-notes repository](https://github.com/foerfelixir/bear-buddy-hardware-notes).

## Related project

- [Bear-Buddy ESP32-C3 web flasher and IR configurator](https://github.com/foerfelixir/bear-buddy-webflasher)
