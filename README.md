# emulators
different types of emulators
v0.0.3

supported emulators:
Chip-8 (emulator, assembler, disassembler)
Gameboy (emulator, debugger)

Gameboy emulator status:
- APU not complete
- Passes the majority of Blargg's test ROMs (cpu_instrs, instr_timing, mem_timing, halt bug, interrupt timing)
- Known failing/incomplete areas:
  - Audio (APU) tests are not yet passing.
  - OEM bug (sprite/OAM corruption quirk) is not emulated.
  - PPU timing is not fully cycle-accurate yet — some ROMs that rely on precise memory access timing during specific PPU modes/scanlines can read/write VRAM/OAM at slightly incorrect points, causing visual or behavioral inaccuracies.

external dependencies:
cmake
sfml

By: Mike Stolls, 2017